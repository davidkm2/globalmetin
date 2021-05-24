#define __LIBTHECORE__
#include "stdafx.h"
#include "memory.h"
#include "srandomdev.h"

LPHEART		thecore_heart = NULL;

volatile int	shutdowned = FALSE;
volatile int	tics = 0;

// newstuff
int	bCheckpointCheck = 1;

static int pid_init(void)
{
#ifdef _WIN32
	return true;
#else
	FILE*	fp;
	if ((fp = fopen("pid", "w")))
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
		sys_log(0, "\nStart of pid: %d\n", getpid());
	}
	else
	{
		printf("pid_init(): could not open file for writing. (filename: ./pid)");
		sys_err("\nError writing pid file\n");
		return false;
	}
	return true;
#endif
}

static void pid_deinit(void)
{
#ifdef _WIN32
    return;
#else
    remove("./pid");
	sys_log(0, "\nEnd of pid\n");
#endif
}

bool thecore_init()
{
#ifdef _WIN32
    srand(static_cast<unsigned int>(time(nullptr)));
#else
    srandom(time(nullptr) + getpid() + getuid());
    srandomdev();
#endif
    signal_setup();

	if (!log_init() || !pid_init())
		return false;

	return true;
}

bool thecore_set(int fps, HEARTFUNC heartbeat_func)
{
	thecore_heart = heart_new(1000000 / fps, heartbeat_func);
	return true;
}

void thecore_shutdown()
{
    shutdowned = TRUE;
}

int thecore_idle(void)
{
    thecore_tick();

    if (shutdowned)
		return 0;

	int pulses = heart_idle(thecore_heart);

    return pulses;
}

void thecore_destroy(void)
{
	delete thecore_heart;
	pid_deinit();
	log_destroy();
}

int thecore_pulse(void)
{
	return (thecore_heart->pulse);
}

float thecore_pulse_per_second(void)
{
	return ((float) thecore_heart->passes_per_sec);
}

float thecore_time(void)
{
	return ((float) thecore_heart->pulse / (float) thecore_heart->passes_per_sec);
}

int thecore_is_shutdowned(void)
{
	return shutdowned;
}

void thecore_tick(void)
{
	++tics;
}
