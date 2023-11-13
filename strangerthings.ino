#include <Regexp.h>
#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
//#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#include "FastLED.h"
// WIFI settings
#define WIFI_SSID "Passwordistaco"
#define WIFI_PASSWORD "xxx"
// Firebase settings
#define FIREBASE_HOST "stranger-things-a19dd.firebaseio.com"
#define FIREBASE_AUTH "xxx"
#define FIREBASE_PATH "/data/message"
// the milliseconds to give each letter
#define MILLIS_PER_LETTER 3000
// number of LEDs in the strip
#define NUM_LEDS 100
// the data pin the green wire from the LEDs are connected to
#define DATA_PIN 5
// an array to keep track of the LEDs
CRGB leds[100];
// the message we will display
String message;
// the time we received the message
unsigned long received;
// we'll use all 26 letters of the alphabet
#define NUM_LETTERS 25
// the LED number (start counting from 0) that we light up to show our message
const int LETTER_LEDS[NUM_LETTERS] = {
 /*A*/  33
,/*B*/  36
,/*C*/  38
,/*D*/  40
,/*E*/  42
,/*F*/  44
,/*G*/  46
,/*H*/  48
,/*I*/  31
,/*J*/  29
,/*K*/  27
,/*L*/  25
,/*M*/  24
,/*N*/  22
,/*O*/  20
,/*P*/  19
,/*Q*/  16
,/*R*/  1
,/*S*/  3
,/*T*/  5
,/*U*/  7
,/*V*/  9
,/*W*/  11
,/*X*/  13
,/*Y*/  15
};

// how many colors to cycle through for the lights
#define NUM_COLORS 6

void setup() {
  // send print statements at 9600 baud
  Serial.begin(9600);

  // initialize the LEDS
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);

  // set them all to be off
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  // connect to Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.print("Firebase begun");
  Firebase.stream(FIREBASE_PATH);

  // this message will show until it is overwritten from Firebase and shown if Firebase fails
  message = "stranger things";
  received = millis();

  
}


void loop() {
  // if Firebase fails, print to the console
  if (Firebase.failed()) {
    Serial.println("streaming error");
    Serial.println(Firebase.error());
  }

  if (Firebase.available()) {
     FirebaseObject event = Firebase.readEvent();
     String eventType = event.getString("type");
     eventType.toLowerCase();
     
     Serial.print("event: ");
     Serial.println(eventType);
     
     // if there is a new data event
     if (eventType == "put") {
       Serial.print("data: ");
       String data = event.getString("data");
       data.toLowerCase();
       Serial.println(data);

       // remember the message and the time it came in
       message = data;
       received = millis();
     }
  }

  // how many milliseconds have elapsed since the last message came in
  unsigned long elapsed = millis() - received;

  // assuming MILLIS_PER_LETTER, what letter (index) ofthe message should we be on?
  int index = (elapsed/MILLIS_PER_LETTER)%message.length();

  // get the character letter we should print
  char letter = message.charAt(index);

  // if the character is between 'a' and 'z' (no numbers, spaces, or punctuations)
  if(letter >= 'a' && letter < 'z'){
    // how bright to make this LED from 0 to 1, this is what makes them fade in and out
    // it calculates what percent we are completed with the letter, and makes it fade in from 0-50% and fade out from 50-100%
    // the formula can be visualized here: https://www.desmos.com/calculator/5qk8imeny4
    float brightness = 1-abs((2*(elapsed%MILLIS_PER_LETTER)/((float)MILLIS_PER_LETTER))-1);
    uint8_t value = 255 * brightness;
    
    // get the LED number the letter should be in, assuming our array starts at 'a' and ends at 'z'
    int letter_index = letter-'a';
    int led = LETTER_LEDS[letter_index];

    // get a rotation of colors, so that every NUM_COLORS lights, it loops
    // e.g. red, yellow, green, blue, red, yellow green blue
    uint8_t hue = (letter_index%NUM_COLORS*255)/NUM_COLORS;

    // set that LED to the color
    leds[led] = CHSV(hue, 255, value);

         // Move a single white led 
   for(int whiteLed = 100; whiteLed > 50; whiteLed = whiteLed - 1) {
      // Turn our current led on to white, then show the leds
      leds[whiteLed] = CRGB::White;

      // Turn our current led back to black for the next loop around
      leds[whiteLed] = CRGB::Black;
   }
    FastLED.show();
    // set it to black so we don't have to remember the last LED we turned on
    leds[led] = CRGB::Black;
    
    Serial.print(letter);
    Serial.print("\t!");
    Serial.print(led);
    Serial.print("\t=");
    Serial.print(brightness);
    Serial.print("\t@");
    Serial.print(elapsed);
    Serial.println();

  } else {
    // if the letter wasn't a-z then, we just turn off all the leds
    FastLED.show();
  }
}
