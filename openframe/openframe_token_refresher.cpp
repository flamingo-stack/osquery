/**
 *  Copyright (c) 2014-present, The osquery authors
 *
 *  This source code is licensed as defined by the LICENSE file found in the
 *  root directory of this source tree.
 *
 *  SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include "openframe_token_refresher.h"
#include "openframe_authorization_manager_provider.h"

namespace osquery {

OpenframeTokenRefresher::OpenframeTokenRefresher(std::shared_ptr<OpenframeTokenExtractor> extractor)
    : running_(false), extractor_(extractor) {
    if (!extractor_) {
        throw std::runtime_error("Token extractor cannot be null");
    }
}

OpenframeTokenRefresher::~OpenframeTokenRefresher() {
    stop();
}

void OpenframeTokenRefresher::start() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (running_) {
        LOG(WARNING) << "Token refresher is already running";
        return;
    }

    running_ = true;
    refresh_thread_ = std::thread([this]() {
        while (running_) {
            process();
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });
    LOG(INFO) << "Started token refresher with 5 second interval";
}

void OpenframeTokenRefresher::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_) {
            return;
        }
        running_ = false;
    }

    if (refresh_thread_.joinable()) {
        refresh_thread_.join();
    }
    LOG(INFO) << "Stopped token refresher";
}

void OpenframeTokenRefresher::process() {
    try {
        auto new_token = extractor_->extractToken();
        if (new_token.empty()) {
            LOG(ERROR) << "Failed to extract new token - empty token received";
            return;
        }

        auto& auth_manager = OpenframeAuthorizationManagerProvider::getInstance();
        auto current_token = auth_manager.getToken();
        
        if (new_token != current_token) {
            auth_manager.updateToken(new_token);
            LOG(INFO) << "Openframe token refreshed";
        }
    } catch (const std::exception& e) {
        LOG(ERROR) << "Error during token refresh: " << e.what();
    } catch (...) {
        LOG(ERROR) << "Unknown error during token refresh";
    }
}

} // namespace osquery 
