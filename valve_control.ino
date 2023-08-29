#define MOTOR_PIN 2

enum Status {
  ON = 1, OFF = 0
};

struct Valve {
  const int pin;
  Status state;
  int durationOpenMinutes;
};

Valve valves[] = {
  {3, OFF, 1800},
  {4, OFF, 1800},
  {5, OFF, 1200},
  {6, OFF, 1800}
};

int currentValve = 0;
int numberOfValves = sizeof(valves) / sizeof(Valve);
unsigned long irrigationStartTime = 0;
unsigned long valveOpenTime = 0;
int irrigationPeriodMinutes = 18000;
Status irrigationStatus = OFF;

void setup() {
  pinMode(MOTOR_PIN, OUTPUT);
  for (int i = 0; i < numberOfValves; i++) { // set all control pins as output
    pinMode(valves[i].pin, OUTPUT);
  }

  stopMotor();
  for (int i = 0; i < numberOfValves; i++) { // close all valves
    closeValve(i);
  }
  
  Serial.begin(9600);
}

void loop() {

  unsigned long timeElapsed = millis() - irrigationStartTime;
  if (timeElapsed > irrigationPeriodMinutes) { // irrigation start
    startIrrigation();
  }

  if (irrigationStatus == ON) {
    if (valves[currentValve].state == OFF) {
      openValve(currentValve);
    }
    timeElapsed = millis() - valveOpenTime;
    if (timeElapsed > valves[currentValve].durationOpenMinutes) {
      closeValve(currentValve);
      currentValve++; // proceed to next valve
      if (currentValve == numberOfValves) {
        stopIrrigation();
      }
    }
  }

  printValvesStatus();
  delay(1000);

  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input = input.substring(0, input.length() - 1); // strip carriage return
    int newStatus = input.toInt();
    switch (newStatus) {

      case ON:
        if (irrigationStatus == OFF) {
          Serial.println("setting status to: ON");
          startIrrigation();
        }
        break;

      case OFF:
        if (irrigationStatus == ON) {
          Serial.println("setting status to: OFF");
          closeValve(currentValve);
          stopIrrigation();
        }
        break;

      default:
        Serial.println("invalid command");

    }
  }
}

void startIrrigation() {
  irrigationStartTime = millis();
  startMotor();
  irrigationStatus = ON;
  currentValve = 0;
}

void stopIrrigation() {
  stopMotor();
  irrigationStatus = OFF;
}

void startMotor() {
  digitalWrite(MOTOR_PIN, HIGH);
}

void stopMotor() {
  digitalWrite(MOTOR_PIN, LOW);
}

void openValve(int index) {
  digitalWrite(valves[index].pin, HIGH);
  valves[index].state = ON;
  valveOpenTime = millis();
}

void closeValve(int index) {
  digitalWrite(valves[index].pin, LOW);
  valves[index].state = OFF;
}

void printValvesStatus() {
  String output = "";
  for (int i = 0; i < numberOfValves; i++) {
    String status = valves[i].state == ON ? "ON" : "OFF";
    output = output + status + ",";
  }
  Serial.println(output.substring(0, output.length() - 1));
}
