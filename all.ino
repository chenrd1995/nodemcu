#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <SimpleDHT.h>
#include "Images.h"
//U8G2 初始化头文件//
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display

//U8G2 初始化头文件//
// wifi 参数
const char* ssid = "openwrt"; // 更改WiFi名称
const char* password = "bushemima"; // 更改WiFi密码
const char* host = "192.168.2.155"; // 更改需要访问的域名
const int httpsPort = 888;  // 更改需要访问的端口
const String url = "/sse";  // 需要访问的地址（不需要更改）
HTTPClient http;
WiFiClient client;

//电脑参数
int CPU_FREQ;
int GPU_FREQ;
int CPU_TEMP;
int GPU_TEMP;
int CPU_FAN;
int GPU_FAN;
int CASE_FAN;
int GPU_USE;
int CPU_USE;

//DHT11参数
SimpleDHT11 dht11(D3);
byte temperature =0;
byte humidity =0;

//button参数
const int button = 16;
int button_switch=0;
byte change =0;


void setup() {
  // 启动串口
  
  Serial.begin(500000);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  u8g2.begin();
  u8g2_prepare();
  u8g2.drawXBMP(32, 0, 64, 64, manlogo);
  u8g2.sendBuffer();
  delay(500);
  while (WiFi.status() != WL_CONNECTED) //连接WiFi
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());//打印出开发板的IP地址

  //button
  pinMode(button,INPUT_PULLUP);
}
int value = 0;
void loop() {
  button_switch = digitalRead(button);
  if(button_switch == HIGH){
    if(change == 1)change =0;
    else change = 1; 
    Serial.println("切换模式");
    Serial.println((int)change);
    }
  if(change == 1){
    sendrequest();
    draw_CPU(); // 显示CPU
    delay(2000);
    draw_GPU(); // 显示GPU
    }
  else{
    draw_dht11();
    }

}


void getPrice(String s) {
  int datStart = 0, datEnd = 0;
  String datstr;

  char cpu_freq[] = "CPU_FREQ";
  datStart = s.indexOf(cpu_freq) + strlen(cpu_freq);
  datEnd = s.indexOf("MHz", datStart);
  datstr = s.substring(datStart, datEnd);
  CPU_FREQ = datstr.toInt();


  char gpu_freq[] = "GPU_FREQ";
  datStart = s.indexOf(gpu_freq) + strlen(gpu_freq);
  datEnd = s.indexOf("MHz", datStart);
  datstr = s.substring(datStart, datEnd);
  GPU_FREQ = datstr.toInt();


  char cpu_temp[] = "CPU_TEMP";
  datStart = s.indexOf(cpu_temp) + strlen(cpu_temp);
  datEnd = s.indexOf("℃", datStart);
  datstr = s.substring(datStart, datEnd);
  CPU_TEMP = datstr.toInt();


  char gpu_temp[] = "GPU_TEMP";
  datStart = s.indexOf(gpu_temp) + strlen(gpu_temp);
  datEnd = s.indexOf("℃", datStart);
  datstr = s.substring(datStart, datEnd);
  GPU_TEMP = datstr.toInt();

  char cpu_fan[] = "CPU_FAN";
  datStart = s.indexOf(cpu_fan) + strlen(cpu_fan);
  datEnd = s.indexOf("RPM", datStart);
  datstr = s.substring(datStart, datEnd);
  CPU_FAN = datstr.toInt();


  char gpu_fan[] = "GPU_FAN";
  datStart = s.indexOf(gpu_fan) + strlen(gpu_fan);
  datEnd = s.indexOf("RPM", datStart);
  datstr = s.substring(datStart, datEnd);
  GPU_FAN = datstr.toInt();

  char case_fan[] = "CASE_FAN";
  datStart = s.indexOf(case_fan) + strlen(case_fan);
  datEnd = s.indexOf("RPM", datStart);
  datstr = s.substring(datStart, datEnd);
  CASE_FAN = datstr.toInt();

  char cpu_use[] = "CPU_USE";
  datStart = s.indexOf(cpu_use) + strlen(cpu_use);
  datEnd = s.indexOf("%", datStart);
  datstr = s.substring(datStart, datEnd);
  CPU_USE = datstr.toInt();

  char gpu_use[] = "GPU_USE";
  datStart = s.indexOf(gpu_use) + strlen(gpu_use);
  datEnd = s.indexOf("%", datStart);
  datstr = s.substring(datStart, datEnd);
  GPU_USE = datstr.toInt();

  //  //Serial.println(s);
  //  Serial.print("CPU_FREQ");
  //  Serial.println(dat.CPU_FREQ);
  //  Serial.print("GPU_FREQ");
  //  Serial.println(dat.GPU_FREQ);
  //  Serial.print("CPU_TEMP");
  //  Serial.println(dat.CPU_TEMP);
  //  Serial.print("GPU_TEMP");
  //  Serial.println(dat.GPU_TEMP);
  //  Serial.print("CPU_FAN");
  //  Serial.println(dat.CPU_FAN);
  //  Serial.print("GPU_FAN");
  //  Serial.println(dat.GPU_FAN);
  //  Serial.print("CASE_FAN");
  //  Serial.println(dat.CASE_FAN);
  //
  //  Serial.println();
}

