#include <stdio.h>
#define M5STACK_MPU6886 
#include <M5Stack.h>
#include <utility/MahonyAHRS.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;

float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;

float pitch = 0.0F;
float roll  = 0.0F;
float yaw   = 0.0F;

//[追加]GyroZのデータを蓄積するための変数
float stockedGyroZs[10];
int stockCnt=0;
float adjustGyroZ=0;
int stockedGyroZLength=0;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      M5.Lcd.println("connect");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      M5.Lcd.println("disconnect");
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
    M5.Lcd.println("read");
    pCharacteristic->setValue("Hello World!");
  }

  void onWrite(BLECharacteristic *pCharacteristic) {
    M5.Lcd.println("write");
    std::string value = pCharacteristic->getValue();
    M5.Lcd.println(value.c_str());
  }
};

void getAhrs() {
   // put your main code here, to run repeatedly:
  M5.IMU.getGyroData(&gyroX,&gyroY,&gyroZ);
  M5.IMU.getAccelData(&accX,&accY,&accZ);
  //[変更]これは使わない
  // M5.IMU.getAhrsData(&pitch,&roll,&yaw); 

  //[追加]起動時にstockedGyroZLengthの数だけデータを貯める
  if(stockCnt<stockedGyroZLength){
    stockedGyroZs[stockCnt]=gyroZ;
    stockCnt++;
  }else{
    if(adjustGyroZ==0){
      for(int i=0;i<stockedGyroZLength;i++){
        adjustGyroZ+=stockedGyroZs[i]/stockedGyroZLength;
      }
    }
    //貯めたデータの平均値を使ってgyroZを補正する
    gyroZ-=adjustGyroZ; 
    //ここでaccelデータと補正したgyroデータを使ってpitch・roll・yawを計算する
    MahonyAHRSupdateIMU(gyroX * DEG_TO_RAD, gyroY * DEG_TO_RAD, gyroZ * DEG_TO_RAD, accX, accY, accZ, &pitch, &roll, &yaw);
  }
}

void setup() {
  Serial.begin(115200);
  M5.begin();
  M5.setWakeupButton(BUTTON_A_PIN);
  M5.IMU.Init();
  M5.Lcd.println("BLE start.");
  m5.Speaker.mute();

  BLEDevice::init("m5-stack");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_INDICATE
                                       );
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  if(M5.BtnA.wasPressed()) {
    M5.powerOFF();
  }
  if (deviceConnected) {
//    M5.IMU.getAhrsData(&pitch,&roll,&yaw);
    getAhrs();
    char buf[128];
    sprintf(buf, "{\"x\": %f, \"y\": %f, \"z\": %f}", accX, accY, accZ);
    pCharacteristic->setValue(buf);
    pCharacteristic->notify();
  }
  M5.update();
}
