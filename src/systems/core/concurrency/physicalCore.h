#pragma once

#include <cstdint>
#include <vector>

#ifdef _WIN32
#include <algorithm>
#include <windows.h>
#elif __linux__
#include <fstream>
#include <sched.h>
#include <set>
#include <sstream>
#endif

namespace vax::core {

struct PhysicalCore {
    static std::vector<PhysicalCore> cores() {
        std::vector<PhysicalCore> cores;

#ifdef _WIN32
        DWORD len = 0;
        GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &len);
        std::vector<uint8_t> buffer(len);
        auto* info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data());

        if (GetLogicalProcessorInformationEx(RelationProcessorCore, info, &len)) {
            uint8_t* ptr = buffer.data();
            while (ptr < buffer.data() + len) {
                auto* item = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(ptr);
                if (item->Relationship == RelationProcessorCore) {
                    PhysicalCore core;
                    core.efficiency_class = item->Processor.EfficiencyClass;
                    KAFFINITY mask = item->Processor.GroupMask[0].Mask;
                    for (uint32_t i = 0; i < 64; ++i) {
                        if (mask & (1ULL << i)) {
                            core.siblings.push_back(i);
                        }
                    }
                    if (!core.siblings.empty()) {
                        core.primary_logical_id = core.siblings[0];
                        cores.push_back(std::move(core));
                    }
                }
                ptr += item->Size;
            }
        }
        std::sort(cores.begin(), cores.end(), [](const auto& a, const auto& b) {
            return a.efficiency_class > b.efficiency_class;
        });

#elif __linux__
        cpu_set_t process_mask;
        CPU_ZERO(&process_mask);
        if (sched_getaffinity(0, sizeof(cpu_set_t), &process_mask) != 0) {
            return cores;
        }

        std::set<uint32_t> visited_logical;
        for (uint32_t cpu = 0; cpu < CPU_SETSIZE; ++cpu) {
            if (!CPU_ISSET(cpu, &process_mask) || visited_logical.count(cpu)) {
                continue;
            }

            PhysicalCore core;
            std::ifstream f("/sys/devices/system/cpu/cpu" + std::to_string(cpu) + "/topology/thread_siblings_list");
            std::string line;
            if (std::getline(f, line)) {
                std::stringstream ss(line);
                std::string item;
                while (std::getline(ss, item, ',')) {
                    auto dash = item.find('-');
                    if (dash != std::string::npos) {
                        uint32_t start = std::stoul(item.substr(0, dash));
                        uint32_t end = std::stoul(item.substr(dash + 1));
                        for (uint32_t s = start; s <= end; ++s) {
                            core.siblings.push_back(s);
                            visited_logical.insert(s);
                        }
                    } else {
                        uint32_t s = std::stoul(item);
                        core.siblings.push_back(s);
                        visited_logical.insert(s);
                    }
                }
            } else {
                core.siblings.push_back(cpu);
                visited_logical.insert(cpu);
            }

            core.primary_logical_id = core.siblings[0];
            cores.push_back(std::move(core));
        }
#endif

        return cores;
    }

    uint32_t primary_logical_id;
    std::vector<uint32_t> siblings;
    uint8_t efficiency_class = 0;
};

} // namespace vax::core