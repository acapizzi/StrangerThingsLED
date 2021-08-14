#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "FastLED.h"

// WIFI settings
#define WIFI_SSID "STRANGER WIFI"

// Network settings
#define DNS_PORT 53
#define WEB_PORT 80

IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(WEB_PORT );

#define PAGE \
"<!DOCTYPE html>"\
"<html>"\
"<head>"\
  "<title>Stranger Lights</title>"\
  "<meta name='viewport' content='initial-scale=1, maximum-scale=1'>"\
"</head>"\
"<body style='text-align: center;'>"\
  "<h3>STRANGER LIGHTS</h3>"\
  "<form>"\
    "<p>Enter your message below!</p>"\
    "<input type='text' name='message'><br>"\
    "<input type='submit' value='Send via the Upside Down'>"\
  "</form>"\
"</body>"\
"</html>"


// the milliseconds to give each letter
#define MILLIS_PER_LETTER 1000

// number of LEDs in the strip
#define NUM_LEDS 50

// the data pin the green wire from the LEDs are connected to
#define DATA_PIN 4

// an array to keep track of the LEDs
CRGB leds[NUM_LEDS];

// the message we will display
String message;

// the time we received the message
unsigned long received;

// the default millis per letter
//int millis_per_letter;

// we'll use all 26 letters of the alphabet
#define NUM_LETTERS 26

// the LED number (start counting from 0) that we light up to show our message
const int LETTER_LEDS[NUM_LETTERS] = {
 /*A*/  7
,/*B*/  8
,/*C*/  9
,/*D*/  10
,/*E*/  11
,/*F*/  12
,/*G*/  13
,/*H*/  14
,/*I*/  32
,/*J*/  31
,/*K*/  30
,/*L*/  29
,/*M*/  28
,/*N*/  26
,/*O*/  25
,/*P*/  24
,/*Q*/  23
,/*R*/  38
,/*S*/  39
,/*T*/  40
,/*U*/  41
,/*V*/  42
,/*W*/  44
,/*X*/  45
,/*Y*/  46
,/*Z*/  47
};

// how many colors to cycle through for the lights
#define NUM_COLORS 4

void setup() {
  // send print statements at 9600 baud
  Serial.begin(9600);

  // initialize the LEDS
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);

  // set them all to be off
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  // create the wifi network
  Serial.print("Creating the Network");
  Serial.println(WIFI_AP);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(WIFI_SSID);

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  // replay to all requests with same HTML
  webServer.onNotFound([]() {
    Serial.println(webServer.uri());
    Serial.print("message: ");
    String data = webServer.arg("message");
    if(data.length()){
      webServer.send(200, "text/html", PAGE);
      data.toLowerCase();
      Serial.println(data);
    
      // remember the message and the time it came in
      message = data;
      received = millis();
    }
    webServer.send(200, "text/html", PAGE);
  });
  webServer.begin();

  // this message will show until it is overwritten
  message = "the quick brown fox jumps over the lazy dog";
  received = millis();
}


void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();

  // how many milliseconds have elapsed since the last message came in
  unsigned long elapsed = millis() - received;

  // assuming MILLIS_PER_LETTER, what letter (index) ofthe message should we be on?
  int index = (elapsed/MILLIS_PER_LETTER)%message.length();

  // get the character letter we should print
  char letter = message.charAt(index);

  // if the character is between 'a' and 'z' (no numbers, spaces, or punctuations)
  if(letter >= 'a' && letter <= 'z'){
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
