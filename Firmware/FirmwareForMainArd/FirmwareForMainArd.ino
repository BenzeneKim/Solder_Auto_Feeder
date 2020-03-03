#include <SPI.h>

#define LCD_RST 8
#define ENC_BTN 4
#define ENC_A 2
#define ENC_B 3


#define LCD_DATA                1       // Data bit
#define LCD_COMMAND             0       // Command bit
#define LCD_CLEAR_SCREEN        0x01    // Clear screen
#define LCD_ADDRESS_RESET       0x02    // The address counter is reset
#define LCD_BASIC_FUNCTION      0x30    // Basic instruction set
#define LCD_EXTEND_FUNCTION     0x34    // Extended instruction set

int forBut = 5;
int bacBut = 6;
int stepPin = 7;
int dirPin = 9;

byte selectedItem = 1;
int8_t encState = 3;

int speedVal = 85;  //Personalize_default_setting
int timeVal = 75;   //Personalize_default_setting/
int statusVal;
float sendData;
// https://github.com/mathertel/RotaryEncoder/blob/master/RotaryEncoder.cpp
const int8_t KNOBDIR[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

// Write a byte to ST7920 in SPI mode
void ST7920_Write(boolean command, byte lcdData) {
	SPI.beginTransaction(SPISettings(200000UL, MSBFIRST, SPI_MODE3));
	digitalWrite(10, HIGH);
	SPI.transfer(command ? 0xFA : 0xF8);
	SPI.transfer(lcdData & 0xF0);
	SPI.transfer((lcdData << 4) & 0xF0);
	digitalWrite(10, LOW);
	SPI.endTransaction();
}

void ST7920_Init() {
	digitalWrite(LCD_RST, LOW);
	delay(100);
	digitalWrite(LCD_RST, HIGH);

	ST7920_Write(LCD_COMMAND, LCD_BASIC_FUNCTION);
	ST7920_Write(LCD_COMMAND, LCD_CLEAR_SCREEN);
	ST7920_Write(LCD_COMMAND, 0x06);
	ST7920_Write(LCD_COMMAND, 0x0C);
}

// enables or disables graphic mode
void ST7920_setGraphicMode(boolean enabled = true) {
	ST7920_Write(LCD_COMMAND, enabled ? LCD_EXTEND_FUNCTION : LCD_BASIC_FUNCTION);
	if (enabled) ST7920_Write(LCD_COMMAND, LCD_EXTEND_FUNCTION | 0x02); // Graphic ON
}

void ST7920_displayString(byte row, const char *p) {
	byte addr;
	switch (row) {
		case 1: addr = 0x90; break;
		case 2: addr = 0x88; break;
		case 3: addr = 0x98; break;
		default: addr = 0x80;
	}

	ST7920_Write(LCD_COMMAND, addr);
	byte length = strlen(p);
	for (byte i = 0; i < length; i++)
		ST7920_Write(LCD_DATA, *p++);
}

void ST7920_ClearGraphicMem() {
	for (byte x = 0; x < 16; x++)
		for (byte y = 0; y < 32; y++) {
			ST7920_Write(LCD_COMMAND, 0x80 | y);
			ST7920_Write(LCD_COMMAND, 0x80 | x);
			ST7920_Write(LCD_DATA, 0x00);
			ST7920_Write(LCD_DATA, 0x00);
		}
}

void ST7920_HighlightMenuItem(byte idx, boolean fill = true) {
	idx &= 0x03; // 4 rows only

	byte y = idx * 16;
	byte x_addr = 0x80;

	// adjust cooridinates and address
	if (y >= 32) {
		y -= 32;
		x_addr = 0x88;
	}

	for (byte x = 0; x < 8; x++) {
		ST7920_Write(LCD_COMMAND, 0x80 | y);
		ST7920_Write(LCD_COMMAND, x_addr | x);
		fill ? ST7920_Write(LCD_DATA, 0xFF) : ST7920_Write(LCD_DATA, 0x00);
		fill ? ST7920_Write(LCD_DATA, 0xFF) : ST7920_Write(LCD_DATA, 0x00);

		ST7920_Write(LCD_COMMAND, 0x80 | y + 15);
		ST7920_Write(LCD_COMMAND, x_addr | x);
		fill ? ST7920_Write(LCD_DATA, 0xFF) : ST7920_Write(LCD_DATA, 0x00);
		fill ? ST7920_Write(LCD_DATA, 0xFF) : ST7920_Write(LCD_DATA, 0x00);
	}

	for (byte y1 = y + 1; y1 < y + 15; y1++) {
		ST7920_Write(LCD_COMMAND, 0x80 | y1);
		ST7920_Write(LCD_COMMAND, x_addr);
		fill ? ST7920_Write(LCD_DATA, 0x80) : ST7920_Write(LCD_DATA, 0x00);
		ST7920_Write(LCD_DATA, 0x00);

		ST7920_Write(LCD_COMMAND, 0x80 | y1);
		ST7920_Write(LCD_COMMAND, x_addr + 7);
		ST7920_Write(LCD_DATA, 0x00);
		fill ? ST7920_Write(LCD_DATA, 0x01) : ST7920_Write(LCD_DATA, 0x00);
	}
}

// https://github.com/mathertel/RotaryEncoder/blob/master/RotaryEncoder.cpp
int getEncoder() {
	int pinA = digitalRead(ENC_A);
	int pinB = digitalRead(ENC_B);
	int8_t r = 0;

	int8_t state = pinA | (pinB << 1);

	if (state != encState) {
		r = KNOBDIR[state | (encState << 2)];
		if (state == 3) r >>= 2;
		encState = state;
	}
	return r;
}

void displayValue(int inputVal) {
	byte addr;
  String p;
  int leng;
	switch (selectedItem) {
		case 1: addr = 0x90 + 4; break;
		case 2: addr = 0x88 + 5; break;
		case 3: addr = 0x98 + 3; break;
	}
	if (selectedItem == 1){
		if (inputVal == 0){
			p = "  Auto";
		}
		else{
			p = "Manual";
		}
    leng = 6;
	}
	
	else if (selectedItem == 2){
		p = " " + (String)inputVal + "%";
    if(p.length() < 4) p = " " + p;
   leng = 4;
	}
	
	else{
		p = (String)inputVal + "0ms";
	  leng = 8;
    while(p.length() < leng) p = " " + p;
	}
	ST7920_Write(LCD_COMMAND, addr);
	for (byte i = 0; i < leng; i++) {
		ST7920_Write(LCD_DATA, p.charAt(i));
	}
}


void changeStatus(int enc){
	if (enc == 1 && statusVal == 0) statusVal = 1;
	else if(enc == -1 && statusVal == 1)statusVal = 0;
	displayValue(statusVal);
}

void changeSpeed(int enc){
	if (enc == 1 && speedVal < 99) speedVal += 1;
	else if (enc == -1 && speedVal > 1) speedVal -= 1;
	displayValue(speedVal);
}

void changeTime(int enc){
	if(enc == 1 && timeVal < 999) timeVal += 1;
	else if(enc == -1 && timeVal > 1) timeVal -= 1;
	displayValue(timeVal);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
	pinMode(LCD_RST, OUTPUT);
	pinMode(ENC_BTN, INPUT_PULLUP);
	pinMode(ENC_A, INPUT);
	pinMode(ENC_B, INPUT);

	pinMode(forBut, INPUT_PULLUP);
	pinMode(bacBut, INPUT_PULLUP);
	SPI.begin();
	pinMode(10, OUTPUT);
	digitalWrite(10, LOW);

	ST7920_Init();

	ST7920_setGraphicMode(true);
	ST7920_ClearGraphicMem();

	// highlight title
	for (byte y = 0; y < 15; y++)
		for (byte x = 0; x < 8; x++) {
			ST7920_Write(LCD_COMMAND, 0x80 | y);
			ST7920_Write(LCD_COMMAND, 0x80 | x);
			ST7920_Write(LCD_DATA, 0xFF);
			ST7920_Write(LCD_DATA, 0xFF);
		}

	// initial menu selection
	ST7920_HighlightMenuItem(1, true);

	ST7920_setGraphicMode(false);

	ST7920_displayString(0, "  SolderFeeder  ");
	ST7920_displayString(1, " Mode     Auto");
	ST7920_displayString(2, " Spd       85%");
	ST7920_displayString(3, " Time    750ms");
	Serial.begin(9600);
}

void loop() {
	if (digitalRead(ENC_BTN) == LOW) {
		ST7920_setGraphicMode(true);
		ST7920_HighlightMenuItem(selectedItem, false);

		selectedItem += 1;
		if (selectedItem > 3) selectedItem = 1;

		ST7920_HighlightMenuItem(selectedItem, true);
		ST7920_setGraphicMode(false);

		delay(200);
	}

	int e = getEncoder();
	if (e != 0) {
		if (selectedItem == 1){
			changeStatus(e);
		}
		else if (selectedItem == 2){
			changeSpeed(e);
		}
		else{
			changeTime(e);
		}
	}
	sendData = 0;
	if (!digitalRead(forBut)){
		if(statusVal == 0) {
			sendData += 100000.0;
			sendData += float(speedVal) * 1000;
			sendData += float (timeVal);
			Serial.print(sendData, 0);
      while(!digitalRead(forBut));
      delay(timeVal * 10);

		}
		if(statusVal == 1){
			sendData = 200000.0 + float(speedVal) * 1000;
      Serial.print(sendData, 0);
      while(!digitalRead(forBut));
			Serial.print(99010.0, 0);
      delay(500);
		}
	}
	else if (!digitalRead(bacBut)){
		sendData += 300000.0;
		sendData += float(speedVal) * 1000;
		Serial.print(sendData, 0);
    while(!digitalRead(bacBut));
		Serial.print(99010.0, 0);
    delay(500);
	}
}
