#include <Ethernet.h>
#include <SPI.h>
#include <SD.h>

#define VALID_CARDS_FILE "cards.txt"

#define LED_PIN 13

//-----
/*
 Log swipes to an SD card's file
 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 */
#define SD_CARD_PIN 10

class Logger {
public:
  Logger() {
    pinMode(SD_CARD_PIN, OUTPUT);
    Serial.begin(9600);
    delay(1000);
  }

  void debug(String data) {
    write("debug.log", data);
    Serial.println("DEBUG: " + data);
  }

  String * readLines(char * file) {
    File dataFile = SD.open(file, FILE_READ);
    // Serial.println("DEBUG: opened ");
    String allLines[200];

    int line = 0;

    int nextChar = dataFile.read();
    String nextString = "";
    while(nextChar != -1) {
      if (nextChar == 13) {
        allLines[line] = nextString;
        line++;
        nextString = "";
      } 
      nextString += nextChar;  
      Serial.println("--- " + nextString);  
    }
    Serial.println("DONE"); 
    return allLines;
  }

  void info(String data) {
    Serial.println("INFO: "+ data);
    if(!write("readings.log", data)) {
      debug("can't open log file!");
    }
  }

  bool write(char *file, String data) {
    File dataFile = SD.open(file, FILE_WRITE);
    // if the file is available, write to it
    if (dataFile) {
      dataFile.println(data);
      dataFile.close();  
      return true;
    } 
    else {
      return false;
    }
  }
};

//-----

/*
 * Manage a list of authorized cards - currently stored on the SD
 */

class Authorizer {
  String *authorizedCards;
  Logger *logger;

public:
  Authorizer(String cards[], Logger *logTo) {
    authorizedCards = cards;
    logger = logTo;

    pinMode(SD_CARD_PIN, OUTPUT);
  }

  bool isAuthorized(String cardNumber) {
    for(int i = 0; i < sizeof(authorizedCards); i++) {
      if(authorizedCards[i] == cardNumber) {
        return true;
      }
    }
    return false;
  }
};

//-----

class RfidReader {
public:
  RfidReader() {
    byte mac[] = { 
      0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED     };
    byte ip[] = { 
      192, 168, 1, 100     };

    Ethernet.begin(mac, ip);
    client = EthernetClient();
    connect();
  }

  void beep() {    
    //    00 01 ff ff
    char* command;
    sprintf(command, "%x%x%x%x", 0x00, 0x01, 0xff, 0xff);    
    client.write(command);
  }

  void connect() {
    byte server[] = { 192, 168, 1, 18 };
    client.connect(server, 50000);
    delay(1000);
  }

  bool connected() {
    return client.connected();
  }

  String lastSwipedCard() {
    if (client.available()) {
      client.read(); // STX

      char dataBytes[8];
      int i;
      for(i = 0; i < 8; i++) { // 8 bytes representation of the card number
        dataBytes[i] = client.read();
      }

      unsigned long value = strtol(dataBytes, NULL, 16);

      unsigned long int code = (value >> 1) & 0x7fff;
      unsigned int facility = (value >> 17) & 0xff;

      Serial.println("c: "+ code);
      Serial.println("f: "+ facility);

      // packet closing
      client.read(); // 0xa
      client.read(); // 0x3

      return String(code); 
    } 
    else {
      return "";
    }
  }

private:
  EthernetClient client;  
};
//-----

#define LATCH_OUTPUT_PIN 13

class MagneticLatch {
public:
  MagneticLatch() {
    pinMode(LATCH_OUTPUT_PIN, OUTPUT);
    digitalWrite(LATCH_OUTPUT_PIN, LOW);
  }

  void ping() {
    digitalWrite(LATCH_OUTPUT_PIN, HIGH);
    delay(3000);
    digitalWrite(LATCH_OUTPUT_PIN, LOW);
  }
};

//-----

RfidReader *reader;
Logger *logger;
Authorizer *authorizer;
MagneticLatch *latch;

void setup() {
  reader = new RfidReader();
  
  logger = new Logger();  

  logger->debug("Initializing...");

  String * validCards = logger->readLines(VALID_CARDS_FILE);
  authorizer = new Authorizer(validCards, logger);

  latch = new MagneticLatch();

  logger->debug("connecting...");

  if (reader->connected()) {
    logger->debug("connected");
  } 
  else {
    logger->debug("connection failed");
  }
}

void loop() {
  if(!reader->connected()) {
    logger->debug("Connection lost - reconnecting...");
    reader->connect();
  }
  
  String c = reader->lastSwipedCard();
  if (c != "") {  

    logger->debug("Card: "+ c);
    if(authorizer->isAuthorized(c)) {
      latch->ping();
      logger->info("Opened: "+ c);
    } else {
      logger->debug("Unauthorized: "+ c);
    }
  }
}

