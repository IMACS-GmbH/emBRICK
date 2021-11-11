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
# Node 1
# Configure here with which Modbus Address/es the Programm should connect
connect.unit_id.append(1)
# Node 2
connect.unit_id.append(2)


##### Example Code #####
# Import the Python Module threading
import threading
import time

# Variable to change the starttime of the threads
startTime_n1 = 0.1
startTime_n2 = 0.2

# Creating a Function for node 1 to start it later as a thread
def node1():
    while True:
        # Read the Potentiometer on Analog Output 1
        AI_voltage = bB.getShort(1, 1, 0)
        # Read the Digital Input 1-4 on the Module 5-131
        Di1 = bB.getBit(1, 1, 4, 0)
        Di2 = bB.getBit(1, 1, 4, 1)
        Di3 = bB.getBit(1, 1, 4, 2)
        Di4 = bB.getBit(1, 1, 4, 3)
        print(f'Ai1: {AI_voltage}, Di1: {Di1}, Di2: {Di2}, Di3: {Di3}, Di4: {Di4}', end="\r")
        # If Digital Input1 set to 1 (High)
        if Di1:
            # Put Relay1 on
            bB.putBit(1, 1, 0, 0, 1)
            # Wait 2 Seconds
            time.sleep(2)
            # Put Relay1 off
            bB.putBit(1, 1, 0, 0, 0)
        if Di2:
            # Put Relay2 on
            bB.putBit(1, 1, 0, 1, 1)
            # Wait 2 Seconds
            time.sleep(2)
            # Put Relay2 off
            bB.putBit(1, 1, 0, 1, 0)

# Creating a Function for node 2 to start it later as a thread
def node2():
    while True:
        Di1 = bB.getBit(2, 1, 0, 0)
        if Di1:
            bB.putBit(2, 1, 0, 0, 1)
            bB.putBit(2, 1, 0, 1, 1)
            bB.putBit(2, 1, 0, 2, 1)
            bB.putBit(2, 1, 0, 3, 1)
            bB.putBit(2, 1, 0, 4, 1)
            bB.putBit(2, 1, 0, 5, 1)
            bB.putBit(2, 1, 0, 6, 1)
            bB.putBit(2, 1, 0, 7, 1)
        else:
            bB.putBit(2, 1, 0, 0, 0)
            bB.putBit(2, 1, 0, 1, 0)
            bB.putBit(2, 1, 0, 2, 0)
            bB.putBit(2, 1, 0, 3, 0)
            bB.putBit(2, 1, 0, 4, 0)
            bB.putBit(2, 1, 0, 5, 0)
            bB.putBit(2, 1, 0, 6, 0)
            bB.putBit(2, 1, 0, 7, 0)

if __name__ == "__main__":
  # The connection with the LWCS will be configured and the BrickBus UpdateCycle will be started
  connect.start()
  # Config the threads node1 & node2
  n1 = threading.Timer(startTime_n1, node1)
  n2 = threading.Timer(startTime_n2, node2)
  # Start the threads
  n1.start()
  n2.start()
