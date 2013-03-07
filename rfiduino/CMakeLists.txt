generate_arduino_firmware(
  RFIDuino
  SKETCH rfidu.ino
  PORT  /dev/ttyACM0
  SERIAL picocom @SERIAL_PORT@
  BOARD uno)
