#include <math.h>
#include <Servo.h>

// Servo setup for default values
int servoAngleX = 90;
int servoAngleY = 90;
int midpointX = 90;
int midpointY = 90;

// Setup for switching between manual and algorithmic arm movement
int SW = 2;
int SW_state_new = 0;
int SW_state_old = 1;
int SW_state = 0; // 0: manual; 1: algorithmic

// Setup for manual arm movement
Servo myservoX;
Servo myservoY;
int VRx = A0;
int VRy = A1;
// For servo calibration
const int XCenterVal = 533, YCenterVal = 484;  //  Values of pot input when centered
const int XpotSpeed = 75, YpotSpeed = 75;  // Scale value for controlling speed of movement (higher=slower)
const int errorAmount = 400;

// Setup for algorithic arm movement
int mode = 0; // 0: found close object; 1: finding close object
// For the swirl formula
const double inside = -9.96668;
double t = 0;
// For sonar distance calculation
const int echoPin = 6;
const int trigPin = 7;
int time = 0;

void setup() {  
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  Serial.begin(9600); // // Serial Communication is starting with 9600 of baudrate speed
  Serial.println("Ultrasonic Sensor HC-SR04 Test"); // print some text in Serial Monitor
  Serial.println("with Arduino UNO R3");

  pinMode(VRx, INPUT); // Joystick X
  pinMode(VRy, INPUT); // Joystick Y
  pinMode(SW, INPUT_PULLUP); // Joystick button

  myservoX.attach(9); // Output for first servo
  myservoY.attach(8); // Output for second servo
}

void loop()
{
  // Toggle for algorithmic or manual crane movement
  SW_state_new = digitalRead(SW);
  if (SW_state_new != SW_state_old) {
    if (SW_state_new == 1) {
      SW_state = !SW_state;
      
      // Algorithm begins at last crane location
      midpointX = servoAngleX;
      midpointY = servoAngleY;
    }
    SW_state_old = SW_state_new;
  }

  // Manual arm movement
  if (SW_state == 0) {
    int XpotVal = analogRead(VRx) - XCenterVal;
    int YpotVal = analogRead(VRy) - YCenterVal;
    if (abs(XpotVal) < errorAmount) { XpotVal = 0; }
    if (abs(YpotVal) < errorAmount) { YpotVal = 0; }
    servoAngleX = servoAngleX + (XpotVal/XpotSpeed);
    servoAngleY = servoAngleY + (YpotVal/YpotSpeed);
    if (servoAngleX > 180) { servoAngleX = 180; }
    if (servoAngleY > 180) { servoAngleY = 180; }
    if (servoAngleX < 0) { servoAngleX = 0; }
    if (servoAngleY < 0) { servoAngleY = 0; }
  }

  // Algorithmic arm movement
  if (SW_state == 1) {
    if (getSonarDistance() > 20 && mode == 0) {
      mode = 1;
      t = 25;
    } else if (getSonarDistance() < 20) {
      mode = 0;
      midpointX = servoAngleX;
      midpointY = servoAngleY;
    }
    if (mode == 1) {
      if (t > 0 && servoAngleX < 180 && servoAngleX > 0 && servoAngleY < 180 && servoAngleY > 0) {
        double r = 180 * exp(t/inside);
        servoAngleX = midpointX + r*cos(t);
        servoAngleY = midpointY + r*sin(t);
        t = t - 0.1;
      } else {
        t = 25;
        midpointX = 90;
        midpointY = 90;
        servoAngleX = 90;
        servoAngleY = 90;
      }
    }
  }

  // Set crane servo angles
  myservoX.write(servoAngleX);
  myservoY.write(servoAngleY);

  Serial.print(mode);
  Serial.print("; ");
  Serial.print(SW_state);
  Serial.print("; ");
  Serial.print(servoAngleX);
  Serial.print("; ");
  Serial.print(servoAngleY);
  Serial.print("; ");  
  Serial.println(getSonarDistance());
  
  delay(100);
}


// serial print variable type
void types(String a) { Serial.println("it's a String"); }
void types(int a) { Serial.println("it's an int"); }
void types(char *a) { Serial.println("it's a char*"); }
void types(float a) { Serial.println("it's a float"); }
void types(bool a) { Serial.println("it's a bool"); }


void printDouble( double val, unsigned int precision){
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

    Serial.print (int(val));  //prints the int part
    Serial.print("."); // print the decimal point
    unsigned int frac;
    if(val >= 0)
        frac = (val - int(val)) * precision;
    else
        frac = (int(val)- val ) * precision;
    Serial.println(frac,DEC) ;
} 


// Calculate the distance of the nearest object that sonar senses
int getSonarDistance() {
  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  int duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  int distance = distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  return distance;
}
