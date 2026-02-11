# vfarm_mqtt

A lightweight **ESP-IDF MQTT + Wi-Fi helper library** built for FreeRTOS-based ESP32 projects.
It provides a **simple C++ wrapper** around ESP-IDF’s MQTT client with **queue-based message dispatching**, plus a minimal Wi-Fi helper for fast setup and server health checks.

Designed for **IoT devices, sensors, actuators, and edge nodes** that need clean MQTT publish/subscribe logic without boilerplate.

---

## Features

### MQTT

* ESP-IDF native MQTT client wrapper
* Global MQTT connection management
* Queue-based subscription model (FreeRTOS queues)
* Multiple subscribers per topic
* Safe message buffering and dispatch
* Optional debug logging
* Automatic reconnect awareness

### Wi-Fi

* Simple STA Wi-Fi initialization
* Credential configuration helpers
* Connection status checks
* Optional HTTP server availability check

---

## Architecture Overview

The library is split into **three core classes**:

```
MqttInitilizer  → Handles MQTT client setup & events
MqttDevice      → Used by application tasks to publish/subscribe
MqttBase        → Shared static state (client, queues, connection flag)
WifiCust        → Wi-Fi setup & health utilities
```

MQTT messages are routed internally using **FreeRTOS queues**, allowing different tasks to safely consume messages without blocking the MQTT event loop.

---

## Installation (ESP-IDF Component)

Place the component inside your project:

```
components/
└── vfarm_mqtt/
    ├── CMakeLists.txt
    ├── include/
    └── src/
```

ESP-IDF will automatically pick it up.

Ensure your project uses **ESP-IDF v5.x** (recommended).

---

## Dependencies

Declared in `CMakeLists.txt`:

* `mqtt`
* `esp_wifi`
* `esp_netif`
* `esp_event`
* `nvs_flash`
* `esp_http_client`
* `driver`

No additional third-party libraries required.

---

## Quick Start

### 1. Initialize Wi-Fi

```cpp
#include "wifi_c.hpp"

vfarm::WifiCust wifi("YOUR_SSID", "YOUR_PASSWORD");
```

Optional server health checking:

```cpp
vfarm::WifiCust wifi("SSID", "PASS", "http://server_ip:5000");
bool server_ok = wifi.server_is_up();
```

---

### 2. Initialize MQTT

```cpp
#include "mqtt_initilizer.hpp"

vfarm::MqttInitilizer mqtt_init(
    "mqtt://broker_ip:1883",
    "device_client_id"
);
```

This:

* Initializes the MQTT client
* Registers the event handler
* Starts the MQTT service

---

### 3. Publish Messages

```cpp
#include "mqtt_device.hpp"

vfarm::MqttDevice device;

const char payload[] = "Hello MQTT";
device.publish("esp32/test", (void*)payload, strlen(payload), 0, 0);
```

Publish is **non-blocking** and safe to call from FreeRTOS tasks.

---

### 4. Subscribe & Receive Messages

```cpp
QueueHandle_t rx_queue =
    device.subscribe("esp32/command", 0, 5);
```

Receive messages inside a task:

```cpp
vfarm::MqttBase::mqtt_message_t msg =
    device.get_msg(rx_queue, portMAX_DELAY);

printf("Topic: %s\n", msg.topic);
printf("Payload length: %d\n", msg.payload_len);
```

Each subscription gets its **own FreeRTOS queue**.

---

## MQTT Message Format

```cpp
struct mqtt_message_t {
    char     topic[MQTT_MAX_TOPIC_LEN];
    size_t   topic_len;
    uint8_t  payload[MQTT_MAX_PAYLOAD_LEN];
    size_t   payload_len;
};
```

Defaults:

* `MQTT_MAX_TOPIC_LEN` → 128 bytes
* `MQTT_MAX_PAYLOAD_LEN` → 2048 bytes

Both are configurable in `mqtt_base.hpp`.

---

## Debug Logging

Enable verbose MQTT debug output:

```cpp
vfarm::MqttBase::enable_debug_logs = true;
```

Logs include:

* Incoming topic names
* Queue states
* Dropped messages
* Subscriber matching

---

## Connection Handling

Check MQTT status:

```cpp
if (device.is_connected()) {
    // safe to publish
}
```

Block until reconnect:

```cpp
device.reconnect(portMAX_DELAY);
```

Connection state is tracked via an `atomic_bool`.

---

## Folder Structure

```
vfarm_mqtt/
├── include/
│   ├── mqtt_base.hpp
│   ├── mqtt_device.hpp
│   ├── mqtt_initilizer.hpp
│   └── wifi_c.hpp
├── src/
│   ├── mqtt_base.cpp
│   ├── mqtt_device.cpp
│   ├── mqtt_initilizer.cpp
│   └── wifi_c.cpp
└── CMakeLists.txt
```

---

## Design Notes

* **Single MQTT client**, shared across devices/tasks
* Message dispatch handled in the MQTT event callback
* Subscriptions matched by **exact topic string**
* Queues prevent blocking inside the MQTT event loop
* Clean separation between init logic and application logic

---

## Typical Use Cases

* IoT actuators & controllers
* Sensor nodes publishing telemetry
* ESP32 MQTT gateways
* Camera / image / binary payload streaming
* Command-response MQTT systems

---

## Limitations & Notes

* Topic matching is exact (no wildcards yet)
* Payload size limited by `MQTT_MAX_PAYLOAD_LEN`
* Not thread-safe to modify subscriptions after runtime start
* One MQTT broker per application instance

---

## License

MIT License
Free to use, modify, and integrate into commercial or open-source projects.

---