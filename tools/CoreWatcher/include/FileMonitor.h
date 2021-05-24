#pragma once

#ifdef __FreeBSD__

#include <cstdint>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <functional>

#include <unistd.h>
#include <sys/event.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>

class CFileMonitor
{
	using TCallbackFunc = std::function<void(const std::string&, int32_t)>;

	using SFileContext = struct
	{
		int32_t			file;
		uint32_t		index;
		TCallbackFunc 	listener;
	};

	using TMonitorFileHashMap = std::unordered_map <std::string, std::shared_ptr <SFileContext>>;
	using TEventList = std::vector <struct kevent>;

public:
	CFileMonitor() = default; 
	~CFileMonitor() noexcept;

	CFileMonitor(const CFileMonitor& s) = default;
	CFileMonitor(CFileMonitor&& s) noexcept = default;
	CFileMonitor& operator=(const CFileMonitor&) = default;
	CFileMonitor& operator=(CFileMonitor&&) = default;

	bool AppendFile(const std::string &filename, TCallbackFunc callback); 
	void OnTick();

private:	
	TMonitorFileHashMap	file_container_;
	TEventList			monitored_events_;
	TEventList			triggered_events_;

	int32_t				kernel_event_queue_{-1};
};

#endif