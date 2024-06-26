#include <Arduino.h>
#include <SD_MMC.h>
#include <HTTPClient.h>
#include <ETH.h> 

#include <ArduinoJson.h> // need to install this library ArduinoJson by Benoit Blanchon

// Define Structs

typedef struct dataStruct {
  const char* meterId;
  int accumulatedValue;
} dataStruct;

dataStruct meters[] = {
  {"EB8D4250-D3E1-4302-A9A9-526BC223FD6E", 0}, // meter 1
  {"6424A2EE-2C46-4486-B8D9-7931CC6269C2", 0}, // meter 2
  {"BFE517CB-5A23-4786-B535-A733059FA959", 0}, // meter 3
  {"3946301E-1C59-4EE5-87FA-F8246F1DBEF1", 0}  // meter 4
};



// Define Variables
xQueueHandle dataQueue;
SemaphoreHandle_t sdCardMutex;
volatile unsigned long lastDebounceTime[4] = {0, 0, 0, 0};
// int accumulation = 0;

// const char* apiUrl = "http://192.168.5.132:2050/api/EnergyData/Test"; // Jonas IIS Api
// const char* apiUrl = "http://192.168.21.7:2050/api/EnergyData"; // Virtuel Server SKP
const char* apiUrl = "http://10.233.134.113:2050/api/EnergyData"; // energymeter room laptop server

const char* filename = "/EnergyData.csv";

// Define the pins
const int impulsePin1 = 16; // Impulse pin
const int impulsePin2 = 32; // Impulse pin
const int impulsePin3 = 33; // Impulse pin
const int impulsePin4 = 36; // Impulse pin

const int builtInBtn = 34; // bultin button to simmulate impulse


// Define the meter ids
// const char* meterId1 = "CCC6C8C4-B9DB-4C8D-39D8-08DBEF4C21FB";
// const char* meterId2 = "222";
// const char* meterId3 = "333";
// const char* meterId4 = "444";


// define functions
void IRAM_ATTR impulseDetected1();
void IRAM_ATTR impulseDetected2();
void IRAM_ATTR impulseDetected3();
void IRAM_ATTR impulseDetected4();
void IRAM_ATTR buttonTest();

void queueDataHandling(void *pvParameters);
void sendToApi(void *pvParameters);

bool setupSdCard();
bool setupMutex();
bool setupInterrupts();


// setup starts here
void setup() {

  Serial.begin(115200);

  pinMode(impulsePin1, INPUT_PULLDOWN); // sets pin to input
  pinMode(impulsePin2, INPUT);
  pinMode(impulsePin3, INPUT);
  pinMode(impulsePin4, INPUT);
  pinMode(builtInBtn, PULLDOWN);


  ETH.begin();

  setupSdCard();

  setupMutex();

  dataQueue = xQueueCreate(20, sizeof(int*));

  setupInterrupts();

  attachInterrupt(builtInBtn, buttonTest, FALLING);

  xTaskCreate(                  // create a new rtos  task
    queueDataHandling,          // the name of what function will run
    "Queue Data Handling",      // the name of the task
    4096,                       // the stack size of the task
    NULL,                       // the parameter passed to the task
    1,                          // the priority of the task
    NULL                        // the task handle
  );


  xTaskCreate(
    sendToApi,
    "Api Call",
    4096,
    NULL,
    2,
    NULL
  );

  Serial.println("Setup done");
}

// setup functions

bool setupMutex(){
  // create semaphore to lock sd-card access
  while(sdCardMutex == NULL)
  {
    sdCardMutex = xSemaphoreCreateMutex();
  }
  return true;
}


bool setupSdCard(){
   // initialize sd card

  SD_MMC.begin();


  // check if sd card file exists
  // if it does not exist create it

  SD_MMC.remove(filename);

  if(!SD_MMC.exists(filename))
  {
    File file = SD_MMC.open(filename, FILE_WRITE);
    if(file)
    {
      file.println("EnergyMeterID,AccumulatedValue");
    }
    file.close();
  }

  return true;

}


bool setupInterrupts(){
  attachInterrupt(impulsePin1, impulseDetected1, RISING); // sets interrupt when pin goes from low to high

  attachInterrupt(impulsePin2, impulseDetected2, RISING);

  attachInterrupt(impulsePin3, impulseDetected3, RISING); 
    
  attachInterrupt(impulsePin4, impulseDetected4, RISING);
  

  return true;
}




// Interrupt functions

void IRAM_ATTR impulseDetected1() {
  int meter = 0;
  if(millis() - lastDebounceTime[meter] >= 80)
  {
    xQueueSendFromISR(dataQueue, &meter, 0);
    lastDebounceTime[meter] = millis();
  }
}


