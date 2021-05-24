#include "stdafx.h"

#include "fdwatch.h"

#include <algorithm>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cassert>

#if defined(_WIN32)
#define USE_SELECT 1
#endif

#ifdef _WIN32
namespace
{
	int win32_init_refcount = 0;

	bool win32_init()
	{
		if (win32_init_refcount > 0)
		{
			win32_init_refcount++;
			return true;
		}

		WORD wVersion = MAKEWORD(2, 0);
		WSADATA wsaData;

		if (WSAStartup(wVersion, &wsaData) != 0)
			return false;

		win32_init_refcount++;
		return true;
	}

	void win32_deinit()
	{
		if (--win32_init_refcount <= 0)
			WSACleanup();
	}
}
#endif

#if !USE_SELECT

LPFDWATCH fdwatch_new(int nfiles)
{
	int kq = kqueue();
	if (kq == -1)
	{
		sys_err("kqueue() failed with %d", errno);
		return NULL;
	}

	LPFDWATCH fdw = new FDWATCH;

	fdw->kq = kq;
	fdw->nfiles = nfiles;
	fdw->nkqevents = 0;

	fdw->kqevents = new KEVENT[nfiles * 2];
	fdw->kqrevents = new KEVENT[nfiles * 2];
	fdw->fd_event_idx = new int[nfiles];
	fdw->fd_rw = new int[nfiles];
	fdw->fd_data = new void*[nfiles];

	memset(fdw->kqevents, 0, sizeof(KEVENT) * nfiles * 2);
	memset(fdw->kqrevents, 0, sizeof(KEVENT) * nfiles * 2);
	memset(fdw->fd_event_idx, 0, sizeof(int) * nfiles);
	memset(fdw->fd_rw, 0, sizeof(int) * nfiles);
	memset(fdw->fd_data, 0, sizeof(void*) * nfiles);

	return fdw;
}

void fdwatch_delete(LPFDWATCH fdw)
{
	delete[] fdw->fd_data;
	delete[] fdw->fd_rw;
	delete[] fdw->kqevents;
	delete[] fdw->kqrevents;
	delete[] fdw->fd_event_idx;
	delete fdw;
}

int fdwatch(LPFDWATCH fdw, struct timeval *timeout)
{
	int	i, r;
	struct timespec ts;

	if (!timeout)
	{
		ts.tv_sec = 0;
		ts.tv_nsec = 0;

		r = kevent(fdw->kq, fdw->kqevents, fdw->nkqevents,
			fdw->kqrevents, fdw->nfiles, &ts);
	}
	else
	{
		ts.tv_sec = timeout->tv_sec;
		ts.tv_nsec = timeout->tv_usec * 1000;

		r = kevent(fdw->kq, fdw->kqevents, fdw->nkqevents,
			fdw->kqrevents, fdw->nfiles, &ts);
	}

	fdw->nkqevents = 0;

	if (r == -1)
		return -1;

	memset(fdw->fd_event_idx, 0, sizeof(int) * fdw->nfiles);

	for (i = 0; i < r; i++)
	{
		int fd = fdw->kqrevents[i].ident;

		if (fd < fdw->nfiles)
		{
			if (fdw->kqrevents[i].filter == EVFILT_WRITE)
				fdw->fd_event_idx[fd] = i;
		}
		else
		{
			sys_err("ident overflow %d nfiles: %d", fdw->kqrevents[i].ident, fdw->nfiles);
		}
	}

	return r;
}

void fdwatch_register(LPFDWATCH fdw, int flag, int fd, int rw)
{
	if (flag == EV_DELETE)
	{
		if (fdw->fd_rw[fd] & FDW_READ)
		{
			fdw->kqevents[fdw->nkqevents].ident = fd;
			fdw->kqevents[fdw->nkqevents].flags = flag;
			fdw->kqevents[fdw->nkqevents].filter = EVFILT_READ;
			++fdw->nkqevents;
		}

		if (fdw->fd_rw[fd] & FDW_WRITE)
		{
			fdw->kqevents[fdw->nkqevents].ident = fd;
			fdw->kqevents[fdw->nkqevents].flags = flag;
			fdw->kqevents[fdw->nkqevents].filter = EVFILT_WRITE;
			++fdw->nkqevents;
		}
	}
	else
	{
		fdw->kqevents[fdw->nkqevents].ident = fd;
		fdw->kqevents[fdw->nkqevents].flags = flag;
		fdw->kqevents[fdw->nkqevents].filter = (rw == FDW_READ) ? EVFILT_READ : EVFILT_WRITE;

		++fdw->nkqevents;
	}
}

void fdwatch_clear_fd(LPFDWATCH fdw, socket_t fd)
{
	fdw->fd_data[fd] = NULL;
	fdw->fd_rw[fd] = 0;
}

