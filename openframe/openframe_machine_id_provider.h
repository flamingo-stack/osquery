#pragma once
#include <string>
#include <boost/noncopyable.hpp>

namespace osquery {

class OpenframeMachineIdProvider : private boost::noncopyable {
 public:
  /**
   * @brief Get the singleton instance
   */
  static OpenframeMachineIdProvider& getInstance();

  /**
   * @brief Get the machine ID, reading from file if not cached
   *
   * @return The machine ID string, or empty string if not available
   */
  std::string getMachineId();

  /**
   * @brief Force re-read of machine ID from file
   */
  void refresh();

 private:
  OpenframeMachineIdProvider() = default;
  ~OpenframeMachineIdProvider() = default;

  std::string readFromFile();
  std::string getFilePath();

  std::string cached_machine_id_;
  bool initialized_ = false;
};

} // namespace osquery
