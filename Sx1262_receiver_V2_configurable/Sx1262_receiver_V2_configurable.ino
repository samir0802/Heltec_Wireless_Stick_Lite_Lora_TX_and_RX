// include the library
#include <RadioLib.h>
#include <SPI.h>
#include <Preferences.h>
#include <MD5.h>

// SX1262 has the following connections with esp32 S3:
#define SCK_PIN   9     // SCK pin (clock)
#define MISO_PIN  11    // MISO pin (master-in, slave-out)
#define MOSI_PIN  10    // MOSI pin (master-out, slave-in)
#define NSS_pin   8
#define DIO1_pin  14
#define NRST_pin  12
#define BUSY_pin  13

//--------------------LORA Configuration-------------------------------//
#define LORA_Frequency    868.0   // MHz
#define LORA_Bandwidth    125.0   // KHz 
#define LORA_SF           12      // 7...12
#define LORA_CodingRate   5       // 4/5, 4/6, 4/7, 4/8
#define LORA_SyncWord     0x081   // Unique Id  // test id 0x123
#define LORA_Power        22      // Dbm
#define LORA_PreambleLen  8       // symbols
#define LORA_CurrentLim   140     // Ma 

//#define INITIATING_NODE                     //Set as transmitter
SX1262 radio = new Module(NSS_pin, DIO1_pin, NRST_pin, BUSY_pin);
Preferences preferences;
int lora_SF;
int lora_CR;
float lora_BW; 
float lora_FQ;
bool lora_rx_booster;
bool lora_retransmitter_enabled = false; 
String sending_packet;
String Receiver_name = "CS";              // Reveiver Name


void setup() {
  Serial.begin(9600);
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN);  //Initialize SPI on Custom Pins for LORA
  
 preferences.begin("Sx1262_lora");
  sending_packet  = preferences.getString("lora_content", "Chandragiri Test");
  lora_SF         = preferences.getInt("lora_SF", 12);
  lora_BW         = preferences.getFloat("lora_BW", 125.0);
  lora_CR         = preferences.getInt("lora_CR", 5);
  lora_FQ         = preferences.getFloat("lora_FQ", 868.0);
  lora_rx_booster = preferences.getBool("lora_rx_booster", false);
  preferences.end();

  Serial.println("-----------------");
  // Serial.print("[Info] Lora Content: ");
  // Serial.println(sending_packet);
  Serial.print("[Info] Lora SF: ");
  Serial.println(lora_SF);
  Serial.print("[Info] Lora Bandwidth: ");
  Serial.println(lora_BW);
  Serial.print("[Info] Lora Coding Rate: ");
  Serial.println(lora_CR);
  Serial.print("[Info] Lora Frequency: ");
  Serial.println(lora_FQ);

 
  // initialize SX1262 with default settings
  Serial.print(F("[SX1262] Initializing ... "));
  int state = radio.begin(lora_FQ, lora_BW, lora_SF, LORA_CodingRate, LORA_SyncWord, LORA_Power, LORA_PreambleLen);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }
   radio.setRxBoostedGainMode(lora_rx_booster);
   radio.setPacketReceivedAction(setFlag);

  // start listening for LoRa packets
  Serial.print(F("[SX1262] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    // while (true);
  }

  
     String instruction = "\n\n\n--------------- To Change Parameters-----------\n";
    //  instruction += "Type 'lora_content=" + sending_packet+ "', Set the content to be sent through LORA \n";
     instruction += "Type 'lora_FQ=" + String(lora_FQ) + "', Accepted values are in Float, eg: 865.125";
     instruction += "Type 'lora_SF=" + String(lora_SF) + "', to set the Lora SF-> Spreading Factor 7-12 \n";
     instruction += "Type 'lora_BW=" + String(lora_BW) + "', Acceped Input is 1-10 \n\t1:7.8Khz \n\t2:10.4Khz";
     instruction += "\n\t3:15.6Khz \n\t4:20.8Khz  \n\t5:31.25Khz \n\t6:41.7Khz \n\t7:62.5Khz \n\t8:125.0Khz \n\t9:250.0Khz \n\t10:500.0Khz \n\n";
     instruction += "Type 'lora_CR=" + String(lora_CR) + "', Acceped Input is 5-8 \n\t5:4/5 \n\t6:4/6";
     instruction += "\n\t7:4/7 \n\t8:4/8 \n\n";
      instruction += "Type 'lora_Rx_Boost=" + String(lora_rx_booster) + "', 1-> Enable, 0->Disable";
     Serial.print(instruction);

}

void handle_acknowledgement(String recv_hash, int tx_rssi, int rx_state);
// flag to indicate that a packet was received
volatile bool receivedFlag = false;
ICACHE_RAM_ATTR
void setFlag(void) {
  receivedFlag = true; // we got a packet, set the flag
}


