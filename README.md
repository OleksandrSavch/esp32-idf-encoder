# 🚀 ESP32 Encoder Counter with LCD Display

This project uses an ESP32 to count encoder pulses, calculate distance and speed, display the data on an I2C 16x2 LCD, and optionally transmit the data over a network.

## 📌 Features

- 📈 Counts quadrature encoder pulses with direction
- 📏 Calculates distance (in meters) based on wheel diameter
- 🚀 Calculates speed (m/s) with smoothing filter
- 💡 Displays speed and distance on a 16x2 I2C LCD
- 🔁 Resets via hardware button (GPIO12)
- 📡 Planned: REST API, WebSocket, OTA updates

## 🧰 Hardware Requirements

- ESP32 development board
- Quadrature encoder (600 pulses/rev)
- Wheel (diameter: 10 cm)
- 16x2 LCD with I2C interface (e.g. PCF8574)
- Push button (connected to GPIO12)
- Optional: logic analyzer or serial monitor

## 🔌 Wiring

| Component    | ESP32 GPIO |
|--------------|------------|
| Encoder A    | GPIO13     |
| Encoder B    | GPIO14     |
| LCD SDA      | GPIO21     |
| LCD SCL      | GPIO22     |
| Reset Button | GPIO12     |

## ⚙️ Build Instructions

This project uses [PlatformIO](https://platformio.org/) with the ESP-IDF framework.  
Make sure you have PlatformIO installed in [VSCode](https://platformio.org/install/ide?install=vscode) or available via CLI.

### 📂 Clone and Open

```bash
git clone https://github.com/oleksandr-s/esp32-idf-encoder.git
cd esp32-encoder
code .

pio run                       # Build the project
pio run -t upload             # Compile and upload
pio device monitor            # Open serial monitor
pio run -t uploadfs           # Upload SPIFFS (if used)
pio run -t clean              # Clean build
rm -rf .pio/build             # Manual clean

pio run -t upload && pio device monitor
```

## 📝 Attribution

This project uses a driver for the 16x2 I2C LCD partially based on:

esp32-i2c-lcd1602
Copyright © 2018 David Antliff
Licensed under the MIT License.

See [NOTICE.md](./NOTICE.md) for third-party acknowledgments.

## 📚 References

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [esp32-i2c-lcd1602](https://github.com/feugy/esp32-i2c-lcd1602)


## 📄 License

This project is licensed under the terms of the [MIT License](./LICENSE).

This project tracks development transparently, including exploratory and test commits.
The v1.0.0 tag represents a stable milestone.
Earlier commits reflect iterative design and integration phases.