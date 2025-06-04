// Acknowledgments

// Creator: Anany Sharma at the University of Florida working under NSF grant. 2405373

// This material is based upon work supported by the National Science Foundation under Grant No. 2405373. 
// Any opinions, findings, and conclusions or recommendations expressed in this material are those of the authors and do not necessarily reflect the views of the National Science Foundation.



// These are the 'brains' (libraries) we need for our sensor and the screen.


#include "Adafruit_APDS9960.h" // The library for our gesture sensor
#include <Adafruit_GFX.h>      // Core graphics library - helps draw shapes and text
#include <Adafruit_ST7789.h>   // Library for the ST7789 TFT screen

// --- Screen Pin Definitions ---
// These lines tell the Arduino which pins are connected to the screen.
// Think of them as addresses so the Arduino knows where to send screen data.
// IMPORTANT: These pin numbers might be different for your specific Arduino board or wiring.
// These are common for ESP32 boards.
#define TFT_CS    33  // TFT Chip Select pin: Tells the screen when we're sending it data.
#define TFT_DC    25  // TFT Data/Command pin: Tells the screen if we're sending a command or actual data to display.
#define TFT_RST   26  // TFT Reset pin: Can be used to reset the screen if something goes wrong. (Sometimes connected to the Arduino's reset pin).

// --- Color Definitions ---
// Let's define some colors we can use. Computers understand colors as numbers.
#define BLACK   0x0000  // No color (black)
#define WHITE   0xFFFF  // All colors mixed (white)
#define GREEN   0x07E0  // A nice green color for our gestures

// --- Create our sensor and screen objects ---
// An 'object' is like a special variable that knows how to control a piece of hardware.
Adafruit_APDS9960 apds; // This is our gesture sensor object.
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST); // This is our screen object. We give it the pin numbers we defined.

// This variable will remember the last gesture we showed on the screen.
// We use it so we don't keep re-drawing the same gesture over and over.
// 0 means no gesture has been shown yet, or it's an unknown one.
uint8_t last_gesture_shown = 0;

// The setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200); // Starts talking to your computer so you can see messages (for debugging).

  // --- Initialize the Screen ---
  // For a 1.9" 170x320 TFT screen. If your screen is different, these numbers might change.
  tft.init(170, 320);     // Tell the screen to get ready, specifying its pixel width and height.
  Serial.println("TFT initialized"); // Message for computer: screen is ready.
  tft.setRotation(3);     // Rotate the screen. '3' often means landscape mode (320 wide, 170 tall).
  tft.fillScreen(BLACK);  // Make the whole screen black to start with.

  // --- Display Initial Instructions on the Screen ---
  tft.setTextSize(2);             // Set text size to medium.
  tft.setTextColor(WHITE);        // Set text color to white.
  tft.setCursor(5, 5);            // Place the 'pen' (cursor) near the top-left. X=5, Y=5.
  tft.println("1. Wave hand CLOSE to sensor"); // First instruction line.
  tft.setCursor(5, 25);           // Move cursor down for the next line.
  tft.println("   to activate gesture mode.");
  
  // --- Initialize the Gesture Sensor ---
  if (!apds.begin()) {
    Serial.println("Failed to initialize device! Please check your wiring.");
    // Also show error on TFT if possible
    tft.fillRect(0, 85, tft.width(), 60, RED); // Red box for error
    tft.setCursor(10, 90);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.println("Sensor Error!");
    tft.println("Check Wiring.");
    while (1); // Stop here if sensor fails
  } else {
    Serial.println("Device initialized!");
  }

  // Gesture mode will be entered once proximity mode senses something close.
  apds.enableProximity(true); // Turn on the 'nearness' sensor.
  apds.enableGesture(true);   // Turn on the gesture sensing magic!
}

// The loop function runs over and over again forever
void loop() {
  // Try to read a gesture from the sensor.
  // 'apds.readGesture()' will return a number that tells us the gesture.
  uint8_t gesture = apds.readGesture();

  // If no gesture was detected (it returns APDS9960_NONE),
  // or if the detected gesture is the same as the one we last showed on screen,
  // then we don't need to update the screen. Just 'return' and start 'loop()' again.
  if (gesture == APDS9960_NONE || gesture == last_gesture_shown) {
    return; // Nothing new to show, so skip the rest.
  }

  // --- A New Gesture is Detected! ---
  // Remember this new gesture as the last one we've shown.
  last_gesture_shown = gesture;

  // Let's display the gesture nice and big!
  // Define an area on the screen where the gesture symbol will appear.
  // This is below our instructions.
  int gesture_display_y_start = 90;  // Y-coordinate where this area starts (below instructions).
  int gesture_display_height = 70;   // How tall this display area is.

  // Clear this part of the screen by drawing a black rectangle over it.
  // This erases any old gesture that was there.
  // Parameters: x, y, width, height, color
  tft.fillRect(0, gesture_display_y_start, tft.width(), gesture_display_height, BLACK);

  // Set up how our gesture symbol will look.
  tft.setTextSize(7);            // Make the text (our symbol) really big!
  tft.setTextColor(GREEN);       // Let's use a fun green color.

  // Figure out where to put the symbol so it's centered in our display area.
  // A single character at text size 7 is about 42 pixels wide (7*6) and 56 pixels tall (7*8).
  int16_t symbol_width = 42;
  int16_t symbol_height = 56;
  // Calculate X position to be in the middle of the screen width.
  int16_t x_position = (tft.width() - symbol_width) / 2;
  // Calculate Y position to be in the middle of our 'gesture_display_area'.
  int16_t y_position = gesture_display_y_start + (gesture_display_height - symbol_height) / 2;
  tft.setCursor(x_position, y_position); // Move our 'pen' to this spot.

  // Now, check which gesture was detected and print the right symbol on the screen.
  // Also, print it to the Serial Monitor (computer screen) for checking.
  if (gesture == APDS9960_UP) {
    Serial.println("^ UP");
    tft.print("^");     // Draw UP arrow symbol
  } else if (gesture == APDS9960_DOWN) {
    Serial.println("v DOWN");
    tft.print("v");     // Draw DOWN arrow symbol
  } else if (gesture == APDS9960_LEFT) {
    Serial.println("< LEFT");
    tft.print("<");     // Draw LEFT arrow symbol
  } else if (gesture == APDS9960_RIGHT) {
    Serial.println("> RIGHT");
    tft.print(">");     // Draw RIGHT arrow symbol
  }
  // Note: APDS9960_NEAR and APDS9960_FAR are also possible gesture types,
  // but the original sketch didn't specifically handle them for display,
  // so we'll stick to the four directions for now.

  delay(50); // A small pause. This helps make sure we don't try to read gestures too fast.
}