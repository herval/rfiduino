generate_arduino_firmware(
  RFIDuino
  SKETCH rfiduino.ino
  PORT @SERIAL_DEVICE@
  SERIAL picocom @SERIAL_DEVICE@
  BOARD uno)
