#ifndef CONSUMER_H
#define CONSUMER_H

#include <zephyr/zbus/zbus.h>

struct msg_acc_data {
	int x;
	int y;
	int z;
};

struct msg_acc_data_consumed {
	int count;
};

ZBUS_CHAN_DECLARE(chan_acc_data);          /* Type: struct msg_acc_data */
ZBUS_CHAN_DECLARE(chan_acc_data_consumed); /* Type: struct msg_acc_data_ack */

#endif // !CONSUMER_H
