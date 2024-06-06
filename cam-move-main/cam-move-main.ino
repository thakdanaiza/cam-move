#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "rainawifi_2.4G";
const char* password = "map38bok";

// const char* ssid = "RAZERBLADE";
// const char* password = "091707fifa";

//Your Domain name with URL path or IP address with path
const char* serverNameGet = "http://10.22.1.81:1880/get-move";
const char* serverNameSent = "http://10.22.1.81:1880/update-move";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
unsigned long terminate = 0;
unsigned long timerDelay = 5000;
unsigned long timernet = 30000;
unsigned long timeOut = 120000;

String sensorReadings;
float sensorReadingsArr[3];

String nodeName = "Master1";  // Name needs to be unique

int i = 0;
int count = 0;
int net = 0;
int wf = 0;
int sumSend = 0;

void feedback() {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverNameSent);
  http.addHeader("Content-Type", "application/json");
  // String testString;
  String testString = "{\"message\":\"done\"}";
  int httpResponseCode = http.POST(testString);
  // Serial.print("msg = ");
  // Serial.println(testString);
  // Serial.print("HTTP Response code posting: ");
  // Serial.println(httpResponseCode);
  http.end();
}
void move() {
  Serial.println("mode 1 step");
}
void go_home() {
  Serial.println("go_home");
}

void go_max() {
  Serial.println("go_max");
}
void go_min() {
  Serial.println("go_min");
}
void setup() {
  Serial.begin(115200);
  Serial.print("setup-done");
}

void loop() {
  if (millis() >= timernet && wf == 0) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(10);
    }
    wf = 1;
    Serial.print("wifi-setup-done");
  }
  // Send an HTTP GET request every 5 seconds
  if ((millis() - lastTime) > timerDelay && wf == 1) {
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      net = 1;
      sensorReadings = httpGETRequest(serverNameGet);
      // Serial.println(sensorReadings);
      if (sensorReadings.equals("move")) {
        move();
        feedback();
      } else if (sensorReadings.equals("max")) {
        go_max();
        feedback();
      } else if (sensorReadings.equals("min")) {
        go_min();
        feedback();
      } else if (sensorReadings.equals("home")) {
        go_home();
        feedback();
      }
    } else {
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(10);
      }
      Serial.println("WiFi Disconnected");
      // timernet = millis()+30000;
      if (net == 1) {
        ESP.restart();
      }
    }
    lastTime = millis();
  }
}

String httpGETRequest(const char* serverNameGet) {
  WiFiClient client;
  HTTPClient http;

  // Your Domain name with URL path or IP address with path
  http.begin(client, serverNameGet);

  // If you need Node-RED/server authentication, insert user and password below
  //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "";

  if (httpResponseCode > 0) {
    // Serial.print("HTTP Response code getting: ");
    // Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code getting: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
