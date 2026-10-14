#include "esp_camera.h"
#include <HardwareSerial.h>

#define BAUD 115200   // use 115200 first if you get garbage, then raise it (must match STM32)

// AI Thinker ESP32-CAM pins
#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM   0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27
#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    21
#define Y4_GPIO_NUM    19
#define Y3_GPIO_NUM    18
#define Y2_GPIO_NUM     5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22

HardwareSerial STM32Serial(2);

void sendFrame(const uint8_t *data, uint32_t len) {
  uint8_t hdr[6] = {0xAA, 0x55,
                    (uint8_t)len, (uint8_t)(len >> 8),
                    (uint8_t)(len >> 16), (uint8_t)(len >> 24)};
  STM32Serial.write(hdr, 6);
  delay(20);                       // gives the STM32 time to re-arm its receive
  if (len == 0) return;

  uint32_t sum = 0;
  for (uint32_t i = 0; i < len; i++) sum += data[i];

  STM32Serial.write(data, len);
  uint8_t cs[4] = {(uint8_t)sum, (uint8_t)(sum >> 8),
                   (uint8_t)(sum >> 16), (uint8_t)(sum >> 24)};
  STM32Serial.write(cs, 4);
  STM32Serial.flush();
}

void takeAndSendPicture() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (fb) { esp_camera_fb_return(fb); }   // drop a possibly stale frame
  fb = esp_camera_fb_get();               // fresh frame
  if (!fb) {
    Serial.println("Capture failed");
    sendFrame(nullptr, 0);
    return;
  }
  Serial.printf("Sending %u bytes\n", fb->len);
  sendFrame(fb->buf, fb->len);
  esp_camera_fb_return(fb);
}

void setup() {
  Serial.begin(115200);   // debug (FTDI)
  STM32Serial.setRxBufferSize(256);
  STM32Serial.begin(BAUD, SERIAL_8N1, 14, 15);

  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
config.pin_sscb_sda = SIOD_GPIO_NUM;
config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size   = FRAMESIZE_96X96;   // 320x240, roughly 5-15 KB
  config.pixel_format = PIXFORMAT_RGB565;  // raw pixels, no JPEG decode on the STM32
  config.fb_count     = 1;
  config.fb_location  = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;               // 0-63, lower = better quality/bigger
  config.grab_mode    = CAMERA_GRAB_LATEST;

  if (esp_camera_init(&config) != ESP_OK) {
  Serial.println("Camera init failed");
  return;
}

sensor_t *s = esp_camera_sensor_get();
if (s) {
  s->set_ae_level(s, -1);      // darker, less washed-out frames
  s->set_whitebal(s, 1);
  s->set_gain_ctrl(s, 1);
  s->set_exposure_ctrl(s, 1);
  s->set_lenc(s, 1);
  s->set_sharpness(s, 1);
  s->set_contrast(s, 1);
}
}
void loop() {
  if (STM32Serial.available()) {
    String s = STM32Serial.readStringUntil('\n');
    s.trim();
    if (s == "TP") takeAndSendPicture();
  }
}
