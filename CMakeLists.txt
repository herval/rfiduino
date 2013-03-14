set(SERIAL_DEVICE "/dev/tty.usbmodem411")
set(SERIAL_PORT "/dev/tty.usbmodem411")
set(CMAKE_TOOLCHAIN_FILE $ENV{ARDUINO_TOOLCHAIN})

cmake_minimum_required(VERSION 2.8)

project(RFIDuino C CXX)

print_board_list()
print_programmer_list()

# link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
add_subdirectory(rfidu)
