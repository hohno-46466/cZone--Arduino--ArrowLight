//
// sketch_20210906a_74HC595
//

// Prev update: Fri Sep 10 22:28:35 JST 2021
// Last update: Tue Nov 30 21:21:15 JST 2021 by @hohno_at_kuimc

// ---------------------------------------------------------

// Hardware: ARDUINO UNO + AE-7SEG-BOARD * 2

// Description: Read the lines of the following format and perform the actions corresponding to each value.
//              Format: val1 val2 val3 val4[LF]
//                      val1 - Light turn on furation (-1, 0, 1..32767)
//                      val2 - Buzzer turn on duration (-1, 0, 1..32767)
//                      val3 - 7segment LED 1 (tens place) (0..37) // 37 == (sizeof digits) / (sizeof digits[0]) - 1
//                      val4 - 7segment LED 2 (ones place) (0..37)

// ---------------------------------------------------------

#include<SPI.h>

#define BUFFSIZE    (32)
char buff[BUFFSIZE];

const int PIN_SCK   = 13; //  9;
const int PIN_SDI   = 11; // 10;
const int PIN_LATCH =  8; //  8;

const int PIN_RELAY = A0;
const int PIN_BEEP  = 12; // 10
const int PIN_TONE  =  9;
const int PIN_LED   = 13; // Overlapped with PIN_SCK
const int PIN_AUX   =  7;
const int PIN_GND   =  6;

// int scroll_speed = 400;

/*
     A
    ---
 F | G |B
    ---
 E |   |C
    ---  dp
     D
 */

const byte digits[] = {
// This config.    // Original Data
//  ABCEDFG.       //   ABCDEFG.
  0b11111100, // 0 // 0b11111100, // 0 // ABCDEF__
  0b01100000, // 1 // 0b01100000, // 1 // _BC_____
  0b11011010, // 2 // 0b11011010, // 2 // AB_DE_G_
  0b11101010, // 3 // 0b11110010, // 3 // ABCD__G_
  0b01100110, // 4 // 0b01100110, // 4 // _BC__FG_
  0b10101110, // 5 // 0b10110110, // 5 // A_CD_FG_
  0b10111110, // 6 // 0b10111110, // 6 // A_CDEFG_
  0b11100000, // 7 // 0b11100000, // 7 // ABC_____
  0b11111110, // 8 // 0b11111110, // 8 // ABCDEFG_
  0b11101110, // 9 // 0b11110110, // 9 // ABCD_FG_
  0b11110110, // A // 0b11101110, // A // ABC_EFG_
  0b00111110, // b // 0b00111110, // b // __CDEFG_
  0b00011010, // c // 0b00011010, // c // ___DE_G_
  0b10011100, // C // 0b10011100, // C // A__DEF__
  0b01111010, // d // 0b10011110, // D // _BCDE_G_
  0b10011110, // E // 0b10011110, // E // A__DEFG_
  0b10010110, // F // 0b10001110, // F // A___EFG_
  0b10111100, // G // 0b10111100, // G // A_CDEF__
  0b01110110, // H // 0b01101110, // H // _BC_EFG_
  0b01101000, // j // 0b01110000, // j // _BCD____
  0b00011100, // L // 0b00011100, // L // ___DEF__
  0b00110010, // n // 0b00101010, // n // __C_E_G_
  0b00111010, // o // 0b00111010, // o // __CDE_G_
  0b11010110, // P // 0b11001110, // P // AB__EFG_
  0b11100110, // q // 0b11100110, // q // ABC___G_
  0b00010110, // r // 0b00001110, // r // ____EFG_
  0b00011110, // t // 0b00011110, // t // ___DEFG_
  0b00111000, // u // 0b00111000, // u // __CDE___
  0b01111100, // U // 0b01111100, // U // _BCDEF__
  0b01101110, // y // 0b01110110, // y // _BCD_FG_
  0b10000000, // ~ // 0b10000000, // ~ // A_______
  0b00000010, // - // 0b00000010, // - // ______G_
  0b00001000, // _ // 0b00010000, // _ // ___D___
  0b10001010, // ≡// 0b10010010, // ≡// A__D__G_
  0b01010010, // / // 0b01001010, // / // _B__E_G_
  0b00000000, //   // 0b00000000, //   // ________
  0b00000001, // . // 0b00000001, // . // ________
};

// ---------------------------------------------------------
// setup()
// ---------------------------------------------------------

int NN = 0;

void setup() {
  NN = (sizeof digits) / (sizeof digits[0]);

  Serial.begin(57600);

  pinMode(PIN_LED,  OUTPUT);
  pinMode(PIN_BEEP, OUTPUT);
  pinMode(PIN_RELAY,OUTPUT);
	pinMode(PIN_AUX,  OUTPUT);
	pinMode(PIN_GND,  OUTPUT);

  digitalWrite(PIN_LED,  HIGH); // ON
  digitalWrite(PIN_BEEP, LOW);  // OFF
  digitalWrite(PIN_RELAY,HIGH); // OFF (negative logic)
  digitalWrite(PIN_AUX,  LOW);  // OFF
	digitalWrite(PIN_GND,  LOW);  // OFF (pseudo GND)

  for(int i = 0; i < BUFFSIZE; i++) {
    buff[i] = '\0';
  }
  pinMode(PIN_LATCH, OUTPUT);
  digitalWrite(PIN_LATCH, 1);

  pinMode(PIN_SCK, OUTPUT);
  pinMode(PIN_SDI, OUTPUT);
  SPI.begin();
  SPI.setBitOrder(LSBFIRST);
  SPI.setDataMode(0);

  for (int i = 0; i < 3; i++) {
    digitalWrite(PIN_BEEP, HIGH);
    digitalWrite(PIN_LATCH, 0);
    SPI.transfer (0xff);//10の桁 +1でドット表示
    SPI.transfer (0xff);//1の桁
    digitalWrite(PIN_LATCH, 1);
    delay(750);
    //
    digitalWrite(PIN_BEEP, LOW);
    digitalWrite(PIN_LATCH, 0);
    SPI.transfer (0);//10の桁 +1でドット表示
    SPI.transfer (0);//1の桁
    digitalWrite(PIN_LATCH, 1);
    delay(250);
  }
  delay(500);
}

