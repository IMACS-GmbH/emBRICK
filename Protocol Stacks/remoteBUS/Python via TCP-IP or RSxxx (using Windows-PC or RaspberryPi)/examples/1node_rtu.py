# Load from emBrick serial Module the connect class
from emBRICK.modbus_rtu import connect
# Here you can change with:
# connect.port = "COM*" or "/dev/ttySC0" |your port
# connect.unit_id = [1,2] |your connected nodes
# connect.timeout = 0.1 | to change the Timeout for the Modbus Connection
# connect.baudrate = 460800 | to change the Baudrate
# connect.updateRate = 0.005 | for +5 ms that will be slow down the updateCycle between LWCS and RemoteMaster
# Load from emBrick serial Module the bB class
from emBRICK.modbus_rtu import bB
# for the 3 get & 3 put Functions:
# bB.getShort(node, module, bytePos)
# bB.getByte(node, module, bytePos)
# bB.getBit(node, module, bytePos, bitPos)
# bB.putShort(node, module, bytePos, value)
# bB.putByte(node, module, bytePos, value)
# bB.putBit(node, module, bytePos, bitPos, value)

# With os we can detect, which Operation system we use
import os
# nt is the Operation System "Windows"
if os.name == "nt":
# Windows: check please your COM port in "Geräte-Manger" and change the Port if you have a another Port
  connect.port = "COM5"
# posix is the Operation System "LINUX"
elif os.name == "posix":
# RaspberryPi: Here is the Port always the same if you use our PiBrick Driver
  connect.port = "/dev/ttySC0"
# The baudrate is preconfigured to 460800 if you want u can change it
#connect.baudrate = 460800
# Configure here with which Modbus Address/es the Programm should connect
connect.unit_id.append(1)
# The connection with the LWCS will be configured and the BrickBus UpdateCycle will be started
connect.start()


##### Example Code #####
import time

while True:
  # Get the current State of Digial-Input1 on Bit0
  led1 = bB.getBit(1, 1, 4, 0)
  # If the Digital-Input get 1 (High)
  if led1:
    # Put the Relay2 on the first Module 5-131 on
    bB.putBit(1, 1, 0, 1, 1)
  else:
    #  Digital Input get 0 (Low) put the Relay 2 off
    bB.putBit(1, 1, 0, 1, 0)
  # Put the Relay1 on the first Module 5-131 on
  bB.putBit(1, 1, 0, 0, 1)
  # Wait for 1 second
  time.sleep(1)
  # Put the Relay1 off
  bB.putBit(1, 1, 0, 0, 0)
  # Wait a 1 second again
  time.sleep(1)
