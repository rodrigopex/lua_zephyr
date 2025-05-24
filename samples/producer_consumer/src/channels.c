#include "channels.h"
#include <zephyr/zbus/zbus.h>

ZBUS_CHAN_DEFINE(chan_acc_data, struct msg_acc_data, NULL, NULL, ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(.x = 0, .y = 0, .z = 0));

ZBUS_CHAN_DEFINE(chan_acc_data_consumed, struct msg_acc_data_consumed, NULL, NULL,
		 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.count = 0));

ZBUS_CHAN_DEFINE(chan_version, struct msg_version, NULL, NULL, ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(.major = 4, .minor = 7, .patch = 98, .hardware_id = "RPA9"));
