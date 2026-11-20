#pragma once

#include <string>
#include <vector>

namespace vax {
class FsLogger final {
  public:
    FsLogger(const std::string& path, int inMemoryMessages = 10000)
        : _path(path)
        , _inMemoryMessages(inMemoryMessages) {
        _messages.reserve(_inMemoryMessages);
    };

    ~FsLogger();

    void log(const std::string& message);
    void dump();

  private:
    std::string _path;
    int _inMemoryMessages;
    std::vector<std::string> _messages;
};
} // namespace vax