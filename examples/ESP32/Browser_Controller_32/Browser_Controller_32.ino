#include <LifxApi.h>

/*******************************************************************
 *  An example of lifx request that makes a request to the server  *
 *  Go to: https://cloud.lifx.com/settings for your token          *
 *  written by Joeybab3 (joeybab3 on Github)                       *
 *******************************************************************/


#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

MDNSResponder mdns;
#include <ArduinoOTA.h>
#include <WiFiClientSecure.h>
#include <ctype.h>

#define API_KEY "API_TOKEN"  // your lifx API Token
#define SELECTOR "all" // find more selectors here https://api.developer.lifx.com/docs/selectors

const char* ssid     = "SSID";
const char* password = "PASSWORD";

void handleSerialRead();
void ethConnectError();

WiFiClientSecure client;
LifxApi api(API_KEY, client);

int api_mtbs = 15000; //mean time between api requests
long api_lasttime;   //last time api request has been done
int duration = 0;
String lastCmd;

#define HTTPPORT 88

WebServer server(HTTPPORT);
void handleRoot();
void handleCmd();
void handleNotFound();
void handleResponse();
void handleGet();
void handleGt();

void setup()
{
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("connected to WiFi");
  if (MDNS.begin("LifxController")) {
    Serial.println("MDNS responder started");
  }

  ArduinoOTA.setHostname("lifx");
  ArduinoOTA.setPort(8266);
  
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /* WebServer stuff */
  server.on("/", handleRoot);
  server.on("/cmd", handleCmd);
  server.on("/get", handleGt);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.print("HTTP server started on ");
  Serial.println(HTTPPORT);
  api.getBulbInfo(SELECTOR);
}

void loop()
{
  ArduinoOTA.handle();
  server.handleClient();
}

String handleGet(String cmd)
{
    api.getBulbInfo(SELECTOR);
    Serial.println("Handling command get-" + cmd);
    if (cmd == "brightness")
    {
      return String(api.bulbinfo.brightness);
    }
    else if (cmd == "hue")
    {
      return String(api.bulbinfo.color.hue);
    }
    else if (cmd == "kelvin")
    {
      return String(api.bulbinfo.color.kelvin);
    }
    else if (cmd == "saturation")
    {
      return String(api.bulbinfo.color.saturation);
    }
    else if(cmd == "power")
    {
      return String(api.bulbinfo.power);
    }
    else if(cmd == "label")
    {
      return String(api.bulbinfo.label);
    }
    else if(cmd == "id")
    {
      return String(api.bulbinfo.id);
    }
	else{
		return "Could not get \""+cmd+"\"...";
	}
}

String handleInput(String cmd)
{
    // Play
    Serial.println("Handling command " + cmd);
    if (cmd == "tp" || cmd == "toggle" || cmd == "togglepower" || cmd == "power")
    {
      if(api.togglePower("id:"+api.bulbinfo.id))
      {
        return "Toggle power worked.";
      }
      else
      {
        return "Toggle power failed.";
      }
    }
    // Pause
    else if (cmd == "t" || cmd == "test")
    {
      Serial.println("test cmd recieved");
      return "Test cmd recieved";
    }
	else if (cmd.startsWith("d") || cmd.startsWith("D"))
    {
      Serial.println("Set duration to"+cmd.substring(1,cmd.length()));
	  duration = cmd.substring(1,cmd.length()).toInt();
      return "Setting duration.";
    }
	else if (cmd.startsWith("c") || cmd.startsWith("C"))
    {
      Serial.println("Set color to"+cmd.substring(1,cmd.length()));
	  if(api.setState(SELECTOR, "color", "\"#"+cmd.substring(1,cmd.length())+"\"", duration))
      {
        return "Set color.";
      }
      else
      {
        return "Set color failed.";
      }
    }
	else if (cmd.startsWith("b") || cmd.startsWith("B"))
    {
      Serial.println("Set brightness to"+cmd.substring(1,cmd.length()));
	  if(api.setState(SELECTOR, "brightness", cmd.substring(1,cmd.length()), duration))
      {
        return "Set brightness.";
      }
      else
      {
        return "Set brightness failed.";
      }
    }
	else if (isDigit(cmd.charAt(0)))
    {
      return "You typed a digit.";
    }
	else{
		return cmd+" is not a command...";
	}
}

