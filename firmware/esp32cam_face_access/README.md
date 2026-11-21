# Firmware

`esp32cam_face_access.ino` is the sketch written for this project: camera
initialization, Wi-Fi connection, and the access-control logic that drives the
relay and the LEDs.

The sketch does not compile on its own. It needs three companion files from the
Espressif **CameraWebServer** example, which must sit in the same folder:

| File | Origin | Change needed |
|---|---|---|
| `camera_pins.h` | Espressif example, unmodified | none |
| `camera_index.h` | Espressif example, unmodified | none |
| `app_httpd.cpp` | Espressif example, **modified** | declare `extern boolean matchFace;` and set `matchFace = true;` where the recognizer returns an enrolled face ID |

Get them from the Arduino IDE: **File → Examples → ESP32 → Camera →
CameraWebServer** (ESP32 core 1.0.x, which still ships the face-recognition
variant of this example).

## Before flashing

Replace the placeholders at the top of the sketch:

```cpp
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

Keep real credentials out of version control.
