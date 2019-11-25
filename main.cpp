#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
 
// Replace with your network credentials
const char* ssid = "NodeMCU";
const char* password = "arcsnode";

typedef struct {
  String content;
} TEXT;

unsigned short numTexts = 0;
unsigned short textSize = 10;
TEXT* texts = (TEXT*)calloc(textSize, sizeof(TEXT));
 
ESP8266WebServer server(80);   //instantiate server at port 80 (http port)

String page = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Arc's Node</title>"
/* CSS */
"<style>"
"body { text-align: center; margin: 0px; background-color: #F2E8DC; }"
"div { display: flex; flex-direction: column; margin: auto; }"
"header { font-size: 30px; color: #F2E8DC; background-color: #363940; text-align: left; padding-left: 35px; }"
"textarea { background-color: #D96262; color: #363940; height: 120px; padding: 10px; font-size: 18px; font-family: sans-serif; border: 0px; }"
"textarea::placeholder { color: #363940; }"
"button { background-color: #A8BFB7; border: 0px; padding: 15px; font-size: 24px; color: #363940; cursor: pointer; }"
"button:active { background-color: #91A59E; }"
"#msgs { padding: 20px; overflow-y: auto; max-height: 320px; }"
"#msgs div { text-align:left; background-color: #A8BFB7; margin: 0px 5px 10px 5px; padding: 15px; font-family: sans-serif; color: #363940; border: transparent 2px; border-radius: 15px; }"
"#footer { position: fixed; bottom: 0; width: 100%; }"
"</style></head><body>"
/* HTML */
"<header>Arc's Node</header>"
"<div id='msgs'></div>"
"<div id='footer'>"
  "<textarea id='text' maxlength='100' placeholder='Type your message here...'></textarea>"
  "<button onclick='msg()'>Send Message</button>"
"</div>"
/* JavaScript */
"<script>"
  "function msg() {"
    "fetch('/message', {method: 'POST', body: document.getElementById('text').value, headers: {'Content-Type': 'text/plain'}});"
    "var node = document.createElement('div');"
    "node.appendChild(document.createTextNode(document.getElementById('text').value));"
    "document.getElementById('msgs').appendChild(node);"
    "document.getElementById('text').value = '';"
  "}"
  "function refreshMsgs() {"
    "fetch('/getmsgs').then(response => response.json()).then(data => {"
      "var parentNode = document.getElementById('msgs');"
      "while(parentNode.firstChild) {"
        "parentNode.removeChild(parentNode.firstChild);"
      "}"
      "for(let i = 0; i < data.length; i++) {"
        "var node = document.createElement('div');"
        "node.appendChild(document.createTextNode(data[i].text));"
        "parentNode.appendChild(node);"
      "}"
    "})"
  "}"
  "setInterval(refreshMsgs, 1500);"
  "refreshMsgs();"
"</script></body></html>";

void setup(void){
  Serial.begin(9600);
  WiFi.softAP(ssid, password); //begin WiFi access point
  
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP()); 
  
  server.on("/", [](){
    server.send(200, "text/html", page);
  });
  
  server.on("/message", [](){
    if (server.hasArg("plain") != false) {
      if (numTexts < textSize - 2) {
        (texts + numTexts++)->content = server.arg("plain");
      }
      else {
        textSize += 10;
        texts = (TEXT*)realloc(texts, textSize);
        (texts + numTexts++)->content = server.arg("plain");
      }
      server.send(200, "text/plain", "Success");
    }
    else server.send(400, "text/plain", "Bad Request");
  });
  
  server.on("/getmsgs", [](){
    String msgs = "[";
    unsigned short i;
    for(i = 0; i < numTexts; i++) {
      msgs += "{\"text\":\"" + (texts + i)->content + "\"}";
      if (i < numTexts - 1) msgs += ",";
    }
    msgs += "]";
    server.send(200, "application/json", msgs);
  });
  
  server.begin();
  Serial.println("Web server started!");
}
 
void loop(void){
  server.handleClient();
}
