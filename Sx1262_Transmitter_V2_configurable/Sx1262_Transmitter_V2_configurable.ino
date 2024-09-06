// include the library
#include <RadioLib.h>
#include <SPI.h>
#include <Preferences.h>

// SX1262 has the following connections with esp32 S3:
#define SCK_PIN   9     // SCK pin (clock)
#define MISO_PIN  11    // MISO pin (master-in, slave-out)
#define MOSI_PIN  10    // MOSI pin (master-out, slave-in)
#define NSS_pin   8
#define DIO1_pin  14
#define NRST_pin  12
#define BUSY_pin  13

//--------------------LORA Configuration-------------------------------//
// #define LORA_Frequency    868.0    // MHz
// #define LORA_Bandwidth    125.0    // KHz 
// #define LORA_SF           12       // 7...12
#define LORA_CodingRate   5           // 4/5, 4/6, 4/7, 4/8
#define LORA_SyncWord     0x123       // Unique Id
#define LORA_Power        22          // Dbm
#define LORA_PreambleLen  8           // symbols
#define LORA_CurrentLim   140         // Ma 

const int wifi_led = 35;             // Built in LED for Heltecwireless stick V3.
const int ADC_Enable_Pin = 37;  //turn low to activate reading
const int ADC_PIN = 1;          // ADC0 pin

const float VOLTAGE_DIVIDER_RATIO = 4.9;  // surce voltage/adc voltage Voltage divider ratio
const float VOLTAGE_REF = 3.28;           // Reference voltage (in volts)
const float BATTERY_MAX_VOLTAGE = 4.2;    // Maximum voltage of the battery (in volts)
const float BATTERY_MIN_VOLTAGE = 3.2;    // Minimum voltage of the battery (in volts)



//#define INITIATING_NODE                     //Set as transmitter
SX1262 radio = new Module(NSS_pin, DIO1_pin, NRST_pin, BUSY_pin);
Preferences preferences;

// save transmission state between loops
int transmissionState = RADIOLIB_ERR_NONE;
int percentage;
int lora_SF;
float lora_BW;
int  lora_CR;
float lora_FQ;
String sending_packet;


void setup() {
  Serial.begin(9600);                                  
  Serial.print(F("[SX1262] Initializing ... ")); 
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN);  //Initialize SPI on Custom Pins for LORA
  pinMode(ADC_Enable_Pin, OUTPUT);
  pinMode(ADC_PIN, INPUT);
  pinMode(wifi_led, OUTPUT);
  analogReadResolution(12); // Set ADC resolution to 12 bits (0-4095)
 
  preferences.begin("Sx1262_lora");
  sending_packet  = preferences.getString("lora_content", "Chandragiri Test");
  lora_SF         = preferences.getInt("lora_SF", 12);
  lora_BW         = preferences.getFloat("lora_BW", 125.0);
  lora_CR         = preferences.getFloat("lora_CR", 5);
  lora_FQ         = preferences.getFloat("lora_FQ", 868.0);
  preferences.end();

  Serial.println("-----------------");
  Serial.print("[Info] Lora Content: ");
  Serial.println(sending_packet);
  Serial.print("[Info] Lora SF: ");
  Serial.println(lora_SF);
  Serial.print("[Info] Lora Bandwidth: ");
  Serial.println(lora_BW);
  Serial.print("[Info] Lora Coding Rate: ");
  Serial.println(lora_CR);
  Serial.print("[Info] Lora Frequency: ");
  Serial.println(lora_FQ);

  
  int state;
  state = radio.begin(lora_FQ, lora_BW, lora_SF, LORA_CodingRate, LORA_SyncWord, LORA_Power, LORA_PreambleLen);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
    radio.setCurrentLimit(LORA_CurrentLim);
    radio.setSpreadingFactor(lora_SF);
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

     String instruction = "\n\n\n--------------- To Change Parameters-----------\n";
     instruction += "Type 'lora_content=" + sending_packet+ "', Set the content to be sent through LORA \n";
     instruction += "Type 'lora_FQ=" + String(lora_FQ) + "', Accepted values are in Float, eg: 865.125";
     instruction += "Type 'lora_SF=" + String(lora_SF) + "', to set the Lora SF-> Spreading Factor 7-12 \n";
     instruction += "Type 'lora_BW=" + String(lora_BW) + "', Acceped Input is 1-10 \n\t1:7.8Khz \n\t2:10.4Khz";
     instruction += "\n\t3:15.6Khz \n\t4:20.8Khz  \n\t5:31.25Khz \n\t6:41.7Khz \n\t7:62.5Khz \n\t8:125.0Khz \n\t9:250.0Khz \n\t10:500.0Khz \n\n";
     instruction += "Type 'lora_CR=" + String(lora_CR) + "', Acceped Input is 5-8 \n\t5:4/5 \n\t6:4/6";
     instruction += "\n\t7:4/7 \n\t8:4/8 \n\n"; 
     Serial.print(instruction);
 
} //End setup
int count = 0;                  // counter to keep track of transmitted packet

