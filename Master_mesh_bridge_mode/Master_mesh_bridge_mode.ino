#include "namedMesh.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJSON.h>
#include <HardwareSerial.h>

#define MESH_SSID "Robotechfarm"
#define MESH_PASSWORD "Robotechfarm1234"
#define MESH_PORT 5555

// const char* ssid = "AWIFI";
// const char* password = "p1234wifi";

// const char* ssid = "RAZERBLADE";
// const char* password = "091707fifa";

const char* ssid = "rainawifi_2.4G";
const char* password = "map38bok";
//map38bok
// const char* ssid = "JKRowling";
// const char* password = "12345678";

// const char* ssid = "DESKTOPFIFA";
// const char* password = "091707fifa";

//Your Domain name with URL path or IP address with path
const char* serverNameGet = "http://10.22.1.81:1880/get-sensor";
const char* serverNameSent = "http://10.22.1.81:1880/update-sensor";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
unsigned long terminate = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
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

Scheduler userScheduler;  // to control your personal task
namedMesh mesh;

// User stub
// void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage(TASK_SECOND * 8, TASK_FOREVER, []() {
  // String msg = String("This is a message from: ") + nodeName + String(" for all nodes");
  String num = String("1231234");
  String in = "Node";
  // String msg = String("1");
  String now_read = in+num[count];
  mesh.sendSingle(now_read, now_read);
  // mesh.sendBroadcast(String(msg[count]));
  Serial.println(now_read);
  // mesh.sendBroadcast(String(msg));
  count++;
  if (count > 6) {
    count = 0;
    // if (sumSend <= 4){
    //   ESP.restart();
    // }
    sumSend = 0;
  }
});  // start with a one second interval

void setup() {
  Serial.begin(115200);
  // Serial2.begin(9600, SERIAL_8N1, 16, 17);

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.setName(nodeName);

  mesh.onReceive([](String& from, String& msg) {
    WiFiClient client;
    HTTPClient http;

    // Your Domain name with URL path or IP address with path
    http.begin(client, serverNameSent);

    // If you need Node-RED/server authentication, insert user and password below
    //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

    // Specify content-type header
    http.addHeader("Content-Type", "application/json");
    String testString;
    testString += msg;
    int httpResponseCode = http.POST(testString);
    Serial.print("msg = ");
    Serial.println(testString);
    Serial.print("HTTP Response code posting: ");
    Serial.println(httpResponseCode);

    // Free resources
    http.end();
    // testString = "";
    sumSend++;
    terminate = millis();
    // testString += ",";
    // i++;
    // if (i == 6) {  //must be change to 6 before use
    //   i = 0;
      // Serial.println(testString);
    //   testString = "";
    //   // memset(testString, NULL, sizeof(testString));
    // }
    // Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
  });
  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });
  mesh.stationManual(ssid, password);
  // mesh.setRoot(true);
  // // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  // mesh.setContainsRoot(true);
  userScheduler.addTask(taskSendMessage);
  // taskSendMessage.enable();
}

void loop() {
  // put your main code here, to run repeatedly:
  mesh.update();

  if(millis() >= timernet && wf == 0){
    mesh.stationManual(ssid, password);
    WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(10);
      }
    taskSendMessage.enable();
    wf = 1;
  }
  // Send an HTTP GET request every 5 seconds
  if ((millis() - lastTime) > timerDelay && wf == 1) {
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      net = 1;
      sensorReadings = httpGETRequest(serverNameGet);
      String out = "T-relay";
      Serial.println(sensorReadings);
      if (sensorReadings.equals("ESP_Reset.")){
        ESP.restart();
      }
      else if (String(sensorReadings[0]).equals("O")){
        mesh.sendSingle(out, sensorReadings); 
      }
      // if (sensorReadings.equals("Mega_Reset.")){
      //   pinMode(23, OUTPUT);
      //   digitalWrite(23, LOW);
      //   Serial.println("Resetting Mega...");
      //   digitalWrite(23, HIGH);
      // }
      // Serial2.print(sensorReadings);
      
      
    } 
    else {
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(10);
      }
      Serial.println("WiFi Disconnected");
      // timernet = millis()+30000;
      if (net == 1){
        ESP.restart();
      }
    }
    lastTime = millis();
  }

  // if (millis() - terminate > timeOut) {
  //   ESP.restart();
  // }
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
    Serial.print("HTTP Response code getting: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code getting: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
