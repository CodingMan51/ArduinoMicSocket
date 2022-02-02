#include <Arduino.h>
#include <WiFi.h>
#include "esp_adc_cal.h"


#define DEFAULT_VREF 1100
esp_adc_cal_characteristics_t *adc_chars;

hw_timer_t * timer = NULL;

#define RATE 10240
#define CHUNK 1024
#define MICPIN ADC1_CHANNEL_0

const int micPin = 35;

uint8_t audioBuffer[CHUNK];
uint8_t sendBuffer[CHUNK];
uint32_t bufferPointer = 0;
volatile bool sendData = false;

TaskHandle_t Task1;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


WiFiClient client;
const char* ssid = "Haimovich";
const char* password = "0503391882";

const char* ip = "10.0.0.7";
const uint16_t port = 9999;

void wifiInit(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("...");
  }

  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());

  while(!client.connect(ip, port)){
    Serial.println("Error Connecting");
    delay(4000);
    return;
  }

  Serial.println("Connected to server successful!");
}


void calAdc(){
  Serial.println("Started up");

  //Range 0-4096
  adc1_config_width(ADC_WIDTH_BIT_12);
  // full voltage range
  adc1_config_channel_atten(MICPIN, ADC_ATTEN_DB_11);

  // check to see what calibration is available
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
  {
    Serial.println("Using voltage ref stored in eFuse");
  }
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
  {
    Serial.println("Using two point values from eFuse");
  }
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_DEFAULT_VREF) == ESP_OK)
  {
    Serial.println("Using default VREF");
  }
  //Characterize ADC
  adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
}


void IRAM_ATTR onTimer(){
  portENTER_CRITICAL_ISR(&timerMux); // para rodar um código crítico sem ser interrompido.
  int milliVolts = adc1_get_voltage(MICPIN);
  // get the calibrated value
  milliVolts = map(milliVolts, 0, 4095, 0, 255);
  audioBuffer[bufferPointer] = milliVolts;
  bufferPointer++;
  if(bufferPointer == CHUNK){
    memmove(sendBuffer, audioBuffer, sizeof(audioBuffer));
    bufferPointer = 0;
    sendData = true;
    
  }
  portEXIT_CRITICAL_ISR(&timerMux); // prioridade no código crítico

}



void setup() {
  Serial.begin(115200);

  wifiInit();
  
  Serial.println("start timer ");

  
  xTaskCreatePinnedToCore(
      sendDataToServer, /* Function to implement the task */
      "Task1", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &Task1,  /* Task handle. */
      0); /* Core where the task should run */
  
  timer = timerBegin(0, 80, true);  // timer 0, MWDT clock period = 12.5 ns * TIMGn_Tx_WDT_CLK_PRESCALE -> 12.5 ns * 80 -> 1000 ns = 1 us
  timerAttachInterrupt(timer, &onTimer, true); // edge (not level) triggered  => 12.5 * prescaler * 0.001 * ticks
  timerAlarmWrite(timer, 100, true); // 1000000 * 1 us = 1 s, autoreload true //100
  timerAlarmEnable(timer); // enable

  
}

void sendDataToServer( void * parameter) {
  for(;;) {
    if(sendData){
      sendData = false;
      client.write((const uint8_t *)sendBuffer, sizeof(sendBuffer));
    }
  }
}

void loop() {
  // nope nothing here
  vTaskDelay(portMAX_DELAY); // wait as much as posible ...
}