// ---------------------------------------------------------
// loop()
// ---------------------------------------------------------

#define DISPLAY_TIME  (3000)
#define GUARD_TIME    (1000)

#define BEEP_LIMIT  (5000)
#define RELAY_LIMIT (5000)

#define TIMEOUT2    (200)

uint32_t endTime1R = 0;
uint32_t endTime1B = 0;
uint32_t endTime2  = 0;
uint32_t endTime3  = 0; // LED に値を表示する期間の終了時刻（起動時からのミリ秒）
uint32_t endTime4  = 0; // LED に値を表示し終えたあとのガードタイムの終了時刻（起動時からのミリ秒）

int cnt = 0;
int d10 = 0, d01 = 0;
boolean flagLED = false;

void loop() {

  int val1 = 0, val2 = 0;
  static int prev1 = -9; // , prev2 = -9;
  static boolean flag = false;
  int32_t tmpT = (millis() / 1000) % 2;

	if ((flagLED == false) && (tmpT == 1)) {
		flagLED = true;
		digitalWrite(PIN_LED, flagLED);
	} else if ((flagLED == true) && (tmpT == 0)) {
		flagLED = false;
		digitalWrite(PIN_LED, flagLED);
	}

  if (Serial.available() > 0) {

    int tmpVal1  = 0, tmpVal2 = 0, tmpd10 = -1, tmpd01 = -1, n;
    String str = Serial.readStringUntil('\n');
    if ((n = sscanf(str.c_str(), "%d %d %d %d", &tmpVal1, &tmpVal2, &tmpd10, &tmpd01)) > 0) { // Light Buzzer Digit1 Digit2
      // n == 1
      val1 = tmpVal1;
      val2 = val1;
      if (n >= 2) { val2 = tmpVal2; }
      if (n >= 3) { d10 = tmpd10; endTime3 = millis() + DISPLAY_TIME; endTime4 = endTime3 + GUARD_TIME; flag = true; }
      if (n >= 4) { d01 = tmpd01; }
    }

    // val1 == 0 : turn off the light
    // val1 > 0  : turn on the light up to val1/10 sec.
    // val1 < 0  : do nothing for the light

    // val2 == 0 : turn off the buzzer
    // val2 > 0  : turn on the buzzer up to val1/10 sec.
    // val2 < 0  : do nothing for the buzzer

    endTime1R = (val1 == 0) ? millis() : millis() + 100L * val1;
    if ((endTime1R - millis()) > RELAY_LIMIT) {
      endTime1R = millis() + RELAY_LIMIT;
    }

    endTime1B = (val2 == 0) ? millis() : millis() + 100L * val2;
    if ((endTime1B - millis()) > BEEP_LIMIT) {
      endTime1B = millis() + BEEP_LIMIT;
    }

    if ((val1 != prev1) || flag) {
      Serial.print(n);   Serial.print(", ");
      Serial.print(val1); Serial.print(", ");
      Serial.print(d10); Serial.print(", ");
      Serial.print(d01); Serial.println();
      prev1 = val1;
      flag = false;
    }
  }

  if (millis() < endTime1R) {
    digitalWrite(PIN_RELAY, LOW);
  } else {
    digitalWrite(PIN_RELAY, HIGH);
  }

  if (millis() < endTime1B) {
    digitalWrite(PIN_BEEP, HIGH);
  } else {
    digitalWrite(PIN_BEEP, LOW);
  }

  if (endTime3 > millis()) {

		int j = d10 % NN;
		int i = d01 % NN;
		digitalWrite(PIN_LATCH, 0);
		SPI.transfer (digits[j] + 0); // 10の桁 (+1でドット表示)
		SPI.transfer (digits[i] + 0); //  1の桁 (+1でドット表示)
		digitalWrite(PIN_LATCH, 1);

  } else if (endTime4 > millis()) {

		digitalWrite(PIN_LATCH, 0);
		SPI.transfer (digits[NN-2] + 0); // 10の桁 (+1でドット表示)
		SPI.transfer (digits[NN-2] + 0); //  1の桁 (+1でドット表示)
		digitalWrite(PIN_LATCH, 1);

  } else {
    if (millis() >= endTime2) {

      endTime2 = millis() + TIMEOUT2;
      int j = (cnt / NN) % NN;
      int i =  cnt % NN;
      digitalWrite(PIN_LATCH, 0);
      SPI.transfer (digits[j] + 1); // 10の桁 (+1でドット表示)
      SPI.transfer (digits[i] + 1); //  1の桁 (+1でドット表示)
      digitalWrite(PIN_LATCH, 1);
      Serial.print(cnt);        Serial.print(", ");
      if ((cnt % NN) == 0) { Serial.println(); }
      cnt++;
      cnt %= (NN * NN);
    }
  }

  /*
  for (int j = 0; j < NN; j ++) {//１０の桁
    for (int i = 0; i < NN; i ++) {//１の桁
      //SPI
      digitalWrite(PIN_LATCH, 0);
      SPI.transfer (digits[j]);//10の桁 +1でドット表示
      SPI.transfer (digits[i]);//1の桁
      digitalWrite(PIN_LATCH, 1);
      //
      delay(scroll_speed);
    }
  }
  */

}

// ---------------------------------------------------------
