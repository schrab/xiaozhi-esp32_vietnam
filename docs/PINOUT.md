# D-Solution / OSTB Xiaozhi — Pinout & Hardware Reference

**MCU:** ESP32-S3 N16R8 (16MB Flash, 8MB Octal PSRAM)  
**Board SKU:** `ostb-xiaozhi-3st` / Box D-Solution 1.83" / 2" 2-Mic  
**Firmware Version:** Xiaozhi ESP32 v2.4.2.1+  
**Audio Codecs:** ES8311 (Speaker DAC / Amp) + ES7210 (Mic ADC array, TDM Slave)  
**Display:** NV3030B / NV3023 (284×240 SPI)  
**Touch Controller:** CST816S Capacitive Touch (IC ID: 184)  
**Partition Table:** [PARTITIONS.md](file:///home/schrab/Projects/xiaozhi-esp32_vietnam/docs/PARTITIONS.md) (5.25MB Dual OTA + 5.375MB Assets)  
**Connectivity:** Wi-Fi 2.4 GHz + Bluetooth 5 (LE)  

---

## 1. Display — NV3030B / NV3023 (SPI Interface)

| Signal | Pin Label | GPIO | Description |
| :--- | :--- | :--- | :--- |
| **LCD_DC** | DC | **GPIO 8** | Data / Command Selection |
| **LCD_CS** | CS | **GPIO 14** | Chip Select |
| **LCD_SCL** | SCL | **GPIO 9** | SPI Clock (80 MHz) |
| **LCD_SDA** | SDA | **GPIO 10** | SPI MOSI (host → display) |
| **LCD_RST** | RES | **GPIO 18** | Hardware Reset |
| **LCD_BL** | BL | **GPIO 13** | Backlight Control (PWM) |

**Display Configuration:**
- **Resolution:** 284 × 240
- **Interface:** SPI3 @ 80 MHz
- **Swap XY:** `true`
- **Mirror X:** `false`
- **Mirror Y:** `true` (180° rotation)
- **Offset X / Y:** `0, 0`
- **LVGL Image Cache:** 2 MB PSRAM allocated

---

## 2. Touchscreen — CST816S

- **Touch Controller IC:** CST816S (Chip ID: `184`)
- **Interface:** I2C bus

---

## 3. Audio Subsystem — ES8311 (DAC) + ES7210 (ADC) + NS4150 (Amp)

| Signal | Pin Label | GPIO | Description |
| :--- | :--- | :--- | :--- |
| **AUDIO_MCLK** | MCLK | **GPIO 5** | Master Clock for audio codecs |
| **AUDIO_BCLK** | BCLK | **GPIO 15** | I2S Bit Clock |
| **AUDIO_WS** | WS | **GPIO 16** | I2S Word Select (Frame Sync) |
| **AUDIO_DOUT** | DOUT | **GPIO 6** | I2S Data Out (ESP32 → ES8311 DAC) |
| **AUDIO_DIN** | DIN | **GPIO 7** | I2S Data In (ES7210 ADC → ESP32) |
| **AUDIO_PA** | PA | **GPIO 4** | Power Amplifier Enable (NS4150 on/off) |
| **AUDIO_I2C_SCL** | SCL | **GPIO 11** | Codec I2C Clock |
| **AUDIO_I2C_SDA** | SDA | **GPIO 12** | Codec I2C Data |

**Audio Parameters:**
- **Speaker DAC:** ES8311 (Slave mode)
- **Power Amplifier:** NS4150
- **Microphone ADC:** ES7210 (Slave mode, TDM multi-mic input)
- **Input / Output Sample Rate:** 24,000 Hz duplex
- **Audio Input Reference:** `true`

---

## 4. Power Management & System Buttons

| Function | Pin Label | GPIO | Description |
| :--- | :--- | :--- | :--- |
| **Detect Charge** | CHG | **GPIO 47** | Battery charging status input |
| **Battery Level** | BAT | **GPIO 38** | Battery ADC voltage monitor |
| **Sleep Control** | SLEEP | **GPIO 21** | RTC Sleep / Power Hold Control |
| **Boot Button** | BOOT | **GPIO 0** | Boot / Mode Selection (Active Low) |
| **Volume Up** | V+ | **GPIO 39** | Volume Up button |
| **Volume Down** | V- | **GPIO 40** | Volume Down button |

---

## 5. Console & Debug UART

| Signal | GPIO | Description |
| :--- | :--- | :--- |
| **UART TX** | **GPIO 43** | Serial Debug Output (115200 baud) |
| **UART RX** | **GPIO 44** | Serial Debug Input (115200 baud) |
| **USB CDC** | Built-in USB-Serial/JTAG | Connected via `/dev/ttyACM0` |

---

## Complete GPIO Pinout Summary

| GPIO | Function | Direction | Description |
| :--- | :--- | :--- | :--- |
| **GPIO 0** | BOOT Button | Input | Bootloader mode (Active Low) |
| **GPIO 4** | AUDIO_PA | Output | NS4150 Amplifier Power Enable |
| **GPIO 5** | AUDIO_MCLK | Output | Codec Master Clock (ES8311 / ES7210) |
| **GPIO 6** | AUDIO_DOUT | Output | I2S Data Out (ESP32 → ES8311) |
| **GPIO 7** | AUDIO_DIN | Input | I2S Data In (ES7210 → ESP32) |
| **GPIO 8** | LCD_DC | Output | Display Data / Command Select |
| **GPIO 9** | LCD_SCL | Output | Display SPI Clock (80 MHz) |
| **GPIO 10** | LCD_SDA | Output | Display SPI MOSI (Data In) |
| **GPIO 11** | AUDIO_I2C_SCL | Output | Codec / Peripheral I2C Clock |
| **GPIO 12** | AUDIO_I2C_SDA | In/Out | Codec / Peripheral I2C Data |
| **GPIO 13** | LCD_BL | Output (PWM) | Display Backlight Brightness Control |
| **GPIO 14** | LCD_CS | Output | Display SPI Chip Select |
| **GPIO 15** | AUDIO_BCLK | Output | I2S Bit Clock |
| **GPIO 16** | AUDIO_WS | Output | I2S Word Select / LRCK |
| **GPIO 18** | LCD_RST | Output | Display Hardware Reset |
| **GPIO 21** | SLEEP Control | Output | RTC Sleep / Power Hold Control |
| **GPIO 38** | BAT Monitor | Input (ADC) | Battery Voltage Sensing |
| **GPIO 39** | Volume Up | Input | Volume + Button |
| **GPIO 40** | Volume Down | Input | Volume - Button |
| **GPIO 43** | UART TX | Output | Console UART TX |
| **GPIO 44** | UART RX | Input | Console UART RX |
| **GPIO 47** | CHG Detect | Input | Charger status detection |

