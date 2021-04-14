/*
 * Please excuse the scappiness of this code, it was written without maintainence in mind
 * but hopefully helps someone else to hack and play.
 *
 * Note that the source for `INDEX_HTML` is in this repo in a non-compressed form so it's
 * easier to read and modify.
 *
 * - Remy Sharp, 2021 / https://remysharp.com
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <ps2dev.h>

#define DEBOUNCE_LENGTH 2

// via https://chewett.co.uk/blog/1066/pin-numbering-for-wemos-d1-mini-esp8266/
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2

PS2dev keyboard(D1 , D2); //clock, data

const int buttons[] = {D3, D4};
const int btn_length = 2;
int last[] = {LOW, LOW};

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

static const char ssid[] = "sharpwifi";
static const char password[] = "remysharp";
MDNSResponder mdns;

ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>remote ps/2</title>
    <style>html{--c-bg:#f9f9f9;--c-text:#212121;--c-input-fg:#212121;--c-input-border:#e0e0e0}@media (prefers-color-scheme: dark){html{--c-bg:#262626;--c-text:#d6d6d6;--c-input-fg:#fff;--c-input-border:#000}}html{background:var(--c-bg);color:var(--c-text);height:100%}button{border-radius:5px;cursor:pointer;color:var(--c-input-fg);border:2px solid var(--c-input-border);background:rgba(1, 1, 1, 0.1);padding:8px 16px;height:auto;margin:4px}*,button{font-family:monospace;font-size:1.1em}div{display:flex;justify-content:center;flex-wrap:wrap;margin-bottom:20px}#controls{font-size:0.8em}body[data-status]:after{content:"Status: "attr(data-status);display:block;font-size:0.8em;padding:16px 0;width:8px;height:8px;text-align:center;color:white;display:flex;align-items:center;justify-content:center;position:absolute;bottom:0;width:100%;left:0;box-sizing:border-box;background:#f44336}body[data-status="1"]:after{background:#4caf50}body[data-status="1"] #container button{opacity:1}#container button{opacity:0.5}</style>
  </head>
  <body>
    <div id="container"></div>
    <div id="controls">
      <button id="make">Add button</button>
      <button id="reset">Reset buttons</button>
    </div>
    <p>Type on your keyboard or add buttons to control the PS/2</p>
    <script>const map=getMap();let ws=null;let hostname="192.168.1.150";connect();init();function init(){const buttons=(localStorage.getItem("buttons")||"F12").split(",");buttons.forEach(makeButton)}function makeButton(text){const b=document.createElement("button");b.textContent=text;document.querySelector("#container").appendChild(b)}function connect(){ws=new WebSocket(`ws://${ hostname }:81/`);ws.binaryType="arraybuffer";document.body.dataset.status=ws.readyState;let heartbeat=null;ws.onopen=()=>{document.body.dataset.status=ws.readyState;heartbeat=setInterval(()=>{ws.send(Uint8Array.of(0))},1000)};ws.onclose=()=>{document.body.dataset.status=ws.readyState;ws=null;console.log("connection closed");clearTimeout(heartbeat);setTimeout(()=>connect(),500)};ws.onerror=(evt)=>{document.body.dataset.status=ws.readyState;console.log(evt)};ws.onmessage=function(evt){const data=new Uint8Array(evt.data);if(data.length===0&&data[0]===0){console.log("heartbeat")}else{console.log(evt.data.byteLength,new Uint8Array(evt.data))}}}document.querySelector("#make").addEventListener("click",(e)=>{const chr=prompt("Which key?");const code=chr.toUpperCase().trim();if(!map[code]){alert(`${ code } isn't available`);return}if(!confirm(`Adding ${ code }, correct?`)){return}const buttons=(localStorage.getItem("buttons")||"F12").split(",");buttons.push(code);localStorage.setItem("buttons",buttons.join(","));makeButton(code)});document.querySelector("#reset").addEventListener("click",()=>{localStorage.setItem("buttons","");window.location.reload()});document.addEventListener("click",(e)=>{if(e.target.nodeName!=="BUTTON"){return}if(e.target.id){return;}const code=e.target.textContent;send(code)});document.addEventListener("keydown",(e)=>{send(e.code,"make")});document.addEventListener("keyup",(e)=>{send(e.code,"brk")});function send(code,mode="both"){if(code.startsWith("Key")){code=code.substr(3)}if(code.startsWith("Arrow")){code=code.substr(5)}if(code.startsWith("Digit")){code=code.substr(5)}const key=map[code.toUpperCase()];if(!key){return}let codes;if(mode==="both"){codes=[...key.make.split(",").map((_)=>parseInt(_,16)),...key.brk.split(",").map((_)=>parseInt(_,16))]}else{codes=key[mode].split(",").map((_)=>parseInt(_,16))}console.log(key,codes);ws.send(Uint8Array.from(codes))}function getMap(){return{0:{make:"45",brk:"F0,45"},1:{make:"16",brk:"F0,16"},2:{make:"1E",brk:"F0,1E"},3:{make:"26",brk:"F0,26"},4:{make:"25",brk:"F0,25"},5:{make:"2E",brk:"F0,2E"},6:{make:"36",brk:"F0,36"},7:{make:"3D",brk:"F0,3D"},8:{make:"3E",brk:"F0,3E"},9:{make:"46",brk:"F0,46"},A:{make:"1C",brk:"F0,1C"},BRACKETLEFT:{make:"54",brk:"F0,54"},B:{make:"32",brk:"F0,32"},BACKQUOTE:{make:"0E",brk:"F0,0E"},INSERT:{make:"E0,70",brk:"E0,F0,70"},C:{make:"21",brk:"F0,21"},MINUS:{make:"4E",brk:"F0,4E"},HOME:{make:"E0,6C",brk:"E0,F0,6C"},D:{make:"23",brk:"F0,23"},EQUAL:{make:"55",brk:"F0,55"},PAGEUP:{make:"E0,7D",brk:"E0,F0,7D"},E:{make:"24",brk:"F0,24"},BACKSLASH:{make:"5D",brk:"F0,5D"},DELETE:{make:"E0,71",brk:"E0,F0,71"},F:{make:"2B",brk:"F0,2B"},BACKSPACE:{make:"66",brk:"F0,66"},END:{make:"E0,69",brk:"E0,F0,69"},G:{make:"34",brk:"F0,34"},SPACE:{make:"29",brk:"F0,29"},PAGEDOWN:{make:"E0,7A",brk:"E0,F0,7A"},H:{make:"33",brk:"F0,33"},TAB:{make:"0D",brk:"F0,0D"},UP:{make:"E0,75",brk:"E0,F0,75"},I:{make:"43",brk:"F0,43"},CAPS:{make:"58",brk:"F0,58"},LEFT:{make:"E0,6B",brk:"E0,F0,6B"},J:{make:"3B",brk:"F0,3B"},SHIFTLEFT:{make:"12",brk:"F0,12"},DOWN:{make:"E0,72",brk:"E0,F0,72"},K:{make:"42",brk:"F0,42"},CONTROLLEFT:{make:"14",brk:"F0,14"},RIGHT:{make:"E0,74",brk:"E0,F0,74"},L:{make:"4B",brk:"F0,4B"},LGUI:{make:"E0,1F",brk:"E0,F0,1F"},NUM:{make:"77",brk:"F0,77"},M:{make:"3A",brk:"F0,3A"},ALTLEFT:{make:"11",brk:"F0,11"},"KP/":{make:"E0,4A",brk:"E0,F0,4A"},N:{make:"31",brk:"F0,31"},SHIFTRIGHT:{make:"59",brk:"F0,59"},"KP*":{make:"7C",brk:"F0,7C"},O:{make:"44",brk:"F0,44"},CONTROLRIGHT:{make:"E0,14",brk:"E0,F0,14"},"KP-":{make:"7B",brk:"F0,7B"},P:{make:"4D",brk:"F0,4D"},RGUI:{make:"E0,27",brk:"E0,F0,27"},"KP+":{make:"79",brk:"F0,79"},Q:{make:"15",brk:"F0,15"},ALTRIGHT:{make:"E0,11",brk:"E0,F0,11"},KPEN:{make:"E0,5A",brk:"E0,F0,5A"},R:{make:"2D",brk:"F0,2D"},APPS:{make:"E0,2F",brk:"E0,F0,2F"},"KP.":{make:"71",brk:"F0,71"},S:{make:"1B",brk:"F0,1B"},ENTER:{make:"5A",brk:"F0,5A"},KP0:{make:"70",brk:"F0,70"},T:{make:"2C",brk:"F0,2C"},ESC:{make:"76",brk:"F0,76"},KP1:{make:"69",brk:"F0,69"},U:{make:"3C",brk:"F0,3C"},F1:{make:"05",brk:"F0,05"},KP2:{make:"72",brk:"F0,72"},V:{make:"2A",brk:"F0,2A"},F2:{make:"06",brk:"F0,06"},KP3:{make:"7A",brk:"F0,7A"},W:{make:"1D",brk:"F0,1D"},F3:{make:"04",brk:"F0,04"},KP4:{make:"6B",brk:"F0,6B"},X:{make:"22",brk:"F0,22"},F4:{make:"0C",brk:"F0,0C"},KP5:{make:"73",brk:"F0,73"},Y:{make:"35",brk:"F0,35"},F5:{make:"03",brk:"F0,03"},KP6:{make:"74",brk:"F0,74"},Z:{make:"1A",brk:"F0,1A"},F6:{make:"0B",brk:"F0,0B"},KP7:{make:"6C",brk:"F0,6C"},F7:{make:"83",brk:"F0,83"},KP8:{make:"75",brk:"F0,75"},F8:{make:"0A",brk:"F0,0A"},KP9:{make:"7D",brk:"F0,7D"},F9:{make:"01",brk:"F0,01"},BRACKETRIGHT:{make:"5B",brk:"F0,5B"},F10:{make:"09",brk:"F0,09"},SEMICOLON:{make:"4C",brk:"F0,4C"},F11:{make:"78",brk:"F0,78"},QUOTE:{make:"52",brk:"F0,52"},F12:{make:"07",brk:"F0,07"},COMMA:{make:"41",brk:"F0,41"},F13:{make:"E0,12,E0,7C",brk:"E0,F0,7C,E0,F0,12"},PERIOD:{make:"49",brk:"F0,49"},SCROLL:{make:"7E",brk:"F0,7E"},SLASH:{make:"4A",brk:"F0,4A"},PAUSE:{make:"E1,14,77,E1,F0,14,F0,77",brk:""}}}</script>
  </body>
</html>
)rawliteral";


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        // Send the current LED status
        webSocket.sendTXT(num, "ACK", 3);
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\r\n", num, payload);
      // send data to all connected clients
      webSocket.broadcastTXT(payload, length);
      break;
    case WStype_BIN:

      if (length == 1 && payload[0] == 0) {
        Serial.printf("[%u] heartbeat\r\n", num);
        webSocket.sendBIN(num, payload, length);
        break;
      }
    
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      digitalWrite(BUILTIN_LED, LOW);
      
      for (int i = 0; i < length; i++)
      {
        keyboard.write(payload[i]);
        delay(10); // 10ms is too fast for things like F12
      }
      digitalWrite(BUILTIN_LED, HIGH);
      break;
    case WStype_PONG:
      Serial.printf("[%u] got pong\r\n", num);
      break;
    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
  }
}

void handleRoot()
{
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void initButtons() {
  for (int i = 0; i < btn_length; i++) {
    pinMode(buttons[i], INPUT);
    // initilise the pull-up on the button pin
    digitalWrite(buttons[i], HIGH);
  }
}

void setup()
{
  Serial.begin(115200);

  initButtons();

  pinMode(BUILTIN_LED, OUTPUT);

  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);

  //Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for(uint8_t t = 2; t > 0; t--) {
    Serial.printf("[SETUP] BOOT WAIT %d...\r\n", t);
    Serial.flush();
    blink();
    delay(1000);
  }

  WiFiMulti.addAP(ssid, password);

  while(WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (mdns.begin("espWebSock", WiFi.localIP())) {
    Serial.println("MDNS responder started");
    mdns.addService("http", "tcp", 80);
    mdns.addService("ws", "tcp", 81);
  }
  else {
    Serial.println("MDNS.begin failed");
  }
  Serial.print("Connect to http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // LED: LOW = on, HIGH = off
  Serial.println("Boot signal");
  for (int i = 0; i < 10; i++)
  {
    blink();
  }
}

void blink() {
  digitalWrite(BUILTIN_LED, LOW);
  delay(100);
  digitalWrite(BUILTIN_LED, HIGH);
  delay(100);
}

unsigned long last_10sec = 0;
unsigned int counter = 0;

int getButtonDown() {
  int state[btn_length];
  int changed = -1;
  int i = 0;
  for (i = 0; i < btn_length; i++) {
    state[i] = digitalRead(buttons[i]);
  }
  delay(DEBOUNCE_LENGTH);
  for (i = 0; i < btn_length; i++) {
    if (state[i] == digitalRead(buttons[i])) {
      if (state[i] != last[i]) {
        last[i] = state[i];
        if (state[i] == LOW) {
          changed = i;
          return changed;
        }
      }
    }
  }

  return changed;
}

void sendF12() {
  digitalWrite(BUILTIN_LED, LOW);
  keyboard.write(0x7);
  delay(50);
  keyboard.write(0xF0);
  delay(50);
  keyboard.write(0x7);
  digitalWrite(BUILTIN_LED, HIGH);
}

void sendScroll() {
  digitalWrite(BUILTIN_LED, LOW);
  keyboard.write(0x7E);
  delay(50);
  keyboard.write(0xF0);
  delay(50);
  keyboard.write(0x7E);
  digitalWrite(BUILTIN_LED, HIGH);
}


void loop()
{
  unsigned long t = millis();
  webSocket.loop();
  server.handleClient();

  int button = getButtonDown();

  if (button == 0) {
    Serial.println("Key 1: SCROLL");
    sendScroll();
  }
  if (button == 1) {
    Serial.println("Key 2: F12");
    sendF12();
  }

  if((t - last_10sec) > 10 * 1000) {
    counter++;
    bool ping = (counter % 2);
    int i = webSocket.connectedClients(ping);
    last_10sec = millis();
  }
}
