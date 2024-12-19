#include <WiFi.h>
#include <MQTT.h>

#define in1  12
#define in2  14
#define in3  27
#define in4  26

const char ssid[] = "iPhone T";
const char pass[] = "12345678";


const char mqtt_broker[] = "test.mosquitto.org";
const char mqtt_publish_topic[] = "group4/command";
const char mqtt_subscribe_topic[] = "group4/command";
const char mqtt_client_id[] = "mqtt-tiles-1.17.0-e4650135";
int MQTT_PORT = 1883;

const int stepsPerRevolution = 2048; 
const int stepDelay = 2; 

int stepState = 0;
float currentAngle = 0.0; 

WiFiClient net;
MQTTClient client;

void connect();
void singleStep(bool forward);
void stepForward(int steps);
void stepBackward(int steps);
void moveToAngle(float targetAngle);
void moveToItem(int target);
void messageReceived(String &topic, String &payload);

void setup() {
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  client.begin(mqtt_broker, MQTT_PORT, net);
  client.onMessage(messageReceived);

  connect();
}

void loop() {
  client.loop();
  delay(10);

  if (!client.connected()) {
    connect();
  }
  
  delay(1000);
}

void connect() {
  Serial.print("Checking WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nWiFi connected!");
  Serial.print("Connecting to MQTT...");
  while (!client.connect(mqtt_client_id)) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nMQTT connected!");
  client.subscribe(mqtt_subscribe_topic);
}

void messageReceived(String &topic, String &payload) {
  Serial.print("Message received: ");
  Serial.println(payload);

  int target = payload.toInt();

  if (target >= 1 && target <= 4) {
    Serial.print("Moving to [");
    Serial.print(target);
    Serial.println("]...");
    moveToItem(target);
    Serial.println("Movement complete.");
  } else {
    Serial.println("Invalid input. Please send a value between 1 and 4.");
  }
}

void singleStep(bool forward) {
  static const int stepSequence[4][4] = {
    {HIGH, LOW, LOW, LOW},  // Step 1
    {LOW, HIGH, LOW, LOW},  // Step 2
    {LOW, LOW, HIGH, LOW},  // Step 3
    {LOW, LOW, LOW, HIGH}   // Step 4
  };

  stepState = (forward) ? (stepState + 1) % 4 : (stepState + 3) % 4;

  digitalWrite(in1, stepSequence[stepState][0]);
  digitalWrite(in2, stepSequence[stepState][1]);
  digitalWrite(in3, stepSequence[stepState][2]);
  digitalWrite(in4, stepSequence[stepState][3]);

  delay(stepDelay);
}

void stepForward(int steps) {
  for (int i = 0; i < steps; i++) {
    singleStep(true);
  }
}

void stepBackward(int steps) {
  for (int i = 0; i < steps; i++) {
    singleStep(false);
  }
}

void moveToAngle(float targetAngle) {
  float angleDifference = targetAngle - currentAngle;
  if (angleDifference > 180) angleDifference -= 360;
  if (angleDifference < -180) angleDifference += 360;

  int steps = (int)(stepsPerRevolution * (angleDifference / 360.0));
  if (steps > 0) {
    stepForward(steps);
  } else if (steps < 0) {
    stepBackward(-steps);
  }

  currentAngle = targetAngle;
}

void moveToItem(int target){
  if (target == 1) {
    moveToAngle(0);
  } else if (target == 2) {
    moveToAngle(90);
  } else if (target == 3) {
    moveToAngle(180);
  } else if (target == 4) {
    moveToAngle(270);
  } else {
    Serial.println("Invalid! Please enter a number between 1 - 4.");
  }
}
