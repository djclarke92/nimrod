#include <Arduino.h>
//#include "FS.h"
//#include "esp_system.h"
#include <esp_wifi.h>
#include <string.h>
#include <WiFi.h>
#include <Preferences.h>  // WiFi storage
#include <OneWire.h>
#include <DallasTemperature.h>

#define ENABLE_DEBUG        // To enable debugging
#define ENABLE_ERROR_STRING // To show details in error
#define DEBUG_PORT Serial   // To define the serial port for debug printing
#include <ESP_SSLClient.h>
#include <WiFiClient.h>


//const char* gszSSID = NULL;  
//const char* gszPassword = NULL;
String gsPrefSSID;
String gsPrefPassword;
int giWFstatus = 255; //WL_IDLE_STATUS;
int giWiFiUpCount = 0;
int32_t giRssi = 0;  
String gsGetSsid;
String gsGetPass;
String gsMAC;
Preferences gPrefStorage;

// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);


// nimrod definitions
#define NIMROD_PORT  54011          // TODO: replace this with your own port number
#define NIMROD_IP  "192.168.0.1"    // TODO: replace this with the IP address of your nimrod server
#define PING_PERIOD   30
#define MSG_LENGTH   22
#define INPUT_1_PIN  25
#define INPUT_2_PIN  26
#define INPUT_3_PIN  27
#define INPUT_4_PIN  33
#define OUTPUT_LED_PIN   2
#define OUTPUT_1_PIN  16
#define OUTPUT_2_PIN  17
#define OUTPUT_3_PIN  18
#define OUTPUT_4_PIN  19
#define MAX_DEVICES   4
String sEspName = "ESP000"; // default to 000 - nimrod will allocate a unique number for us
int giStartup = -1;
int giButton1Changed = -1;
int giButton2Changed = -1;
int giButton3Changed = -1;
int giButton4Changed = -1;
int giLedState = 0;
int giSocketConnected = -1;
int giSecSinceLastSend = 0;
int giSecSinceLastRecv = 0;
int giSecSinceLastTemp[MAX_DEVICES] = { 0, 0, 0, 0 };
int giLedTimer = 20;
int giNumDevices = 0;
int giTempCount = 0;
int iOutput1Timer = -1;
int iOutput2Timer = -1;
int iOutput3Timer = -1;
int iOutput4Timer = -1;
int iOutput1Period = 10;  // seconds
int iOutput2Period = 30;  // seconds
int iOutput3Period = 30;  // seconds
int iOutput4Period = 30;  // seconds
int iOutput1State = LOW;
int iOutput2State = LOW;
int iOutput3State = LOW;
int iOutput4State = LOW;
int giBtn1Active = 0;
int giBtn2Active = 0;
int giBtn3Active = 0;
int giBtn4Active = 0;
unsigned long gulLastLoopMillis = 0;
unsigned long gulBtn1Time = 0;
unsigned long gulBtn2Time = 0;
unsigned long gulBtn3Time = 0;
unsigned long gulBtn4Time = 0;
float gfLastTemp[MAX_DEVICES] = { 0.0, 0.0, 0.0, 0.0 };
hw_timer_t * timer1 = NULL;
hw_timer_t * timer2 = NULL;
hw_timer_t * timer3 = NULL;
hw_timer_t * timer4 = NULL;

ESP_SSLClient ssl_client;

WiFiClient basic_client;

DeviceAddress wDeviceAddr;
// end nimrod definitions

void setup() 
{
  Serial.begin(115200);
  Serial.printf("\nNimrod ESP32 starting\n");

  // Start the DS18B20 sensor
  sensors.begin();
  
  nimrodSetup();

  wifiInit();

  // ignore server ssl certificate verification
  ssl_client.setInsecure();
  //ssl_client.allowSelfSignedCerts();

  // Set the receive and transmit buffers size in bytes for memory allocation (512 to 16384).
  ssl_client.setBufferSizes(1024 /* rx */, 512 /* tx */);

  /** Call setDebugLevel(level) to set the debug
   * esp_ssl_debug_none = 0
   * esp_ssl_debug_error = 1
   * esp_ssl_debug_warn = 2
   * esp_ssl_debug_info = 3
   * esp_ssl_debug_dump = 4
   */
  ssl_client.setDebugLevel(1);

  // In case ESP32 WiFiClient, the session timeout should be set,
  // if the TCP session was kept alive because it was unable to detect the server disconnection.
  ssl_client.setSessionTimeout(120); // Set the timeout in seconds (>=120 seconds)

  // Assign the basic client to use in non-secure mode.
  ssl_client.setClient(&basic_client, false /* set enable SSL option to false */);

} 

