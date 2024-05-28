#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>

// Replace with your network credentials
const char* ssid = "yongjer_phone";
const char* password = "0963019917";

// Motor 1 pins
int enablePin1 = 5; // D1
int motor1Pin1 = 4; // D2
int motor1Pin2 = 0; // D3

// Motor 2 pins
int motor2Pin1 = 14; // D5
int motor2Pin2 = 12; // D6
int enablePin2 = 13; // D7

// LED pin
int ledPin = 15;
bool ledState = false;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if(info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = String((char*)data);
    
    if(message == "forward") {
      digitalWrite(enablePin1, HIGH);
      digitalWrite(motor1Pin1, HIGH);
      digitalWrite(motor1Pin2, LOW);
      digitalWrite(enablePin2, HIGH);
      digitalWrite(motor2Pin1, HIGH);
      digitalWrite(motor2Pin2, LOW);
    } else if(message == "backward") {
      digitalWrite(enablePin1, HIGH);
      digitalWrite(motor1Pin1, LOW);
      digitalWrite(motor1Pin2, HIGH);
      digitalWrite(enablePin2, HIGH);
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, HIGH);
    } else if(message == "left") {
      digitalWrite(enablePin1, HIGH);
      digitalWrite(motor1Pin1, LOW);
      digitalWrite(motor1Pin2, HIGH);
      digitalWrite(enablePin2, HIGH);
      digitalWrite(motor2Pin1, HIGH);
      digitalWrite(motor2Pin2, LOW);
    } else if(message == "right") {
      digitalWrite(enablePin1, HIGH);
      digitalWrite(motor1Pin1, HIGH);
      digitalWrite(motor1Pin2, LOW);
      digitalWrite(enablePin2, HIGH);
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, HIGH);
    } else if(message == "stop") {
      digitalWrite(enablePin1, LOW);
      digitalWrite(motor1Pin1, LOW);
      digitalWrite(motor1Pin2, LOW);
      digitalWrite(enablePin2, LOW);
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, LOW);
    } else if(message == "toggle_led") {
      ledState = !ledState;
      digitalWrite(ledPin, ledState ? HIGH : LOW);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if(type == WS_EVT_DATA) {
    handleWebSocketMessage(arg, data, len);
  }
}

void setup() {
  Serial.begin(115200);
  
  // Set motor pins as outputs
  pinMode(enablePin1, OUTPUT);
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enablePin2, OUTPUT);
  
  // Set LED pin as output
  pinMode(ledPin, OUTPUT);

  // Initialize motor pins to LOW
  digitalWrite(enablePin1, LOW);
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
  digitalWrite(enablePin2, LOW);
  
  // Initialize LED pin to LOW
  digitalWrite(ledPin, LOW);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to the WiFi network");
  Serial.println(WiFi.localIP());

  // Set up WebSocket
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // Serve the HTML file
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", R"rawliteral(
      <html>
      <head>
        <title>Robot Car Control</title>
        <style>
          body { font-family: Arial, sans-serif; text-align: center; padding: 50px; }
          h1 { color: #333; }
          button { width: 100px; height: 50px; margin: 5px; font-size: 18px; cursor: pointer; border: none; border-radius: 5px; background-color: #4CAF50; color: white; }
          button:hover { background-color: #45a049; }
          .button-container { display: flex; justify-content: center; flex-direction: column; align-items: center; }
          .row { display: flex; }
        </style>
      </head>
      <body>
        <h1>Robot Car Control</h1>
        <div class='button-container'>
          <div class='row'>
            <button onclick="sendAction('forward')">Forward</button>
          </div>
          <div class='row'>
            <button onclick="sendAction('left')">Left</button>
            <button onclick="sendAction('stop')">Stop</button>
            <button onclick="sendAction('right')">Right</button>
          </div>
          <div class='row'>
            <button onclick="sendAction('backward')">Backward</button>
          </div>
          <div class='row'>
            <button onclick="sendAction('toggle_led')">Toggle LED</button>
          </div>
        </div>
        <script>
          var websocket;
          window.addEventListener('load', () => {
            websocket = new WebSocket('ws://' + window.location.hostname + '/ws');
            websocket.onopen = () => console.log('WebSocket connection opened');
            websocket.onclose = () => console.log('WebSocket connection closed');
            websocket.onerror = error => console.error('WebSocket error:', error);
          });
          function sendAction(action) {
            websocket.send(action);
          }
        </script>
      </body>
      </html>
    )rawliteral");
  });

  // Start server
  server.begin();
}

void loop() {
  ws.cleanupClients();
}