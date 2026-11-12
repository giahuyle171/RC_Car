#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

class RadioData: public RF24 {
  public:
    int16_t data[6] = {0,0,0,0,0,0};
    // joyleft, joyright, val1, val2, switch1, switch2;
    RadioData(int ce_pin, int csn_pin)
      : RF24(ce_pin, csn_pin) {}
    
    void init() {
      if (!RF24::begin()) {
        Serial.println("Error NRF24L01");
        while(1) {}
      }
    }
    
    bool update() {
      if (RF24::available()) {
        RF24::read(&data, sizeof(data));
        return 1;
      }
      return 0;
    }
    
    int& operator[](int i) { return data[i]; }
};

Servo esc;
Servo servo;
RadioData radioData(7, 8);
volatile uint32_t t_controlTask = 0;

volatile uint32_t t_telemetryTask = 0;
int16_t RPM = 0;
int16_t STEERING = 90;

void setup() {
  Serial.begin(9600);
  delay(1000);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(10, OUTPUT);
  servo.attach(10);
  pinMode(9, OUTPUT);
  esc.attach(9, 1000, 2000);


  radioData.init();
  radioData.openReadingPipe(1, (const uint8_t*)"12345");
  radioData.setPALevel(RF24_PA_MAX);
  radioData.setChannel(90);
  radioData.setDataRate(RF24_250KBPS);  
  // radioData.setAutoAck(false);
  radioData.startListening();
}

void loop() {
  controlTask();
  telemetryTask();

}