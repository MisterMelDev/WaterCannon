#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <AccelStepper.h>
#include <FS.h>

/* Details of the WiFi network that will be created */
#define AP_SSID "Turret"
#define AP_PASS "turret-wifi#!"

#define PITCH_DIR_PIN D6
#define PITCH_STEP_PIN D5
#define YAW_DIR_PIN D8
#define YAW_STEP_PIN D7

#define RELAY_1_PIN D0
#define RELAY_2_PIN D1

AccelStepper pitchStepper = AccelStepper(1, PITCH_STEP_PIN, PITCH_DIR_PIN);
AccelStepper yawStepper = AccelStepper(1, YAW_STEP_PIN, YAW_DIR_PIN);

ESP8266WebServer server(80);
WebSocketsServer socketServer(81);

String receivedStr;

String getContentType(String filename);
bool handleFileRead(String path);

enum Animation {
  NONE,
  SWEEP_HORIZONTAL,
  SWEEP_VERTICAL,
  SWEEP_DIAGONAL_LR,
  SWEEP_DIAGONAL_RL,
  CIRCLE
};
Animation animation = NONE;

void setup() {
  Serial.begin(115200);
  SPIFFS.begin();

  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);

  pitchStepper.setMaxSpeed(600);
  yawStepper.setMaxSpeed(600);
  pitchStepper.setAcceleration(200);
  yawStepper.setAcceleration(200);

  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("AP started, connect to IP ");
  Serial.println(WiFi.softAPIP());

  server.onNotFound([]() {
    if(!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "404 Not Found");
    }
  });
  server.begin();
  Serial.println("HTTP server started");

  socketServer.begin();
  socketServer.onEvent(webSocketEvent);
  Serial.println("WebSocket server started");
}

String getContentType(String filename) {
  if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  return "text/plain";
}

bool handleFileRead(String path) {
  if(path.endsWith("/")) path += "index.html";
  if(SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    String contentType = getContentType(path);
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }

  return false;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      safeState();
      Serial.println("WebSocket disconnected");
      break;
    case WStype_CONNECTED:
      if(num != 0) {
        socketServer.sendTXT(num, "busy");
        socketServer.disconnect(num);
        return;
      }
      Serial.println("WebSocket connected");
      break;
    case WStype_TEXT:
      if(length < 0) {
        return;
      } 
      receivedStr = (char*) payload;

      if(payload[0] == 'j' && animation == NONE) {
        int dividerIndex = receivedStr.indexOf('/');
        int joyX = receivedStr.substring(1, dividerIndex).toInt();
        int joyY = receivedStr.substring(dividerIndex + 1).toInt();

        if(joyX > -25 && joyX < 25) {
          joyX = 0;
        }
        if(joyY > -25 && joyY < 25) {
          joyY = 0;
        }

        float pitchSpeed = ((float) joyX) * 1.5;
        float yawSpeed = ((float) joyY) * 1.5;

        setPitchSpeed(pitchSpeed);
        setYawSpeed(yawSpeed);
        return;
      }

      if(payload[0] == 'r') {
        int relayNum = receivedStr.substring(1, 2).toInt();
        bool enabled = receivedStr.substring(2) == "1";

        if(relayNum != 1 && relayNum != 2) {
          Serial.print("Invalid relay number #");
          Serial.println(relayNum);
          return;
        }

        Serial.print("Relay #");
        Serial.print(relayNum);
        Serial.print(": ");
        Serial.println(enabled ? "Enabled" : "Disabled");

        if(relayNum == 1) {
          digitalWrite(RELAY_1_PIN, enabled ? HIGH : LOW);
        } else if(relayNum == 2) {
          digitalWrite(RELAY_2_PIN, enabled ? HIGH : LOW);
        }
     
        return;
      }

      if(receivedStr.startsWith("ae") && animation == NONE) {
        String animationName = receivedStr.substring(2);

        Serial.print("Animation enabled: ");
        Serial.println(animationName);

        if(animationName == "sweep-horizontal") {
          animation = SWEEP_HORIZONTAL;
        } else if(animationName == "sweep-vertical") {
          animation = SWEEP_VERTICAL;
        } else if(animationName == "sweep-diagonal-lr") {
          animation = SWEEP_DIAGONAL_LR;
        } else if(animationName == "sweep-diagonal-rl") {
          animation = SWEEP_DIAGONAL_RL;
        } else if(animationName == "circle") {
          animation = CIRCLE;
        }

        startAnimation();
        return;
      }

      if(receivedStr.startsWith("ad")) {
        Serial.println("Animation disabled");
        animation = NONE;
        return;
      }
      
      break;
  }
}

void setPitchSpeed(int speedVal) {
  pitchStepper.setSpeed(-speedVal);
}

void setYawSpeed(int speedVal) {
  yawStepper.setSpeed(-speedVal);
}

void safeState() {
  setPitchSpeed(0);
  setYawSpeed(0);
}

void loop() {
  server.handleClient();
  socketServer.loop();
  
  if(animation == NONE) {
    pitchStepper.runSpeed();
    yawStepper.runSpeed();
  } else {
    loopAnimation();
  }
}