/* WebServer Stuff */

void handleRoot() {
  int brightness = api.bulbinfo.brightness;
  String msg = "<html>\n";
  msg += "<head>\n";
  msg += "<title>ESP8266 Lifx Controller</title>\n";
  msg += "<link rel=\"stylesheet\" type=\"text/css\" href=\"http://joeybabcock.me/iot/hosted/hosted-lifx.css\">";
  msg += "<script src=\"https://code.jquery.com/jquery-3.1.1.min.js\"></script>\n";
  msg += "<script src=\"http://joeybabcock.me/iot/hosted/hosted-lifx.js\">var power = \""+api.bulbinfo.power+"\";</script>\n";
  msg += "</head>\n";
  msg += "<body>\n";
  msg += "<div id=\"container\">\n";
  msg += "<h1>Lifx - Esp8266 Web Controller!</h1>\n";
  msg += "<p id=\"linkholder\"><a href=\"#\" id=\"status\" class='off' onclick=\"togglePower();\"></a> \n";
  msg += "<a href=\"#\" onclick=\"updateStuff();\"><img src=\"http://joeybabcock.me/iot/hosted/up.png\"/></a>\n";
  msg += "<input type=\"color\" id=\"colorinput\" /></p>\n";
  msg += "<h3>Brightness: <span id=\"brt\">"+String(brightness)+"</span><input type=\"hidden\" id='brightness' value='"+String(brightness)+"' onchange=\"setBrightness(this.value)\"/></h3><br/>\n";
  msg += "<input type=\"range\" class=\"slider\"  min=\"0\" max=\"99\" value=\""+String(brightness)+"\" id=\"brightness-slider\" onchange=\"setBrightness(this.value)\" />\n";
  msg += "<p>Server Response:<div id=\"response\" class=\"response\">Setting up...</div></p>\n";
  msg += "<p><form action=\"/\" method=\"get\" id=\"console\"><input placeholder=\"Enter a command...\" type=\"text\" id='console_text'/></form></p>\n";
  msg += "<script>\n";
  msg += "$('#console').submit(function(){\n";
  msg += "parseCmd($(\"#console_text\").val());\n";
  msg += "return false;\n";
  msg += "});\n";
  msg += "$('#colorinput').on('input', function() {\n";
  msg += "parseCmd(\"c\"+$(this).val().substr(1));\n";
  msg += "$('#container').css('background-color', $(this).val());\n";
  msg += "});\n";
  msg += "getBrightness();\n";
  msg += "getPower();\n";
  msg += "</script>\n";
  msg += "</div>\n";
  msg += "<div id=\"tips\"></div>\n";
  msg += "</body>\n";
  msg += "</html>\n";
  server.send(200, "text/html", msg);
}

void handleCmd(){
  String resp;
  for (uint8_t i=0; i<server.args(); i++){
    if(server.argName(i) == "cmd") 
    {
      lastCmd = server.arg(i);
      resp = handleInput(lastCmd);
    }
  }
  handleResponse(resp);
}

void handleGt(){
  String resp;
  for (uint8_t i=0; i<server.args(); i++){
    if(server.argName(i) == "cmd") 
    {
      lastCmd = server.arg(i);
      resp = handleGet(lastCmd);
    }
  }
  handleGetResponse(resp);
}

void handleNotFound() {

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);

}

void handleResponse(String response) {
      server.send(200, "text/html", response);
      Serial.println("Got client.");
}

void handleGetResponse(String response) {
      server.send(200, "text/html", response);
      Serial.println("Got client.");
}
