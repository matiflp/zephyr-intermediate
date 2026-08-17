#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(demo, LOG_LEVEL_DBG);

#define PRIO_WORKER 5
#define STACK_SIZE  1024
#define INCREMENTS  1000000

static volatile uint32_t counter;
static struct k_sem sem;

K_MUTEX_DEFINE(counter_mutex);

void t_worker_fn (void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	const char *name = k_thread_name_get(k_current_get());
	LOG_INF("THREAD %s: started", name);

	for (int i = 0; i < INCREMENTS; i++) {
		k_mutex_lock(&counter_mutex, K_FOREVER);
		counter++;
		k_mutex_unlock(&counter_mutex);
	}

	LOG_INF("THREAD %s: finished", name);
	k_sem_give(&sem);
}

K_THREAD_DEFINE(t_worker_1, STACK_SIZE, t_worker_fn, NULL, NULL, NULL, PRIO_WORKER, 0, 0);
K_THREAD_DEFINE(t_worker_2, STACK_SIZE, t_worker_fn, NULL, NULL, NULL, PRIO_WORKER, 0, 0);

int main (void)
{
	LOG_INF("Lesson 2 - Task 1: started");

	k_sem_init(&sem, 0, 2);

	k_sem_take(&sem, K_FOREVER);
	k_sem_take(&sem, K_FOREVER);

	LOG_INF("Final counter value: %u", (unsigned int)counter);

	return 0;
}
