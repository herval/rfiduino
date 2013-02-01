#include <Ethernet.h>
#include <SPI.h>
#include <SD.h>

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
    Serial.println(data);
  }
  
  void info(String data) {
    if(!write("readings.log", data)) {
      debug("can't open log file!");
    }
  }
  
  private:
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
  public:
  Authorizer() {
    pinMode(SD_CARD_PIN, OUTPUT);
    // TODO load the list into memory
  }
  
  bool is_authorized(String cardNumber) {
    return true;
  }
  
  private:
  String cards[];
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
      
      int data = client.read();
      
      while(data != -1) {
        swipeData += String(data);
        data = client.read();
      }

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
  authorizer = new Authorizer();
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
    logger->info(c);
    logger->debug(c);
    
    if(authorizer->is_authorized(c)) {
      latch->ping();
      // TODO log it authorized?
    }

  }
}
