#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/task_wdt/task_wdt.h>   

LOG_MODULE_REGISTER(zbus_pubsub, LOG_LEVEL_DBG);

/* ================================================================== */
/*  Global					                                          */
/* ================================================================== */

static int channel_id;

/* ================================================================== */
/*  MACROs					                                          */
/* ================================================================== */

#define SENSOR_COUNT 10
#define SENSOR_PERIOD_MS 100
#define STACK_SIZE 1024
#define PRIORITY 5
#define LONG_PERIOD_MS 3000
#define QUEUE_DEPTH 4
#define HEALTH_PERIOD_MS 100

/* ================================================================== */
/*  Shared channel message                                            */
/* ================================================================== */

struct sensor_data {
	int32_t temperature_mc;
    uint32_t timestamp_ms;
    uint8_t seq;
};

/* ================================================================== */
/*  Observers                                                         */
/* ================================================================== */

static void listener_cb(const struct zbus_channel *chan);
ZBUS_LISTENER_DEFINE(listener, listener_cb);
static void listener_cb(const struct zbus_channel *chan)
{
	const struct sensor_data *msg = (const struct sensor_data *)zbus_chan_const_msg(chan);

    LOG_INF("[DISPLAY-LIS] thread=%s seq=%u temp=%d mC",
            k_thread_name_get(k_current_get()),
            msg->seq,
            msg->temperature_mc);
}

ZBUS_SUBSCRIBER_DEFINE(subscriber, QUEUE_DEPTH);
static void subscriber_thread_fn(void *p1, void *p2, void *p3) 
{
	ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    k_thread_name_set(k_current_get(), "subscriber");

    const struct zbus_channel *chan;

    while (true) {
        int ret = zbus_sub_wait(&subscriber, &chan, K_FOREVER);
        if (ret != 0) {
            LOG_INF("[LOGGER-SUB] timeout, done");
            break;
        }

        struct sensor_data msg;
        ret = zbus_chan_read(chan, &msg, K_MSEC(100));
        if (ret != 0) {
            LOG_WRN("[LOGGER-SUB] read failed ret=%d", ret);    
            continue;
        }

		LOG_INF("[LOGGER-SUB] ok seq=%u temp=%d mC",
                    msg.seq,
                    msg.temperature_mc);
        
        task_wdt_feed(channel_id);

        k_msleep(LONG_PERIOD_MS);
    }
}

/* ================================================================== */
/*  Channel                                                           */
/* ================================================================== */

ZBUS_CHAN_DEFINE(
	sensor_channel, 
	struct sensor_data, 
	NULL, 
	NULL, 
	ZBUS_OBSERVERS(listener, subscriber), 
	ZBUS_MSG_INIT(.temperature_mc = 0, .timestamp_ms = 0, .seq = 0)
);

/* ================================================================== */
/*  Producer                                                          */
/* ================================================================== */

static void producer_thread_fn(void *p1, void *p2, void *p3) 
{
	ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    k_thread_name_set(k_current_get(), "sensor");

    for (int i = 0; i < SENSOR_COUNT; i++) {

        struct sensor_data data = {
            .temperature_mc = 24000 + (i * 350),
            .timestamp_ms = k_uptime_get_32(),
            .seq = (uint8_t)i,
        };

        LOG_INF("[SENSOR] publish seq=%u temp=%d mC",
                data.seq,
                data.temperature_mc);

        int ret = zbus_chan_pub(&sensor_channel, &data, K_MSEC(100));
        if (ret != 0) {
            LOG_WRN("[SENSOR] publish failed ret=%d seq=%u", ret, data.seq);
        }

        k_msleep(SENSOR_PERIOD_MS);
    }

    LOG_INF("[SENSOR] done");
}

/* ================================================================== */
/*  Health                                                            */
/* ================================================================== */

void health_thread_fn(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    k_thread_name_set(k_current_get(), "health");

    while (1) {

        // Get pending notifications
        uint32_t used = k_msgq_num_used_get(subscriber.queue);
        uint32_t usage_percentage = used * 100 / QUEUE_DEPTH;
        if(usage_percentage == 75) {
            LOG_WRN("[HEALTH] subscriber queue is almost full: %u/%u", used, QUEUE_DEPTH);
        }
        else if(usage_percentage > 75) {
            LOG_ERR("[HEALTH] subscriber queue is over 75%% full: %u/%u", used, QUEUE_DEPTH);
        }
        else
        {
            LOG_INF("[HEALTH] subscriber queue is healthy: %u/%u", used, QUEUE_DEPTH);
        }

        k_msleep(HEALTH_PERIOD_MS);
    }
}

/* ================================================================== */
/*  Threads Defines                                                   */
/* ================================================================== */

K_THREAD_DEFINE(producer_thread, STACK_SIZE, producer_thread_fn, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(subscriber_thread, STACK_SIZE, subscriber_thread_fn, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(health_thread, STACK_SIZE, health_thread_fn, NULL, NULL, NULL, PRIORITY, 0, 0);

/* ================================================================== */
/*  WDT                                                               */
/* ================================================================== */

void task_wdt_cb(int channel_id, void *user_data)
{
    LOG_ERR("Task watchdog triggered, channel_id=%d, thread with issues: %s", channel_id, k_thread_name_get(user_data));
}

int task_wdt_initialize(void)
{
    int ret = task_wdt_init(NULL);
    if(ret < 0) {
        LOG_ERR("Failed to initialize task watchdog: %d", ret);
    }

    channel_id = task_wdt_add(2000,
                         task_wdt_cb,
                         (void *)subscriber_thread);
    if(channel_id < 0) {
        LOG_ERR("Failed to add task watchdog channel: %d", channel_id);
    }
    return ret;
}

int main (void)
{
	LOG_INF("=== L5 Homework: WDT ===");
    
    task_wdt_initialize();
    
	return 0;
}