void loop() 
{
  if ( gulLastLoopMillis + 100 < millis() )
  {
    gulLastLoopMillis = millis();

    nimrodLoop();

    if ( giWFstatus != WiFi.status() )
    { // wifi status has changed
      giWFstatus = getWifiStatus();
    }
    
    if ( WiFi.status() != WL_CONNECTED ) 
    {  // WiFi DOWN
      if ( giWiFiUpCount == 0 )
      {
        Serial.printf("Starting Wifi...\n" );
        
        WiFi.begin(gsPrefSSID.c_str(), gsPrefPassword.c_str());
        giWiFiUpCount += 1;
      }
      else if ( giWiFiUpCount > 200 )
      { // try again
        giWiFiUpCount = 0;
        
        Serial.printf("WiFi still not connected\n");
      }
      else
      {
        giWiFiUpCount += 1;
      }
    }  
  }
}  

void wifiInit() 
{
  WiFi.mode(WIFI_AP_STA);  // required to read NVR before WiFi.begin()

  // load credentials from NVR, a little RTOS code here
//  wifi_config_t conf;
//  esp_wifi_get_config(WIFI_IF_STA, &conf);  // load wifi settings to struct comf
//  gszSSID = reinterpret_cast<const char*>(conf.sta.ssid);
//  gszPassword = reinterpret_cast<const char*>(conf.sta.password);

  // Open Preferences with "wifi" namespace. Namespace is limited to 15 chars
  gPrefStorage.begin("wifi", false);
  gsPrefSSID = gPrefStorage.getString("ssid", "none");  // NVS key ssid
  gsPrefPassword = gPrefStorage.getString("password", "none");  // NVS key password
  gPrefStorage.end();

  // keep from rewriting flash if not needed
  if (!checkPrefsStore())    
  { // NV and Prefs are different, setup with SmartConfig
    if (gsPrefSSID == "none")
    { // new wifi config
      //delay(10000);
      initSmartConfig();
      
      delay(3000);
      
      ESP.restart();  // reboot with new wifi configuration
    }
  }


  Serial.printf( "WiFi starting\n" );
  WiFi.begin(gsPrefSSID.c_str(), gsPrefPassword.c_str());
  giWiFiUpCount += 1;
}  

// match WiFi IDs in NVS to Pref store,  assumes WiFi.mode(WIFI_AP_STA);  was
// executed
bool checkPrefsStore() 
{
  bool bRet = true;
//  String NVssid, NVpass;
  String prefssid, prefpass;

//  NVssid = getSsidPass("ssid");
//  NVpass = getSsidPass("pass");

  // Open Preferences with my-app namespace. Namespace name is limited to 15
  // chars
  gPrefStorage.begin("wifi", false);
  prefssid = gPrefStorage.getString("ssid", "none");  // NVS key ssid
  prefpass = gPrefStorage.getString("password", "none");  // NVS key password
  gPrefStorage.end();

  if ( prefssid == "none" || prefssid == "" || prefpass == "none" || prefpass == "") 
  { // wifi creds are not setup
    bRet = false;
  }

  return bRet;
}

// call this function any time to force a new wifi config
void initSmartConfig() 
{
  int loopCounter = 0;

  WiFi.mode(WIFI_AP_STA);  // Init WiFi, start SmartConfig
  Serial.printf("Entering SmartConfig\n");

  WiFi.beginSmartConfig();

  while (!WiFi.smartConfigDone()) 
  {
    Serial.printf(".");
    if (loopCounter >= 30) 
    {
      loopCounter = 0;
      Serial.printf("%s\n", esp_smartconfig_get_version());
    }
    delay(500);
    ++loopCounter;
  }
  loopCounter = 0;


  Serial.printf("SmartConfig received.\nWaiting for WiFi\n\n");
  delay(2000);

  while (WiFi.status() != WL_CONNECTED)  
  { // wait forever until connected
    Serial.printf("-");
    if (loopCounter >= 30) 
    {
      loopCounter = 0;
      Serial.printf("%s\n", esp_smartconfig_get_version());
    }
    delay(500);
    ++loopCounter;
  }

  PrintIPinfo();

  // save the new wifi credentials
  gPrefStorage.begin("wifi", false);
  gPrefStorage.putString("ssid", gsGetSsid);
  gPrefStorage.putString("password", gsGetPass);
  gPrefStorage.end();

  delay(300);
} 

