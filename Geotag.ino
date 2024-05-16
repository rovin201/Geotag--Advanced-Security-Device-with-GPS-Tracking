##include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
TinyGPSPlus gps; // The TinyGPS++ object
SoftwareSerial ss(4, 5); // The serial connection to the GPS device
const char* ssid = "LAB-320";
const char* password = "lab320ap";
float latitude , longitude;
int year , month , date, hour , minute , second;
String date_str , time_str , lat_str , lng_str;
int pm;
WiFiServer server(80);
const char* writeAPIKey = "U8167Y7D8HL7RIKB";
const char* thingspeakServer = "api.thingspeak.com";
// WiFi settings for ThingSpeak
const char* ts_ssid = "LAB-320";
const char* ts_pass = "lab320ap";
const char* ts_server = "api.thingspeak.com";
WiFiClient ts_client;
void sendToThingSpeak(float lat, float lon, float gasLevel) {
if (ts_client.connect(ts_server, 80)) {
String ts_data = "api_key=" + String(writeAPIKey) + "&field1=" + String(lat, 6) +
"&field2=" + String(lon, 6) + "&field3=" + String(gasLevel);
ts_client.println("POST /update HTTP/1.1");
ts_client.println("Host: api.thingspeak.com");
ts_client.println("Connection: close");
ts_client.println("X-THINGSPEAKAPIKEY: " + String(writeAPIKey));
ts_client.println("Content-Type: application/x-www-form-urlencoded");
ts_client.println("Content-Length: " + String(ts_data.length()));
ts_client.println();
ts_client.println(ts_data);
ts_client.println();
delay(1000);
ts_client.stop();
Serial.println("Data sent to ThingSpeak: " + ts_data);
} else {
Serial.println("Connection to ThingSpeak failed");
20
}
delay(15000); // Send data to ThingSpeak every 15 seconds (ThingSpeak limit)
}
void setup() {
Serial.begin(115200);
ss.begin(9600);
Serial.println();
Serial.print("Connecting to ");
Serial.println(ssid);
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
Serial.println("");
Serial.println("WiFi connected");
server.begin();
Serial.println("Server started");
Serial.println(WiFi.localIP());
}
void loop() {
while (ss.available() > 0) {
if (gps.encode(ss.read())) {
if (gps.location.isValid()) {
latitude = gps.location.lat();
lat_str = String(latitude , 6);
longitude = gps.location.lng();
lng_str = String(longitude , 6);
}
if (gps.date.isValid()) {
date_str = "";
date = gps.date.day();
month = gps.date.month();
year = gps.date.year();
if (date < 10)
date_str = '0';
date_str += String(date);
date_str += " / ";
if (month < 10)
date_str += '0';
date_str += String(month);
date_str += " / ";
if (year < 10)
date_str += '0';
date_str += String(year);
21
}
if (gps.time.isValid()) {
time_str = "";
hour = gps.time.hour();
minute = gps.time.minute();
second = gps.time.second();
minute = (minute + 30);
if (minute > 59) {
minute = minute - 60;
hour = hour + 1;
}
hour = (hour + 5) ;
if (hour > 23)
hour = hour - 24;
if (hour >= 12)
pm = 1;
else
pm = 0;
hour = hour % 12;
if (hour < 10)
time_str = '0';
time_str += String(hour);
time_str += " : ";
if (minute < 10)
time_str += '0';
time_str += String(minute);
time_str += " : ";
if (second < 10)
time_str += '0';
time_str += String(second);
if (pm == 1)
time_str += " PM ";
else
time_str += " AM ";
}
}
}
// Read gas level
float gasLevel = analogRead(A0) / 1023.0 * 100.0;
// Send data to ThingSpeak
22
sendToThingSpeak(latitude, longitude, gasLevel);
// Check if a client has connected
WiFiClient client = server.available();
if (!client) {
return;
}
// Prepare the response
String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n <!DOCTYPE html>
<html> <head> <title>GPS Interfacing with NodeMCU</title> <style>";
s += "a:link {background-color: YELLOW;text-decoration: none;}";
s += "table, th, td {border: 1px solid black;} </style> </head> <body> <h1 style=";
s += "font-size:300%;";
s += " ALIGN=CENTER> GPS Interfacing with NodeMCU</h1>";
s += "<p ALIGN=CENTER style=""font-size:150%;""";
s += "> <b>Location Details</b></p> <table ALIGN=CENTER style=";
s += "width:50%";
s += "> <tr> <th>Latitude</th>";
s += "<td ALIGN=CENTER >";
s += lat_str;
s += "</td> </tr> <tr> <th>Longitude</th> <td ALIGN=CENTER >";
s += lng_str;
s += "</td> </tr> <tr> <th>Date</th> <td ALIGN=CENTER >";
s += date_str;
s += "</td></tr> <tr> <th>Time</th> <td ALIGN=CENTER >";
s += time_str;
s += "</td> </tr> </table> ";
if (gps.location.isValid()) {
s += "<p align=center><a style=""color:RED;font-size:125%;""
href=""http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=";
s += lat_str;
s += "+";
s += lng_str;
s += """ target=""_top"">Click here!</a> To check the location in Google maps.</p>";
}
s += "</body> </html> \n";
client.print(s);
delay(100);
}