void loop() {  
 bool serial_debug;
 Serial_config();

 //----------------------------- Handle Lora Receiving -----------------------------------//
  //  if(receivedFlag) {
  //   // reset flag
  //   receivedFlag = false;

  //   String received_message;
  //   int state = radio.readData(received_message);

  //   if (state == RADIOLIB_ERR_NONE) {
  //     // packet was successfully received
  //     Serial.println(F("[SX1262] Received packet!"));

  //     // print data of the packet
  //     Serial.print(F("[SX1262] Data:\t\t"));
  //     Serial.println(received_message);

  //     // print RSSI (Received Signal Strength Indicator)
  //     Serial.print(F("[SX1262] RSSI:\t\t"));
  //     Serial.print(radio.getRSSI());
  //     Serial.println(F(" dBm"));

  //     // print SNR (Signal-to-Noise Ratio)
  //     Serial.print(F("[SX1262] SNR:\t\t"));
  //     Serial.print(radio.getSNR());
  //     Serial.println(F(" dB"));

  //     // print frequency error
  //     Serial.print(F("[SX1262] Frequency error:\t"));
  //     Serial.print(radio.getFrequencyError());
  //     Serial.println(F(" Hz"));

  //   } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
  //     // packet was received, but is malformed
  //     Serial.println(F("CRC error!"));

  //   } else {
  //     // some other error occurred
  //     Serial.print(F("failed, code "));
  //     Serial.println(state);

  //   }


  // }
  receive_pakage();
}

