#ifndef CONSUMER_H
#define CONSUMER_H

#include <lualib.h>
#include <zephyr/zbus/zbus.h>

struct msg_acc_data {
	int x;
	int y;
	int z;
	const char source[16];
};
ZBUS_CHAN_DECLARE(chan_acc_data); /* Type: struct msg_acc_data */

struct msg_acc_data_consumed {
	int count;
};
ZBUS_CHAN_DECLARE(chan_acc_data_consumed); /* Type: struct msg_acc_data_ack */

struct msg_version {
	uint32_t major;
	uint32_t minor;
	uint32_t patch;
	const char *hardware_id;
};
ZBUS_CHAN_DECLARE(chan_version); /* Type: struct msg_acc_data_ack */

#endif /* !CONSUMER_H */
