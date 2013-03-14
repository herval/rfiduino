generate_arduino_firmware(
  RFIDuino
  SKETCH rfidu.ino
  PORT @SERIAL_DEVICE@
  SERIAL picocom @SERIAL_DEVICE@
  BOARD uno)
