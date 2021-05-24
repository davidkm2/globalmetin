#include "../include/FileMonitor.h"

#ifdef __FreeBSD__

#include <iostream>
#include <errno.h>

CFileMonitor::~CFileMonitor() noexcept
{
	if (kernel_event_queue_ != -1)
	{
		close(kernel_event_queue_);
		kernel_event_queue_ = -1;
	}

	for (const auto & _file : file_container_)
		close(_file.second->file);

	file_container_.clear();

	monitored_events_.clear();
	triggered_events_.clear();
}

bool CFileMonitor::AppendFile(const std::string& filename, TCallbackFunc callback)
{
	auto file_handle = open(filename.c_str(), O_RDONLY);
	if (file_handle == -1)
	{
		fprintf(stderr, "File: %s can't open. Error: %d\n", filename.c_str(), errno);
		return false; 
	}


	if (kernel_event_queue_ == -1)
		kernel_event_queue_ = kqueue();

	if (kernel_event_queue_ == -1)
	{
		fprintf(stderr, "Create kqueue fail. Error: %d\n", errno);
		return false;
	}

	if (file_container_.find(filename) != file_container_.end())
	{
		fprintf(stderr, "Duplicate file request! File: '%s'\n", filename.c_str());
		return false;
	}


	auto monitorctx = std::make_shared<SFileContext>();

	monitorctx->file = file_handle;
	monitorctx->index = monitored_events_.size();
	monitorctx->listener = callback;

	file_container_[filename] = monitorctx;


	struct kevent trigger_event, monitor_event;

   	EV_SET(&trigger_event,
   		file_handle,
   		EVFILT_VNODE, 
		EV_ADD | EV_ENABLE | EV_ONESHOT,
		NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_LINK | NOTE_RENAME | NOTE_REVOKE,
		0, 0
	);

	triggered_events_.emplace_back(trigger_event);
	monitored_events_.emplace_back(monitor_event);

	return true;
}

void CFileMonitor::OnTick()
{
	if (kernel_event_queue_ == -1 || file_container_.empty())
		return;

	auto event_count = kevent(
		kernel_event_queue_,
		&triggered_events_[0], triggered_events_.size(),
		&monitored_events_[0], monitored_events_.size(),
		nullptr
	);

	if (event_count == -1)
		return;
	
	if (event_count > 0)
	{
		for (auto i = 0; i < event_count; ++i)
		{
			auto event_flags = monitored_events_[i].flags;

			if (event_flags & NOTE_LINK)
			{
				for (const auto& [file, ctx] : file_container_)
				{
					if (ctx->index == i)
					{
						ctx->listener(file, event_flags);
						break;					
					}
				}
			}
		}
	}
}

#endif
