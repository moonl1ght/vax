#include "fsLogger.h"
#include <fstream>

using namespace vax;

FsLogger::~FsLogger() {
    dump();
}

void FsLogger::log(const std::string& message) {
    _messages.push_back(message);
    if (_messages.size() >= _inMemoryMessages) {
        dump();
    }
}

void FsLogger::dump() {
    std::ofstream file(_path, std::ios::app);
    for (const auto& message : _messages) {
        file << message << std::endl;
    }
    _messages.clear();
}