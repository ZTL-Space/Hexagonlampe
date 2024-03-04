
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include <PubSubClient.h>
#include <WiFi.h>

#define PIN_WS2812B 16 // The ESP32 pin GPIO16 connected to WS2812B
#define NUM_PIXELS 5   // The number of LEDs (pixels) on WS2812B LED strip

Adafruit_NeoPixel ws2812b(NUM_PIXELS, PIN_WS2812B, NEO_GRBW + NEO_KHZ800);

const char *ssid = "";
const char *password = "";
const char *mqtt_server = "";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi()
{

    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1')
    {
        // turn pixels to green one-by-one with delay between each pixel
        for (int pixel = 0; pixel < NUM_PIXELS; pixel++)
        {                                                              // for each pixel
            ws2812b.setPixelColor(pixel, ws2812b.Color(0, 255, 0, 0)); // it only takes effect if pixels.show() is
                                                                       // called update to the WS2812B Led Strip
        }
        ws2812b.show();
    }
    else
    {
        // turn pixels to green one-by-one with delay between each pixel
        for (int pixel = 0; pixel < NUM_PIXELS; pixel++)
        {                                                              // for each pixel
            ws2812b.setPixelColor(pixel, ws2812b.Color(255, 0, 0, 0)); // it only takes effect if pixels.show() is
                                                                       // called update to the WS2812B Led Strip
        }
        ws2812b.show();
    }
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str() /*, "ztl", "Ztl2019!"*/))
        {
            Serial.println("connected");
            // Once connected, publish an announcement...
            client.publish("ztl_out", "hello world");
            // ... and resubscribe
            client.subscribe("ztl_in");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup()
{
    ws2812b.begin();              // initialize WS2812B strip object (REQUIRED)
    pinMode(BUILTIN_LED, OUTPUT); // Initialize the BUILTIN_LED pin as an output
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    ws2812b.begin(); // initialize WS2812B strip object (REQUIRED)
    for (int pixel = 0; pixel < NUM_PIXELS; pixel++)
    {                                                              // for each pixel
        ws2812b.setPixelColor(pixel, ws2812b.Color(0, 0, 0, 255)); // it only takes effect if pixels.show() is
                                                                   // called update to the WS2812B Led Strip
    }
    ws2812b.show();
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    unsigned long now = millis();
    if (now - lastMsg > 2000)
    {
        lastMsg = now;
        ++value;
        snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
        Serial.print("Publish message: ");
        Serial.println(msg);
        client.publish("ztl_out", msg);
    }
}
