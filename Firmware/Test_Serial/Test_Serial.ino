int button = 2;
float a = 290000;
float b = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(button, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print(a,0);
  delay(5000);
  Serial.print(a, 0);
  delay(3000);
}
