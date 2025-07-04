#include <GyverOLED.h>
GyverOLED<SSH1106_128x64> oled; // Ensure this matches your OLED model

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

const float STILL_THRESHOLD_LOW = 9.0;
const float STILL_THRESHOLD_HIGH = 10.5;

// Debounce counters
const int DEBOUNCE_COUNT_MOVING = 5; // How many consecutive 'moving' readings to confirm motion
const int DEBOUNCE_COUNT_STILL = 10; // How many consecutive 'still' readings to confirm stillness

int movingCounter = 0;
int stillCounter = 0;

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10);

  oled.init(); oled.clear(); oled.home(); oled.print("Starting MPU6050..."); oled.update(); delay(500);
  Serial.println("Adafruit MPU6050 test!");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    oled.clear(); oled.home(); oled.print("MPU6050 Init Failed!"); oled.update(); while (1) delay(10);
  }
  Serial.println("MPU6050 Found!");
  oled.clear(); oled.home(); oled.print("MPU6050 Found!"); oled.update(); delay(1000);

  Serial.println("MPU6050 initialized for magnitude detection.");
  oled.clear(); oled.home(); oled.print("Ready for motion!"); oled.update(); delay(1000);

  oled.clear(); oled.home(); oled.setScale(2); oled.print("Still"); oled.update();
  Serial.println("Initial state: Still");
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float accelMagnitude = sqrt(
    (a.acceleration.x * a.acceleration.x) +
    (a.acceleration.y * a.acceleration.y) +
    (a.acceleration.z * a.acceleration.z)
  );

  bool rawMotionDetected = (accelMagnitude < STILL_THRESHOLD_LOW || accelMagnitude > STILL_THRESHOLD_HIGH);

  // Debouncing logic
  if (rawMotionDetected) {
    movingCounter++;
    stillCounter = 0; // Reset still counter
  } else {
    stillCounter++;
    movingCounter = 0; // Reset moving counter
  }

  static bool wasMoving = false; // Tracks the previous state for OLED/Serial updates
  bool currentStateMoving; // Current determined state

  if (movingCounter >= DEBOUNCE_COUNT_MOVING) {
    currentStateMoving = true;
  } else if (stillCounter >= DEBOUNCE_COUNT_STILL) {
    currentStateMoving = false;
  } else {
    currentStateMoving = wasMoving; // Maintain previous state during debounce period
  }

  if (currentStateMoving && !wasMoving) { // Transition from Still to Moving
    oled.clear();
    oled.home();
    oled.setScale(2);
    oled.print("Moving");
    oled.update();
    Serial.println("\n--- Motion Detected ---");
    wasMoving = true;
  } else if (!currentStateMoving && wasMoving) { // Transition from Moving to Still
    oled.clear();
    oled.home();
    oled.setScale(2);
    oled.print("Still");
    oled.update();
    Serial.println("\n--- Still ---");
    wasMoving = false;
  }

  // Print sensor data only if currently in the 'Moving' state
  if (currentStateMoving) {
    Serial.print("AccelX:"); Serial.print(a.acceleration.x, 2);
    Serial.print(", AccelY:"); Serial.print(a.acceleration.y, 2);
    Serial.print(", AccelZ:"); Serial.print(a.acceleration.z, 2);
    Serial.print(" | Magnitude:"); Serial.print(accelMagnitude, 2);
    Serial.print(" | GyroX:"); Serial.print(g.gyro.x, 2);
    Serial.print(", GyroY:"); Serial.print(g.gyro.y, 2);
    Serial.print(", GyroZ:"); Serial.print(g.gyro.z, 2);
    Serial.println("");
  }

  delay(600); // Reduced delay to allow more frequent sampling for debounce
}