void PrintIPinfo() 
{
  gsGetSsid = WiFi.SSID();
  gsGetPass = WiFi.psk();
  giRssi = WiFi.RSSI(); 
  giWFstatus = getWifiStatus();
  gsMAC = WiFi.macAddress();

  Serial.printf("SSID '%s', %d dbm\n", gsGetSsid.c_str(), giRssi );
  Serial.printf("IP address: %s / %s\n", WiFi.localIP().toString().c_str(), WiFi.subnetMask().toString().c_str() );
  Serial.printf("Gateway IP: %s\n", WiFi.gatewayIP().toString().c_str() );
  Serial.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str() );
  Serial.printf("MAC: %s\n", gsMAC.c_str() );
}

int getWifiStatus() 
{
  int WiFiStatus;
  
  WiFiStatus = WiFi.status();
  Serial.printf("Status %d", WiFiStatus);
  switch (WiFiStatus) 
  {
  default:
    Serial.printf(", WiFi UNKNOWN \n");
    break;
  case WL_IDLE_STATUS:     // 0
    Serial.printf(", WiFi IDLE \n");
    break;
  case WL_NO_SSID_AVAIL:    // 1
    Serial.printf(", NO SSID AVAIL \n");
    break;
  case WL_SCAN_COMPLETED:   // 2
    Serial.printf(", WiFi SCAN_COMPLETED \n");
    break;
  case WL_CONNECTED:        // 3
    Serial.printf(", WiFi CONNECTED \n");
    break;
  case WL_CONNECT_FAILED:   // 4
    Serial.printf(", WiFi WL_CONNECT FAILED\n");
    break;
  case WL_CONNECTION_LOST:  // 5
    Serial.printf(", WiFi CONNECTION LOST\n");
    WiFi.persistent(false);  // don't write FLASH
    break;
  case WL_DISCONNECTED:     // 6
    Serial.printf(", WiFi DISCONNECTED ==\n");
    WiFi.persistent(false);  // don't write FLASH when reconnecting
    break;
  }
  
  return WiFiStatus;
}

// Returns String NONE, ssid or pass according to request
// return "NONE" if wrong key sent
String getSsidPass(String sKey) 
{
  String sVal = "NONE";  
  
  sKey.toUpperCase();
  if (sKey.compareTo("SSID") == 0) 
  {
    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf);
    sVal = String(reinterpret_cast<const char*>(conf.sta.ssid));
  }
  
  if (sKey.compareTo("PASS") == 0) 
  {
    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf);
    sVal = String(reinterpret_cast<const char*>(conf.sta.password));
  }
  
  return sVal;
}


//**********************************************************************************************************
//
//  nimrod functions
//
//**********************************************************************************************************

// callbacks for button debounce
/*void IRAM_ATTR button1_cb()
{ // delay 50msec
  if ( giBtn1Active == 0 )
  {
    timerAlarmWrite( timer1, 50000, false );
    timerAlarmEnable( timer1 );
    giBtn1Active = 1;
  }
}

void IRAM_ATTR button2_cb()
{
  if ( giBtn2Active == 0 )
  {
    timerAlarmWrite( timer2, 50000, false );
    timerAlarmEnable( timer2 );
    giBtn2Active = 1;
  }
}

void IRAM_ATTR button3_cb()
{
  if ( giBtn3Active == 0 )
  {
    timerAlarmWrite( timer3, 50000, false );
    timerAlarmEnable( timer3 );
    giBtn3Active = 1;
  }
}

void IRAM_ATTR button4_cb()
{
  if ( giBtn4Active == 0 )
  {
    timerAlarmWrite( timer4, 50000, false );
    timerAlarmEnable( timer4);
    giBtn4Active = 1;
  }
}

void IRAM_ATTR button1Changed()
{
  giButton1Changed = digitalRead(INPUT_1_PIN);
  gulBtn1Time = millis();
}

void IRAM_ATTR button2Changed()
{
  giButton2Changed = digitalRead(INPUT_2_PIN);
  gulBtn2Time = millis();
}

void IRAM_ATTR button3Changed()
{
  giButton3Changed = digitalRead(INPUT_3_PIN);
  gulBtn3Time = millis();
}

void IRAM_ATTR button4Changed()
{
  giButton4Changed = digitalRead(INPUT_4_PIN);
  gulBtn4Time = millis();
} */

