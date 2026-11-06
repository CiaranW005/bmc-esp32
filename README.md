# ESP32 BMC Firmware

The ESP32 is the local hardware bridge for [BMC](../README.md). It maintains a connection to the Oracle server, wakes the PC over the LAN, and forwards shutdown, restart, and telemetry requests to the PC agent.

## Implementation

The firmware targets an ESP32-S3 using Arduino and PlatformIO. Its main responsibilities are split into FreeRTOS tasks:

| Task | Work |
| --- | --- |
| Network | Initializes interfaces and checks connectivity every 500 ms |
| Oracle | Services the WebSocket and sends queued replies |
| Controller | Executes queued commands, communicates with the PC agent, and checks deadlines |
| Logger | Drains a fixed-capacity queue to serial and UDP transports |

`src/network/manager.cc` prefers the W5500 Ethernet interface. At startup it waits up to three seconds for Ethernet, then attempts WiFi. Interface selection changes increment a generation counter so application tasks can recreate their connections. The periodic update selects among connected interfaces; it does not implement a full reconnect/backoff policy.

`src/oracle/oracle_handler.cc` receives commands over `/device`. A mutex protects command and reply handoffs between the Oracle and controller tasks. Only one power command is pending at a time; telemetry has a separate slot that retains the latest waiting request.

`src/controllers/network_controller.cc` talks to the PC agent using newline-delimited JSON over TCP. Wake-on-LAN broadcasts a magic packet to UDP port 9. The wake attempt completes when the agent connects within 50 seconds, or fails on timeout. Shutdown and restart go through the agent; response timeouts are suppressed because those operations can take the agent offline.

The C++ protocol code in `include/protocol` mirrors the Go packet format. Acknowledgement tracking handles delivery, response tracking matches `reply_to`, and duplicate detection prevents repeated execution within its retention window.

Logging uses a 64-entry node pool. A full queue rejects incoming entries. An entry is removed when either available transport sends it successfully; UDP send success does not confirm receipt by the log collector.

## Hardware and configuration

The current configuration uses:

- ESP32-S3 DevKitC-1.
- W5500 SPI wiring defined in `src/network/interfaces/ethernet.cc`.
- Onboard RGB LED on GPIO 48, cleared during startup.
- A BOOT button handler that currently has no assigned action.

Provide the ignored `include/config/secrets.h` and `src/secrets.cc` with declarations/definitions for `WIFI_SSID`, `WIFI_PASSWORD`, `ORACLE_IP`, `LOG_HOST`, `LOG_PORT`, `DEVICE_TOKEN`, and `PC_MAC`. The current Oracle endpoint comes from `include/config/config.h`; `ORACLE_IP` is not used by that connection path. `LOG_HOST` must be an IPv4 address in the current UDP transport.

For production, provide ignored `include/config/device_credentials.h` defining `CLIENT_CERT` and `CLIENT_KEY`. The root CA is in `include/config/config.h`. Keep private keys and credentials out of version control. The device bearer token must match `ESP32_DEVICE_TOKEN` on the Oracle server.

The PC agent address is currently hardcoded in `NetworkManagementController::connect()` as `192.168.1.67:9090`. Update it for your LAN, along with the PC MAC address. Enable Wake-on-LAN on the PC's supported hardware/firmware and operating system.

## Build and upload

From this directory, with PlatformIO installed:

```bash
pio run -e esp32s3
pio run -e esp32s3 -t upload
pio device monitor -b 115200
```

Update `monitor_port` and `upload_port` in `platformio.ini` for your board.

The `esp32s3-local` environment defines `ORACLE_LOCAL`, selecting plain WebSocket at the local address in `config.h` instead of production TLS. Bearer authentication remains enabled:

```bash
pio run -e esp32s3-local
pio run -e esp32s3-local -t upload
```

The credentials header is included unconditionally, so it must still exist for a local build even though the local connection does not use its certificate values.

## Physical power controller

`PowerManagementController` can pulse an optocoupler on GPIO 16 and check feedback on GPIO 17. Its operations block while simulating button presses. This controller is present in the code but is not instantiated by the current task setup; automatic fallback from network commands to physical power control is not implemented.
