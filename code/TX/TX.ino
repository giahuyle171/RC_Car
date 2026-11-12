#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

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
RadioData radioData(8, 7);

// joy1:a3, joy2:A5; val1:A0, val2:A1; switch1:5, switch2:6;

const byte diachi[6] = "12345";

volatile uint32_t t_controlTask = 0;
volatile uint32_t t_telemetryTask = 0;

void setup() 
{
  Serial.begin(9600);

  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  
  radioData.init();
  radioData.openWritingPipe(diachi);
  radioData.setPALevel(RF24_PA_MAX);
  radioData.setChannel(90);
  radioData.setDataRate(RF24_250KBPS);
  // radioData.setAutoAck(false);
  radioData.stopListening();
}

void loop() 
{
  controlTask();
  telemetryTask();

}