void nimrodSetup()
{
  pinMode( OUTPUT_LED_PIN, OUTPUT );
  pinMode( OUTPUT_1_PIN, OUTPUT );
  pinMode( OUTPUT_2_PIN, OUTPUT );
  pinMode( OUTPUT_3_PIN, OUTPUT );
  pinMode( OUTPUT_4_PIN, OUTPUT );

  pinMode( INPUT_1_PIN, INPUT_PULLUP );
  pinMode( INPUT_2_PIN, INPUT_PULLUP );
  pinMode( INPUT_3_PIN, INPUT_PULLUP );
  pinMode( INPUT_4_PIN, INPUT_PULLUP );

  //attachInterrupt( INPUT_1_PIN, button1_cb, FALLING );
  //attachInterrupt( INPUT_2_PIN, button2_cb, FALLING );
  //attachInterrupt( INPUT_3_PIN, button3_cb, FALLING );
  //attachInterrupt( INPUT_4_PIN, button4_cb, FALLING );

  digitalWrite( OUTPUT_LED_PIN, HIGH );
  digitalWrite( OUTPUT_1_PIN, iOutput1State );
  digitalWrite( OUTPUT_2_PIN, iOutput2State );
  digitalWrite( OUTPUT_3_PIN, iOutput3State );
  digitalWrite( OUTPUT_4_PIN, iOutput4State ); 

  // 80 prescale = 1 microsecond timer 
  //timer1 = timerBegin(0, 80, true);
  //timer2 = timerBegin(1, 80, true);
  //timer3 = timerBegin(2, 80, true);
  //timer4 = timerBegin(3, 80, true);

  //timerAttachInterrupt( timer1, &button1Changed, false );
  //timerAttachInterrupt( timer2, &button2Changed, false );
  //timerAttachInterrupt( timer3, &button3Changed, false );
  //timerAttachInterrupt( timer4, &button4Changed, false );

  // print the device addresses
  while ( sensors.getAddress( wDeviceAddr, giNumDevices) )
  { 
    giNumDevices += 1;
    Serial.printf( "Found device with address %02x%02x%02x%02x%02x%02x%02x%02x\n",
          wDeviceAddr[0], wDeviceAddr[1], wDeviceAddr[2], wDeviceAddr[3], 
          wDeviceAddr[4], wDeviceAddr[5], wDeviceAddr[6], wDeviceAddr[7] );
  }
  Serial.printf( "NumDevices = %d\n", giNumDevices );
  if ( giNumDevices > MAX_DEVICES )
  {
    Serial.printf( "Too many Devices = %d (was %d)\n", MAX_DEVICES, giNumDevices );    
    giNumDevices = MAX_DEVICES;
  }
}


