#define PLACE_NAME "kipos"
#define MOTOR_PIN 2

enum Status {
  ON = 1, OFF = 0
};

struct Valve {
  const int pin;
  Status state;
};


struct Stop {
  int valveIndex;
  int durationOpenMinutes;
};

Valve valves[] = {
  {3, OFF},
  {4, OFF},
  {5, OFF},
  {6, OFF}
};


int numberOfValves = sizeof(valves) / sizeof(Valve);
unsigned long irrigationStartTime = 0;
unsigned long valveOpenTime = 0;
int irrigationPeriodMinutes = 18000;
Status irrigationStatus = OFF;
Stop stops[16] = {
  {0, 2800},
  {1, 2800},
  {2, 1200},
  {3, 3800}
};
int numberOfStops = 4;
int currentStop = 0;

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

  int currentValve = stops[currentStop].valveIndex;
  if (irrigationStatus == ON) {
    if (valves[currentValve].state == OFF) {
      openValve(currentValve);
    }
    timeElapsed = millis() - valveOpenTime;
    if (timeElapsed > stops[currentStop].durationOpenMinutes) {
      closeValve(currentValve);
      currentStop++; // proceed to next valve
      if (currentStop == numberOfStops) {
        stopIrrigation();
      }
    }
  }

  printStatus();
  delay(1000);

  if (Serial.available() > 0) {
    int newStatus = parseInput();
    switch (newStatus) {

      case ON:
        if (irrigationStatus == OFF) {
          startIrrigation();
        }
        break;

      case OFF:
        if (irrigationStatus == ON) {
          int currentValve = stops[currentStop].valveIndex;
          closeValve(currentValve);
          stopIrrigation();
        }
        break;

      default:
        Serial.println("invalid command");

    }
  }
}

int parseInput() {
    String tokens[16];
    String input = Serial.readStringUntil('\n');
    input = input.substring(0, input.length() - 1); // strip carriage return
    int index = 0;
    int previousSpaceIndex = 0;
    int spaceIndex = input.indexOf(' ');
    while (spaceIndex != -1) {
      tokens[index] = input.substring(previousSpaceIndex, spaceIndex);
      previousSpaceIndex = spaceIndex;
      spaceIndex = input.indexOf(' ', spaceIndex + 1);
      index++;
    }
    tokens[index] = input.substring(previousSpaceIndex);
    index++;
    char command = tokens[0].charAt(0);
    int stopIndex = 0;
    for (int i = 1; i < index; i = i + 2) {
       stops[stopIndex].valveIndex = tokens[i].toInt() - 1; // valve 1 is at index 0 and so on...
       stops[stopIndex].durationOpenMinutes = tokens[i + 1].toInt();
       stopIndex++;
    }
    if (stopIndex > 0) {
      numberOfStops = stopIndex;
    }
    return command - '0';
}

void startIrrigation() {
  irrigationStartTime = millis();
  startMotor();
  irrigationStatus = ON;
  currentStop = 0;
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

void printStatus() {
  if (irrigationStatus == ON) {
    Serial.println(currentStop + 1);
  } else {
    Serial.println(-1);
  }
}
