/*
Copyright (c) 2016 Joey Babcock. All right reserved.
See: http://www.joeybabcock.me/blog/projects/arduino-lifx-api-control-library-for-esp8266/

ArduinoLifx API - A library to control your lifx bulbs via
ESP8266 on Arduino IDE.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef LifxApi_h
#define LifxApi_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Client.h>

#define HOST "api.lifx.com"
#define SSL_PORT 443
#define HANDLE_MESSAGES 1
#define MAX_BUFFER_SIZE 1250

struct Color{
	float saturation;
	float hue;
	long kelvin;
};

struct BulbInfo{
  String id;
  String uuid;
  String label;
  bool connected;
  String power;
  float brightness;
  Color color;
};

class LifxApi
{
  public:
    LifxApi (String apiKey, Client &client);
    String sendReqToLifx(String command, String type, String content);
    bool getBulbInfo(String selector);
	bool togglePower(String selector);
	bool setState(String selector, String param, String value, int duration);
    BulbInfo bulbinfo;
	//const char* fingerprint = "‎9D:01:5C:8E:FD:4D:DF:71:A4:99:CE:29:93:40:3F:5F:EE:74:d0:95";
    const char* fingerprint = "‎E3:69:05:13:32:74:C0:37:F8:6C:B8:A7:18:98:87:B7:CD:DD:86:F0";  //Lifx https Certificate
	

  private:
    //JsonObject * parseUpdates(String response);
    String _apiKey;
    Client *client;
    const int maxMessageLength = 1500;
    bool checkForOkResponse(String response);
};

#endif