// called form the main loop() function
void nimrodLoop()
{
  unsigned long ulTimeNow = millis();
  
  if ( ssl_client.connected() )
  { // socket is connected
    if ( ssl_client.available() == 8 )
    {
      giSecSinceLastRecv = 0;
      Serial.printf( "msg is available, %d bytes\n", ssl_client.available() );

      int i;
      char szMsg[MSG_LENGTH+1];
      for ( i = 0; i < 8; i++ )
      {
        szMsg[i] = ssl_client.read();
      }
      szMsg[i] = '\0';
      
      Serial.printf( "received '%s'\n", szMsg );

      HandleMessage( szMsg );
    }
  }


  // flash the led
  if ( giSocketConnected <= 0 )
  { // wifi is disconnected
    if ( giLedState == 0 )
      digitalWrite( OUTPUT_LED_PIN, HIGH );
    else
      digitalWrite( OUTPUT_LED_PIN, LOW );
  }
  else
  { // wifi is connected
    if ( giLedTimer == 0 )
    { // stop blinking the led after 30 seconds
      digitalWrite( OUTPUT_LED_PIN, LOW ) ; 
    }
    else if ( giLedState == 0 or giLedState == 2 )
    {
      digitalWrite( OUTPUT_LED_PIN, HIGH );
    }
    else
    {
      digitalWrite( OUTPUT_LED_PIN, LOW );
    }
  }

  if ( giButton1Changed == 0 )
  {
    buttonChanged(INPUT_1_PIN, giButton1Changed );
    giButton1Changed = -1;
  }
  else if ( gulBtn1Time + 150 < ulTimeNow )
  {
    giBtn1Active = 0;
  }
  if ( giButton2Changed == 0 )
  {
    buttonChanged(INPUT_2_PIN, giButton2Changed);
    giButton2Changed = -1;
  }
  else if ( gulBtn2Time + 150 < ulTimeNow )
  {
    giBtn2Active = 0;
  }
  if ( giButton3Changed == 0 )
  {
    buttonChanged(INPUT_3_PIN, giButton3Changed);
    giButton3Changed = -1;
  }
  else if ( gulBtn3Time + 150 < ulTimeNow )
  {
    giBtn3Active = 0;
  }
  if ( giButton4Changed == 0 )
  {
    buttonChanged(INPUT_4_PIN, giButton4Changed);
    giButton4Changed = -1;
  }
  else if ( gulBtn4Time + 150 < ulTimeNow )
  {
    giBtn4Active = 0;
  }
  
  SetStartupOutputs();

  giLedState += 1;
  if ( giLedState >= 10 )
  { // every second
    giLedState = 0;
    giSecSinceLastSend += 1;
    giSecSinceLastRecv += 1;
    for ( int i = 0; i < MAX_DEVICES; i++ )
    {
      giSecSinceLastTemp[i] += 1;
    }

    // check wifi
    if ( giSocketConnected == 1 )
    {
      giTempCount += 1;
      if ( giTempCount >= 5 )
      {
        giTempCount = 0;
        ReadTemperatures();
      }
    
      if ( WiFi.status() != WL_CONNECTED )
      {
        giSocketConnected = -1;
        giLedTimer = 20;
      }
      else if ( giLedTimer > 0 )
      {
        giLedTimer -= 1;
      }
    }
    else if ( giWFstatus == WL_CONNECTED )
    { // try to connect our socket
      if ( giStartup > 0 )
      {
        giStartup = 0;
        SetStartupOutputs();
      }
      
      PrintIPinfo();
      
      Serial.printf( "Try to connect the socket\n" );
      if (!ssl_client.connect(NIMROD_IP, NIMROD_PORT)) 
      {
        Serial.printf("Connection to %s:%d failed\n", NIMROD_IP, NIMROD_PORT );
      }
      else
      {
        giSocketConnected = 1;
      
        Serial.printf( "Connected to %s:%d\n", NIMROD_IP, NIMROD_PORT );
  
        Serial.print("Upgrade to HTTPS...");
        if (!ssl_client.connectSSL())
        {
            Serial.println(" failed\r\n");
            ssl_client.stop();
            giSocketConnected = -1;
        }
        else
        {
          char szMsg[MSG_LENGTH+1];
          // 01234567890123456789
          // ESP001CLK1
          // ESPxxxCIDyyyyyyyy
          sprintf( szMsg, "%sCID%c%c%c%c%c%c%c%c%c%c%c%c", sEspName, gsMAC[0], gsMAC[1], gsMAC[3], gsMAC[4], gsMAC[6], gsMAC[7],
              gsMAC[9], gsMAC[10], gsMAC[12], gsMAC[13], gsMAC[15], gsMAC[16] );
          socket_send( szMsg );
        }
      }      
    }

    
    if ( giSocketConnected == 1 )
    {
      if ( giSecSinceLastRecv >= PING_PERIOD+5 )  
      { // socket is dead
        giSecSinceLastRecv = 0;
        
        Serial.printf( "socket is dead\n" );
        ssl_client.stop();
        giSocketConnected = -1;
      }
      else if ( giSecSinceLastSend >= PING_PERIOD )
      { // send a ping
        char szMsg[MSG_LENGTH+1];
        sprintf( szMsg, "%sPG", sEspName );
        socket_send( szMsg );
      }

    }
    
    
    // check output timers
    if ( iOutput1Timer > 0 )
    { // timer is active
      iOutput1Timer = iOutput1Timer - 1;
      
      if ( iOutput1Timer == 0 )
      { // expired, turn off output
        Serial.printf( "output 1 now off\n" );
        iOutput1Timer = -1;
        iOutput1State = LOW;
        digitalWrite( OUTPUT_1_PIN, iOutput1State );
      }
    }
    
    if ( iOutput2Timer > 0 )
    { // timer is active
      iOutput2Timer = iOutput2Timer - 1;
      
      if ( iOutput2Timer == 0 )
      { // expired, turn off output
        Serial.printf( "output 2 now off\n" );
        iOutput2Timer = -1;
        iOutput2State = LOW;
        digitalWrite( OUTPUT_2_PIN, iOutput2State );
      }
    }
    
    if ( iOutput3Timer > 0 )
    { // timer is active
      iOutput3Timer = iOutput3Timer - 1;
      
      if ( iOutput3Timer == 0 )
      { // expired, turn off output
        Serial.printf( "output 3 now off\n" );
        iOutput3Timer = -1;
        iOutput3State = LOW;
        digitalWrite( OUTPUT_3_PIN, iOutput3State );
      }
    }
    
    if ( iOutput4Timer > 0 )
    { // timer is active
      iOutput4Timer = iOutput4Timer - 1;
      
      if ( iOutput4Timer == 0 )
      { // expired, turn off output
        Serial.printf( "output 4 now off\n" );
        iOutput4Timer = -1;
        iOutput4State = LOW;
        digitalWrite( OUTPUT_4_PIN, iOutput4State );
      }
    }
    
  }
      
}

