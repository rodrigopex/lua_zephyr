#include "channels.h"

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_consumer);
ZBUS_MSG_SUBSCRIBER_DEFINE(msub_consumer_arr);

ZBUS_CHAN_ADD_OBS(chan_acc_data, msub_consumer, 3);
ZBUS_CHAN_ADD_OBS(chan_acc_data_array, msub_consumer_arr, 3);

void consumer_thread(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	const struct zbus_channel *chan;
	struct msg_acc_data msg;
	struct msg_acc_data_consumed ack_msg = {.count = 0};

	while (1) {
		zbus_sub_wait_msg(&msub_consumer, &chan, &msg, K_FOREVER);
		++ack_msg.count;

		printk(" %d - Accelerometer data x=%02d,y=%02d,z=%02d from source: %s \n",
		       ack_msg.count, msg.x, msg.y, msg.z,
		       (strlen(msg.source) == 0) ? "unknown" : msg.source);

		zbus_chan_pub(&chan_acc_data_consumed, &ack_msg, K_MSEC(250));
	}
}

K_THREAD_DEFINE(consumer_thread_id, 4096, consumer_thread, NULL, NULL, NULL, 3, 0, 0);

void consumer_thread_array(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	const struct zbus_channel *chan;
	struct msg_acc_data_array msg;

	while (1) {
		zbus_sub_wait_msg(&msub_consumer_arr, &chan, &msg, K_FOREVER);

		printk(" %d - Accelerometer data array data=[", msg.count);
		for (size_t i = 0; i < msg.count; i++) {
			printk("%02d%s", msg.data[i], (i == msg.count - 1) ? "" : ",");
		}
		printk("]\n");
	}
}

K_THREAD_DEFINE(consumer_thread_array_id, 4096, consumer_thread_array, NULL, NULL, NULL, 3, 0, 0);
