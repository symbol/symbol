/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "MemoryCounters.h"
#include "src/catapult/utils/DiagnosticCounter.h"
#include "src/catapult/utils/FileSize.h"
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#else
#include <string>
#endif

namespace catapult { namespace local {

	namespace {
#ifdef _WIN32
		PROCESS_MEMORY_COUNTERS GetMemoryInfo() {
			PROCESS_MEMORY_COUNTERS info;
			return GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info)) ? info : PROCESS_MEMORY_COUNTERS();
		}

#define GET_MEMORY_VALUE(NAME) utils::FileSize::FromBytes(GetMemoryInfo().NAME).megabytes()
#elif defined(__APPLE__)
		mach_task_basic_info GetMemoryInfo() {
			mach_task_basic_info info;
			mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
			return KERN_SUCCESS == task_info(mach_task_self(), MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info), &count)
					? info
					: mach_task_basic_info();
		}

#define GET_MEMORY_VALUE(NAME) utils::FileSize::FromBytes(GetMemoryInfo().NAME).megabytes()
#else
		// All fields in kilobytes, parsed from /proc/self/status.
		// VmHWM and VmRSS come from the same kernel accounting so VmHWM >= VmRSS is always guaranteed,
		// unlike mixing getrusage(ru_maxrss) with /proc/self/statm which can produce inconsistent MB values.
		struct MemoryInfo {
			uint64_t vmRss;   // VmRSS  – current resident set size
			uint64_t vmHwm;   // VmHWM  – peak resident set size
			uint64_t vmSize;  // VmSize – virtual memory size
			uint64_t rssFile; // RssFile – file-backed resident pages (proxy for shared RSS)
		};

		MemoryInfo GetMemoryInfo() {
			MemoryInfo info = {};
			std::ifstream fin("/proc/self/status");
			std::string line;
			while (std::getline(fin, line)) {
				std::istringstream iss(line);
				std::string key;
				uint64_t value;
				if ((iss >> key >> value) && !key.empty()) {
					if (key == "VmRSS:") info.vmRss = value;
					else if (key == "VmHWM:") info.vmHwm = value;
					else if (key == "VmSize:") info.vmSize = value;
					else if (key == "RssFile:") info.rssFile = value;
				}
			}
			return info;
		}

#define GET_MEMORY_VALUE(NAME) utils::FileSize::FromKilobytes(GetMemoryInfo().NAME).megabytes()
#endif

		utils::DiagnosticCounterId MakeId(const char* name) {
			return utils::DiagnosticCounterId(std::string("MEM ") + name);
		}
	}

	void AddMemoryCounters(std::vector<utils::DiagnosticCounter>& counters) {
#ifdef _WIN32
		counters.emplace_back(MakeId("CUR RSS"), []() { return GET_MEMORY_VALUE(WorkingSetSize); });
		counters.emplace_back(MakeId("MAX RSS"), []() { return GET_MEMORY_VALUE(PeakWorkingSetSize); });
		counters.emplace_back(MakeId("CUR CMT"), []() { return GET_MEMORY_VALUE(PagefileUsage); });
		counters.emplace_back(MakeId("MAX CMT"), []() { return GET_MEMORY_VALUE(PeakPagefileUsage); });
#elif defined(__APPLE__)
		counters.emplace_back(MakeId("CUR RSS"), []() { return GET_MEMORY_VALUE(resident_size); });
		counters.emplace_back(MakeId("MAX RSS"), []() { return GET_MEMORY_VALUE(resident_size_max); });
		counters.emplace_back(MakeId("CUR VIRT"), []() { return GET_MEMORY_VALUE(virtual_size); });
#else
		counters.emplace_back(MakeId("CUR RSS"), []() { return GET_MEMORY_VALUE(vmRss); });
		counters.emplace_back(MakeId("MAX RSS"), []() { return GET_MEMORY_VALUE(vmHwm); });
		counters.emplace_back(MakeId("CUR VIRT"), []() { return GET_MEMORY_VALUE(vmSize); });
		counters.emplace_back(MakeId("SHR RSS"), []() { return GET_MEMORY_VALUE(rssFile); });
#endif
		}
}}
