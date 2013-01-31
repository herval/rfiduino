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
    // TODO v2: move to a file
    Serial.println(data);
  }
  
  void info(String data) {
    File dataFile = SD.open("log.txt", FILE_WRITE);
    // if the file is available, write to it
    if (dataFile) {
      dataFile.println(data);
      dataFile.close();
    } else {
      debug("can't open log file!");
    }
  }
};

//-----

/*
 * Manage a list of authorized cards - currently stored on the SD
 */
class Authorizater {
  public:
  Authorizater() {
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

RfidReader *reader;
Logger *logger;
Authorizer *authorizer;

void setup()
{
  reader = new RfidReader();
  logger = new Logger();  
  authorizer = new Authorizater();
  
  reader->connect();

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
    // activate the lock
    // log to log file
  }
}