void fdwatch_add_fd(LPFDWATCH fdw, socket_t fd, void* client_data, int rw, int oneshot)
{
	if (fd >= fdw->nfiles)
	{
		sys_err("fd overflow %d", fd);
		return;
	}

	if (fdw->fd_rw[fd] & rw)
		return;

	fdw->fd_rw[fd] |= rw;

	int flag;
	if (!oneshot) {
		flag = EV_ADD;
	}
	else {
		flag = EV_ADD | EV_ONESHOT;
		fdw->fd_rw[fd] |= FDW_WRITE_ONESHOT;
	}

	fdw->fd_data[fd] = client_data;
	fdwatch_register(fdw, flag, fd, rw);
}

void fdwatch_del_fd(LPFDWATCH fdw, socket_t fd)
{
	fdwatch_register(fdw, EV_DELETE, fd, 0);
	fdwatch_clear_fd(fdw, fd);
}

void fdwatch_clear_event(LPFDWATCH fdw, socket_t fd, unsigned int event_idx)
{
    assert((int)event_idx < fdw->nfiles * 2);

	if (fdw->kqrevents[event_idx].ident != static_cast<unsigned int>(fd))
		return;

	fdw->kqrevents[event_idx].ident = 0;
}

int fdwatch_check_event(LPFDWATCH fdw, socket_t fd, unsigned int event_idx)
{
    assert((int)event_idx < fdw->nfiles * 2);

	if (fdw->kqrevents[event_idx].ident != static_cast<unsigned int>(fd))
		return 0;

	if (fdw->kqrevents[event_idx].flags & EV_ERROR)
	{
		sys_err("EV_ERROR: %s", strerror(fdw->kqrevents[event_idx].data));
		return FDW_EOF;
	}

	if (fdw->kqrevents[event_idx].flags & EV_EOF)
		return FDW_EOF;

	if (fdw->kqrevents[event_idx].filter == EVFILT_READ)
	{
		if (fdw->fd_rw[fd] & FDW_READ)
			return FDW_READ;
	}
	else if (fdw->kqrevents[event_idx].filter == EVFILT_WRITE)
	{
		if (fdw->fd_rw[fd] & FDW_WRITE)
		{
			if (fdw->fd_rw[fd] & FDW_WRITE_ONESHOT)
				fdw->fd_rw[fd] &= ~FDW_WRITE;

			return FDW_WRITE;
		}
	}
	else
	{
		sys_err("fdwatch_check_event: Unknown filter %d (descriptor %d)",
			fdw->kqrevents[event_idx].filter, fd);
	}

	return 0;
}

int fdwatch_get_ident(LPFDWATCH fdw, unsigned int event_idx)
{
    assert((int)event_idx < fdw->nfiles * 2);
	return fdw->kqrevents[event_idx].ident;
}

int fdwatch_get_buffer_size(LPFDWATCH fdw, socket_t fd)
{
	int event_idx = fdw->fd_event_idx[fd];

	if (fdw->kqrevents[event_idx].filter == EVFILT_WRITE)
		return fdw->kqrevents[event_idx].data;

	return 0;
}

void* fdwatch_get_client_data(LPFDWATCH fdw, unsigned int event_idx)
{
    assert((int)event_idx < fdw->nfiles * 2);

	int fd = fdw->kqrevents[event_idx].ident;
	if (fd >= fdw->nfiles)
		return NULL;

	return fdw->fd_data[fd];
}

#else // __USE_SELECT__

LPFDWATCH fdwatch_new(int nfiles)
{
#if defined(_WIN32)
	if (!win32_init())
		return NULL;
#endif

	LPFDWATCH fdw = new FDWATCH;

	// nfiles value is limited to FD_SETSIZE (64)
	fdw->nfiles = std::min<uint32_t>(nfiles, FD_SETSIZE);
	fdw->nselect_fds = 0;

	FD_ZERO(&fdw->rfd_set);
	FD_ZERO(&fdw->wfd_set);

	fdw->select_fds = new socket_t[nfiles];
	fdw->select_rfdidx = new int[nfiles];
	fdw->fd_rw = new int[nfiles];
	fdw->fd_data = new void*[nfiles];

	memset(fdw->select_fds, 0, sizeof(socket_t) * nfiles);
	memset(fdw->select_rfdidx, 0, sizeof(int) * nfiles);
	memset(fdw->fd_rw, 0, sizeof(int) * nfiles);
	memset(fdw->fd_data, 0, sizeof(void*) * nfiles);

	return fdw;
}

void fdwatch_delete(LPFDWATCH fdw)
{
	delete[] fdw->fd_data;
	delete[] fdw->fd_rw;
	delete[] fdw->select_rfdidx;
	delete[] fdw->select_fds;
	delete fdw;

#if defined(_WIN32)
	win32_deinit();
#endif
}

static int fdwatch_get_fdidx(LPFDWATCH fdw, socket_t fd)
{
	for (int i = 0; i < fdw->nselect_fds; ++i)
	{
		if (fdw->select_fds[i] == fd)
			return i;
	}

	return -1;
}

