#include "FS.h"
#include "esp_system.h"
#include <esp_wifi.h>
#include <string.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>  // WiFi storage


const char* gszNimrodRootCA= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIECTCCAvGgAwIBAgIULPJ1Z9nziVj6H0pcv4Sc981YAo8wDQYJKoZIhvcNAQEL\n" \
"BQAwgZMxCzAJBgNVBAYTAk5aMQ8wDQYDVQQIDAZCdWxsZXIxEzARBgNVBAcMCkNo\n" \
"YXJsZXN0b24xEjAQBgNVBAoMCUZsYXRDYXRJVDELMAkGA1UECwwCQ0ExJzAlBgkq\n" \
"hkiG9w0BCQEWGGRqY2xhcmtlQGZsYXRjYXRpdC5jby5uejEUMBIGA1UEAwwLMTky\n" \
"LjE2OC4wLjEwHhcNMjEwNjA0MjM0NTAxWhcNMzEwNjAyMjM0NTAxWjCBkzELMAkG\n" \
"A1UEBhMCTloxDzANBgNVBAgMBkJ1bGxlcjETMBEGA1UEBwwKQ2hhcmxlc3RvbjES\n" \
"MBAGA1UECgwJRmxhdENhdElUMQswCQYDVQQLDAJDQTEnMCUGCSqGSIb3DQEJARYY\n" \
"ZGpjbGFya2VAZmxhdGNhdGl0LmNvLm56MRQwEgYDVQQDDAsxOTIuMTY4LjAuMTCC\n" \
"ASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANlhhh+IgP73c5R7azrT12yg\n" \
"GIWnSKRUYG5vzePiRQ46LFgkOoP1OBYxD3bhSeppj9BA09CFKVGeuglpD8flmIGI\n" \
"uGNZCrIHCkZ7zvYmkJlgJclVI5SkE7eUNBMhRmI0ccITLvKiC5smDh9u31XvDXX2\n" \
"RJYHrtIa1KszYIwKTfFTiG0Bpy9AaHE6XSP/BwnjzWED9OCixAVlepaNf2tImiFo\n" \
"/trlHFv8qY0J9yU/L57RjX4jqeRdM/WWSYGwCMNdpctAWfChojKEHssIibiYnQFh\n" \
"JYJsA2F8ZZl5nziFR55D6VJT0N7GH9pCPDwJmWSyaNMpaWIyiHXz661M6d+FuFUC\n" \
"AwEAAaNTMFEwHQYDVR0OBBYEFI+Noc2V3uJkkr3peq3PWHz7v6rzMB8GA1UdIwQY\n" \
"MBaAFI+Noc2V3uJkkr3peq3PWHz7v6rzMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZI\n" \
"hvcNAQELBQADggEBAI+o7Ahr/GF0Kr81DoovEO0hl4xmoqRcFGamyrssfk5M2Rcy\n" \
"to/4tj5b2QR7p8A9iCRBKMCxQJsQAuYN77u3ZCxW4ARmxFdOEgWm8oftxkur7yy2\n" \
"20QYlmodo/AFraHqx5u8DtW3W4xxbvhwEVmHDLcwJxr55vXejrQ9LD7q8voBVVVx\n" \
"whmiQ4VoRagBDOPco9C8GHc7Hgqh0o89NhCznOEXY5YRV6qzLKXvqZE+LGcQ+5zh\n" \
"cdPOWNTgtQvAe+HhX+IimmfoM9jkjcY/llz/yPvR2BimbayFZqkXwBEfRUhtY38w\n" \
"zMwM0EuD+iasPmY0N0bgwgk4IqbYweDq+hfksDQ=\n" \
"-----END CERTIFICATE-----\n";

const char* gszNimrodCertKey = "";
const char* gszNimrodCert = "";

     
const char* gszSSID = NULL;  
const char* gszPassword = NULL;
String gsPrefSSID;
String gsPrefPassword;
int giWFstatus = 255; //WL_IDLE_STATUS;
int giWiFiUpCount = 0;
int32_t giRssi = 0;  
String gsGetSsid;
String gsGetPass;
String gsMAC;
Preferences gPrefStorage;


