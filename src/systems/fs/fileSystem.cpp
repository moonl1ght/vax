#include "fileSystem.h"

namespace vax::fs {
std::optional<std::filesystem::path> getLatestFolder(const std::filesystem::path& parent_dir) {
    namespace fs = std::filesystem;
    fs::path latest_folder;
    fs::file_time_type latest_time;
    bool found = false;

    if (!fs::exists(parent_dir) || !fs::is_directory(parent_dir)) {
        return std::nullopt;
    }

    for (const auto& entry : fs::directory_iterator(parent_dir)) {
        if (entry.is_directory()) {
            auto current_time = fs::last_write_time(entry);

            if (!found || current_time > latest_time) {
                latest_time = current_time;
                latest_folder = entry.path();
                found = true;
            }
        }
    }

    if (!found) {
        return std::nullopt;
    }

    return latest_folder;
}
} // namespace vax::fs