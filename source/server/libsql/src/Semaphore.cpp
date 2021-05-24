#include "stdafx.h"
#include "Semaphore.h"

#if defined(_WIN32)

CSemaphore::CSemaphore()
	: m_sem(NULL)
{
	Initialize();
}

CSemaphore::~CSemaphore()
{
	Destroy();
}

bool CSemaphore::Initialize()
{
	Clear();

	m_sem = ::CreateSemaphore(NULL, 0, 32, NULL);
	if (m_sem == NULL)
		return false;

	return true;
}

void CSemaphore::Destroy()
{
	Clear();
}

void CSemaphore::Clear()
{
	if (m_sem) {
		::CloseHandle(m_sem);
		m_sem = nullptr;
	}
}

bool CSemaphore::Wait()
{
	assert(m_sem && "Not initialized");

	uint32_t dwWaitResult = ::WaitForSingleObject(m_sem, INFINITE);
	switch (dwWaitResult) {
	case WAIT_OBJECT_0:
		return true;

	default:
		break;
	}

	return false;
}

bool CSemaphore::Release(int32_t count)
{
	assert(m_sem  && "Not initialized");

	::ReleaseSemaphore(m_sem, count, NULL);
	return true;
}

#else

CSemaphore::CSemaphore()
{
	Initialize();
}

CSemaphore::~CSemaphore()
{
	Destroy();
}

bool CSemaphore::Initialize()
{
	if (sem_init(&m_sem, 0, 0) == -1) {
		sys_err("sem_init() failed with %d", errno);
		return false;
	}

	return true;
}

void CSemaphore::Destroy()
{
	Clear();
}

void CSemaphore::Clear()
{
	sem_destroy(&m_sem);
}

bool CSemaphore::Wait()
{
	while (true) {
		int32_t re = sem_wait(&m_sem);
		if (re == 0)
			break;

		if (errno != EINTR) {
			sys_err("sem_wait() failed with %d",
				errno);
			return false;
		}
	}

	return true;
}

bool CSemaphore::Release(int32_t count)
{
	for (int32_t i = 0; i < count; ++i) {
		if (sem_post(&m_sem) != 0) {
			sys_err("sem_post(%p) failed with %d",
				&m_sem, errno);
			return false;
		}
	}

	return true;
}

#endif