// nimrod definitions
#define NIMROD_PORT  54011
#define NIMROD_IP  "192.168.0.1"   // connect to the nimrod host
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
#define OUTPUT_ON     1
#define OUTPUT_OFF    0
String sEspName = "ESP000"; // default to 000 - nimrod will allocate a unique number for us
int iLedState = 0;
int iSocketConnected = -1;
int iSecSinceLastSend = 0;
int iSecSinceLastRecv = 0;
int iLedTimer = 20;
int iOutput1Timer = -1;
int iOutput2Timer = -1;
int iOutput3Timer = -1;
int iOutput4Timer = -1;
int iOutput1Period = 10;  // seconds
int iOutput2Period = 30;  // seconds
int iOutput3Period = 30;  // seconds
int iOutput4Period = 30;  // seconds
int iOutput1State = OUTPUT_OFF;
int iOutput2State = OUTPUT_OFF;
int iOutput3State = OUTPUT_OFF;
int iOutput4State = OUTPUT_OFF;
int iBtn1Count = 0;
int iBtn2Count = 0;
int iBtn3Count = 0;
int iBtn4Count = 0;
int iBtn1LastCount = 0;
int iBtn2LastCount = 0;
int iBtn3LastCount = 0;
int iBtn4LastCount = 0;
hw_timer_t * timer1 = NULL;
hw_timer_t * timer2 = NULL;
hw_timer_t * timer3 = NULL;
hw_timer_t * timer4 = NULL;
WiFiClientSecure client;
// end nimrod definitions

void setup() 
{
  Serial.begin(115200);
  Serial.printf("\nNimrod ESP32 starting\n");

  nimrodSetup();

} 

void loop() 
{
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
      ++giWiFiUpCount;
    }
    else if ( giWiFiUpCount > 200 )
    { // try again
      giWiFiUpCount = 0;
      
      Serial.printf("WiFi still not connected\n");
    }
  }  

  delay(100);
}  

void wifiInit() 
{
  WiFi.mode(WIFI_AP_STA);  // required to read NVR before WiFi.begin()

  // load credentials from NVR, a little RTOS code here
  wifi_config_t conf;
  esp_wifi_get_config(WIFI_IF_STA, &conf);  // load wifi settings to struct comf
  gszSSID = reinterpret_cast<const char*>(conf.sta.ssid);
  gszPassword = reinterpret_cast<const char*>(conf.sta.password);

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
      initSmartConfig();
      
      delay(3000);
      
      ESP.restart();  // reboot with new wifi configuration
    }
  }


  WiFi.begin(gsPrefSSID.c_str(), gsPrefPassword.c_str());

  int WLcount = 0;
  while (WiFi.status() != WL_CONNECTED && WLcount < 200)  
  { // can take a couple of seconds
    delay(100);
    Serial.printf(".");
    ++WLcount;
    if ( (WLcount % 60) == 0 )  
    { // keep from scrolling sideways forever
      Serial.printf("\n");
    }
  }
  
  delay(3000);
}  

