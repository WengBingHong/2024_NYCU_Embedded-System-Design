# Run this on pc to get control of E9V3
sudo screen
screen /dev/ttyUSB0 115200

# There might be no sudo command, plz login as root
root
embedsky

# Run executable on board
LIBRARY_PATH=. ./demo
