/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include <osquery/logger/logger.h>
#include <osquery/remote/http_client.h>

#include <boost/asio/connect.hpp>

namespace osquery {
namespace http {

const std::string kHTTPSDefaultPort{"443"};
const std::string kHTTPDefaultPort{"80"};
const std::string kProxyDefaultPort{"3128"};

const long kSSLShortReadError{0x140000dbL};

void Client::callNetworkOperation(std::function<void()> callback) {
  if (client_options_.timeout_) {
    timer_.async_wait(
        std::bind(&Client::timeoutHandler, this, std::placeholders::_1));
  }

  callback();

  {
    boost::asio::io_context::count_type handlers_executed = ioc_.run();
    ioc_.restart();
    if (handlers_executed == 0) {
      ec_ = boost::asio::error::operation_aborted;
    }
  }
}

void Client::cancelTimerAndSetError(boost::system::error_code const& ec) {
  if (client_options_.timeout_) {
    timer_.cancel();
  }

  if (ec_ != boost::asio::error::timed_out) {
    ec_ = ec;
  }
}

void Client::postResponseHandler(boost::system::error_code const& ec) {
  if ((ec.category() == boost::asio::error::ssl_category) &&
      (ec.value() == kSSLShortReadError)) {
    // Ignoring short read error, set ec_ to success.
    ec_.clear();
    // close connection for security reason.
    LOG(INFO) << "SSL SHORT_READ_ERROR: http_client closing socket";
    closeSocket();
  } else if (ec_ != boost::asio::error::timed_out) {
    ec_ = ec;
  }
}

bool Client::isSocketOpen() {
  return sock_.is_open();
}

void Client::closeSocket() {
  if (isSocketOpen()) {
    boost::system::error_code rc;
    sock_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, rc);
    sock_.close(rc);
  }
}

void Client::timeoutHandler(boost::system::error_code const& ec) {
  if (!ec) {
    closeSocket();
    ec_ = boost::asio::error::make_error_code(boost::asio::error::timed_out);
  }
}

void Client::connectHandler(boost::system::error_code const& ec,
                            boost::asio::ip::tcp::endpoint const&) {
  cancelTimerAndSetError(ec);
}

void Client::handshakeHandler(boost::system::error_code const& ec) {
  cancelTimerAndSetError(ec);
}

void Client::writeHandler(boost::system::error_code const& ec, size_t) {
  cancelTimerAndSetError(ec);
}

void Client::readHandler(boost::system::error_code const& ec, size_t) {
  if (client_options_.timeout_) {
    timer_.cancel();
  }
  postResponseHandler(ec);
}

Status Client::createConnection() {
  std::string port = (client_options_.proxy_hostname_)
                         ? kProxyDefaultPort
                         : *client_options_.remote_port_;

  std::string connect_host = (client_options_.proxy_hostname_)
                                 ? *client_options_.proxy_hostname_
                                 : *client_options_.remote_hostname_;

  std::size_t pos;
  if ((pos = connect_host.find(":")) != std::string::npos) {
    port = connect_host.substr(pos + 1);
    connect_host = connect_host.substr(0, pos);
  }

  // We can resolve async, but there is a handle leak in Windows.
  auto results = r_.resolve(connect_host, port, ec_);
  if (!ec_) {
    callNetworkOperation([&]() {
      boost::asio::async_connect(sock_,
                                 results,
                                 std::bind(&Client::connectHandler,
                                           this,
                                           std::placeholders::_1,
                                           std::placeholders::_2));
    });
  }

  if (ec_) {
    std::string error("Failed to connect to ");
    if (client_options_.proxy_hostname_) {
      error += "proxy host ";
    }
    error += connect_host + ':' + port;
    error += ": " + ec_.message();
    return Status::failure(error);
  }

  if (client_options_.keep_alive_) {
    boost::asio::socket_base::keep_alive option(true);
    sock_.set_option(option);
  }

  if (client_options_.proxy_hostname_) {
    std::string remote_host = *client_options_.remote_hostname_;
    std::string remote_port = *client_options_.remote_port_;

    beast_http_request req;
    req.method(beast_http::verb::connect);
    req.target(remote_host + ':' + remote_port);
    req.version(11);
    req.prepare_payload();

    callNetworkOperation([&]() {
      beast_http::async_write(sock_,
                              req,
                              std::bind(&Client::writeHandler,
                                        this,
                                        std::placeholders::_1,
                                        std::placeholders::_2));
    });

    if (ec_) {
      return Status::failure(ec_.message());
    }

    boost::beast::flat_buffer b;
    beast_http_response_parser rp;
    rp.skip(true);

    callNetworkOperation([&]() {
      beast_http::async_read_header(sock_,
                                    b,
                                    rp,
                                    std::bind(&Client::readHandler,
                                              this,
                                              std::placeholders::_1,
                                              std::placeholders::_2));
    });

    if (ec_) {
      return Status::failure(ec_.message());
    }

    if (beast_http::to_status_class(rp.get().result()) !=
        beast_http::status_class::successful) {
      return Status::failure(rp.get().reason().data());
    }
  }

  return Status::success();
}

Status Client::encryptConnection() {
  boost::asio::ssl::context ctx{boost::asio::ssl::context::sslv23};

  if (client_options_.always_verify_peer_) {
    ctx.set_verify_mode(boost::asio::ssl::verify_peer);
  } else {
    ctx.set_verify_mode(boost::asio::ssl::verify_none);
  }

  if (client_options_.server_certificate_) {
    ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    ctx.load_verify_file(*client_options_.server_certificate_);
  }

  if (client_options_.verify_path_) {
    ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    ctx.add_verify_path(*client_options_.verify_path_);
  }

  if (client_options_.ciphers_) {
    ::SSL_CTX_set_cipher_list(ctx.native_handle(),
                              client_options_.ciphers_->c_str());
  }

  if (client_options_.ssl_options_) {
    ctx.set_options(client_options_.ssl_options_);
  }

  if (client_options_.client_certificate_file_) {
    ctx.use_certificate_chain_file(*client_options_.client_certificate_file_);
  }

  if (client_options_.client_private_key_file_) {
    ctx.use_private_key_file(*client_options_.client_private_key_file_,
                             boost::asio::ssl::context::pem);
  }

  ssl_sock_ = std::make_shared<ssl_stream>(sock_, ctx);
  ::SSL_set_tlsext_host_name(ssl_sock_->native_handle(),
                             client_options_.remote_hostname_->c_str());

  ssl_sock_->set_verify_callback(boost::asio::ssl::host_name_verification(
      *client_options_.remote_hostname_));

  callNetworkOperation([&]() {
    ssl_sock_->async_handshake(
        boost::asio::ssl::stream_base::client,
        std::bind(&Client::handshakeHandler, this, std::placeholders::_1));
  });

  if (ec_) {
    return Status::failure(ec_.message());
  }

  return Status::success();
}

template <typename STREAM_TYPE>
Status Client::sendRequest(STREAM_TYPE& stream,
                           Request& req,
                           beast_http_response_parser& resp) {
  req.target((req.remotePath()) ? *req.remotePath() : "/");
  req.version(11);

  if (req[beast_http::field::host].empty()) {
    std::string host_header_value = *client_options_.remote_hostname_;
    if (client_options_.ssl_connection_ &&
        (kHTTPSDefaultPort != *client_options_.remote_port_)) {
      host_header_value += ':' + *client_options_.remote_port_;
    } else if (!client_options_.ssl_connection_ &&
               kHTTPDefaultPort != *client_options_.remote_port_) {
      host_header_value += ':' + *client_options_.remote_port_;
    }
    req.set(beast_http::field::host, host_header_value);
  }

  req.prepare_payload();
  req.keep_alive(true);

  if (client_options_.timeout_) {
    timer_.async_wait(
        [=](boost::system::error_code const& ec) { timeoutHandler(ec); });
  }

  beast_http_request_serializer sr{req};

  callNetworkOperation([&]() {
    beast_http::async_write(stream,
                            sr,
                            std::bind(&Client::writeHandler,
                                      this,
                                      std::placeholders::_1,
                                      std::placeholders::_2));
  });

  if (ec_) {
    return Status::failure(ec_.message());
  }

  boost::beast::flat_buffer b;

  callNetworkOperation([&]() {
    beast_http::async_read(stream,
                           b,
                           resp,
                           std::bind(&Client::readHandler,
                                     this,
                                     std::placeholders::_1,
                                     std::placeholders::_2));
  });

  if (ec_) {
    return Status::failure(ec_.message());
  }

  if (resp.get()["Connection"] == "close") {
    closeSocket();
  }

  if (!client_options_.keep_alive_) {
    closeSocket();
  }

  return Status::success();
}

Status Client::initHTTPRequest(Request& req, bool& create_connection) {
  create_connection = true;
  if (req.remoteHost()) {
    std::string hostname = *req.remoteHost();
    std::string port;

    if (hostname == kInstanceMetadataAuthority) {
      client_options_.proxy_hostname_ = boost::none;
    }

    if (req.remotePort()) {
      port = *req.remotePort();
    } else if (req.protocol() && (*req.protocol()).compare("https") == 0) {
      port = kHTTPSDefaultPort;
    } else {
      port = kHTTPDefaultPort;
    }

    bool ssl_connection = false;
    if (req.protocol() && (*req.protocol()).compare("https") == 0) {
      ssl_connection = true;
    }

    if (!isSocketOpen() || new_client_options_ ||
        hostname != *client_options_.remote_hostname_ ||
        port != *client_options_.remote_port_ ||
        client_options_.ssl_connection_ != ssl_connection) {
      client_options_.remote_hostname_ = hostname;
      client_options_.remote_port_ = port;
      client_options_.ssl_connection_ = ssl_connection;
      new_client_options_ = false;
      closeSocket();
    } else {
      create_connection = false;
    }
  } else {
    if (!client_options_.remote_hostname_) {
      return Status::failure("Remote hostname missing");
    }

    if (!client_options_.remote_port_) {
      if (client_options_.ssl_connection_) {
        client_options_.remote_port_ = kHTTPSDefaultPort;
      } else {
        client_options_.remote_port_ = kHTTPDefaultPort;
      }
    }
    closeSocket();
  }
  return Status::success();
}

Status Client::sendHTTPRequest(Request& req, Response& response) {
  if (client_options_.timeout_) {
    timer_.expires_from_now(
        boost::posix_time::seconds(client_options_.timeout_));
  }

  size_t redirect_attempts = 0;
  bool init_request = true;
  do {
    bool create_connection = true;
    if (init_request) {
      auto status = initHTTPRequest(req, create_connection);
      if (!status.ok()) {
        return status;
      }
    }

    beast_http_response_parser resp;
    if (create_connection) {
      auto status = createConnection();
      if (!status.ok()) {
        closeSocket();
        if (init_request && ec_ != boost::asio::error::timed_out) {
          init_request = false;
          continue;
        }
        ec_.clear();
        return status;
      }

      if (client_options_.ssl_connection_) {
        auto status2 = encryptConnection();
        if (!status2.ok()) {
          closeSocket();
          if (init_request && ec_ != boost::asio::error::timed_out) {
            init_request = false;
            continue;
          }
          ec_.clear();
          return status2;
        }
      }
    }

    Status send_status;
    if (client_options_.ssl_connection_) {
      send_status = sendRequest(*ssl_sock_, req, resp);
    } else {
      send_status = sendRequest(sock_, req, resp);
    }

    if (!send_status.ok()) {
      closeSocket();
      if (init_request && ec_ != boost::asio::error::timed_out) {
        init_request = false;
        continue;
      }
      ec_.clear();
      return send_status;
    }

    switch (resp.get().result()) {
    case beast_http::status::moved_permanently:
    case beast_http::status::found:
    case beast_http::status::see_other:
    case beast_http::status::not_modified:
    case beast_http::status::use_proxy:
    case beast_http::status::temporary_redirect:
    case beast_http::status::permanent_redirect: {
      if (!client_options_.follow_redirects_) {
        response = Response(resp.release());
        return Status::success();
      }

      if (redirect_attempts++ >= 10) {
        return Status::failure("Exceeded max of 10 redirects");
      }

      std::string redir_url = Response(resp.release()).headers()["Location"];
      if (!redir_url.size()) {
        return Status::failure("Location header missing in redirect response");
      }

      VLOG(1) << "HTTP(S) request re-directed to: " << redir_url;
      if (redir_url[0] == '/') {
        // Relative URI.
        if (req.remotePort()) {
          redir_url.insert(0, *req.remotePort());
          redir_url.insert(0, ":");
        }
        if (req.remoteHost()) {
          redir_url.insert(0, *req.remoteHost());
        }
        if (req.protocol()) {
          redir_url.insert(0, "://");
          redir_url.insert(0, *req.protocol());
        }
      } else {
        // Absolute URI.
        init_request = true;
      }
      req.uri(redir_url);
      break;
    }
    default:
      response = Response(resp.release());
      return Status::success();
    }
  } while (true);
}

Status Client::put(Request& req,
                   Response& response,
                   std::string const& body,
                   std::string const& content_type) {
  req.method(beast_http::verb::put);
  req.body() = body;
  if (!content_type.empty()) {
    req.set(beast_http::field::content_type, content_type);
  }
  return sendHTTPRequest(req, response);
}

Status Client::post(Request& req,
                    Response& response,
                    std::string const& body,
                    std::string const& content_type) {
  req.method(beast_http::verb::post);
  req.body() = body;
  if (!content_type.empty()) {
    req.set(beast_http::field::content_type, content_type);
  }
  return sendHTTPRequest(req, response);
}

Status Client::put(Request& req,
                   Response& response,
                   std::string&& body,
                   std::string const& content_type) {
  req.method(beast_http::verb::put);
  req.body() = std::move(body);
  if (!content_type.empty()) {
    req.set(beast_http::field::content_type, content_type);
  }
  return sendHTTPRequest(req, response);
}

Status Client::post(Request& req,
                    Response& response,
                    std::string&& body,
                    std::string const& content_type) {
  req.method(beast_http::verb::post);
  req.body() = std::move(body);
  if (!content_type.empty()) {
    req.set(beast_http::field::content_type, content_type);
  }
  return sendHTTPRequest(req, response);
}

Status Client::get(Request& req, Response& response) {
  req.method(beast_http::verb::get);
  return sendHTTPRequest(req, response);
}

Status Client::head(Request& req, Response& response) {
  req.method(beast_http::verb::head);
  return sendHTTPRequest(req, response);
}

Status Client::delete_(Request& req, Response& response) {
  req.method(beast_http::verb::delete_);
  return sendHTTPRequest(req, response);
}
} // namespace http
} // namespace osquery

