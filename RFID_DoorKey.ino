#include <M5Stack.h>
#include <WiFi.h>
#include <NTPClient.h>
#include "MFRC522_I2C.h"
#include <HTTPClient.h>

// WiFi接続情報
// const char *ssid     = "Buffalo-2G-47C0";
// const char *password = "7ksad38v5svea";
const char *ssid     = "elecom2g-f297fb";
const char *password = "97pn7752rjdj";

//slack WebhookURL
const char *webhookUrl = "";

// NTPクライアントの設定
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 9 * 3600, 60000);

//RFIDタグの初期化
MFRC522 mfrc522(0x28);

//ドアが開いている時間（＝RFIDが検出されていない時間）
unsigned long tagDetectionDuration = 0;

//今日の通知を送ったかどうか
bool isMessage = false; 


//slackへ通知を送る
void postMessage(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(webhookUrl);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"text\":\"" + message + "\"}";

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode == 200) {
      M5.Lcd.println("Notification sent to Slack!");
    } else {
      M5.Lcd.println("Failed to send notification.");
    }
    http.end();
  } else {
    M5.Lcd.println("No Wi-Fi connection.");
  }
}

//現在の時刻をm5stackに出力
void GetTime(){
    timeClient.update();

    //現在時刻の出力
    M5.Lcd.println("Current time:");
    String formattedTime = timeClient.getFormattedTime();//format: 13:47:40
    M5.Lcd.println(formattedTime);
    M5.Lcd.println("");
}

//日付を監視し、isMessageを更新する
void updateFlag(){
    timeClient.update();

    //時間と日付の取得
    int currentDay = timeClient.getDay();
    int currentHour = timeClient.getHours();
    int currentSeconds = timeClient.getSeconds();//テスト用

    //m5stackに出力
    M5.Lcd.print("Hour: ");
    M5.Lcd.println(currentHour);
    M5.Lcd.print("Seconds: ");
    M5.Lcd.println(currentSeconds);

    //本番（0時になったらisMessageを更新）
    // if(currentHour == 0){
    //   isMessage = false;
    // }

    //テスト（0秒になったら(1分ごと)isMessageを更新）
    if(currentSeconds == 0){
      isMessage = false;
    }
}

void setup() {
    //共通のセットアップ
    Serial.begin(9600);
    M5.begin();   //M5stackオブジェクトの初期化         
    M5.Power.begin();
    M5.Lcd.begin();
    M5.Lcd.setTextColor(WHITE); //テキストカラー
    M5.lcd.setTextSize(2);  //テキストサイズ
    M5.Lcd.setCursor(0, 0); //描画位置の設定
    M5.Lcd.fillScreen(BLACK); //背景色の設定
    Wire.begin();  
    
    //Wifiのセットアップ
    M5.Lcd.println("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      M5.Lcd.print(".");
    }
    M5.Lcd.println("\nWi-Fi connected.");
    M5.Lcd.println("Getting time...");

    //timeClientのセットアップ
    timeClient.begin();
    timeClient.update();

    //RFIDのセットアップ
    mfrc522.PCD_Init();    
}

void loop() {
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.fillScreen(BLACK);

    //NTPサーバーから現在の時刻を取得し、出力
    GetTime();

    //1日一回だけ、最初のドアの施錠を検知し、slackへ通知する
    if(!isMessage){
      if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {//タグ検出OK
        tagDetectionDuration =0;
      } else {//タグ検出NG（＝ドアが開いている可能性）
        tagDetectionDuration += 50;
      }

      if (tagDetectionDuration >= 150) { // タグが一定時間検出できない場合、ドアが開いているとする（Durationを設けないと、OpenedとClosedが交互に出力されてしまうため）
        M5.Lcd.println("Door Opened");
        postMessage("key");
        isMessage = true;
      } else {
        M5.Lcd.println("Door Closed");
      }
      delay(500);
    }else{
      //日付の更新とともにisMessageを更新
      updateFlag();
      delay(1000);
      return;
    }
}