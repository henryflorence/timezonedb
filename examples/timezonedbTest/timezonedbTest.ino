#include <ESP8266WiFi.h>

WiFiClient client;

#include "timezonedb.h"

const char* ssid     = "internet -2g";
const char* password = "60Hewittav";


void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  long offset;
  getTimezone(&offset);

  delay(1000);
  Serial.println("\n");
  getTimezoneInfo();

  delay(1000);
  Serial.println("\n");
  long dstStart, dstEnd, gmtOffset;
  boolean dst;
  getTimezoneDst(&dst, &dstStart, &dstEnd, &gmtOffset);
  Serial.printf("dst: %d, dstStart: %ld, dstEnd: %ld, gmtOffset: %ld\n", dst, dstStart, dstEnd, gmtOffset);

}

void loop() {
  // put your main code here, to run repeatedly:

}
