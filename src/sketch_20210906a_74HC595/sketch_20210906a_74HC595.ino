//
// sketch_20210906a_74HC595
//

// Prev update: Fri Sep 10 22:28:35 JST 2021
// Prev update: Tue Nov 30 21:21:15 JST 2021 by @hohno_at_kuimc
// Last update: Sat Dec  4 22:43:17 JST 2021 by @hohno_at_kuimc

// ---------------------------------------------------------

// Hardware: ARDUINO UNO + AE-7SEG-BOARD * 2

// Description: Read the lines of the following format and perform the actions corresponding to each value.
//              Format: val1 val2 val3 val4[LF]
//                      val1 - Relay(Light) turn on furation (-1, 0, 1..32767)
//                      val2 - Buzzer turn on duration (-1, 0, 1..32767)
//                      val3 - 7segment LED 1 (tens place) (0..37) // 37 == (sizeof digits) / (sizeof digits[0]) - 1
//                      val4 - 7segment LED 2 (ones place) (0..37)

// ---------------------------------------------------------

// #define USE_BGKA

#include <SPI.h>
#include "myself.h"

#define BUFFSIZE    (32)
char buff[BUFFSIZE];

const int PIN_SCK   = 13; // SCK
const int PIN_SDO   = 12; // MISO // Not used
const int PIN_SDI   = 11; // MOSI
const int PIN_LATCH =  8;

#ifdef USE_BGKA
const int PIN_RELAY =  2; // --> A1?
const int PIN_BEEP  = 12;
const int PIN_TONE  =  5; // on GBKA
const int PIN_LED   =  4; // on GBKA
const int PIN_AUX   = 10;
const int PIN_GND   =  9;
/*
(1) Pinout on GBKA
    D2 - (not used)
    D3 - Temperature & Humidity Sensor on GBKA
    D4 - LED on GBKA
    D5 - Piezo Buzzer on GBKA
    D6 - Button on GBKA
    D7~13  - (not used)
    A0 - Rotary Potentiometer
    A1 - (not used)
    A2 - Sound
    A3 - (not used)
    A4 - I2C SDA
    A5 - I2C SCK
    A6 - Light (only on GBKA)
    A7 - (not used) (only on GBKA)
(2) PWM on Arduino UNO
    D3,5,6,9,10
 */
#else  // USE_BGKA_AND_SS
const int PIN_RELAY = A0; // --> A1?
const int PIN_BEEP  = 12;
const int PIN_TONE  =  9; // --> 5?
const int PIN_LED   = 13; // --> 4? // D13 is conflict with SPI SCK
const int PIN_AUX   =  7; // --> 10?
const int PIN_GND   =  6; // --> 9?
#endif // USE_BGKA_AND_SS

/*
 *      A
 *     ===
 *  F | G |B
 *     ===
 *  E |   |C
 *     ===  dp
 *      D
 */

const byte digits[] = {
// For this H/W      // Original Data
//  ABCEDFG.       //     ABCDEFG.
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
  0b10001010, // ≡ // 0b10010010, // ≡ // A__D__G_
  0b01010010, // / // 0b01001010, // / // _B__E_G_
  0b00000000, //   // 0b00000000, //   // ________
  0b00000001, // . // 0b00000001, // . // ________
};

#define INDEX_DOT	(NN-1)        // ← ドット
#define INDEX_SPC	(NN-2)        // ← 空白
#define INDEX_UDB	(NN-5)        // ← 下線
#define INDEX_DASH	(NN-6)        // ← マイナス

#define INDEX_d10   INDEX_SPC     // ガードタイム時に表示する文字（10の位）
#define INDEX_d01   INDEX_SPC     // ガードタイム時に表示する文字（1の位）

// ---------------------------------------------------------
// setup()
// ---------------------------------------------------------

int NN = 0;

