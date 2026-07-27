# Industrial Edge Monitor — Qt Dashboard

Real-time system monitoring dashboard built with modern C++ and Qt 6. Reads CPU temperature, CPU usage, RAM and disk metrics with live charts and CSV data logging.

Cross-platform: reads real hardware sensors on Linux, simulates values on Windows.

## Features

- 4 sensor cards with real-time values and alarm indicators
- Live scrolling chart (temperature, CPU, RAM history)
- CSV data logging with timestamps (monitor_log.csv)
- Start/Stop monitoring control
- Color-coded alarm system (blue = OK, red = ALARM)
- Dark mode professional UI
- Cross-platform (Linux: real sensors, Windows: simulated)

## Screenshot

![Qt Dashboard](docs/qt-dashboard-chart.png)

## Tech Stack

- **Language**: C++17
- **Framework**: Qt 6 (Widgets, Charts)
- **Build**: CMake
- **CI/CD**: GitHub Actions
- **Platform**: Linux / Windows

## How it works

```text
                     System Hardware
                            |
                            v

+-------------------+
|   Sensor Readers  |
|   (C++ classes)   |
+-------------------+
| /proc/stat        |
| /proc/meminfo     |
| /sys/thermal      |
| statvfs           |
+-------------------+
      |          \
      |           \
      v            v

+-------------------+    +-------------------+
| Sensor Widgets    |    |    Live Chart     |
|  (Qt UI cards)    |    |     (QChart)      |
+-------------------+    +-------------------+

      |
      v

+-------------------+
|    CSV Logger     |
| monitor_log.csv   |
+-------------------+
```

On Linux: reads real values from /proc and /sys
On Windows: generates realistic simulated values

## Sensor Thresholds

| Sensor | Source | Threshold |
|--------|--------|-----------|
| CPU Temperature | /sys/class/thermal | > 70 C |
| CPU Usage | /proc/stat | > 80% |
| RAM Usage | /proc/meminfo | > 85% |
| Disk Usage | statvfs("/") | > 90% |

## Build

### Prerequisites
- Qt 6.x (Widgets + Charts modules)
- CMake 3.16+
- C++17 compiler (GCC, MinGW or MSVC)

### Build on Linux
```bash
sudo apt install qt6-base-dev qt6-charts-dev cmake g++
mkdir build && cd build
cmake ..
make
./EdgeMonitorQt
```

### Build on Windows

1. Install Qt 6 with Qt Creator
2. Open CMakeLists.txt in Qt Creator
3. Build and Run (Ctrl+R)

## CSV Log Format

The application logs all sensor readings to `monitor_log.csv`:

```csv
timestamp,cpu_temp,cpu_usage,ram_usage,disk_usage,state
2026-07-27 14:30:00,54.6,12.3,45.2,67.8,OK
2026-07-27 14:30:02,68.2,78.5,44.1,67.8,OK
2026-07-27 14:30:04,72.1,82.3,46.7,67.9,ALARM
```

## Related Projects

- [Industrial Protocol Bridge (Yocto ARM64)](https://github.com/meka23749/yocto-industrial-bridge) — Modbus TCP to MQTT bridge on custom Yocto Linux
- [BeagleY-AI Edge Monitor](https://github.com/meka23749/beagley-edge-monitor) — Real hardware monitoring on BeagleY-AI

## Author

**Steve Meka** — Embedded Software Engineer

- Website: [stevkmef.com](https://www.stevkmef.com)
- GitHub: [meka23749](https://github.com/meka23749)
