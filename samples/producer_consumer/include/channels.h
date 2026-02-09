#ifndef CHANNELS_H
#define CHANNELS_H

#include <zephyr/zbus/zbus.h>
#include "proto/channels.pb.h"

ZBUS_CHAN_DECLARE(chan_acc_data);
ZBUS_CHAN_DECLARE(chan_acc_data_consumed);
ZBUS_CHAN_DECLARE(chan_version);
ZBUS_CHAN_DECLARE(chan_sensor_config);

#endif /* !CHANNELS_H */
