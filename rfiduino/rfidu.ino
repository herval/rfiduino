#include <Ethernet.h>
#include <SPI.h>
#include <SD.h>

#define VALID_CARDS_FILE "cards.txt"
#define MASTER_CARD_FILE "master.txt"

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
    }
      
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
    } else {
      return false;
    }
  }
};

//-----

/*
 * Manage a list of authorized cards - currently stored on the SD
 */
 
class Authorizer {
  bool recording;
  String masterCard;
  String *authorizedCards;
  Logger *logger;

  public:
  Authorizer(String master, String cards[], Logger *logTo) {
    authorizedCards = cards;
    masterCard = master;
    logger = logTo;
    recording = false;
    
    pinMode(SD_CARD_PIN, OUTPUT);
    // TODO load the list into memory
  }
  
  bool isMaster(String card) {
    return card == masterCard;
  }
  
  void recordMode() {
    recording = true;
  }
  
  void readerMode() {
    recording = false;
  }
  
  bool isAuthorized(String cardNumber) {
    if(recording) {
      logger->write(VALID_CARDS_FILE, cardNumber);
      logger->info("Recorded as valid: " + cardNumber);
      return true;
    };
    
    for(int i = 0; i < sizeof(authorizedCards); i++) {
      if(authorizedCards[i] == cardNumber) {
        return true;
      }
    }
    return false;
  }
  
  private:
};

//-----

class RfidReader {
  public:
  RfidReader() {
    byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
    byte ip[] = { 192, 168, 1, 100 };
  
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
      String swipeData = "";
      
      int data = client.read(); // STX
            
      data = client.read(); // first byte
      while(data != -1 && data != 255 && data != 13) {
        char hexChar[3];

        itoa(data, hexChar, 16);
        swipeData += hexChar;

        data = client.read();
      }
      client.read(); // 0xa
      client.read(); // 0x3

      return swipeData;
    } else {
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

void setup()
{
  reader = new RfidReader();
  logger = new Logger();  
  
  String master = logger->readLines(MASTER_CARD_FILE)[0];
  String * validCards = logger->readLines(VALID_CARDS_FILE);
  authorizer = new Authorizer(master, validCards, logger);
  latch = new MagneticLatch();
  
  logger->debug("connecting...");

  if (reader->connected()) {
    logger->debug("connected");
  } else {
    logger->debug("connection failed");
  }
}

void loop()
{
  if(!reader->connected()) {
    logger->debug("Connection lost - reconnecting...");
    reader->connect();
  }
  
  String c = reader->lastSwipedCard();
  if (c != "") {  
    
    if(authorizer->isMaster(c)) {
      authorizer->recordMode();
    } else {
      authorizer->readerMode();
    }
    
    if(authorizer->isAuthorized(c)) {
      latch->ping();
      logger->info("Opened: "+ c);
    } else {
      logger->debug("Unauthorized: "+ c);
    }

  }
}
