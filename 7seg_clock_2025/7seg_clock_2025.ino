#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include "wifi_setting.h"
#include "120pixel.h"
#include <HTTPClient.h>
#define PIN 27        //INが接続されているピンを指定
// #define NUMPIXELS 74  //LEDの数を指定

//プロトタイプ宣言
void ShowTime(int hour, int minute);
void Clock();
void ntpaccess();
void ClockOperation();

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);  //800kHzでNeoPixelを駆動


int flag;//:を点滅させるフラグ
int life;
int change;
unsigned long previousTime;
unsigned long ntptime;
unsigned long waitingtime;

struct tm timeInfo;  //時刻を格納するオブジェクト


void setup() {
  Serial.begin(115200);
  Serial.println("Serial.begin-------- ok");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to the WiFi network!--------------- ok");

  configTime(9 * 3600L, 0, "ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp");

  pixels.begin();  // NeoPixel開始

  flag = 1;
  life = 1;
  change = 1;

  ntpaccess();  // 時刻初期化
  Serial.println("ntpaccess-------- ok");
}




void ShowTime(int hour, int minute) {
  /* ShowTime関数は、構造型(struct)の変数を受け取れるので、ShowTime関数の中に、LED画面に表示するコードを追加する。*/
  int time_4_digits = hour * 100 + minute;
  //Serial.println(time_4_digits);
  int s[4];
  for (int i = 3; i >= 0; i--) {
    s[i] = time_4_digits % 10;
    time_4_digits = (time_4_digits - s[i]) / 10;
  }

  // Serial.print(s[0]);
  // Serial.print(s[1]);
  // Serial.print(s[2]);
  // Serial.println(s[3]);


  pixels.clear();

  //  int digitSegments1 = digitSegments[s[i]][x] - 1;
  for (int i = 0; i < 4; i++) {     //各ケタ
    for (int j = 0; j < digit; j++) {  //1ケタ分のLED（18個）の表示
      int index = i * digit + j;

      int huestart = 16363;//0;//始まりの色（書き換える）
      int huefin = 32766;//65536;終わりの色（書き換える)
      int hue = ((huefin - huestart) / 37) * index + huestart;//色の範囲を指定している(ここは書き換えない（0~65535))
      int sat = 255;
      int val = 200;
      
   
      if (i == 2) {
        pixels.setPixelColor(index + coron_number, pixels.ColorHSV(hue, sat, val * digitSegments[s[i]][j]));//hue(色),色彩、明るさ
      } else if (i == 3) {
        pixels.setPixelColor(index + coron_number, pixels.ColorHSV(hue, sat, val * digitSegments[s[i]][j]));
      } else {
        pixels.setPixelColor(index, pixels.ColorHSV(hue, sat, val * digitSegments[s[i]][j]));
      }
    }
  }
  

  for (int i = 0; i < coron_number; i++){
    pixels.setPixelColor(coron_digits[i], pixels.Color(flag*100, flag*100, flag*100));//1の時[:]点灯
  }

  // pixels.setPixelColor(36, pixels.Color(flag*100, flag*100, flag*100));//1の時[:]点灯
  // pixels.setPixelColor(37, pixels.Color(flag*100, flag*100, flag*100));


  pixels.show();  //LEDに色を反映
  
  
}


void loop() {
  life = 1;  // ← 常にNTPモードを強制

  if(life == 1){
    if(change == 0){
      ntpaccess();
      change =1;
    }
    Clock();//1秒カウントアップ
    ShowTime(timeInfo.tm_hour, timeInfo.tm_min);
    if(millis() - ntptime > 180*1000){//ntpをとった時間から180秒たったら
      ntpaccess();
    }
    // ClockOperation();
  }else if(life == 2){
    Serial.print("OK");
    Clock();
    ShowTime(timeInfo.tm_hour, timeInfo.tm_min);
  }else if(life == 0){
    Clock();
    ClockOperation();
    ShowTime(timeInfo.tm_hour, timeInfo.tm_min);
  }
}


void Clock(){
  if (millis() - previousTime >= 1000) {   //プログラムが経過した時間が1秒経ったら
    previousTime = millis();   //基準時間に現在時間を代入
    flag = flag ^1; //1秒ごとに点灯
    timeInfo.tm_sec+=1; //1秒カウントアップ
    if(timeInfo.tm_sec == 60){
      timeInfo.tm_sec = 0;
      timeInfo.tm_min += 1;
      if(timeInfo.tm_min == 60){
        timeInfo.tm_min = 0;
        timeInfo.tm_hour += 1;
        if (timeInfo.tm_hour == 24){
          timeInfo.tm_hour = 0;
        }
      }
    }
  }
}

void ntpaccess(){
  getLocalTime(&timeInfo);  //tmオブジェクトのtimeInfoに現在時刻を入れ込む
  Serial.println("ntpaccess!");
  ntptime = millis();
  Serial.print(timeInfo.tm_hour);
  Serial.print(timeInfo.tm_min);
  Serial.print(timeInfo.tm_sec);
}

void ClockOperation(){
  if (millis() - waitingtime >= 1000) {   //プログラムが経過した時間が1秒経ったら
    waitingtime = millis();   //基準時間に現在時間を代入
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      // URLの設定
      http.begin(serverUrl);

      // GETリクエストの送信
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
      //   // HTTPレスポンスコードを表示
        //Serial.print("HTTP Response code: ");
        //Serial.println(httpResponseCode);

      //   // ペイロードの取得と表示
        String payload = http.getString();
        //Serial.println("Received payload:");
        //Serial.println(payload);

        // int dataInt = payload.toInt();
        int dataInt = 9999;
        int time[2];
        
        if (dataInt == 9999){//ntpみにいく
          life = 1;
        }else if(dataInt == 9998){//ntpみにいかない　カウントアップのみ
          life = 2;
          change =0;
        }else{
          for (int i = 1; i >= 0; i--){
            time[i] = dataInt % 100;
            dataInt = (dataInt - time[i]) / 100;
          }
          life = 0;
          timeInfo.tm_hour = time[0];
          timeInfo.tm_min = time[1];

          Serial.print(timeInfo.tm_hour);
          Serial.print(timeInfo.tm_min);
        }
        //Serial.println("Data content copied to another variable:");
        //Serial.print(dataInt);

      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }

      // リクエストを終了
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
    // 過剰なリクエストを避けるために遅延を追加  // 必要に応じて遅延を調節
  }
  delay(100);
}