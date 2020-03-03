int stepPin = 7;
int dirPin = 9;
int enPin = 12;

void setup() {
  // put your setup code here, to run once:
  
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enPin, OUTPUT);
  Serial.begin(9600);
  digitalWrite(enPin, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  stepperMotorControl(0, 500, 250);
}

void stepperMotorControl(bool dirVal, int lengthVal, int speedVal){
  if (dirVal == 1) digitalWrite(dirPin, HIGH);
  if (dirVal == 0) digitalWrite(dirPin, LOW);
  
  lengthVal = lengthVal * 3; //lengthVal * ratio
  speedVal = speedVal * 3;   //speedVal * ratio

  for(int i = 0; i < lengthVal; i++){
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(speedVal);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(speedVal);
  }
  Serial.println("spin");

}
