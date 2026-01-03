#include <Arduino.h>
//#include "FS.h"
//#include "esp_system.h"
#include <esp_wifi.h>
#include <string.h>
#include <WiFi.h>
//#include <WiFiClientSecure.h>

#define ENABLE_DEBUG        // To enable debugging
#define ENABLE_ERROR_STRING // To show details in error
#define DEBUG_PORT Serial   // To define the serial port for debug printing
#include <ESP_SSLClient.h>
#include <WiFiClient.h>

#include <Preferences.h>  // WiFi storage
#include <Adafruit_Protomatter.h>
#include <Fonts/FreeSans9pt7b.h> // Large friendly font
#include <Fonts/FreeMono9pt7b.h> // Large friendly font

// matrix pins - waveshare matrix portsl S3
uint8_t rgbPins[]  = {42, 41, 40, 38, 39, 37};
uint8_t addrPins[] = {45, 36, 48, 35, 21};
uint8_t clockPin   = 2;
uint8_t latchPin   = 47;
uint8_t oePin      = 14;


     
//const char* gszSSID = NULL;  
//const char* gszPassword = NULL;
String gsPrefSSID;
String gsPrefPassword;
String gsPrefEspId;   // saved when wifi is setup, if 000 nimrod willl allocate an id
String gsPrefNimrodIp;
int giWFstatus = 255; //WL_IDLE_STATUS;
int giWiFiUpCount = 0;
int32_t giRssi = 0;  
String gsGetSsid;
String gsGetPass;
String gsMAC;
Preferences gPrefStorage;


// nimrod definitions
#define NIMROD_PORT       54011          // TODO: replace this with your own port number
#define PING_PERIOD       10
#define MSG_LENGTH        22
#define OUTPUT_LED_PIN    4
#define MSG_TIMEOUT       (20*1000)
bool gbDrawMatrix = true;
int giStartup = -1;
int giLedState = 0;
int giSocketConnected = -1;
int giSecSinceLastSend = 0;
int giSecSinceLastRecv = 0;
int giLedTimer = 20;
int giNumDevices = 0;
int giDisplayState = 0;
double gdWeight = 0;
int giHours = 0;
int giMinutes = 0;
int giSeconds = 0;
int giTimeCount = 0;
unsigned long gulNimrodLoopMillis = 0;
unsigned long gulLastMsgMillis = 0;
char gszCurrentTime[10] = "00:00:00";
String gsCardNo = "";
String gsTruckRego = "";
String gsTrailerRego = "";
String gsTruckTare = "";
String gsTrailerTare = "";
String gsTruckLoad = "";
double gdTruckWeight = 0.0;
String gsTrailerWeight = "";
//WiFiClientSecure client;
ESP_SSLClient ssl_client;

WiFiClient basic_client;


// end nimrod definitions

Adafruit_Protomatter matrix(
  128,          // Matrix width in pixels
  6,           // Bit depth -- 6 here provides maximum color options
  1, rgbPins,  // # of matrix chains, array of 6 RGB pins for each
  4, addrPins, // # of address pins (height is inferred), array of pins
  clockPin, latchPin, oePin, // Other matrix control pins
  true);       // HERE IS THE MAGIC FOR DOUBLE-BUFFERING!

// Sundry globals used for animation ---------------------------------------

int16_t  text1X;        // Current text position (X)
int16_t  text1Y;        // Current text position (Y)
int16_t  text1Min;      // Text pos. (X) when scrolled off left edge
char     str1[64];      // Buffer to hold scrolling message text
int16_t  text2X;        // Current text position (X)
int16_t  text2Y;        // Current text position (Y)
int16_t  text2Min;      // Text pos. (X) when scrolled off left edge
char     str2[64];      // Buffer to hold scrolling message text

int giLastMillis = 0;
uint16_t ww, hh;

