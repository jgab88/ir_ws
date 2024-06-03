#include <SPI.h>
#include <NativeEthernet.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(172, 16, 0, 110);
IPAddress server(172, 16, 0, 45);

const int PWM_PIN_LEFT = 2;
const int DIR_PIN_LEFT = 3;
const int PWM_PIN_RIGHT = 4;
const int DIR_PIN_RIGHT = 5;

EthernetClient ethClient;
PubSubClient client(ethClient);

float scaleFactor = 1.0; // Default scale factor
int leftSpeed = 0;
int rightSpeed = 0;

// Function prototype for updateMotorFrequency
void updateMotorFrequency(int pin, int speed);

void callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  String payloadStr = "";
  for (unsigned int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }

  if (topicStr == "IR/Controller/Left/Speed") {
    // Handle left speed
    Serial.print("Left Speed: ");
    Serial.println(payloadStr);
    int speed = payloadStr.toInt();

    if (speed >= 0) {
      digitalWrite(DIR_PIN_LEFT, HIGH);
    } else {
      digitalWrite(DIR_PIN_LEFT, LOW);
    }

    // Use the absolute value of speed for frequency calculation
    speed = abs(speed);
    leftSpeed = speed;
    updateMotorFrequency(PWM_PIN_LEFT, leftSpeed);
    
  } else if (topicStr == "IR/Controller/Right/Speed") {
    // Handle right speed
    Serial.print("Right Speed: ");
    Serial.println(payloadStr);
    int speed = payloadStr.toInt();

    if (speed >= 0) {
      digitalWrite(DIR_PIN_RIGHT, HIGH);
    } else {
      digitalWrite(DIR_PIN_RIGHT, LOW);
    }

    // Use the absolute value of speed for frequency calculation
    speed = abs(speed);
    rightSpeed = speed;
    updateMotorFrequency(PWM_PIN_RIGHT, rightSpeed);

  } else if (topicStr == "IR/Controller/Dir_mode") {
    // Handle direction mode
    Serial.print("Direction Mode: ");
    Serial.println(payloadStr);
    // Add your code to handle the direction mode here

  } else if (topicStr == "IR/Controller/Scale_factor") {
    // Handle scale factor
    Serial.print("Scale Factor: ");
    Serial.println(payloadStr);

    // Parse the payload as a float value (scale factor)
    scaleFactor = payloadStr.toFloat();
    updateMotorFrequency(PWM_PIN_LEFT, leftSpeed);
    updateMotorFrequency(PWM_PIN_RIGHT, rightSpeed);
  }
}

void updateMotorFrequency(int pin, int speed) {
  // Scale the speed value to frequency range
  int frequency = map(speed, 0, 1096, 0, 10000);
  // or using the mathematical formula
  // int frequency = (speed * 10000) / 1096;

  // Apply the scale factor to the frequency value
  frequency = frequency * scaleFactor;

  // Set the PWM frequency based on the scaled value
  analogWriteFrequency(pin, frequency);
}

void setup() {
  Serial.begin(115200);

  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac, ip);
  delay(1500); // Allow the hardware to sort itself out

  // Set the PWM pins as outputs
  pinMode(PWM_PIN_LEFT, OUTPUT);
  pinMode(DIR_PIN_LEFT, OUTPUT);
  pinMode(PWM_PIN_RIGHT, OUTPUT);
  pinMode(DIR_PIN_RIGHT, OUTPUT);

  // Set the initial PWM frequency and duty cycle
  analogWriteFrequency(PWM_PIN_LEFT, 1000);
  analogWriteFrequency(PWM_PIN_RIGHT, 1000);
  analogWrite(PWM_PIN_LEFT, 128);
  analogWrite(PWM_PIN_RIGHT, 128);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      client.subscribe("IR/Controller/Left/Speed");
      client.subscribe("IR/Controller/Right/Speed");
      client.subscribe("IR/Controller/Dir_mode");
      client.subscribe("IR/Controller/Scale_factor");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // No need to call any functions here since the PWM is generated automatically
}