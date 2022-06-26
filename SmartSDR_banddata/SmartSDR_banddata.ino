/*
Bandata implementation for Arduino Zero/MKRZERO/Empyrean
Will not work with AVR series Arduino, due to lack of speed. 

Made to be used with transverters - one board in each transverter
Gives posibility to daisy-chain transverters into one single coax. 

Observe that the band data (max and min frequency) are integers!

Uses DHCP by default, Fixed IP can be set. 

  Thomas S. Knutsen LA3PNA 25/6-2022 

 */
#include <SPI.h>
#include <Ethernet.h>


 int freqmin = 68;  // this defines the lower frequency it will trigger on
 int freqmax = 72; // this defines the higher frequency it will trigger on
 
 int  led = 32; // port for LED
 int ena = 7;   // port for enable PTT
 int seq = 8;  // sequencer enable

// define debug  // uncomment to output serial data onto USB port for debugging


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xAA, 0xAA, 0xBA, 0xAD, 0xAA, 0xAA
};

// Enter the IP address of the radio here:
IPAddress server(192, 168, 1, 60);

// if the board will have fixed IP, set it here: 
IPAddress ip(192, 168, 1, 65);

EthernetClient client;

void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  pinMode(led,OUTPUT);
  digitalWrite(led,LOW);
  pinMode(ena,OUTPUT);
  digitalWrite(ena,LOW);  
  pinMode(seq,OUTPUT);
  digitalWrite(seq,LOW);

  // start the Ethernet connection:
  Ethernet.begin(mac);
  //   Ethernet.begin(mac, ip);

  
  #ifdef debug
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  #endif

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }

  reloop:
  while (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
    delay(500);
    goto reloop; // yeah the dreaded goto! world will end!
  }

  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server, 4992)) {
    Serial.println("connected");
    delay(1000);
    client.println("c1|sub slice all");
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
    goto reloop; //end of the world due to goto!
    // here goes the code to light the red led for error...
    // probably should code up a while() to have it retry...
  }
}

String mystr;
int slice = -1;
bool enable = false;
bool connection = false;

void loop() {

  if (client.available()) {

            String c=client.readStringUntil('\n');
            connection = true;
            int index = c.indexOf("RF_frequency");
            int indexslice = c.indexOf("slice");
            if(index >= 0){
              
              String frequency = c.substring(index+13, index+21);
              int freq = frequency.toInt();
              String slices = c.substring(indexslice+6,indexslice+7);
              int slicetemp = slices.toInt();
      #ifdef debug
              Serial.println(c);
              Serial.print(index);
              Serial.print(" found freq  ");
              Serial.print(freq);
              Serial.print(" slice: ");
              Serial.print(slice);
              Serial.print(" slicetemp: ");
              Serial.print(slicetemp);
              Serial.println();
      #endif
            
              if((freq<freqmax)&&(freq>freqmin)){
                #ifdef debug
                Serial.println(" frequency within limits ");
                #endif
                // here goes code to give a output
                enable = HIGH;
                slice = slicetemp;
                
                }

                if((slice == slicetemp)&&(slice > -1))
                {
                 if (!((freq<freqmax)&&(freq>freqmin)))
                 {
                  #ifdef debug
                  Serial.println(" receiver outside bounds, shutting down ");
                  #endif
                  enable = LOW;
                  slice = -1;
                  
                  }
                }
           

              
              }
// if the active slice is terminated, we need to end stuff...
          if(indexslice >=0)
            {
              
            String slices2 = c.substring(indexslice+6,indexslice+7);
            int slicetemp2 = slices2.toInt();
            int containsno = c.indexOf("in_use=0");
            #ifdef debug
              Serial.println("slice:");
              Serial.println(c);
              Serial.print(" slices ");
              Serial.print(slicetemp2);        
              Serial.print(" contains in_use= ");
              Serial.print(containsno);
              Serial.println();
              #endif
              if((slicetemp2 == slice)&&(containsno >=0))
              {
                #ifdef debug
                Serial.println(" quitting ");
                #endif
                enable = LOW;
                  slice = -1;
                }
              
              }
  }


  // if the server's disconnected, stop the client:
  if (!client.connected()&&connection) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    connection = false;
  }


  digitalWrite(led,enable);
  digitalWrite(ena,enable);
  digitalWrite(seq,enable);
// here goes the rest of the code that the arduino is to do... 
  
}
