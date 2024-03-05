
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include <PubSubClient.h>
#include <WiFi.h>

#define PIN_WS2812B 16 // The ESP32 pin GPIO16 connected to WS2812B
#define NUM_PIXELS 5   // The number of LEDs (pixels) on WS2812B LED strip
#define PIN_BUTTON 27

Adafruit_NeoPixel ws2812b(NUM_PIXELS, PIN_WS2812B, NEO_GRBW + NEO_KHZ800);

const char *ssid = "";
const char *password = "";
const char *mqtt_server = "";

const char *topic = "ztl";

WiFiClient espClient;
PubSubClient client(espClient);

bool ButtonMessageReceivedFlag = false;
bool IAmTheFirstSender = false;

enum State
{
    OFF,
    PRIMARY_COLOR,
    SECONDARY_COLOR
};

void HandleSignal();
void HandleButton();

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
        ButtonMessageReceivedFlag = true;
    }
}

void reconnect()
{
    ws2812b.fill(ws2812b.Color(255, 0, 0, 0));
    ws2812b.show();

    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str()))
        {
            Serial.println("connected");
            client.subscribe(topic);
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

    ws2812b.fill(ws2812b.Color(0, 0, 0, 0));
    ws2812b.show();
}

void setup()
{
    pinMode(PIN_BUTTON, INPUT_PULLUP);

    ws2812b.begin();
    ws2812b.fill(ws2812b.Color(0, 0, 255, 0));
    ws2812b.show();

    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    HandleSignal();
    HandleButton();
}

void HandleSignal()
{
    static State state = State::OFF;
    static unsigned long timeout = 0xFFFFFFFF;
    unsigned long now = millis();

    switch (state)
    {
    case State::OFF:
        if (ButtonMessageReceivedFlag)
        {
            ws2812b.fill(ws2812b.Color(255, 0, 0, 0));
            ws2812b.show();
            timeout = millis() + 30 * 1000;
            state = State::PRIMARY_COLOR;
        }
        break;
    case State::PRIMARY_COLOR:
        if (ButtonMessageReceivedFlag)
        {
            ws2812b.fill(ws2812b.Color(0, 0, 0, 255));
            ws2812b.show();
            timeout = millis() + 30 * 1000;
            state = State::SECONDARY_COLOR;
        }
        else if (now > timeout)
        {
            state = State::OFF;
            ws2812b.fill(ws2812b.Color(0, 0, 0, 0));
            ws2812b.show();
            IAmTheFirstSender = false;
        }
        break;
    case State::SECONDARY_COLOR:
        if (now > timeout)
        {
            state = State::OFF;
            ws2812b.fill(ws2812b.Color(0, 0, 0, 0));
            ws2812b.show();
            IAmTheFirstSender = false;
        }
        break;
    }

    ButtonMessageReceivedFlag = false;
}

void HandleButton()
{
    if (!digitalRead(PIN_BUTTON) && IAmTheFirstSender == false)
    {
        client.publish(topic, "1");
        IAmTheFirstSender = true;
    }
}