/*
  Lighbarrier & telegram
  by Andreas Loeffler
*/

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#define INPUT_GPIO  32

const char* SSID = "mySSID";
const char* PASSWORD = "myPASSWORD";

  // Initialize Telegram BOT
#define BOTTOKEN "myTELEGRAM_BOT_TOKEN"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group (/getid)
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "myCHATID"

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
NTPClient timeClient(ntpUDP, 2*3600);

WiFiClientSecure client;
UniversalTelegramBot bot(BOTTOKEN, client);

char c_buf[1024];

bool inputState = false;

// initialize wifi connection
void initWiFi() 
{
  Serial.print("ESP Board MAC Address:  "); Serial.println(WiFi.macAddress());
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print('.');
    delay(500);
  }
  Serial.print("IP: "); Serial.println(WiFi.localIP());
}

// interrupt service routine function
void IRAM_ATTR Ext_INT1_ISR()
{
  inputState = true;
}

// the setup function runs once when you press reset or power the board
void setup() 
{
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  initWiFi();
  client.setInsecure();
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());

  // start ntp client
  Serial.println("Starting NTP client");
  timeClient.begin();
  while(!timeClient.update())
  {
    Serial.println("NTP client not working yet");    
    delay(1000);
  }
  Serial.print("NTP client successfully started at "); Serial.println(timeClient.getFormattedTime());

  // init telegram bot
  sprintf(c_buf, "ESP32 is starting up at %s", timeClient.getFormattedTime());
  while(!bot.sendMessage(CHAT_ID, c_buf, ""))
  {
    Serial.println("Telegram not working yet");    
    delay(1000);
  }
  Serial.println("Telegram successfully started!");

  //setup GPIO
  pinMode(INPUT_GPIO, INPUT_PULLDOWN);
  attachInterrupt(INPUT_GPIO, Ext_INT1_ISR, RISING);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(250);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(250);                       // wait for a second
  Serial.print("RRSI: "); Serial.println(WiFi.RSSI());
  timeClient.update();
  Serial.print("Time = "); Serial.println(timeClient.getFormattedTime());
  Serial.print("Input = "); Serial.println(inputState);
  if (inputState)
  {
    sprintf(c_buf, "FOUND an EVENT at %s", timeClient.getFormattedTime());
    Serial.println(c_buf);
    bot.sendMessage(CHAT_ID, c_buf, "");
    inputState = false;
  }
  Serial.println("------------------------------------------");
}