void IRAM_ATTR impulseDetected2() {
  int meter = 1;
  if(millis() - lastDebounceTime[meter] >= 80)
  {
    xQueueSendFromISR(dataQueue, &meter, 0);
    lastDebounceTime[meter] = millis();
  }
}


void IRAM_ATTR impulseDetected3() {
  int meter = 2;
  if(millis() - lastDebounceTime[meter] >= 80)
  {
    xQueueSendFromISR(dataQueue, &meter, 0);
    lastDebounceTime[meter] = millis();
  }
}


void IRAM_ATTR impulseDetected4() {
  int meter = 3;
  if(millis() - lastDebounceTime[meter] >= 80)
  {
    xQueueSendFromISR(dataQueue, &meter, 0);
    lastDebounceTime[meter] = millis();
  }

}

void IRAM_ATTR buttonTest(){
  int meter = 0;
  if(millis() - lastDebounceTime[meter] >= 80)
  {
    xQueueSendFromISR(dataQueue, &meter, 0);
    lastDebounceTime[meter] = millis();
  }
}


// RTOS Functions
// need to change if the sd_card library isn't working
void queueDataHandling(void *pvParameters){
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  while(1)
  {
    vTaskDelay(20 / portTICK_PERIOD_MS);
    // Serial.println("QueueDataHandling Started");

    int meterIndex;

    // take mutex
    if(xSemaphoreTake(sdCardMutex, portMAX_DELAY) != pdTRUE){
      Serial.println("Mutex failed to be taken within max delay");
      xSemaphoreGive(sdCardMutex);
      return;
    }


    if(xQueueReceive(dataQueue, &meterIndex, 0))
    {
      meters[meterIndex].accumulatedValue++;

      // open sd card
      File file = SD_MMC.open(filename, FILE_APPEND);

      // check if file opened
      if(!file){
        Serial.println("Failed to open file for appending");
        xSemaphoreGive(sdCardMutex);
        return;

      }
      // write data to sd card
    
      file.print(meters[meterIndex].meterId);
      file.print(",");
      file.print(meters[meterIndex].accumulatedValue);

      file.println();
      // close sd card
      file.close();

      // give mutex
      xSemaphoreGive(sdCardMutex);

      // Serial.print(meters[meterIndex].meterId);
      // Serial.print(",");
      // Serial.print(meters[meterIndex].accumulatedValue);
      // Serial.println();

      // data is removed from queue on recieve
    }
    else{
      xSemaphoreGive(sdCardMutex);
    }

  }

}



void sendToApi(void *pvParameters){
  vTaskDelay(10000 / portTICK_PERIOD_MS);
  while(1)
  {
    // Serial.println("sendToApi Started");

    // take mutex
                      // mutex     // max delay
    if(xSemaphoreTake(sdCardMutex, portMAX_DELAY) != pdTRUE){
      Serial.println("Mutex failed to be taken within max delay");
      xSemaphoreGive(sdCardMutex);
      return;
    }

    // read file from sd card
    File file = SD_MMC.open(filename, FILE_READ);

    if(!file){
      xSemaphoreGive(sdCardMutex);
      return;
    }

    // Skip the first line (header)
    if (file.available()) {
      file.readStringUntil('\n');
    }

    int httpResponseCode = 0;

    if(file.available()){


    String payload = "[";

    while (file.available()) {
      String line = file.readStringUntil('\n');
      Serial.println(line);

      // Split the CSV line into values
      String name = line.substring(0, line.indexOf(','));
      String value = line.substring(line.indexOf(',') + 1);

      // Add data to the JSON document
      JsonDocument jsonDocument;
      jsonDocument["meterId"] = name;
      jsonDocument["accumulatedValue"] = value.toInt();


      String temp;
      serializeJson(jsonDocument, temp);
      payload += temp;
      if (file.available())
      {
        payload += ",";
      }
    }

    payload += "]";
    // Serial.println(payload);
    HTTPClient http;
      
    // send data to api
    http.begin(apiUrl);
    http.addHeader("Content-Type", "application/json");

    httpResponseCode = http.POST(payload);

    Serial.println(httpResponseCode);

    // close http.client
    http.end();

    }

    // close file
    file.close();
    if(httpResponseCode == 204)
    {
      // overwrite file
      file = SD_MMC.open(filename, FILE_WRITE);
      if(file)
      {
        file.println("meterId,accumulatedValue");
      }
      file.close();  
    }
    
    // give mutex
    xSemaphoreGive(sdCardMutex);

    vTaskDelay(10000 / portTICK_PERIOD_MS);

  }
}


void loop(){

}