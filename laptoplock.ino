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
    // TODO open log file?
  }
  
  bool info(String data) {
    File dataFile = SD.open("log.txt", FILE_WRITE);
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

class RfidReader {
  public:
  RfidReader() {
    byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
    byte ip[] = { 192, 168, 1, 100 };
    byte server[] = { 192, 168, 1, 18 };
  
    Ethernet.begin(mac, ip);
    client = EthernetClient();
    client.connect(server, 50000);
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
//-----

RfidReader *reader;
Logger *logger;

void setup()
{
  reader = new RfidReader();
  logger = new Logger();
  delay(1000);

  Serial.begin(9600);
  delay(1000);
  Serial.println("connecting...");

  if (reader->connected()) {
    Serial.println("connected");
  } else {
    // TODO?
    Serial.println("connection failed");
  }
}

void loop()
{
  String c = reader.lastSwipedCard();
  Serial.print(c);
  if (c != "") {  
    logger.info(c);
    Serial.println(c);
    // activate the lock
    // log to log file
  }
  Serial.print("HELLO");
}
