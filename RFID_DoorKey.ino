#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiSTA.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiType.h>
#include <WiFiUdp.h>

#include <M5Stack.h>
#include "MFRC522_I2C.h"

MFRC522 mfrc522(0x28);
    unsigned long tagDetectionDuration = 0;
    bool isMessage = false; //今日の通知を送ったかどうか

void setup() {
  //セットアップ
    Serial.begin(9600);
    M5.begin();   //M5stackオブジェクトの初期化         
    M5.Power.begin();
    M5.Lcd.begin();
    M5.Lcd.setTextColor(WHITE);
    M5.lcd.setTextSize(2);  
    Wire.begin();  

    mfrc522.PCD_Init(); //RFIDの初期化
    M5.Lcd.setCursor(0, 0); //描画位置の設定
    M5.Lcd.fillScreen(BLACK); //背景色の設定
    M5.Lcd.println("Please put the card");
}

void loop() {

    M5.Lcd.setCursor(40, 47);
    M5.Lcd.fillRect(42, 47, 320, 20, BLACK); //Please put the cardを邪魔しないため

    //1日一回だけ実行、sendMessage送ったらもう実行しない。日付が変わればまた
    if(!isMessage){
      if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {//タグが検出できている
        tagDetectionDuration =0;
      } else {//タグが認識できていない＝ドアが開いている可能性
        tagDetectionDuration += 50;
      }

      if (tagDetectionDuration >= 500) { // タグが1000ms以上検出できない場合、ドアが開いているとする
        M5.Lcd.println("Door Opened");
        sendNotification();
      } else {
        M5.Lcd.println("Door Closed");
      }
      delay(100);
    }else{
      //誰かが鍵を開けた

      //次の日になったら、isMessageをfalseにする

      return;
    }
}

void sendNotification() {
  //slackへの通知
  //isMessage = true;
}