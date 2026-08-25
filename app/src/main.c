/*
 * Lecture 3 - Homework Starter Code
 *
 * GOAL: Convert a polling loop to an event-driven workqueue architecture.
 *
 * The starter code works but is INEFFICIENT.
 * polling_thread wakes every 10ms to check a flag.
 * sensor_sim fires every 100ms - that's 10 wasted wake-ups per event.
 *
 *
 * ================================================================
 * TASKS
 * ================================================================
 *
 * TASK 1 (starter - already works, just run it):
 *   Run the starter. Count wake-ups vs real events in the log.
 *   Expected: ~10 wake-ups per sensor event. Confirm this.
 *
 * TASK 2 (implement):
 *   Replace polling_thread with a k_work handler.
 *   sensor_sim should call k_work_submit() instead of setting a flag.
 *   The handler should do what polling_thread currently does.
 *
 *   Steps:
 *   - Define a work item with K_WORK_DEFINE
 *   - Write the handler function
 *   - In sensor_sim: call k_work_submit() (remove k_sem_give + flag)
 *   - Remove the polling_thread entirely
 *
 * TASK 3 (verify):
 *   Add k_uptime_get_32() to your handler's LOG_INF.
 *   Confirm handler runs only when sensor_sim fires (every ~100ms).
 *   No unnecessary wake-ups.
 *
 * BONUS (debounce):
 *   Change sensor_sim to fire 5 events within 20ms (not 1 per 100ms).
 *   Use k_work_reschedule with 30ms delay so only ONE handler
 *   call occurs after the burst - not 5.
 *   Log the reschedule timestamps to confirm the burst collapses.
 *
 * ================================================================
 */

#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(homework, LOG_LEVEL_DBG);

#define STACK_SIZE  1024
#define SENSOR_MS   20 /* sensor fires every 100ms */
#define EVENT_COUNT 5  /* total sensor events to produce */

// Bonus
#define DEBOUNCE_MS 30
#define INTERVAL_MS SENSOR_MS / EVENT_COUNT

/* ================================================================
 * STARTER CODE -- inefficient polling version
 * Run this first, then replace with workqueue in Task 2.
 * ================================================================ */

/* Statistics */
static int total_events;
static int total_processed;

static void sensor_handler (struct k_work *work)
{
	ARG_UNUSED(work);

	total_processed++;

	LOG_INF("[HANDLER] CONTEXT=%s processed=%d events=%d tick=%u", k_thread_name_get(k_current_get()),
	        total_processed, total_events, k_uptime_get_32());
}

K_WORK_DELAYABLE_DEFINE(debounce_work, sensor_handler);

/* ------------------------------------------------------------------ */
/*  sensor_sim - fires EVENT_COUNT events, 100ms apart               */
/* ------------------------------------------------------------------ */

static void sensor_sim_fn (void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	for (int i = 0; i < EVENT_COUNT; i++) {
		k_msleep(INTERVAL_MS);

		total_events++;

		uint32_t tick = k_uptime_get_32();

		int ret = k_work_reschedule(&debounce_work, K_MSEC(DEBOUNCE_MS));

		LOG_INF("[SENSOR] event=%d/%d reschedule_tick=%u deadline~%u ret=%d", total_events, EVENT_COUNT, tick,
		        tick + DEBOUNCE_MS, ret);
	}

	LOG_INF("[SENSOR] burst completed: %d events within ~%d ms", total_events, SENSOR_MS);
}

K_THREAD_DEFINE(sensor_thread, STACK_SIZE, sensor_sim_fn, NULL, NULL, NULL, 5, 0, 0);

int main (void)
{
	LOG_INF("=== L3 Homework: Debounce ===");

	/* Wait long enough for all events to complete */
	k_msleep((EVENT_COUNT + 2) * SENSOR_MS + 500);

	LOG_INF("[SUMMARY] total_events=%d total_processed=%d", total_events, total_processed);

	return 0;
}