void setup() 
{
  Serial.begin(115200);
  Serial.printf("\nNimrod ESP32 S3 starting\n");

  // Initialize matrix...
  ProtomatterStatus status = matrix.begin();
  Serial.print("Protomatter begin() status: ");
  Serial.println((int)status);
  if(status != PROTOMATTER_OK) {
    // DO NOT CONTINUE if matrix setup encountered an error.
    for(;;) {
      Serial.printf("Matrix setup error\n");
      delay(500);
    }
  }

    // Set up the scrolling message...
  sprintf(str1, "GTWB");
  sprintf(str2, "DRIVE ON");
  matrix.setFont(&FreeSans9pt7b); // Use nice bitmap font
  matrix.setTextWrap(false);           // Allow text off edge
  matrix.setTextColor(0xFFFF);         // White
  int16_t  x1, y1;
  //uint16_t w, h;
  matrix.getTextBounds(str1, 0, 0, &x1, &y1, &ww, &hh); // How big is it?
  text1Min = -ww; // All text is off left edge when it reaches this point
  text1X = 0; //matrix.width(); // Start off right edge
  text1Y = hh-1;//matrix.height() / 2 - (y1 + h / 2); // Center text vertically

  matrix.getTextBounds(str2, 0, 0, &x1, &y1, &ww, &hh); // How big is it?
  text2Min = -ww; // All text is off left edge when it reaches this point
  text2X = 0; //matrix.width(); // Start off right edge
  text2Y = matrix.height()-(y1+hh);

  matrix.setCursor(0,16);
  matrix.print("Startup");
  matrix.show();

  // Note: when making scrolling text like this, the setTextWrap(false)
  // call is REQUIRED (to allow text to go off the edge of the matrix),
  // AND it must be BEFORE the getTextBounds() call (or else that will
  // return the bounds of "wrapped" text).

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
  if ( gulNimrodLoopMillis + 100 < millis() )
  {
    gulNimrodLoopMillis = millis();

    // update time
    giTimeCount += 1;
    if ( giTimeCount >= 10 ) {
      gbDrawMatrix = true;
      giTimeCount = 0;
      giSeconds += 1;
      if ( giSeconds >= 60 ) {
        giSeconds = 0;
        giMinutes += 1;
        if ( giMinutes >= 60 ) {
          giMinutes = 0;
          giHours += 1;
          if ( giHours >= 24 ) {
            giHours = 0;
          }
        }
      }
    }

    nimrodLoop();

    if ( giDisplayState > 0 ) {
      if ( gulLastMsgMillis + MSG_TIMEOUT < millis() ){
        giDisplayState = 0;
        gbDrawMatrix = true;
      }
    }

    if ( giWFstatus != WiFi.status() )
    { // wifi status has changed
      giWFstatus = getWifiStatus();
    }
    
    if ( WiFi.status() != WL_CONNECTED ) 
    {  // WiFi DOWN
      if ( giWiFiUpCount == 0 )
      {
        Serial.printf("Starting Wifi...'%s'\n", gsPrefSSID.c_str() );
        
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

  if ( gbDrawMatrix )
  {
    gbDrawMatrix = false;

    drawMatrix();
  }
}  

void drawMatrix()
{
  char szLine[20];
  String sLine1;
  String sLine2;
  // Every frame, we clear the background and draw everything anew.
  // This happens "in the background" with double buffering, that's
  // why you don't see everything flicker. It requires double the RAM,
  // so it's not practical for every situation.

  matrix.fillScreen(0); // Fill background black

  switch ( giDisplayState ) {
  default:
  case 0:
    if ( giHours >= 19 || giHours < 6 ) {
      sLine1 = "";
      sLine2 = "";
      sprintf( gszCurrentTime, "%02d:%02d:%02d", giHours, giMinutes, giSeconds );
    } else {
      sLine1 = str1;
      //sLine2 = str2;
      if ( gdWeight >= 1000 )
        sprintf( szLine, "%d KG", (int)gdWeight );
      else
        sprintf( szLine, "%.1f KG", gdWeight );
      sLine2 = szLine;

      sprintf( gszCurrentTime, "%02d:%02d:%02d", giHours, giMinutes, giSeconds );
    }

    matrix.setTextColor(0xFFFF);         // White
    matrix.setCursor(text1X, text1Y);
    matrix.print(sLine1);

    matrix.setTextColor(matrix.color565(0xff,0x00,0x00)); 
    matrix.setCursor(58, 12);
    matrix.print(gszCurrentTime);

    matrix.setTextColor(matrix.color565(0x00,0x00,0xff)); 
    matrix.setCursor(text2X, text2Y);
    matrix.print(sLine2);
    break;

  case 1:
    // card disabled
    sLine1 = "#";
    sLine1 += gsCardNo;
    sLine2 = "DISABLED";

    matrix.setTextColor(0xFFFF);         // White
    matrix.setCursor(text1X, text1Y);
    matrix.print(sLine1);
    matrix.setTextColor(matrix.color565(0xff,0x00,0x00)); 
    matrix.setCursor(text2X, text2Y);
    matrix.print(sLine2);
    break;

  case 2:
    // card not found
    sLine1 = "#";
    sLine1 += gsCardNo;
    sLine2 = "NOT FOUND";

    matrix.setTextColor(0xFFFF);         // White
    matrix.setCursor(text1X, text1Y);
    matrix.print(sLine1);
    matrix.setTextColor(matrix.color565(0xff,0x00,0x00)); 
    matrix.setCursor(text2X, text2Y);
    matrix.print(sLine2);
    break;
    
  case 3:
    // card found
    sLine1 = "#";
    sLine1 += gsCardNo;
//    sLine2 = "DISABLED";

    matrix.setTextColor(0xFFFF);         // White
    matrix.setCursor(text1X, text1Y);
    matrix.print(sLine1);
//    matrix.setTextColor(matrix.color565(0x00,0x00,0xff)); 
//    matrix.setCursor(text2X, text2Y);
//    matrix.print(sLine2);
    break;
    
  case 4:
    // truck rego
    sLine1 = "#";
    sLine1 += gsCardNo;
    sLine2 = "REGO ";
    sLine2 += gsTruckRego;

    matrix.setTextColor(0xFFFF);         // White
    matrix.setCursor(text1X, text1Y);
    matrix.print(sLine1);
    matrix.setTextColor(matrix.color565(0x00,0x00,0xff)); // green
    matrix.setCursor(text2X, text2Y);
    matrix.print(sLine2);
    break;
    
  case 5:
    // truck tare
    sLine1 = "REGO ";
    sLine1 += gsTruckRego;
    sLine2 = "TARE ";
    sLine2 += gsTruckTare.toInt();
    sLine2 += " KG";

    matrix.setTextColor(0xFFFF);         // White
    matrix.setCursor(text1X, text1Y);
    matrix.print(sLine1);
    matrix.setTextColor(matrix.color565(0x00,0x00,0xff));   // green
    matrix.setCursor(text2X, text2Y);
    matrix.print(sLine2);
    break;
    
  case 6:
    // truck weight
    if ( gdTruckWeight >= 1000 )
      sprintf( szLine, "%d LOAD", (int)gdTruckWeight - gsTruckTare.toInt() );
    else
      sprintf( szLine, "%.1f LOAD", gdTruckWeight - (double)gsTruckTare.toInt() );
    sLine1 = szLine;
    if ( gdTruckWeight >= 1000 )
      sprintf( szLine, "%d WEIGHT", (int)gdTruckWeight );
    else
      sprintf( szLine, "%.1f WEIGHT", gdTruckWeight );
    sLine2 = szLine;

    if ( (double)gsTruckTare.toInt() + (double)gsTruckLoad.toInt() < gdTruckWeight)
      matrix.setTextColor(matrix.color565(0xff,0x00,0x00));   // red - overloaded
    else
      matrix.setTextColor(0xFFFF);         // White
    matrix.setCursor(text1X, text1Y);
    matrix.print(sLine1);

    if ( (double)gsTruckTare.toInt() + (double)gsTruckLoad.toInt() < gdTruckWeight)
      matrix.setTextColor(matrix.color565(0xff,0x00,0x00));   // red - overloaded
    else
      matrix.setTextColor(matrix.color565(0x00,0x00,0xff));   // green
    matrix.setCursor(text2X, text2Y);
    matrix.print(sLine2);
    break;
    
  case 7:
    // trailer rego
    sLine1 = "#";
    sLine1 += gsCardNo;
    sLine2 = "REGO ";
    sLine2 = gsTrailerRego;

    matrix.setTextColor(0xFFFF);         // White
    matrix.setCursor(text1X, text1Y);
    matrix.print(sLine1);
    matrix.setTextColor(matrix.color565(0x00,0x00,0xff));   //green
    matrix.setCursor(text2X, text2Y);
    matrix.print(sLine2);
    break;
    
  case 8:
    // trailer tare
    sLine1 = "REGO ";
    sLine1 += gsTrailerRego;
    sLine2 = "TARE ";
    sLine2 = gsTrailerTare.toInt();
    sLine2 += "KG";

    matrix.setTextColor(0xFFFF);         // White
    matrix.setCursor(text1X, text1Y);
    matrix.print(sLine1);
    matrix.setTextColor(matrix.color565(0x00,0x00,0xff));   // green
    matrix.setCursor(text2X, text2Y);
    matrix.print(sLine2);
    break;
    
  case 9:
    // trailer weight
    sLine1 = "REGO ";
    sLine1 += gsTrailerRego;
    sLine2 = "WEIGHT ";
    sLine2 = gsTrailerWeight.toInt();
    sLine2 += "KG";

    matrix.setTextColor(0xFFFF);         // White
    matrix.setCursor(text1X, text1Y);
    matrix.print(sLine1);
    matrix.setTextColor(matrix.color565(0x00,0x00,0xff));   // green
    matrix.setCursor(text2X, text2Y);
    matrix.print(sLine2);
    break;
    
  case 10:
    // trailer rego missing
    sLine1 = "TRAILER";
    //sLine1 += gsTrailerRego;
    sLine2 = "NO REGO";
    //sLine2 = gsTrailerWeight;

    matrix.setTextColor(0xFFFF);         // White
    matrix.setCursor(text1X, text1Y);
    matrix.print(sLine1);
    matrix.setTextColor(matrix.color565(0xff,0x00,0x00)); 
    matrix.setCursor(text2X, text2Y);
    matrix.print(sLine2);
    break;
    
  case 11:
    // already swiped
    sLine1 = "REGO ";
    sLine1 += gsTruckRego;
    sLine2 = "CARD IGNORED";
    //sLine2 = gsTrailerWeight;

    matrix.setTextColor(0xFFFF);         // White
    matrix.setCursor(text1X, text1Y);
    matrix.print(sLine1);
    matrix.setTextColor(matrix.color565(0xff,0x00,0x00)); 
    matrix.setCursor(text2X, text2Y);
    matrix.print(sLine2);
    break;
    
  case 12:
    // empty truck
    sLine1 = "REGO ";
    sLine1 += gsTruckRego;
    if ( gdTruckWeight >= 1000 )
      sprintf( szLine, "EMPTY %d KG", (int)gdTruckWeight );
    else
      sprintf( szLine, "EMPTY %.1f KG", gdTruckWeight );
    sLine2 = szLine;

    matrix.setTextColor(0xFFFF);         // White
    matrix.setCursor(text1X, text1Y);
    matrix.print(sLine1);
    matrix.setTextColor(matrix.color565(0xff,0x00,0x00)); 
    matrix.setCursor(text2X, text2Y);
    matrix.print(sLine2);
    break;
  }
  // AFTER DRAWING, A show() CALL IS REQUIRED TO UPDATE THE MATRIX!
  matrix.show();
}

void wifiInit() 
{
  WiFi.mode(WIFI_AP_STA);  // required to read NVR before WiFi.begin()

  // Open Preferences with "wifi" namespace. Namespace is limited to 15 chars
  gPrefStorage.begin("wifi", false);
  gsPrefSSID = gPrefStorage.getString("ssid", "none");  // NVS key ssid
  gsPrefPassword = gPrefStorage.getString("password", "none");  // NVS key password
  gsPrefEspId = gPrefStorage.getString("espid", "000");
  gsPrefNimrodIp = gPrefStorage.getString("nimrodip", "none");
  gPrefStorage.end();

  // keep from rewriting flash if not needed
  if (!checkPrefsStore())    
  { // NV and Prefs are different, setup with SmartConfig
    String sBuf;

    readSerialLine("Enter SSID", sBuf);
    gsPrefSSID = sBuf;
    Serial.printf("Got SSID '%s'\n", gsPrefSSID.c_str());

    sBuf = "";
    readSerialLine("Enter Password", sBuf);
    gsPrefPassword = sBuf;
    Serial.printf("Got Password '%s'\n", gsPrefPassword.c_str());

    sBuf = "";
    readSerialLine("Enter 3 Digit ESP Id", sBuf);
    gsPrefEspId = sBuf;
    Serial.printf("Got ESP Id '%s'\n", gsPrefEspId.c_str());

    sBuf = "";
    readSerialLine("Enter Nimrod Server IP", sBuf);
    gsPrefNimrodIp = sBuf;
    Serial.printf("Got Nimrod IP '%s'\n", gsPrefNimrodIp.c_str());

    // save the new wifi credentials
    gPrefStorage.begin("wifi", false);
    gPrefStorage.putString("ssid", gsPrefSSID);
    gPrefStorage.putString("password", gsPrefPassword);
    gPrefStorage.putString("espid", gsPrefEspId);
    gPrefStorage.putString("nimrodip", gsPrefNimrodIp);
    gPrefStorage.end();
    
    Serial.printf("Restarting\n");
    delay(3000);
    
    ESP.restart();  // reboot with new wifi configuration
  }


  Serial.printf( "WiFi starting\n" );
  WiFi.begin(gsPrefSSID.c_str(), gsPrefPassword.c_str());
  giWiFiUpCount += 1;
}  

void readSerialLine( const char* szMsg, String& sBuf )
{
  int iLoop = 0;

  while ( !Serial.available() )
  {
    iLoop += 1;
    if ( iLoop == 1 )
    {
      Serial.printf("\n%s\n", szMsg);
      Serial.printf("%s/%s/%s/%s\n", gsPrefSSID.c_str(), gsPrefPassword.c_str(), gsPrefEspId.c_str(), gsPrefNimrodIp.c_str());
    } 
    else if ( iLoop > 20 )
    {
      iLoop = 0;
    }
    else
    {
      Serial.printf(".");
    }
    delay(500);
  }

  sBuf = Serial.readString();
  sBuf.replace("\n","");
  sBuf.replace("\r","");
}

bool checkPrefsStore() 
{
  bool bRet = true;
  String prefssid, prefpass, prefespid, prefnimrodip;

  // Open Preferences with my-app namespace. Namespace name is limited to 15 chars
  gPrefStorage.begin("wifi", false);
  prefssid = gPrefStorage.getString("ssid", "none");  // NVS key ssid
  prefpass = gPrefStorage.getString("password", "none");  // NVS key password
  prefespid = gPrefStorage.getString("espid", "000");
  prefnimrodip = gPrefStorage.getString("nimrodip", "none");
  gPrefStorage.end();

  if (prefssid == "none" || prefssid == "" || prefpass == "none" || prefpass == "" || prefespid == "000" || prefespid == "" || prefnimrodip == "none" || prefnimrodip == "") 
  {
    bRet = false;
  }

  return bRet;
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


//**********************************************************************************************************
//
//  nimrod functions
//
//**********************************************************************************************************


void nimrodSetup()
{
  pinMode( OUTPUT_LED_PIN, OUTPUT );

  digitalWrite( OUTPUT_LED_PIN, HIGH );

}


// called form the main loop() function
void nimrodLoop()
{
  unsigned long ulTimeNow = millis();
  
  if ( ssl_client.connected() )
  { // socket is connected
    while ( ssl_client.available() >= 8 )
    {
      giSecSinceLastRecv = 0;
      Serial.printf( "msg is available, %d bytes\n", ssl_client.available() );

      int iAvail = ssl_client.available();
      int i;
      char szMsg[MSG_LENGTH+1];
      for ( i = 0; i < iAvail && i < MSG_LENGTH; i++ )
      {
        szMsg[i] = ssl_client.read();
      }
      szMsg[i] = '\0';
      
      Serial.printf( "received '%s'\n", szMsg );

      HandleMessage( szMsg );
    }
  }

  if ( Serial.available() )
  {
    char szBuf[50];
    Serial.read( szBuf, sizeof(szBuf));

    if ( strncmp( szBuf, "erasewifi", 9) == 0 )
    {
      Serial.printf("Clearing wifi credsentials\n");
      // clear the wifi credentials
      gPrefStorage.begin("wifi", false);
      gPrefStorage.putString("ssid", "none");
      gPrefStorage.putString("password", "none");
      gPrefStorage.putString("espid", "000");
      gPrefStorage.end();
      
      Serial.printf("Restarting\n");
      delay(3000);
      
      ESP.restart();  // reboot with new wifi configuration
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

  giLedState += 1;
  if ( giLedState >= 10 )
  { // every second
    giLedState = 0;
    giSecSinceLastSend += 1;
    giSecSinceLastRecv += 1;

    // check wifi
    if ( giSocketConnected == 1 )
    {
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
      }
      
      PrintIPinfo();
      
      Serial.printf( "Try to connect the socket\n" );
      if (!ssl_client.connect(gsPrefNimrodIp.c_str(), NIMROD_PORT)) 
      {
        Serial.printf("Connection to %s:%d failed\n", gsPrefNimrodIp.c_str(), NIMROD_PORT );
      }
      else
      {
        giSocketConnected = 1;
      
        Serial.printf( "Connected to %s:%d\n", gsPrefNimrodIp.c_str(), NIMROD_PORT );
  
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
          sprintf( szMsg, "ESP%sCID%c%c%c%c%c%c%c%c%c%c%c%c", gsPrefEspId.c_str(), gsMAC[0], gsMAC[1], gsMAC[3], gsMAC[4], gsMAC[6], gsMAC[7],
              gsMAC[9], gsMAC[10], gsMAC[12], gsMAC[13], gsMAC[15], gsMAC[16] );
          socket_send( szMsg );
        }
      }      
    }

    
    if ( giSocketConnected == 1 )
    {
      if ( giSecSinceLastRecv >= 15 )  
      { // socket is dead
        giSecSinceLastRecv = 0;
        
        Serial.printf( "socket is dead (lastrecv)\n" );
        ssl_client.stop();
        giSocketConnected = -1;
      }
      else if ( giSecSinceLastSend >= PING_PERIOD )
      { // send a ping
        char szMsg[MSG_LENGTH+1];
        sprintf( szMsg, "ESP%sPG", gsPrefEspId.c_str() );
        socket_send( szMsg );
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

  int bytes = ssl_client.print( msg );
  if ( bytes != MSG_LENGTH )
  {
    Serial.printf("Wrote %d of %d bytes\n", bytes, MSG_LENGTH );
  }
  
  giSecSinceLastSend = 0;
}


void HandleMessage( const char* c )
{

  // 0123456789012
  // OKyxxxxx   output control
  // PG000000   ping from nimrod
  // NNESPxxx   our new name
  // TMhhmmss   current time
  // XD<cardno> card disabled
  // XE<weight> empty
  // XN<cardno> card not found
  // XF<cardno> card found
  // XR<rego>   truck rego
  // XS<cardno> 2nd swipes
  // XT<tare>   truck tare
  // XW<weight> truck weight
  // XL<load>   truck load
  // YR<rego>   trailer rego
  // YT<tare>   trailer tare
  // YW<weight> trailer weight
  // YN<cardno> no trailer rego
  if ( strncmp(c,"OK",2) == 0 )
  {
    Serial.printf("Ignore msg OK\n");
  } 
  else if ( strncmp( c, "TM", 2 ) == 0 )
  { // current time
    gszCurrentTime[0] = c[2];
    gszCurrentTime[1] = c[3];
    gszCurrentTime[2] = ':';
    gszCurrentTime[3] = c[4];
    gszCurrentTime[4] = c[5];
    gszCurrentTime[5] = ':';
    gszCurrentTime[6] = c[6];
    gszCurrentTime[7] = c[7];
    gszCurrentTime[8] = '\0';

    giHours = atoi(&gszCurrentTime[0]);
    giMinutes = atoi(&gszCurrentTime[3]);
    giSeconds = atoi(&gszCurrentTime[6]);

    gbDrawMatrix = true;
  }
  else if ( strncmp(c,"NN",2) == 0 )
  { // TODO: save this to PrefStorage
    gsPrefEspId = &c[5];
    
    Serial.printf( "New name ESP%s\n", gsPrefEspId.c_str() );
  } 
  else if ( strncmp(c, "WW", 2 ) == 0 ) {
    gdWeight = atof(&c[2]);
    gbDrawMatrix = true;
  } else if ( c[0] == 'X' ) {
    // card or truck message
    gulLastMsgMillis = millis();
    gbDrawMatrix = true;
    if ( c[1] == 'D' ) {
      // card disabled
      giDisplayState = 1;
      gsCardNo = &c[2];
    }
    else if ( c[1] == 'N' ) {
      // card not found
      giDisplayState = 2;
      gsCardNo = &c[2];
    }
    else if ( c[1] == 'F' ) {
      // card found
      giDisplayState = 3;
      gsCardNo = &c[2];
    }
    else if ( c[1] == 'R' ) {
      // truck rego
      giDisplayState = 4;
      gsTruckRego = &c[2];
    }
    else if ( c[1] == 'T' ) {
      // truck tare
      giDisplayState = 5;
      gsTruckTare = &c[2];
    }
    else if ( c[1] == 'W' ) {
      // truck weight
      giDisplayState = 6;
      gdTruckWeight = atof(&c[2]);
    }
    else if ( c[1] == 'S' ) {
      // 2nd swipe
      giDisplayState = 11;
    }
    else if ( c[1] == 'E' ) {
      // empty truck
      giDisplayState = 12;
      gdTruckWeight = atof(&c[2]);
    }
    else if ( c[1] == 'L' ) {
      // truck load
      gsTruckLoad = &c[2];
    }
  } else if ( c[0] == 'Y' ) {
    // trailer message
    gulLastMsgMillis = millis();
    gbDrawMatrix = true;
    if ( c[1] == 'R' ) {
      // trailer rego
      giDisplayState = 7;
      gsTrailerRego = &c[2];
    }
    else if ( c[1] == 'T' ) {
      // trailer tare
      giDisplayState = 8;
      gsTrailerTare = &c[2];
    }
    else if ( c[1] == 'W' ) {
      // trailer weight
      giDisplayState = 9;
      gsTrailerWeight = &c[2];
    }
    else if ( c[1] == 'N' ) {
      // trailer rego missing
      giDisplayState = 10;
    }
  }
  else
  {
    Serial.printf("Unknown msg type '%s'\n", c );
  }
}