void Serial_config(){           // Device Configuration Via Serial terminal  

 if (Serial.available()) {   
   String set_lora_content = "lora_content=";
   String set_lora_SF = "lora_SF=";
   String set_lora_freq = "lora_FQ=";
   String set_lora_bandwidh = "lora_BW=";
   String set_lora_codingrate = "lora_CR=";
   String enable_rx_booster = "lora_Rx_Boost=";

  
    String receivedData = Serial.readStringUntil('\n');
    if (receivedData.startsWith("hello") || receivedData.startsWith("AT") || receivedData.startsWith("ramlaxman")) {
     String instruction = "\n\n\n--------------- To Change Parameters-----------\n";
    //  instruction += "Type 'lora_content=" + sending_packet+ "', Set the content to be sent through LORA \n";
     instruction += "Type 'lora_FQ=" + String(lora_FQ) + "', Accepted values are in Float, eg: 865.125";
     instruction += "Type 'lora_SF=" + String(lora_SF) + "', to set the Lora SF-> Spreading Factor 7-12 \n";
     instruction += "Type 'lora_BW=" + String(lora_BW) + "', Acceped Input is 1-10 \n\t1:7.8Khz \n\t2:10.4Khz";
     instruction += "\n\t3:15.6Khz \n\t4:20.8Khz  \n\t5:31.25Khz \n\t6:41.7Khz \n\t7:62.5Khz \n\t8:125.0Khz \n\t9:250.0Khz \n\t10:500.0Khz \n\n";
     instruction += "Type 'lora_CR=" + String(lora_CR) + "', Acceped Input is 5-8 \n\t5:4/5 \n\t6:4/6";
     instruction += "\n\t7:4/7 \n\t8:4/8 \n\n";
      instruction += "Type 'lora_Rx_Boost=" + String(lora_rx_booster) + "', 1-> Enable, 0->Disable";
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
    
    } else if (receivedData.startsWith(enable_rx_booster)){
        preferences.begin("Sx1262_lora");
        String data = receivedData.substring(enable_rx_booster.length());
        int value = data.toInt();
        if(value > 0)
          lora_rx_booster = true; 
          else
          lora_rx_booster = false;

        radio.setRxBoostedGainMode(lora_rx_booster);
        preferences.putBool("lora_rx_booster", lora_rx_booster);
        preferences.end();
        Serial.println("[Info] Saved successfully...\t\t Sx1262 Rx Booster: " + String(lora_rx_booster));
    
    }
    
     else if (receivedData.startsWith(set_lora_freq)){         // [LORA Frequency]
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
// flag to indicate that a packet was received
int transmissionState = RADIOLIB_ERR_NONE;
int packetlost_counter;
bool IsRetransmittedPackage = false;
void receive_pakage(){
  int RSSI; 
  String received_message;

 if(receivedFlag)
 {  

    String message_to_retransmit; 
     int state = radio.readData(received_message);
     Serial.print("LORA Received Message: ");
     Serial.println(received_message);
     RSSI = radio.getRSSI();

      if(received_message.indexOf(Receiver_name) == -1){                          // Return this function if ID doesnot match
      Serial.println(F("[Warning] This Transmitter Type is not supported")); 
      receivedFlag = false;
      return;                                                                   // if received data doesnot specify name Of this device, return from listening.   
      } else {
      
        if (received_message.startsWith("RTP_")){ // RTP-> Re-Transmitted Package
          Serial.println("[Info] Retransmitted Pakage Received... ");
          IsRetransmittedPackage = true; 
          received_message = received_message.substring(4); //Clear early RTP char from the string
        }
        //If messgae not Retransmitted message, retransmit it so that it reaches to other receivers
        else { 
        IsRetransmittedPackage = false; 
        message_to_retransmit = "RTP_" + received_message; // 
        }
      }
  
  
    //-----------------------Generate a MD5 Hash to send as acknowledge to signal a successful transmission----------------------//
      char inputString[300];                          // Adjust the size as per your requirements
      strcpy(inputString, received_message.c_str());
      unsigned char* hash=MD5::make_hash(inputString);
      char *md5str = MD5::make_digest(hash, 16);
        free(hash);
      
        // Extract the last characters from the hash string for comparison
        int numCharsToExtract = 4;
        String extractedhash;
        String hashString(md5str);  // Convert char array into String
        free(md5str);               // free char array from memory

        extractedhash = hashString.substring(hashString.length() - 4); 
        Serial.print("\n\n[Info] MD-5 Hash of Received LORA signal : ");
        Serial.println(hashString);
    //----------------------------------------------------------------------------//

      if (state == RADIOLIB_ERR_NONE) {                 // If packet was successfully received
      Serial.println(F("[SX1262] Received LORA packet!"));
      Serial.print(F("[SX1262] Data:\t\t"));
      Serial.println(received_message);

      Serial.print(F("[SX1262] RSSI:\t\t"));            // Higher is better
      Serial.print(RSSI);
      Serial.println(F(" dBm"));

      // print SNR (Signal-to-Noise Ratio)
      Serial.print(F("[SX1262] SNR:\t\t"));             // Higher is better
      Serial.print(radio.getSNR());
      Serial.println(F(" dB"));

      // print frequency error
      Serial.print(F("[SX1262] Freq error:\t"));        // Lower is Better
      Serial.print(radio.getFrequencyError());
      Serial.println(F(" Hz"));
      
      handle_acknowledgement(extractedhash, RSSI, state);      // Send acknowledgement before processing data further.       
      if(IsRetransmittedPackage != true){                      // If messgae not already Retransmitted message, retransmit so it reaches to other receivers
      if (lora_retransmitter_enabled)                          // If receiver configured as retransmitter, Retransmit the packet
       handle_data_retransmission(message_to_retransmit);
      }
      start_data_receiving();
    } 
    else {     
     if (state == RADIOLIB_ERR_CRC_MISMATCH) {                    // Checksum didnot match
        Serial.println(F("\n\n[Warning] CRC error!...\t"));
        packetlost_counter++;
        Serial.print("[Warning] Packet Lost: ");
        Serial.println(packetlost_counter); 
        } else{
        Serial.println(F("[Warning] Receiving Failed, code "));
        Serial.println(state);
        }
        handle_acknowledgement(extractedhash, RSSI, state);        // Send acknowledgement before processing data further.       
        start_data_receiving();
     } 
  } 
} 

void handle_acknowledgement(String recv_hash, int tx_rssi, int rx_state){
   //-------------------------------------- send acknowledgement of signal reception -----------------------------------//
    //Acknowledgemnt data structure :- Receiver name + received State  + Received Signal strength

    transmissionState = radio.transmit(recv_hash + "," + String(rx_state) + "RSSI=" + String(tx_rssi));    
    Serial.println("\n\n[Info] Sending LORA Acknowledgement: " + transmissionState);
     if (transmissionState == RADIOLIB_ERR_NONE) 
     { 
      Serial.println(F("[Info] Acknowledgement Sending finished!"));
      Serial.print(F("[Info] Data Speed:\t"));
      Serial.print(radio.getDataRate());
      Serial.println(F(" bps"));
     }
     else 
     {
      Serial.print(F("[Warning] sending failed, code "));
      Serial.println(transmissionState);
     }     
}


// After the received acknowledgement is sent to the transmitter, retransmit the data to another receiver
void handle_data_retransmission (String message_to_retransmit){
  transmissionState = radio.transmit(message_to_retransmit);    
    Serial.println("\n\n[Info] Retransmitting LORA Packet: " + transmissionState);
     if (transmissionState == RADIOLIB_ERR_NONE) 
     { 
      Serial.println(F("[Info] Packet Retransmission Completed..."));
      Serial.print(F("[Info] Data Speed:\t"));
      Serial.print(radio.getDataRate());
      Serial.println(F(" bps"));
     }
     else 
     {
      Serial.print(F("[Warning] Retransmission failed, code "));
      Serial.println(transmissionState);
     }
}

void start_data_receiving(){
      radio.finishTransmit();
      radio.startReceive();   // Activate Receiving Mode
      receivedFlag = false;
      Serial.println("\n[Info] LORA Receiving Started...");
}