void setup() {
  // digits[] のサイズを確定
  NN = (sizeof digits) / (sizeof digits[0]);

  // シリアルポート初期化
  Serial.begin(57600);
  // Serial.print("\n# sketch_20210906a_74HC595\n\n");
  showOpeningMessage();

  // 出力ピン設定
  pinMode(PIN_LED,  OUTPUT);
  pinMode(PIN_BEEP, OUTPUT);
  pinMode(PIN_RELAY,OUTPUT);
  pinMode(PIN_AUX,  OUTPUT);
  pinMode(PIN_GND,  OUTPUT);
  //
  digitalWrite(PIN_LED,  HIGH); // ON
  digitalWrite(PIN_BEEP, LOW);  // OFF
  digitalWrite(PIN_RELAY,HIGH); // OFF (negative logic)
  digitalWrite(PIN_AUX,  LOW);  // OFF
  digitalWrite(PIN_GND,  LOW);  // OFF (pseudo GND)

  // buff[] 初期化
  for(int i = 0; i < BUFFSIZE; i++) {
    buff[i] = '\0';
  }

  // SPI 初期化
  pinMode(PIN_LATCH, OUTPUT);
  digitalWrite(PIN_LATCH, 1);
  pinMode(PIN_SCK, OUTPUT);
  pinMode(PIN_SDI, OUTPUT);
  //
  SPI.begin();
  SPI.setBitOrder(LSBFIRST);
  SPI.setDataMode(0);

  // 3回点滅
  for (int i = 0; i < 3; i++) {
    // ブザー ON
    digitalWrite(PIN_BEEP, HIGH);
    // 全セグメント点灯
    digitalWrite(PIN_LATCH, 0);
    SPI.transfer(0xff);//10の桁 +1でドット表示
    SPI.transfer(0xff);//1の桁
    digitalWrite(PIN_LATCH, 1);
    // 待機
    delay(750);
    // ブザー OFF
    digitalWrite(PIN_BEEP, LOW);
    // 全セグメント消灯
    digitalWrite(PIN_LATCH, 0);
    SPI.transfer (0);//10の桁 +1でドット表示
    SPI.transfer (0);//1の桁
    digitalWrite(PIN_LATCH, 1);
    // 待機
    delay(250);
  }
  delay(500);
}

// ---------------------------------------------------------
// loop()
// ---------------------------------------------------------

#define DISPLAY_TIME  (3000)  // 7segment LED 表示時間（ただしリレー動作より早く終了することはできないように調整が入る）（単位：ミリ秒）
#define GUARD_TIME    (1000)  // ガードタイム（単位：ミリ秒）

#define BEEP_LIMIT  (10000L)  // 最長ブザー鳴動時間（単位：ミリ秒）
#define RELAY_LIMIT (10000L)  // 最長リレー動作時間（単位：ミリ秒）

#define TIMEOUT2    (200)

uint32_t endTime1R = 0; // リレー動作終了時刻（起動時からのミリ秒）
uint32_t endTime1B = 0; // ブザー鳴動終了時刻（起動時からのミリ秒）
uint32_t endTime2  = 0; // カウントアップ動作中の次の更新時刻（起動時からのミリ秒）
uint32_t endTime3  = 0; // LED に値を表示する期間の終了時刻（起動時からのミリ秒）
uint32_t endTime4  = 0; // LED に値を表示し終えたあとの「ガードタイム」の終了時刻（起動時からのミリ秒）

int cnt = 0;            // ループカウンタ
int d10 = 0, d01 = 0;     // 7セグメントLED の 10位と1位
boolean flagLED = false;