// read temperature every 5 seconds
void ReadTemperatures()
{
  if ( giNumDevices > 0 )
  {
    sensors.requestTemperatures(); 

    for ( int i = 0; i < giNumDevices; i++ )
    {
      float fTempC = sensors.getTempCByIndex(i);
      if ( fabs(gfLastTemp[i] - fTempC) >= 0.5 || giSecSinceLastTemp[i] >= 60 )
      { // send our temperature
        char szMsg[MSG_LENGTH+1];
        // 01234567890123456789
        // ESP001CLK1
        // ESP001TMPn:xx.x
        // ESPxxxCIDyyyyyyyy
        sprintf( szMsg, "%sTMP%d:%.1f", sEspName, i+1, fTempC );
        socket_send( szMsg );      

        giSecSinceLastTemp[i] = 0;
      }
      gfLastTemp[i] = fTempC;
    }
  }
}

void SetStartupOutputs()
{
  if ( giStartup == 0 )
  {
    giStartup = -2;
    digitalWrite( OUTPUT_1_PIN, LOW );  
    digitalWrite( OUTPUT_2_PIN, LOW );  
    digitalWrite( OUTPUT_3_PIN, LOW );  
    digitalWrite( OUTPUT_4_PIN, LOW );  

  }
  else
  {
    switch ( giStartup )
    {
    default:
      break;
    case -1:
    case 1:
      digitalWrite( OUTPUT_1_PIN, HIGH );
      digitalWrite( OUTPUT_4_PIN, LOW );
      giStartup = 2;
      break;
    case 2:
      digitalWrite( OUTPUT_2_PIN, HIGH );
      digitalWrite( OUTPUT_1_PIN, LOW );
      giStartup = 3;
      break;
    case 3:
      digitalWrite( OUTPUT_3_PIN, HIGH );
      digitalWrite( OUTPUT_2_PIN, LOW );
      giStartup = 4;
      break;
    case 4:
      digitalWrite( OUTPUT_4_PIN, HIGH );
      digitalWrite( OUTPUT_3_PIN, LOW );
      giStartup = 1;
      break;
    }
  }
}

void socket_send( char* msg )
{
  while ( strlen(msg) < MSG_LENGTH )
  {
    strcat( msg, "." );
  }

  Serial.printf( "skt send: %s\n", msg );

  ssl_client.print( msg );
  
  giSecSinceLastSend = 0;
}

