#include <M5Stack.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "MFRC522_I2C.h"

// WiFi接続情報
// const char *ssid     = "Buffalo-2G-47c0";
// const char *password = "7ksad38v5svea";
const char *ssid     = "elecom2g-f297fb";
const char *password = "97pn7752rjdj";

// NTPクライアントの設定
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 9 * 3600, 60000);

//RFIDタグの初期化
MFRC522 mfrc522(0x28);
unsigned long tagDetectionDuration = 0;//ドアが開いている時間（RFIDが検出されていない時間）
bool isMessage = false; //今日の通知を送ったかどうか


void setup() {
    //共通の設定
    Serial.begin(9600);
    M5.begin();   //M5stackオブジェクトの初期化         
    M5.Power.begin();
    M5.Lcd.begin();
    M5.Lcd.setTextColor(WHITE);
    M5.lcd.setTextSize(2);  
    Wire.begin();  
    
    //Wifi
    M5.Lcd.println("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      M5.Lcd.print(".");
    }

    M5.Lcd.println("\nWi-Fi connected.");
    M5.Lcd.println("Getting time...");

    timeClient.begin();
    timeClient.update();



    //RFID
    mfrc522.PCD_Init(); //RFIDの初期化
    M5.Lcd.setCursor(0, 0); //描画位置の設定
    M5.Lcd.fillScreen(BLACK); //背景色の設定
    M5.Lcd.println("Please put the card");
}

void loop() {
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.fillScreen(BLACK);

    //NTPサーバーから時刻取得のため更新
    GetTime();

    //1日一回だけ実行、sendMessage送ったらもう実行しない。日付が変わればまた
    if(!isMessage){
      if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {//タグが検出できている
        tagDetectionDuration =0;
      } else {//タグが認識できていない＝ドアが開いている可能性
        tagDetectionDuration += 50;
      }

      if (tagDetectionDuration >= 150) { // タグが1000ms以上検出できない場合、ドアが開いているとする
        M5.Lcd.println("Door Opened");
        sendNotification();
      } else {
        M5.Lcd.println("Door Closed");
      }
      delay(500);
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

void GetTime(){
timeClient.update();
    M5.Lcd.println("Current time:");
    String formattedTime = timeClient.getFormattedTime();
    M5.Lcd.println(formattedTime);
    M5.Lcd.println("");
    //delay(1000);
}