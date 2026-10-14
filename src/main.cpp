#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "tasks/tasks.h"
#include "board/button_event.h"

#define RGB_LED_PIN 48

Adafruit_NeoPixel rgb(1, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    rgb.begin();
    rgb.clear();
    rgb.show();

    Serial.begin(115200);

    startTasks();
}

void loop() {
  handleBootButton();
}