void loop() {

  int val1 = 0, val2 = 0;
  static int prev1 = -9; // , prev2 = -9;
  static boolean flag = false;
  int32_t tmpT = 0;

  // LED の点滅（Arduino UNO 本体の LED（Pin13）は SPI で使っているため別のLED を用意する必要がある）
  tmpT = (millis() / 1000) % 2;
  if ((flagLED == false) && (tmpT == 1)) {
    flagLED = true;
    digitalWrite(PIN_LED, flagLED);
  } else if ((flagLED == true) && (tmpT == 0)) {
    flagLED = false;
    digitalWrite(PIN_LED, flagLED);
  }

  // 現在時刻を取得（変数けちらなくていいのに...）
  tmpT = millis();

  if (Serial.available() > 0) {

    // UART から何か読みだせた場合
    int tmpVal1  = 0, tmpVal2 = 0, tmpd10 = -1, tmpd01 = -1, n = 0;
    String str = Serial.readStringUntil('\n'); // 要再検討：中途半端な（行末文字で終端していない）文字列が送られてくるとここで待機してしまう

    // 読み取った行をスキャンする

    if ((n = sscanf(str.c_str(), "%d %d %d %d", &tmpVal1, &tmpVal2, &tmpd10, &tmpd01)) > 0) { // Relay(Light) Buzzer Digit1 Digit2

      // この時点で少なくとも n == 1 なのでとりあえず val1 と val2 はスキャンして得た第１文字列にする
      val1 = tmpVal1;
      val2 = val1;

      if (n >= 2) {
        // n が少なくとも 2 なら val2 は第2文字列にする
        val2 = tmpVal2;
      }
      if (n >= 3) {
        // n が少なくとも 3 なら d10 は第3文字列にする
        d10 = tmpd10;
        // endTime3 と endTime4 を設定（endTIme1R 確定後に調整が入ることがある）
        endTime3 = millis() + DISPLAY_TIME;
        endTime4 = endTime3 + GUARD_TIME;
        // flag を立てておく（val1 に変化がなくても d10 に変化があったら 7segment LED の表示を更新する必要があるため）
        flag = true;
      }
      if (n >= 4) {
        // n が少なくとも 4 なら d01 は第4文字列にする
        d01 = tmpd01;
      }
    }
    // val1 と val2 から endTimeR（リレー用）と endTimeB（ブザー用）を更新
    //
    // val1 == 0 : turn off the relay(light)
    // val1 > 0  : turn on the relay(light) up to val1/10 sec.
    // val1 < 0  : do nothing for the relay(light)
    //
    // val2 == 0 : turn off the buzzer
    // val2 > 0  : turn on the buzzer up to val1/10 sec.
    // val2 < 0  : do nothing for the buzzer

    // endTime1R を更新
    endTime1R = (val1 == 0) ? tmpT : tmpT + 100L * val1;
    if ((endTime1R - tmpT) > RELAY_LIMIT) {
      endTime1R = tmpT + RELAY_LIMIT;
    }

    // endTime3 が短かくなりすぎないように調整
    //（少なくともリレーがオンの間は 7segment LED には指定した文字が表示されているようにする）
    if (endTime1R > endTime3) {
      endTime3 = endTime1R;
      endTime4 = endTime3 + GUARD_TIME;
    }

    // endTime1B を更新
    endTime1B = (val2 == 0) ? tmpT : tmpT + 100L * val2;
    if ((endTime1B - tmpT) > BEEP_LIMIT) {
      endTime1B = tmpT + BEEP_LIMIT;
    }

    // val1 が更新されていたか flag が立っていたら 取得した情報を表示
    if ((val1 != prev1) || flag) {
      Serial.print(n);    Serial.print(" ");
      Serial.print(val1); Serial.print(" ");
      Serial.print(val2); Serial.print(" ");
      Serial.print(d10);  Serial.print(" ");
      Serial.print(d01);  Serial.println();
      // prev1 を更新
      prev1 = val1;
      // flag を下げる
      flag = false;
    }
  }

  // リレーの ON/OFF
  if (tmpT < endTime1R) {
    digitalWrite(PIN_RELAY, LOW);
  } else {
    digitalWrite(PIN_RELAY, HIGH);
  }

  // ブザーの ON/OFF
  if (tmpT < endTime1B) {
    digitalWrite(PIN_BEEP, HIGH);
  } else {
    digitalWrite(PIN_BEEP, LOW);
  }

  // 7segnent LED の表示を更新
  if (endTime3 > tmpT) {
    // タイムアウト前なら d10 と d01 を表示する
    int j = d10 % NN;
    int i = d01 % NN;
    // 7segment LED の表示を更新
    digitalWrite(PIN_LATCH, 0);
    SPI.transfer (digits[j] + 0); // 10の桁 (+1でドット表示)
    SPI.transfer (digits[i] + 0); //  1の桁 (+1でドット表示)
    digitalWrite(PIN_LATCH, 1);

  } else if (endTime4 > tmpT) {
    // ガードタイム中．INDEX_d10 と　INDEX_d01 を表示
    digitalWrite(PIN_LATCH, 0);
    SPI.transfer (digits[INDEX_d10] + 0); // 10の桁 (+1でドット表示)
    SPI.transfer (digits[INDEX_d01] + 0); //  1の桁 (+1でドット表示)
    digitalWrite(PIN_LATCH, 1);

  } else {
    if (tmpT >= endTime2) {
      // カウントアップ動作中かつ表示を更新する時期になった

      // endTime2 を更新し，次に表示を更新する時刻を設定
      endTime2 = tmpT + TIMEOUT2;

      // digits[] の内容をカウントアップ表示
      int j = (cnt / NN) % NN;
      int i =  cnt % NN;
      digitalWrite(PIN_LATCH, 0);
      SPI.transfer (digits[j] + 1); // 10の桁 (+1でドット表示)
      SPI.transfer (digits[i] + 1); //  1の桁 (+1でドット表示)
      digitalWrite(PIN_LATCH, 1);

      // ループカウンタ（cnt）を更新して適宜表示
      // Serial.print(cnt);        Serial.print(", ");
      // if ((cnt % NN) == 0) { Serial.println(); }
      cnt++;
      if (cnt == ((NN * NN) - 1)) {
        Serial.println("--MARK--");
      }
      cnt %= (NN * NN);
    }
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

// ---------------------------------------------------------
