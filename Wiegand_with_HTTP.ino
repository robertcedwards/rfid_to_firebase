//More crazy shit by Mike Rozycki. www.mikerozycki.com. 
//github: a betteridea.
//SET UP YOUR LIBRARIES AND SHIT

#include <SPI.h>
#include <Ethernet.h>

  /////////////////
 // MODIFY HERE //
/////////////////
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x7E, 0x00 };   // Be sure this address is unique in your network. 
//it's on the back of the shield. the 0x is constant on each bit.

//Numeric Pin where you connect your switch
uint8_t pinDevid1 = 3; // Example : the mailbox switch is connect to the Pin 3

// Debug mode
boolean DEBUG = true;
  //////////////
 //   End    //
//////////////

// GET YOUR GLOBAL VARIABLES ON

char serverName[] = "https://mikerozycki.firebaseio.com";
IPAddress ip(192,168,0,177);



template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }

long previousMillis = 0;
int interval = 3000;

char msg[16] = {'\0'};
long imsg = 0;
int element = 0;

volatile long bit_holder = 0;
volatile int bit_count = 0;
char cardID[] = "I will be overwritten";
EthernetClient client;

void DATA0(void) {
    bit_count++;
    bit_holder = bit_holder << 1;
}

void DATA1(void) {
   bit_count++;
   bit_holder = bit_holder << 1;
   bit_holder |= 1;
}


void setup()
{
  Serial.begin(57600);
  
  clearinterrupts();
  
  attachInterrupt(0, DATA0, RISING);
  attachInterrupt(1, DATA1, RISING);
  delay(10);

  digitalWrite(13, HIGH);  // show Arduino has finished initilisation
  Serial.println("READER_0001");
  

 
}


void loop() {
  if (millis() - previousMillis > interval) {
    bit_count = 0; bit_holder = 0;
    previousMillis = millis();
  }  
  if (bit_count == 26) {
      previousMillis = millis();
      
      bit_holder = (bit_holder >> 1) & 0x7fff;
      //Serial << "C: " << bit_holder << "\n";
      Serial.println("Here comes the request");
                                                                  // HERE WE CALL THE HTTPREQUEST!!
      httpRequest((char*)bit_holder);


      bit_count = 0; bit_holder = 0;
      delay(10);
    } 
    else if (bit_count && bit_count % 4 == 0 && bit_count <= 12) {
      previousMillis = millis();
      buildcode(bit_holder);
      Serial << "K: " << buildicode(bit_holder) << "\n";
      bit_count = 0;
      bit_holder = 0;
      delay(5);
    } 
    delay(50);
    //Serial << "B: " << bit_count << "\n";
}

char decodeByte(int x) { //too simple
  if (x == 10) return '*';
  if (x == 11) return '#';
  return x+48; //int to ascii
}

void buildcode(int buf) { //probably too complex for its own good
  msg[element++] = buf;
  if (buf == 11) {
    Serial.print("K: ");
    element = 0;
    while(msg[element++] != 11) {
      Serial.print(msg[element]);
      msg[element] = '\0';
    }
    element = 0;
    Serial.println();
  }
}

long buildicode(int buf) { //builds a DEC and returns when # detected
  if (buf == 11) return imsg;
  if (buf != 10) imsg = imsg * 10 + buf;
}

void clearinterrupts () {
    // the interrupt in the Atmel processor mises out the first negitave pulse as the inputs are already high,
  // so this gives a pulse to each reader input line to get the interrupts working properly.
  // Then clear out the reader variables.
  // The readers are open collector sitting normally at a one so this is OK
  for(int i = 2; i<4; i++){
    pinMode(i, OUTPUT);
    digitalWrite(i, HIGH); // enable internal pull up causing a one
    digitalWrite(i, LOW); // disable internal pull up causing zero and thus an interrupt
    pinMode(i, INPUT);
    digitalWrite(i, HIGH); // enable internal pull up
  }
  delay(10);
}

void httpRequest(char cardID[]){
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");
 
  client.stop();
  if(DEBUG){Serial.println("connecting...");}
  
  if (client.connect(serverName, 80)) {
    if(DEBUG){Serial.println("connected");}

    if(DEBUG){Serial.println("sendind request");}
    client.print("GET /firebaseTest.php?rfidid=");//The other important bit
    client.print(cardID);//This is the variable you'll be passing
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(serverName);
    client.println("User-Agent: Arduino");
    client.println();
  } 
  else {
    if(DEBUG){Serial.println("connection failed");}
  }
}

