/*
 * Secure Access Control System Using Facial Recognition
 * ESP32-CAM (AI-Thinker) + relay + 12 V electromagnetic solenoid lock
 *
 * Ikhlass Kennich - Project II, Obuda University, Kando Kalman Faculty
 *
 * Based on the Espressif CameraWebServer example (Apache-2.0), which provides
 * the streaming server, the web UI and the on-device face enrollment and
 * recognition. The access-control layer below is the part added for this
 * project: a recognized face drives a relay that releases the solenoid lock
 * for a fixed time, with a red/green LED indicating the lock state.
 *
 * The `matchFace` flag is raised in app_httpd.cpp when the recognizer returns
 * an enrolled subject; this sketch owns the actuation and the re-lock timer.
 */

#include "esp_camera.h"
#include <WiFi.h>

//
// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER

#define Relay 2    // relay IN -> releases the solenoid lock
#define Red   13   // red LED   -> door locked
#define Green 12   // green LED -> door unlocked

#include "camera_pins.h"

const char* ssid     = "YOUR_WIFI_SSID";      // set your own credentials
const char* password = "YOUR_WIFI_PASSWORD";  // do not commit real ones

void startCameraServer();

boolean matchFace     = false;  // raised by app_httpd.cpp on a recognized face
boolean activateRelay = false;  // true while the door is unlocked
long    prevMillis    = 0;
int     interval      = 5000;   // door stays unlocked for 5 s

void setup() {
  pinMode(Relay, OUTPUT);
  pinMode(Red,   OUTPUT);
  pinMode(Green, OUTPUT);
  digitalWrite(Relay, LOW);   // locked at power-up
  digitalWrite(Red,   HIGH);
  digitalWrite(Green, LOW);

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size   = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count     = 2;
  } else {
    config.frame_size   = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count     = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        // flip it back
    s->set_brightness(s, 1);   // up the brightness just a bit
    s->set_saturation(s, -2);  // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  // a face was recognized and the door is currently locked -> unlock
  if (matchFace == true && activateRelay == false) {
    activateRelay = true;
    digitalWrite(Relay, HIGH);
    digitalWrite(Green, HIGH);
    digitalWrite(Red,   LOW);
    prevMillis = millis();
  }

  // the unlock window has elapsed -> lock again
  if (activateRelay == true && millis() - prevMillis > interval) {
    activateRelay = false;
    matchFace     = false;
    digitalWrite(Relay, LOW);
    digitalWrite(Green, LOW);
    digitalWrite(Red,   HIGH);
  }
}
