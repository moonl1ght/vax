#pragma once

#include <filesystem>
#include <optional>

namespace vax::fs {
std::optional<std::filesystem::path> getLatestFolder(const std::filesystem::path& parent_dir);

class FileSystem final {
  public:
    FileSystem();
    ~FileSystem();
};
} // namespace vax::fs