void loop() {
  bool serial_debug;  
  float batt_volt  =  calculate_batt_vol(false);
  int Percentage   = percentage;    
    // transmit_message("RamLaxman Range Test, Chandragiri! Batt V" + String(batt_volt) + "Per" + String(Percentage) + "% Counter: " + String(count++));  
    transmit_message(sending_packet + " "+String(count++));
    delay(5000);
    Serial_config(); // Allow user to change device configuration Via Serial Monitor

}

void Serial_config(){           // Device Configuration Via Serial terminal  

 if (Serial.available()) {   
   String set_lora_content = "lora_content=";
   String set_lora_SF = "lora_SF=";
   String set_lora_freq = "lora_FQ=";
   String set_lora_bandwidh = "lora_BW=";
   String set_lora_codingrate = "lora_CR=";

  
  String receivedData = Serial.readStringUntil('\n');
    if (receivedData.startsWith("hello") || receivedData.startsWith("AT") || receivedData.startsWith("ramlaxman")) {
     String instruction = "\n\n\n--------------- To Change Parameters-----------\n";
     instruction += "Type 'lora_content=" + sending_packet+ "', Set the content to be sent through LORA \n";
     instruction += "Type 'lora_FQ=" + String(lora_FQ) + "', Accepted values are in Float, eg: 865.125";
     instruction += "Type 'lora_SF=" + String(lora_SF) + "', to set the Lora SF-> Spreading Factor 7-12 \n";
     instruction += "Type 'lora_BW=" + String(lora_BW) + "', Acceped Input is 1-10 \n\t1:7.8Khz \n\t2:10.4Khz";
     instruction += "\n\t3:15.6Khz \n\t4:20.8Khz  \n\t5:31.25Khz \n\t6:41.7Khz \n\t7:62.5Khz \n\t8:125.0Khz \n\t9:250.0Khz \n\t10:500.0Khz \n\n";
     instruction += "Type 'lora_CR=" + String(lora_CR) + "', Acceped Input is 5-8 \n\t5:4/5 \n\t6:4/6";
     instruction += "\n\t7:4/7 \n\t8:4/8 \n\n";
     Serial.print(instruction);
    }

    else if (receivedData.startsWith(set_lora_content)){             // [LORA Content]
        preferences.begin("Sx1262_lora");
        String data = receivedData.substring(set_lora_content.length());
        preferences.putString("lora_content", data);
        sending_packet = data;
        preferences.end();
        Serial.println("[Info] Saved successfully... LORA Content: " + String(data));

    } else if (receivedData.startsWith(set_lora_SF)){
        preferences.begin("Sx1262_lora");
        String data = receivedData.substring(set_lora_SF.length());
        int value = data.toInt();
        lora_SF = value;
        radio.setSpreadingFactor(lora_SF);

        preferences.putInt("lora_SF", value);
        preferences.end();
        Serial.println("[Info] Saved successfully...\t\t Sx1262 LORA SF: " + String(value));
    
    }   else if (receivedData.startsWith(set_lora_bandwidh)){         //[LORA Bandwidh]
        String data = receivedData.substring(set_lora_bandwidh.length());
        int value = data.toInt();
        
        switch(value){
           case 1 :
             lora_BW = 7.8;
            break;

            case 2 :
             lora_BW = 10.4;
            break;
             
            case 3 :
             lora_BW = 15.6;
            break;
            
            case 4 :
             lora_BW = 20.8;
            break;
            
            case 5 :
             lora_BW = 31.25;
            break;
            
            case 6 :
             lora_BW = 41.7;
            break;
            
            case 7 :
             lora_BW = 62.5;
            break;
             
            case 8 :
             lora_BW = 125.0;
            break;
            
            case 9 :
             lora_BW = 250.0;
             break;
                          
            case 10 :
             lora_BW = 500.0;
             break;
             
             default : 
               lora_BW = 125.0;
              break;
        }
                        
        preferences.begin("Sx1262_lora");
        radio.setBandwidth(lora_BW);
        preferences.putFloat("lora_BW", lora_BW);
        preferences.end();
        Serial.println("[Info] Saved successfully...\t\t Sx1262 LORA_BW: " + String(lora_BW));
    
    } else if (receivedData.startsWith(set_lora_codingrate)){
        preferences.begin("Sx1262_lora");
        String data = receivedData.substring(set_lora_codingrate.length());
        int value = data.toInt();
        lora_CR = value;
        radio.setCodingRate(lora_CR);

        preferences.putInt("lora_CR", value);
        preferences.end();
        Serial.println("[Info] Saved successfully...\t\t Sx1262 LORA CR: " + String(value));
    
    } else if (receivedData.startsWith(set_lora_freq)){         // [LORA Frequency]
        preferences.begin("Sx1262_lora");
        String data = receivedData.substring(set_lora_freq.length());
        float value = data.toFloat();
        lora_FQ = value;      
        preferences.putFloat("lora_FQ", lora_FQ);
        preferences.end();
        Serial.println("[Info] Saved successfully...\t\t Sx1262 LORA_FQ: " + String(lora_FQ));
        Serial.println("Restarting Device to Transmit in Set Frequency Channel");
        delay(2000);
        ESP.restart();
   
   } else {
      Serial.println("\nInvalid Input Parameter!!!");
   } // End of serial Config
  }
}


