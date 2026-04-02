# AtsPLC
[![Ats PLC](https://github.com/viordash/AtsPLC/actions/workflows/actions.yml/badge.svg)](https://github.com/viordash/AtsPLC/actions/workflows/actions.yml)

**Open-source Programmable Logic Controller** for ESP8266 with on-device ladder diagram programming.

AtsPLC provides a complete PLC implementation that runs on the ESP8266 microcontroller with an OLED display and optional web interface. Program ladder diagrams directly on the device without requiring a separate computer.

---

## Key Features

- **On-Device Programming** — Create and edit Ladder Diagram programs directly on the controller using a 0.96" OLED display
- **Web Interface** — Remote monitoring and control via browser
- **WiFi Integration** — Network scanning, AP mode, station mode with RSSI monitoring
- **I/O Bindings** — Connect WiFi networks and date/time to PLC variables
- **Backup/Restore** — Save and restore programs (up to 4 backup slots)
- **Comprehensive Testing** — Unit tests with CI/CD pipeline
- **OTA Updates** — Over-the-air firmware updates

---

## Project Structure

```
AtsPLC/
├── PLC_esp8266/              # Main ESP8266 application (embedded firmware)
│   ├── main/                 # Source code (264 .cpp/.h files)
│   │   ├── LogicProgram/     # PLC logic engine
│   │   │   ├── Inputs/       # Input elements (NO, NC, Timers, Comparators)
│   │   │   ├── Outputs/      # Output elements (Direct, Set, Reset, Inc/Dec)
│   │   │   ├── Flow/         # Network continuation elements
│   │   │   ├── Bindings/     # WiFi and DateTime bindings
│   │   │   ├── Settings/     # System configuration element
│   │   │   └── Serializer/   # Program serialization
│   │   ├── Display/          # SSD1306 OLED driver and rendering
│   │   ├── WiFi/             # WiFi service (Station, AP, Scanner)
│   │   ├── Datetime/         # RTC and SNTP synchronization
│   │   ├── HttpServer/       # REST API for web interface
│   │   ├── Maintenance/      # Service mode (SmartConfig, Backup/Restore)
│   │   └── main.cpp          # Entry point
│   ├── Makefile              # ESP-IDF build system
│   └── sdkconfig             # SDK configuration
│
├── Tests_esp8266/            # Unit testing framework (runs on Linux)
│   ├── src/                  # Test files
│   └── lib/                  # Mock implementations
│
├── SDK/                      # Dependencies
│   ├── ESP8266_RTOS_SDK/     # Espressif RTOS SDK (submodule)
│   ├── xtensa-lx106-elf/     # Cross-compiler toolchain
│   └── MigrateAnyCppData/    # Data migration utility (submodule)
│
├── TestsFramework/
│   └── CppUTest/             # Unit testing framework (submodule)
│
├── Tools/                    # Utility applications
│   ├── Bmp2Ssd1306/          # Bitmap-to-display converter
│   └── DisplayControls/      # Display control utilities
│
└── .github/workflows/        # CI/CD pipeline (GitHub Actions)
```

---

## Ladder Diagram Elements

### Inputs

| Element | Description |
|---------|-------------|
| **InputNO** | Normally open contact |
| **InputNC** | Normally closed contact |
| **TimerSecs** | Timer in seconds |
| **TimerMSecs** | Timer in milliseconds (minimum 50 ms) |
| **Comparators** | Value comparison: `==`, `!=`, `<`, `>`, `<=`, `>=` |
| **SquareWaveGenerator** | Square wave generator (10–99990 ms per half-wave) |
| **Indicator** | Value display with scaling (-9999999 to 99999990) |

### Outputs

| Element | Description |
|---------|-------------|
| **DirectOutput** | Direct value assignment |
| **SetOutput** | Set (latch) operation |
| **ResetOutput** | Reset (unlatch) operation |
| **IncOutput** | Increment counter |
| **DecOutput** | Decrement counter |

### Flow Control

| Element | Description |
|---------|-------------|
| **ContinuationIn** | Saves network state to global variable (placed at end of network) |
| **ContinuationOut** | Restores state from ContinuationIn (placed at start of network) |

### Bindings

#### WiFiBinding
Binds WiFi network to a controller variable (V1–V4):
- **Read** — Triggers network scan, writes RSSI of found network to variable (0–255)
- **Write** — Creates WiFi network with specified SSID

Parameters: SSID (up to 24 characters)

#### WiFiStaBinding
Monitors connection to WiFi station. Returns RSSI to bound variable.

Parameters: binding to V1–V4

#### WiFiApBinding
Creates access point with MAC address filtering.

Parameters:
- SSID (up to 24 characters, default "AtsPLC")
- Password (up to 16 characters, default "ats-PLC0")
- MAC filter (12 HEX characters, `*` = wildcard)

Writes number of connected clients to variable.

#### DateTimeBinding
Binds date/time parameters to controller variables (V1–V4):
- Seconds (0–60)
- Minutes (0–59)
- Hours (0–23)
- Day (1–31)
- Day of week (1–7, Monday = 1)
- Month (1–12)
- Year

---

## I/O Controllers

- **ControllerDI** — Digital Inputs (1 slot)
- **ControllerAI** — Analog Inputs (1 slot)
- **ControllerDO** — Digital Outputs (2 slots)
- **ControllerVariable** — Internal variables (4 slots: V1–V4)

---

## Building and Flashing

### Prerequisites

First-time setup (run once):

```bash
cd PLC_esp8266
make project_prepare  # Extract toolchain and apply patches
```

### Build

```bash
cd PLC_esp8266
make build
```

### Flash via USB

```bash
cd PLC_esp8266
make flash
```

### OTA Update

```bash
cd PLC_esp8266
make ota_upload OTA_HOST=192.168.1.100
```

---

## Testing

The project includes a comprehensive unit test suite that runs on Linux:

```bash
cd Tests_esp8266
make        # Build tests
make run    # Run all tests
```

Test coverage:
- Logic program elements (inputs, outputs, bindings)
- WiFi service and request handling
- DateTime functionality
- Display rendering
- GPIO and button handling
- Serialization and storage

---

## VSCode Integration

The project includes VSCode configuration for development:

**Tasks:**
- `PLC_esp8266/` - Build, rebuild, flash, OTA (Ctrl+Shift+P → Tasks: Run Task)
- `Tests_esp8266/` - Build, rebuild, run tests

**Debug:**
- Press F5 to run and debug tests

---

## Service Mode

Enter service mode by powering on with the **UP** button held down.

Available functions:
- **SmartConfig** — WiFi provisioning via mobile app
- **Backup** — Save program to one of 4 slots (ladder_0 – ladder_3)
- **Restore** — Restore program from backup slot
- **Reset Settings** — Restore default settings
- **Reset Ladder program** — Delete working program
- **Reset Backups** — Delete all backups
- **Factory reset** — Complete factory reset

---

## Screenshots

### Device Appearance
![image](https://github.com/user-attachments/assets/2a6a745b-9090-47bf-8d4d-82dcb25c557c)

### Ladder Network Design
![video5255966301494469014-ezgif com-video-to-gif-converter](https://github.com/user-attachments/assets/aa03b495-3384-4e60-8599-d4bca9c7021c)

### Demo: Interaction with Digital Inputs
![video5255966301494469089-ezgif com-video-to-gif-converter](https://github.com/user-attachments/assets/0eefa779-0906-4782-8f3c-0e5050de2d76)

---

## Architecture Highlights

### PLC Execution Cycle
- Controller processes all state changes in a single cycle
- If execution exceeds 2 system ticks (20 ms), cycle pauses for 2 ticks before next processing
- Minimizes latency between element actions

### State Persistence
- UI state (scroll position, selected network) saved to RTC RAM
- Valid date/time values saved every minute
- Program automatically saved after editing network

### WiFi Integration
- Asynchronous request queue for non-blocking WiFi operations
- Station, AP, and Scanner tasks run independently
- Event-driven architecture for WiFi/IP events

---

## CI/CD Pipeline

GitHub Actions workflow includes:
- **Code Analysis** — clang-format style checking, cppcheck, CodeChecker static analysis
- **Unit Tests** — Automated test execution on Linux
- **Build OTA** — Generate flashable firmware binary with artifact upload

---

## License

See LICENSE file for details.

---

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues.