// match WiFi IDs in NVS to Pref store,  assumes WiFi.mode(WIFI_AP_STA);  was
// executed
bool checkPrefsStore() 
{
  bool bRet = false;
  String NVssid, NVpass, prefssid, prefpass;

  NVssid = getSsidPass("ssid");
  NVpass = getSsidPass("pass");

  // Open Preferences with my-app namespace. Namespace name is limited to 15
  // chars
  gPrefStorage.begin("wifi", false);
  prefssid = gPrefStorage.getString("ssid", "none");  // NVS key ssid
  prefpass = gPrefStorage.getString("password", "none");  // NVS key password
  gPrefStorage.end();

  if (NVssid.equals(prefssid) && NVpass.equals(prefpass)) 
  {
    bRet = true;
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
      Serial.printf("\n");
    }
    delay(500);
    ++loopCounter;
  }
  loopCounter = 0;


  Serial.printf("SmartConfig received.\nWaiting for WiFi\n\n");
  delay(2000);

  while (WiFi.status() != WL_CONNECTED)  
  { // wait forever until connected
    delay(500);
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
  giRssi = getRSSI(gszSSID);
  giWFstatus = getWifiStatus();
  gsMAC = getMacAddress();

  Serial.printf("SSID '%s', %d dbm\n", gsGetSsid.c_str(), giRssi );
  Serial.printf("IP address: %s / %s\n", WiFi.localIP(), WiFi.subnetMask() );
  Serial.printf("Gateway IP: %s\n", WiFi.gatewayIP() );
  Serial.printf("DNS: %s\n", WiFi.dnsIP() );
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


// Get the station interface MAC address.
// @return String MAC
String getMacAddress(void) 
{
  uint8_t baseMac[6];
  char macStr[18] = {0};

  WiFi.mode(WIFI_AP_STA);  // required to read NVR before WiFi.begin()
  
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);  // Get MAC address for WiFi station
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1],
          baseMac[2], baseMac[3], baseMac[4], baseMac[5]);

  return String(macStr);
}

// Return RSSI or 0 if target SSID not found
int32_t getRSSI(const char* target_ssid) 
{
  byte available_networks = WiFi.scanNetworks();

  for (int network = 0; network < available_networks; network++) 
  {
    if (strcmp(WiFi.SSID(network).c_str(), target_ssid) == 0) 
    {
      return WiFi.RSSI(network);
    }
  }
  
  return 0;
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
void button1_cb()
{
  timerAlarmWrite( timer1, 50000, false );
  timerAlarmEnable( timer1 );
  iBtn1Count += 1;
}

void button2_cb()
{
//  timerAlarmWrite( timer2, 50000, false );
//  timerAlarmEnable( timer2 );
  iBtn2Count += 1;
}

void button3_cb()
{
//  timerAlarmWrite( timer3, 50000, false );
//  timerAlarmEnable( timer3 );
  iBtn3Count += 1;
}

void button4_cb()
{
//  timerAlarmWrite( timer4, 50000, false );
//  timerAlarmEnable( timer4);
  iBtn4Count += 1;
}

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

  attachInterrupt( INPUT_1_PIN, button1_cb, FALLING );
  attachInterrupt( INPUT_2_PIN, button2_cb, FALLING );
  attachInterrupt( INPUT_3_PIN, button3_cb, FALLING );
  attachInterrupt( INPUT_4_PIN, button4_cb, FALLING );

  digitalWrite( OUTPUT_LED_PIN, OUTPUT_ON );
  digitalWrite( OUTPUT_1_PIN, iOutput1State );
  digitalWrite( OUTPUT_2_PIN, iOutput2State );
  digitalWrite( OUTPUT_3_PIN, iOutput3State );
  digitalWrite( OUTPUT_4_PIN, iOutput4State ); 


  timer1 = timerBegin(0, 80, true);
  timer2 = timerBegin(1, 80, true);
  timer3 = timerBegin(2, 80, true);
  timer4 = timerBegin(3, 80, true);

  timerAttachInterrupt( timer1, &button1Changed, false );
  //timerAttachInterrupt( timer2, &button2Changed, false );
  //timerAttachInterrupt( timer3, &button3Changed, false );
  //timerAttachInterrupt( timer4, &button4Changed, false );

  client.setCACert(gszNimrodRootCA);
//  client.setCertificate(gszNimrodCertKey); // for client verification
//  client.setPrivateKey(gszNimrodCert);  // for client verification
}


