#include <sys/types.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(demo, LOG_LEVEL_DBG);

#define PRIO_LOW   7
#define PRIO_MED   5
#define PRIO_HIGH  3

#define PRIO_COOP  -1

#define STACK_SIZE 1024

void t_low_fn (void *p1, void *p2, void *p3)
{
	while (1) {
		LOG_INF("T_LOW running: tick=%u", k_uptime_get_32());
		k_msleep(300);
	}
}

void t_med_fn (void *p1, void *p2, void *p3)
{
	while (1) {
		LOG_INF("T_MED running: tick=%u", k_uptime_get_32());
		k_msleep(200);
	}
}

void t_high_fn (void *p1, void *p2, void *p3)
{
	while (1) {
		LOG_INF("T_HIGH running: tick=%u", k_uptime_get_32());
		k_msleep(100);
	}
}

void t_coop_1_fn (void *p1, void *p2, void *p3)
{
	LOG_INF("COOPERTIVE THREAD 1 started");

	for (int i = 0; i < 5; i++) {
		LOG_INF("COOPERATIVE THREAD 1: starting k_busy_wait, tick=%u", k_uptime_get_32());
		k_busy_wait(100000);
		LOG_INF("COOPERTIVE THREAD 1: tick=%u", k_uptime_get_32());
	}

	LOG_INF("COOPERTIVE THREAD 1: yielding");
	k_yield();
	LOG_INF("COOPERTIVE THREAD 1: finished");
}

void t_coop_2_fn (void *p1, void *p2, void *p3)
{
	LOG_INF("COOPERTIVE THREAD 2 started");

	for (int i = 0; i < 5; i++) {
		LOG_INF("COOPERTIVE THREAD 2: starting k_busy_wait, tick=%u", k_uptime_get_32());
		k_busy_wait(100000);
		LOG_INF("COOPERTIVE THREAD 2: tick=%u", k_uptime_get_32());
	}

	LOG_INF("COOPERTIVE THREAD 2: yielding");
	k_yield();
	LOG_INF("COOPERTIVE THREAD 2: finished");
}

void t_coop_3_fn (void *p1, void *p2, void *p3)
{
	LOG_INF("COOPERTIVE THREAD 3 started");

	for (int i = 0; i < 5; i++) {
		LOG_INF("COOPERTIVE THREAD 3: starting k_busy_wait, tick=%u", k_uptime_get_32());
		k_busy_wait(100000);
		LOG_INF("COOPERTIVE THREAD 3: tick=%u", k_uptime_get_32());
	}

	LOG_INF("COOPERTIVE THREAD 3: yielding");
	k_yield();
	LOG_INF("COOPERTIVE THREAD 3: finished");
}

void t_coop_4_fn (void *p1, void *p2, void *p3)
{
	LOG_INF("COOPERTIVE THREAD 4 started");

	for (int i = 0; i < 5; i++) {
		LOG_INF("COOPERTIVE THREAD 4: starting k_busy_wait, tick=%u", k_uptime_get_32());
		k_busy_wait(100000);
		LOG_INF("COOPERTIVE THREAD 4: tick=%u", k_uptime_get_32());
	}

	LOG_INF("COOPERTIVE THREAD 4: yielding");
	k_yield();
	LOG_INF("COOPERTIVE THREAD 4: finished");
}

// Thread definitions for the low, medium, and high priority threads
K_THREAD_DEFINE(t_low, STACK_SIZE, t_low_fn, NULL, NULL, NULL, PRIO_LOW, 0, 0);
K_THREAD_DEFINE(t_med, STACK_SIZE, t_med_fn, NULL, NULL, NULL, PRIO_MED, 0, 0);
K_THREAD_DEFINE(t_high, STACK_SIZE, t_high_fn, NULL, NULL, NULL, PRIO_HIGH, 0, 0);

// Cooperative threads
K_THREAD_DEFINE(t_coop_1, STACK_SIZE, t_coop_1_fn, NULL, NULL, NULL, PRIO_COOP, 0, 0);
K_THREAD_DEFINE(t_coop_2, STACK_SIZE, t_coop_2_fn, NULL, NULL, NULL, PRIO_COOP, 0, 0);
K_THREAD_DEFINE(t_coop_3, STACK_SIZE, t_coop_3_fn, NULL, NULL, NULL, PRIO_COOP, 0, 0);
K_THREAD_DEFINE(t_cop_4, STACK_SIZE, t_coop_4_fn, NULL, NULL, NULL, PRIO_COOP, 0, 0);

int main (void)
{
	LOG_INF("Lesson 1 - Task 1: started");
	return 0;
}