void transmit_message(String sending_packet){
    static long transmission_timer;
    String str = sending_packet;
    
    Serial.print("[Info] LORA Sending: ");
    Serial.println(str);
    digitalWrite(wifi_led, HIGH);

    transmission_timer = millis(); // record the instance of transmission
    transmissionState = radio.transmit(str);

    // the packet was successfully transmitted
    if (transmissionState == RADIOLIB_ERR_NONE) {
    digitalWrite(wifi_led, !HIGH);
    Serial.println(F("[Info] success!"));



    // print measured data rate
    Serial.print(F("[SX1262] Datarate:\t"));
    Serial.print(radio.getDataRate());
    Serial.println(F(" bps"));
    Serial.print(F("[Info] Transmission Time: "));
    Serial.println(millis() - transmission_timer);
    Serial.print("\n\n");

  } else if (transmissionState == RADIOLIB_ERR_PACKET_TOO_LONG) {
    // the supplied packet was longer than 256 bytes
    Serial.println(F("too long!"));
    digitalWrite(wifi_led, !HIGH);

  } else if (transmissionState == RADIOLIB_ERR_TX_TIMEOUT) {
    // timeout occured while transmitting packet
    Serial.println(F("timeout!"));
    digitalWrite(wifi_led, !HIGH);


  } else {
    // some other error occurred
    Serial.print(F("failed, code "));
    Serial.println(transmissionState);
    digitalWrite(wifi_led, !HIGH);
  }
}
float calculate_batt_vol(bool serial_debug){
 
 digitalWrite(ADC_Enable_Pin , LOW);
 int adcValue = analogRead(ADC_PIN);
 float voltage = map(adcValue, 0, 4095, 0, 33);
 voltage = ((voltage/10)*(490))/95;
 voltage += 0.1;
  // Calculate battery percentage
  percentage = map(voltage, 3, 4.2, 0, 100);
  percentage = constrain(percentage, 0, 100) ;

  // Print the voltage and battery percentage
  if(serial_debug){ 
  Serial.print("Voltage: ");
  Serial.print(voltage, 2);         // Print voltage with 2 decimal places
  Serial.print("V, Percentage: ");
  Serial.print(percentage);
  Serial.println("%\n");
  }
  return voltage; 
}

