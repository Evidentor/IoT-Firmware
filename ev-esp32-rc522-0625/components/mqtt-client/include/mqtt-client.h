#pragma once

void mqtt_app_start(void (*data_received_callback)(const char *topic, const char *data));

void mqtt_publish_telemetry(const char *data);

void mqtt_subscribe_to_topic(const char *topic, const int qos);