void buttonChanged( int pin, int val )
{
  char szMsg[MSG_LENGTH+1];
  
  
  giLedTimer = 20;
  Serial.printf("Pin %d val now %d\n", pin, val );
  
  if ( val != 0 )
  { // do nothing
  }
  else if ( pin == INPUT_1_PIN )
  { // button 1 is pressed
    if  ( giSocketConnected > 0 )
    {
      Serial.printf( "send CLK1\n" );
      sprintf( szMsg, "%sCLK1", sEspName );
      socket_send( szMsg );
    }
    else
    {
      if ( iOutput1State == LOW )
        activate_output2( 1, iOutput1Period );
      else
        activate_output2( 1, 0 );
    }
  }    
  else if ( pin == INPUT_2_PIN )
  { // button 2 is pressed
    if ( giSocketConnected > 0 )
    {
      Serial.printf( "send CLK2\n" );
      sprintf( szMsg, "%sCLK2", sEspName );
      socket_send( szMsg );
    }
    else
    {
      if ( iOutput2State == LOW )
        activate_output2( 2, iOutput2Period );
      else
        activate_output2( 2, 0 );
    }
  }
  else if ( pin == INPUT_3_PIN )
  { // button 3 is pressed
    if ( giSocketConnected > 0 )
    {
      Serial.printf( "send CLK3\n" );
      sprintf( szMsg, "%sCLK3", sEspName );
      socket_send( szMsg );
    }
    else
    {
      if ( iOutput3State == LOW )
        activate_output2( 3, iOutput3Period );
      else
        activate_output2( 3, 0 );
    }
  }
  else if ( pin == INPUT_4_PIN )
  { //  button 4 is pressed
    if ( giSocketConnected > 0 )
    {
      Serial.printf( "send CLK4\n" );
      sprintf( szMsg, "%sCLK4", sEspName );
      socket_send( szMsg );
    }
    else
    {
      if ( iOutput4State == LOW )
        activate_output2( 4, iOutput4Period );
      else
        activate_output2( 4, 0 );
    }
  }
}

void HandleMessage( const char* c )
{
  // 0123456789012
  // OKyxxxxx   output control
  // PG000000   ping from nimrod
  // NNESPxxx   our new name
  if ( strncmp(c,"OK",2) == 0 )
  {
    int iOutput = (int)(c[2] - '0');
    int iPeriod = atoi(&c[3]);

    if ( iOutput != 0 )
    {
      activate_output2( iOutput, iPeriod );
    }
  } 
  else if ( strncmp(c,"NN",2) == 0 )
  {
    sEspName = &c[2];
    
    Serial.printf( "New name %s\n", sEspName );
  } 
}

void activate_output2( int iOutput, int iPeriod )
{
  Serial.printf( "Output %d on for %d sec\n", iOutput, iPeriod );
    
  if ( iOutput == 1 )
  {      
    if ( iPeriod == 0 )
    { // turn off
      iOutput1Timer = -1;
      iOutput1State = LOW;
    }   
    else
    {
      iOutput1Period = iPeriod;
      iOutput1State = HIGH;
        
      if ( iOutput1Timer < 0 )
        iOutput1Timer = iOutput1Period;
      else
        iOutput1Timer = iOutput1Timer + iOutput1Period;
    }

    digitalWrite( OUTPUT_1_PIN, iOutput1State );
    Serial.printf( "tmr 1 set to %d\n", iOutput1Timer );
  }    
  else if ( iOutput == 2 )
  {
    if ( iPeriod == 0 )
    { // turn off
      iOutput2Timer = -1;
      iOutput2State = LOW;
    }
    else
    {
      iOutput2Period = iPeriod;
      iOutput2State = HIGH;
        
      if ( iOutput2Timer < 0 )
        iOutput2Timer = iOutput2Period;
      else
        iOutput2Timer = iOutput2Timer + iOutput2Period;
    }

    digitalWrite( OUTPUT_2_PIN, iOutput2State );
    Serial.printf( "tmr 2 set to %d\n", iOutput2Timer );
  }
  else if ( iOutput == 3 )
  {
    if ( iPeriod == 0 )
    { // turn off
      iOutput3Timer = -1;
      iOutput3State = LOW;
    }
    else
    {
      iOutput3Period = iPeriod;
      iOutput3State = HIGH;
        
      if ( iOutput3Timer < 0 )
        iOutput3Timer = iOutput3Period;
      else
        iOutput3Timer = iOutput3Timer + iOutput3Period;
    }

    digitalWrite( OUTPUT_3_PIN, iOutput3State );
    Serial.printf( "tmr 3 set to %d\n", iOutput3Timer );
  }
  else if ( iOutput == 4 )
  {
    if ( iPeriod == 0 )
    { // turn off
      iOutput4Timer = -1;
      iOutput4State = LOW;
    }
    else
    {
      iOutput4Period = iPeriod;
      iOutput4State = HIGH;
        
      if ( iOutput4Timer < 0 )
        iOutput4Timer = iOutput4Period;
      else
        iOutput4Timer = iOutput4Timer + iOutput4Period;
    }

    digitalWrite( OUTPUT_4_PIN, iOutput4State );
    Serial.printf( "tmr 4 set to %d\n", iOutput4Timer );
      
  }
}
