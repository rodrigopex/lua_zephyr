#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

struct msg_input {
	int data;
};

ZBUS_CHAN_DEFINE(chan_random_input, struct msg_input, NULL, NULL, ZBUS_OBSERVERS(msub_consumer),
		 ZBUS_MSG_INIT(.data = 0));

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_consumer);

int main(int argc, char *argv[])
{
	const struct zbus_channel *chan;
	struct msg_input msg;

	while (1) {
		zbus_sub_wait_msg(&msub_consumer, &chan, &msg, K_FOREVER);

		printk("Random data %d\n", msg.data);
	}
	return 0;
}