void sendrequest()
{
  WiFiClient client;

  /**
      测试是否正常连接
  */
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  delay(10);
  String postRequest = (String)("GET ") + url + " HTTP/1.1\r\n" +
                       "Content-Type: text/html;charset=utf-8\r\n" +
                       "Host: " + host + "\r\n" +
                       "User-Agent: BuildFailureDetectorESP8266\r\n" +
                       "Connection: close\r\n\r\n";
  client.print(postRequest);  // 发送HTTP请求
  client.print(postRequest);  // 发送HTTP请求
  /**
     展示返回的所有信息
  */

  char buffer[420];
  int numdata = 0;
  delay(100);
  numdata = client.readBytes(buffer, 421);
  String temp = String(buffer);
  Serial.println(temp);
  getPrice(temp);
  delay(1000);
}
void u8g2_prepare(void)
{
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

void draw_GPU(void)
{
  u8g2_prepare();
  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 0, gpulogow, gpulogoh, gpulogo);
  u8g2.drawXBMP(58, 0, temlogow, temlogoh, temlogo);
  u8g2.drawXBMP(58, 23, freqlogow, freqlogoh, freqlogo);
  u8g2.drawXBMP(0, 43, uselogow, uselogoh, uselogo);
  u8g2.drawXBMP(58, 43, fan1logow, fan11ogoh, fan1logo);
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.setCursor(75, 0);
  u8g2.print(GPU_TEMP);
  //  u8g2.setFont(u8g2_font_unifont_t_symbols);
  //  u8g2.setFontPosTop();
  //  u8g2.drawUTF8(100, 0, "℃");
  u8g2.drawStr(100, 0, "℃");
  u8g2.setCursor(75, 22);
  u8g2.print(GPU_FREQ);
  u8g2.setCursor(21, 45);
  u8g2.drawStr(42, 45, "%");
  u8g2.print(GPU_USE);
  u8g2.setCursor(75, 44);
  u8g2.print(GPU_FAN);
  u8g2.sendBuffer();
}

void draw_CPU(void)
{
  u8g2_prepare();
  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 0, cpulogow, cpulogoh, cpulogo);
  u8g2.drawXBMP(58, 0, temlogow, temlogoh, temlogo);
  u8g2.drawXBMP(110, 0, shelogow, shelogoh, shelogo);
  u8g2.drawXBMP(58, 23, freqlogow, freqlogoh, freqlogo);
  u8g2.drawXBMP(0, 43, uselogow, uselogoh, uselogo);
  u8g2.drawXBMP(58, 43, fan1logow, fan11ogoh, fan1logo);
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.setCursor(85, 0);
  u8g2.print(CPU_TEMP);
  //  u8g2.setFont(u8g2_font_unifont_t_symbols);
  //  u8g2.setFontPosTop();
  //  u8g2.drawUTF8(100, 0, "℃");
  u8g2.setCursor(75, 22);
  u8g2.print(CPU_FREQ);
  u8g2.setCursor(21, 45);
  u8g2.drawStr(42, 45, "%");
  u8g2.print(CPU_USE);
  u8g2.setCursor(75, 44);
  u8g2.print(CPU_FAN);
  u8g2.sendBuffer();
}

void get_dht_data(){
  Serial.println("===========================================");
  Serial.println("DHT11");
  

  int err = SimpleDHTErrSuccess; 

  if((err = dht11.read(&temperature,&humidity,NULL))!= SimpleDHTErrSuccess){
    Serial.print("Read DHT11 FAILED,Error =");
    Serial.println(err);
    delay(1000);
    return;
  }
  Serial.print("DHT11 Reading:");
  Serial.print((int)temperature);Serial.print(" ℃, ");
  Serial.print((int)humidity);Serial.print(" %, \n");
  delay(1500);
}

void draw_dht11(){
  get_dht_data();
  u8g2_prepare();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(0, 0, "Tem:");
  u8g2.setCursor(55, 0);
  u8g2.print(temperature);
  u8g2.drawStr(80, 0, "°C");
  u8g2.drawStr(0, 30, "Hum:");
  u8g2.setCursor(60, 30);
  u8g2.print(humidity);
  u8g2.drawStr(85, 30, "%");
  u8g2.sendBuffer();
}
