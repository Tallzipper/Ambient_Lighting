#include <Arduino.h>
#include <FastLED.h>            // Library mapping physicsal LEDs onto memory

#define LED_PIN         16      // GPIO 16 Data Pin
#define NUM_LEDS        75      // LEDS on my machine
#define BRIGHTNESS      60      // 

#define SERIAL_BAUD     115200  // ESP32's baudrate
#define PREFIX_SIZE     6       // Adalight control

CRGB leds[NUM_LEDS];            // Allocates memory to hold the LED's light info
const uint8_t prefix[] = {'A', 'd', 'a'}; // Adalight start sequence

void setup() {
    
    Serial.begin(SERIAL_BAUD);
    Serial.setRxBufferSize(1024); // Improves ring buffer 
    
    Serial.setTimeout(10);

    // Configures the LEDS to the program

    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS); 
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear(true);

    // Will all act as a test bestbench to verify the lights are propperly connected

    fill_solid(leds, NUM_LEDS, CRGB::Green);
    FastLED.show();    
    delay(2000);

    FastLED.clear(true); // wipe lights
    FastLED.show();
  
    Serial.print("Ada\n"); // Sends message that it's ready
}

void loop() {

    // Wait until at least the 6-byte Adalight header is ready
    if (Serial.available() >= PREFIX_SIZE) {

        //Checks header is at the start
        if (Serial.read() == prefix[0]) {
            
            // Wait for remaining 5 header bytes if necessary
            while (Serial.available() < 5){};

            // 5 remaining bytes are read

            uint8_t d   = Serial.read(); 
            uint8_t a   = Serial.read();
            uint8_t hi  = Serial.read();
            uint8_t lo  = Serial.read();
            uint8_t chk = Serial.read();

            // Character validator to check for corruption
            if (d == prefix[1] && a == prefix[2] && chk == (hi ^ lo ^ 0x55)) {
            
                uint8_t rawBuffer[NUM_LEDS * 3]; // Color amounts 

                // Gets bytes needed for the colors, three bytes per LED
                size_t bytesRead = Serial.readBytes ((char*)rawBuffer, NUM_LEDS * 3);

                // LEDs are assigned a color
                if (bytesRead == NUM_LEDS * 3) {
                    for (int i = 0; i < NUM_LEDS; i++) {  
                        leds[i].r = rawBuffer[i * 3 + 0]; // Red
                        leds[i].g = rawBuffer[i * 3 + 1]; // Green
                        leds[i].b = rawBuffer[i * 3 + 2]; // Blue
                    }
                    FastLED.show(); // Instantly push out frame
                }
            }
        } 
        else { // If fails test, remove garbage bytes from program
            while (Serial.available() > 256) {
                Serial.read();
            }
        }
    }
}