void fdwatch_add_fd(LPFDWATCH fdw, socket_t fd, void* client_data, int rw, int oneshot)
{
	int idx = fdwatch_get_fdidx(fdw, fd);
	if (idx < 0)
	{
		if (fdw->nselect_fds >= fdw->nfiles)
			return;

		idx = fdw->nselect_fds;
		fdw->select_fds[fdw->nselect_fds++] = fd;
		fdw->fd_rw[idx] = rw;
	}
	else
	{
		fdw->fd_rw[idx] |= rw;
	}

	fdw->fd_data[idx] = client_data;

	if (rw & FDW_READ)
		FD_SET(fd, &fdw->rfd_set);

	if (rw & FDW_WRITE)
		FD_SET(fd, &fdw->wfd_set);
}

void fdwatch_del_fd(LPFDWATCH fdw, socket_t fd)
{
	if (fdw->nselect_fds <= 0)
		return;

	int idx = fdwatch_get_fdidx(fdw, fd);
	if (idx < 0)
		return;

	std::move(&fdw->select_fds[idx + 1],
		&fdw->select_fds[fdw->nselect_fds],
		&fdw->select_fds[idx]);

	std::move(&fdw->fd_data[idx + 1],
		&fdw->fd_data[fdw->nselect_fds],
		&fdw->fd_data[idx]);

	std::move(&fdw->fd_rw[idx + 1],
		&fdw->fd_rw[fdw->nselect_fds],
		&fdw->fd_rw[idx]);

	--fdw->nselect_fds;

	fdw->select_fds[fdw->nselect_fds] = 0;
	fdw->fd_data[fdw->nselect_fds] = 0;
	fdw->fd_rw[fdw->nselect_fds] = 0;

	FD_CLR(fd, &fdw->rfd_set);
	FD_CLR(fd, &fdw->wfd_set);
}

int fdwatch(LPFDWATCH fdw, struct timeval *timeout)
{
	int r;
	struct timeval tv;

	fdw->working_rfd_set = fdw->rfd_set;
	fdw->working_wfd_set = fdw->wfd_set;

	if (!timeout)
	{
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		r = select(0, &fdw->working_rfd_set, &fdw->working_wfd_set,
			(fd_set*)0, &tv);
	}
	else
	{
		tv = *timeout;
		r = select(0, &fdw->working_rfd_set, &fdw->working_wfd_set,
			(fd_set*)0, &tv);
	}

	if (r == -1)
		return -1;

	int event_idx = 0;

	for (int i = 0; i < fdw->nselect_fds; ++i)
	{
		if (fdwatch_check_fd(fdw, fdw->select_fds[i]))
			fdw->select_rfdidx[event_idx++] = i;
	}

	return event_idx;
}

int fdwatch_check_fd(LPFDWATCH fdw, socket_t fd)
{
	int idx = fdwatch_get_fdidx(fdw, fd);
	if (idx < 0)
		return 0;

	int result = 0;
	if ((fdw->fd_rw[idx] & FDW_READ) && FD_ISSET(fd, &fdw->working_rfd_set))
		result |= FDW_READ;

	if ((fdw->fd_rw[idx] & FDW_WRITE) && FD_ISSET(fd, &fdw->working_wfd_set))
		result |= FDW_WRITE;

	return result;
}

void * fdwatch_get_client_data(LPFDWATCH fdw, unsigned int event_idx)
{
	int idx = fdw->select_rfdidx[event_idx];
	if (idx < 0 || fdw->nfiles <= idx)
		return NULL;

	return fdw->fd_data[idx];
}

int fdwatch_get_ident(LPFDWATCH fdw, unsigned int event_idx)
{
	int idx = fdw->select_rfdidx[event_idx];
	if (idx < 0 || fdw->nfiles <= idx)
		return 0;

	return (int)fdw->select_fds[idx];
}

void fdwatch_clear_event(LPFDWATCH fdw, socket_t fd, unsigned int event_idx)
{
	int idx = fdw->select_rfdidx[event_idx];
	if (idx < 0 || fdw->nfiles <= idx)
		return;

	socket_t rfd = fdw->select_fds[idx];
	if (fd != rfd)
		return;

	FD_CLR(fd, &fdw->working_rfd_set);
	FD_CLR(fd, &fdw->working_wfd_set);
}

int fdwatch_check_event(LPFDWATCH fdw, socket_t fd, unsigned int event_idx)
{
	int idx = fdw->select_rfdidx[event_idx];
	if (idx < 0 || fdw->nfiles <= idx)
		return 0;

	socket_t rfd = fdw->select_fds[idx];
	if (fd != rfd)
		return 0;

	int result = fdwatch_check_fd(fdw, fd);
	if (result & FDW_READ)
		return FDW_READ;
	else if (result & FDW_WRITE)
		return FDW_WRITE;

	return 0;
}

int fdwatch_get_buffer_size(LPFDWATCH fdw, socket_t fd)
{
	return INT_MAX; // XXX TODO
}

#endif

