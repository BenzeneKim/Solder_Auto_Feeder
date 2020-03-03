/**
 * Author Teemu Mäntykallio
 * Initializes the library and turns the motor in alternating directions.
*/
#include <math.h>
#define EN_PIN    7  // Nano v3:  16 Mega:  38  //enable (CFG6)
#define DIR_PIN   5  //      19      55  //direction
#define STEP_PIN  4  //      18      54  //step
#define CS_PIN    6  //      17      64  //chip select

extern volatile unsigned long timer0_millis;

bool dir = true;

#include <TMC2130Stepper.h>
TMC2130Stepper driver = TMC2130Stepper(EN_PIN, DIR_PIN, STEP_PIN, CS_PIN);

void setup() {
	Serial.begin(9600);
	while(!Serial);
	Serial.println("Start...");
	SPI.begin();
	pinMode(MISO, INPUT_PULLUP);
	driver.begin();       // Initiate pins and registeries
	driver.rms_current(600);  // Set stepper current to 600mA. The command is the same as command TMC2130.setCurrent(600, 0.11, 0.5);
	driver.stealthChop(1);  // Enable extremely quiet stepping
	digitalWrite(EN_PIN, LOW);
	driver.shaft_dir(1);
}

void loop() {
	while(!Serial.available());
	float getData = getSerial();
	//Serial.println(getData);
	unsigned int speedVal = (int)((int)floor(getData / 1000) % 100);
	int dirVal = (int)floor(getData / 100000);
	int timeVal = (int)fmod(getData, 1000);
	speedVal = 100 - speedVal;
	//Serial.println(speedVal);
	//Serial.println(dirVal);
	//Serial.println(timeVal);
	if (99000 < getData && getData < 400000 && speedVal < 30){ // prevent the error(speed under 70% isn't useful)
		if(dirVal < 2){
			Serial.println("auto");
			if (dirVal == 1){ driver.shaft_dir(0);}
			else{ driver.shaft_dir(1);}
			//delay(10);
			timer0_millis = 0;
			uint32_t ms = millis();
			while (ms <= 10*timeVal && speedVal < 30){
				ms = millis();
				for (int i = 0; i < 1000; i++){
					digitalWrite(STEP_PIN, HIGH);
					delayMicroseconds(speedVal);
					digitalWrite(STEP_PIN, LOW);
					delayMicroseconds(speedVal);    
  				}

			}
		}
		else{
			Serial.println("manual");
			if(dirVal == 2)	driver.shaft_dir(0);	
			else driver.shaft_dir(1);
			while(!Serial.available() && speedVal < 30){
				digitalWrite(STEP_PIN, HIGH);
				delayMicroseconds(speedVal);
				digitalWrite(STEP_PIN, LOW);
				delayMicroseconds(speedVal);
			}
			Serial.read();
			Serial.read();
			delay(250);
		}
	}
}

float getSerial(){
	float val;
	if(Serial.available()){
		val = Serial.parseFloat();
		Serial.read();
		Serial.read();
	}
	return val;
}