// called form the main loop() function
void nimrodLoop()
{
  if ( client.connected() )
  { // socket is connected
    if ( client.available() == 8 )
    {
      iSecSinceLastRecv = 0;
      Serial.printf( "msg is available, %d bytes\n", client.available() );

      int i;
      char szMsg[MSG_LENGTH+1];
      for ( i = 0; i < 8; i++ )
      {
        szMsg[i] = client.read();
      }
      szMsg[i] = '\0';
      
      Serial.printf( "received '%s'\n", szMsg );

      HandleMessage( szMsg );
    }
  }


  // flash the led
  if ( iSocketConnected <= 0 )
  { // wifi is disconnected
    if ( iLedState == 0 )
      digitalWrite( OUTPUT_LED_PIN, OUTPUT_ON );
    else
      digitalWrite( OUTPUT_LED_PIN, OUTPUT_OFF );
  }
  else
  { // wifi is connected
    if ( iLedTimer == 0 )
    { // stop blinking the led after 30 seconds
      digitalWrite( OUTPUT_LED_PIN, OUTPUT_OFF ) ; 
    }
    else if ( iLedState == 0 or iLedState == 2 )
    {
      digitalWrite( OUTPUT_LED_PIN, OUTPUT_ON );
    }
    else
    {
      digitalWrite( OUTPUT_LED_PIN, OUTPUT_OFF );
    }
  }
  
  iLedState = iLedState + 1;
  if ( iLedState >= 10 )
  { // every second
    iLedState = 0;
    iSecSinceLastSend = iSecSinceLastSend + 1;
    iSecSinceLastRecv = iSecSinceLastRecv + 1;

    //Serial.printf("n\n" );
    
    // check wifi
    if ( iSocketConnected == 1 )
    {
      if ( WiFi.status() != WL_CONNECTED )
      {
        iSocketConnected = -1;
        iLedTimer = 20;
      }
      else if ( iLedTimer > 0 )
      {
        iLedTimer = iLedTimer - 1;
      }
    }
    else if ( giWFstatus == WL_CONNECTED )
    { // try to connect our socket
      Serial.printf( "Try to connect the socket\n" );
      if (!client.connect(NIMROD_IP, NIMROD_PORT)) 
      {
        Serial.printf("Connection to %s:%d failed\n", NIMROD_IP, NIMROD_PORT );
      }
      else
      {
        iSocketConnected = 1;
      
        Serial.printf( "Connected to %s:%d\n", NIMROD_IP, NIMROD_PORT );
  
        char szMsg[MSG_LENGTH+1];
        // 01234567890123456789
        // ESP001CLK1
        // ESPxxxCIDyyyyyyyy
        sprintf( szMsg, "%sCID%02x%02x%02x%02x%02x%02x", sEspName, gsMAC[0], gsMAC[1], gsMAC[2], gsMAC[3], gsMAC[4], gsMAC[5] );
        socket_send( szMsg );
      }      
    }

    
    if ( iSocketConnected == 1 )
    {
      if ( iSecSinceLastRecv >= PING_PERIOD+5 )  
      { // socket is dead
        iSecSinceLastRecv = 0;
        
        Serial.printf( "socket is dead\n" );
        client.stop();
        iSocketConnected = -1;
      }
      else if ( iSecSinceLastSend >= PING_PERIOD )
      { // send a ping
        char szMsg[MSG_LENGTH+1];
        sprintf( szMsg, "%sPG", sEspName );
        socket_send( szMsg );
      }

      if ( iBtn1Count != iBtn1LastCount )
      {
        iBtn1LastCount = iBtn1Count;
        Serial.printf( "Btn1Count %d\n", iBtn1Count );
      }
      if ( iBtn2Count != iBtn2LastCount )
      {
        iBtn2LastCount = iBtn2Count;
        Serial.printf( "Btn2Count %d\n", iBtn2Count );
      }
      if ( iBtn3Count != iBtn3LastCount )
      {
        iBtn3LastCount = iBtn3Count;
        Serial.printf( "Btn3Count %d\n", iBtn3Count );
      }
      if ( iBtn4Count != iBtn4LastCount )
      {
        iBtn4LastCount = iBtn4Count;
        Serial.printf( "Btn4Count %d\n", iBtn4Count );
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
        iOutput1State = OUTPUT_OFF;
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
        iOutput2State = OUTPUT_OFF;
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
        iOutput3State = OUTPUT_OFF;
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
        iOutput4State = OUTPUT_OFF;
        digitalWrite( OUTPUT_4_PIN, iOutput4State );
      }
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

  client.print( msg );
  
  iSecSinceLastSend = 0;
}

void button1Changed()
{
  buttonChanged(INPUT_1_PIN);
}

void button2Changed()
{
  buttonChanged(INPUT_2_PIN);
}

void button3Changed()
{
  buttonChanged(INPUT_3_PIN);
}

void button4Changed()
{
  buttonChanged(INPUT_4_PIN);
}

void buttonChanged( int pin )
{
  char szMsg[MSG_LENGTH+1];
  
  int val = digitalRead(pin);
  
  iLedTimer = 20;
  Serial.printf("Pin %d val now %d\n", pin, val );
  
  if ( val == 0 )
  { // do nothing
  }
  else if ( pin == INPUT_1_PIN )
  { // button 1 is pressed
    if  ( iSocketConnected > 0 )
    {
      Serial.printf( "send CLK1\n" );
      sprintf( szMsg, "%sCLK1", sEspName );
      socket_send( szMsg );
    }
    else
    {
      if ( iOutput1State == OUTPUT_OFF )
        activate_output2( 1, iOutput1Period );
      else
        activate_output2( 1, 0 );
    }
  }    
  else if ( pin == INPUT_2_PIN )
  { // button 2 is pressed
    if ( iSocketConnected > 0 )
    {
      Serial.printf( "send CLK2\n" );
      sprintf( szMsg, "%sCLK2", sEspName );
      socket_send( szMsg );
    }
    else
    {
      if ( iOutput2State == OUTPUT_OFF )
        activate_output2( 2, iOutput2Period );
      else
        activate_output2( 2, 0 );
    }
  }
  else if ( pin == INPUT_3_PIN )
  { // button 3 is pressed
    if ( iSocketConnected > 0 )
    {
      Serial.printf( "send CLK3\n" );
      sprintf( szMsg, "%sCLK3", sEspName );
      socket_send( szMsg );
    }
    else
    {
      if ( iOutput3State == OUTPUT_OFF )
        activate_output2( 3, iOutput3Period );
      else
        activate_output2( 3, 0 );
    }
  }
  else if ( pin == INPUT_4_PIN )
  { //  button 4 is pressed
    if ( iSocketConnected > 0 )
    {
      Serial.printf( "send CLK4\n" );
      sprintf( szMsg, "%sCLK4", sEspName );
      socket_send( szMsg );
    }
    else
    {
      if ( iOutput4State == OUTPUT_OFF )
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
      iOutput1State = OUTPUT_OFF;
    }   
    else
    {
      iOutput1Period = iPeriod;
      iOutput1State = OUTPUT_ON;
        
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
      iOutput2State = OUTPUT_OFF;
    }
    else
    {
      iOutput2Period = iPeriod;
      iOutput2State = OUTPUT_ON;
        
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
      iOutput3State = OUTPUT_OFF;
    }
    else
    {
      iOutput3Period = iPeriod;
      iOutput3State = OUTPUT_ON;
        
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
      iOutput4State = OUTPUT_OFF;
    }
    else
    {
      iOutput4Period = iPeriod;
      iOutput4State = OUTPUT_ON;
        
      if ( iOutput4Timer < 0 )
        iOutput4Timer = iOutput4Period;
      else
        iOutput4Timer = iOutput4Timer + iOutput4Period;
    }

    digitalWrite( OUTPUT_4_PIN, iOutput4State );
    Serial.printf( "tmr 4 set to %d\n", iOutput4Timer );
      
  }
}
