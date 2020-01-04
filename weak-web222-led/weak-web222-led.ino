//
//    weak-web222-led - esp8266 web server with static ip address to control some rgb leds
//
//    jens, 20190922
//          20200104 - code clean up, new status page
//
//    MIT license, see file LICENSE
//

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include <FS.h>                             // file system for web pages
// uploaded with Arduino IDE from directory 'data'

#include <FastLED.h>                        // library to control RGB leds

#define NUM_LEDS 7
#define DATA_PIN D3                         // default is D4 (changed because it is also onboard led)

#define BRIGHTNESS 85                       // reduced brightness, reduced heat, 1/3



// dynamic webpage contents in program memory

const char TOPpage[] PROGMEM = R"=====(
<!doctype html>
<html lang=de>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta http-equiv="refresh" content="5; URL=http://pfitze.selfhost.eu/help.html">
<link rel="stylesheet" href="style.css">
<title>Status Page</title>
</head>
<body>    
<div class="container">
<div class="center"> 
<h1>LED Web Server</h1>
current color
<h2>
)=====";

const char BOTpage[] PROGMEM = R"=====(
</h2>
<br>
<a href="http://pfitze.selfhost.eu/help.html">change color</a>
<br>
<br>
<br>
<small><a href="http://pfitze.selfhost.eu">pfitze.selfhost.eu</a></small>
</div>
</div>      
</body>
</html>
)=====";



CRGB leds[NUM_LEDS];

byte cR=0, cG=0, cB=0;                      // current LED color in r,g,b
String cname = "black";                     // current color name


void showled(byte R, byte G, byte B) {                             
  for (int num = 1; num < NUM_LEDS; num++) {
    leds[num].setRGB(R, G, B);
    FastLED.show();
    delay(25);                              // small fading effect
  }
  leds[0].setRGB(R, G, B);                  // part of fading effect, inner led last
  FastLED.show();
}


void setcolor(byte R, byte G, byte B, String name) {
  
  cR = R; cG = G; cB = B;
  cname = name;
  
  showled(cR,cG,cB);
}


void showevent(int duration) {

  showled(178,89,0);                        // event color: ginger
  delay(duration);
  showled(cR,cG,cB);
}


// ssid / password of wifi router
//const char* ssid = "beiPfitze";
//const char* password = "********";

// ssid / password of wifi router
const char* ssid = "Pfitze";
const char* password = "********************";


ESP8266WebServer server(80);                // web server on port 80

String getContentType(String filename) {    // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  //  else if (filename.endsWith(".js")) return "application/javascript";     // not used, disabled
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}


bool handleFileRead(String path) {              // send static content / file to client, if exists
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/"))
    path += "index.html";                       // folder requested, send index file
  String contentType = getContentType(path);    // get MIME type
  if (SPIFFS.exists(path)) {                    // if file exists ...
    File file = SPIFFS.open(path, "r");                     
    size_t sent = server.streamFile(file, contentType);   // ... send it to the client
    file.close();
    return true;
  }
  Serial.println("\tfile not found");
  return false;
}


//
// functions to handle url requests
//

void handleStatus() {
  Serial.print("current color: ");                                 
  Serial.println(cname);

  int len = sizeof(TOPpage) + cname.length() + sizeof(BOTpage);
  Serial.println(len);                              // DEBUG: content length

  server.setContentLength(len);
  server.send_P(200, "text/html",TOPpage);          // first part from pgm memory
  server.sendContent(cname);                        // dynamic content
  server.sendContent_P(BOTpage);                    // final part, from pgm memory
}

void handleColorRed() {
  setcolor(255, 0, 0, "red");
  handleStatus();
}

void handleColorYellow() {
  setcolor(255, 255, 0, "yellow");
  handleStatus();
}

void handleColorGreen() {
  setcolor(0, 255, 0, "green");
  handleStatus();
}

void handleColorAqua() {
  setcolor(0, 255, 255, "aqua");
  handleStatus();
}

void handleColorBlue() {
  setcolor(0, 0, 255, "blue");
  handleStatus();
}

void handleColorFuchsia() {
  setcolor(255, 0, 255, "fuchsia");
  handleStatus();
}

void handleColorBlack() {
  setcolor(0, 0, 0, "black");
  handleStatus();
}

void handleColorWhite() {
  setcolor(255, 255, 250, "white");
  handleStatus();
}


String logfile = "Logfile:\n";
const int logsize = 1000;


void handleNotFound() {

  if ( !handleFileRead(server.uri()) ) {                // send existing static files
    Serial.print("404");                                // if not successful it is a 404
    Serial.print(" - ");
    Serial.println(server.uri());
    server.send(404, "text/plain", "404: Not Found");   // respond with a 404 status code (not found)

    logfile = logfile + server.uri() + "\n";            // append current url to string logfile
    if ( logfile.length() > logsize) {                  // if to big reset logfile
      logfile = "Logfile:\n";
      Serial.println("new logfile");
    }

    showevent(500);                                     // show event via led
  }
}


void handleLog() {

  server.send(200, "text/plain", logfile);              // just deliver string logfile
}


//
// arduino functions setup & loop
//

void setup(void) {
  Serial.begin(115200);
  delay(100);

  Serial.println("\nweak-web222-led\n");                // DEBUG outputs

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);                      // onboard led off

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);      // init rgb led library
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setCorrection(0xFFA0E0);                      // color correction, based on trial-and-error

  setcolor(179, 89, 0, "ginger");                       // device init led color: ginger

  SPIFFS.begin();                                       // use file system
  Serial.println("Files:");                             // DEBUG: print files and sizes
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    Serial.print(dir.fileName());
    Serial.print(" - ");
    File f = dir.openFile("r");
    Serial.println(f.size());
  }
  Serial.println();

  Serial.println("Network:");                           // DEBUG: start network configuration
  digitalWrite(LED_BUILTIN, LOW);                       // DEBUG: onboard led on

  // used static ip address
  IPAddress staticIP(192, 168, 178, 222);               // static ip
  IPAddress gateway(192, 168, 178, 1);                  // ip address of wifi router
  IPAddress subnet(255, 255, 255, 0);                   // Subnet mask
  IPAddress dns(192, 168, 178, 1);                      // DNS -> typcially gateway

  WiFi.mode(WIFI_STA);                                  // mode station, connect to wifi router

  WiFi.config(staticIP, gateway, subnet, dns);

  WiFi.begin(ssid, password);                           // connect to wifi router

  while (WiFi.status() != WL_CONNECTED) {               // wait for connection
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("connected to ");                        // connection successful, show wlan and IP address
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());                       // IP address configured above, should be *.222

  server.on("/red", handleColorRed);                    // call color functions based on URL path
  server.on("/yellow", handleColorYellow);              // TOFO: use args in url
  server.on("/green", handleColorGreen);
  server.on("/aqua", handleColorAqua);
  server.on("/blue", handleColorBlue);
  server.on("/fuchsia", handleColorFuchsia);
  server.on("/black", handleColorBlack);
  server.on("/white", handleColorWhite);

  server.on("/status", handleStatus);                   // show current status / color
  
  server.on("/logg", handleLog);                        // using unusual name, 'log' is first guess

  server.onNotFound(handleNotFound);                    // all other requests

  server.begin();                                       // start web server
  Serial.println("http server started");
  digitalWrite(LED_BUILTIN, HIGH);                      // DEBUG: onboard led off

  setcolor(0, 0, 0, "black");                           // default: led off
}


void loop(void) {
  server.handleClient();                                // handle web client requests
}
