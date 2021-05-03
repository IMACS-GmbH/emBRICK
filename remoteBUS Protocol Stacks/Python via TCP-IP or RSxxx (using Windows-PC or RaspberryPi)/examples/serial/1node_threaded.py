# Load from emBrick serial Module the connect class
# Here you can change with:
# connect.port = "COM*" or "/dev/ttySC0" |your port
# connect.number = 2 |your connected nodes
# connect.updateRate = 0.1 |for +100 ms that will be slow down the updateCycle between LWC's and RemoteMaster
# connect.timeout = 0.1 | to change the Timeout for the Modbus Connection
# connect.baudrate = 460800 | to change the Baudrate
from emBrick.serial import connect
# Load from emBrick serial Module the bB class
# for the 3 get & 3 put Functions:
# bB.getShort(node, module, bytePos)
# bB.getByte(node, module, bytePos)
# bB.getBit(node, module, bytePos, bitPos)
# bB.putShort(node, module, bytePos, value)
# bB.putByte(node, module, bytePos, value)
# bB.putBit(node, module, bytePos, bitPos, value)
from emBrick.serial import bB
# Here we configure with which port we will be use
# Windows: check please your COM port in "Geräte-Manger" and change the Port if you have a another Port
connect.port = "COM5"
# RaspberryPi Os: Here is the Port always the same if you use our PiBrick Driver
# connect.port = "/dev/ttySC0"
# Here we configure how many nodes we will be connected
connect.number = 1


##### Example Code #####
# Import the Python Module threading
import threading

# Variable to change the update time of the thread
connect.updateRate = 0.1


def userinterface():
  # Put Relay 1 on Module 1 (1-513) on
  bB.putBit(1, 1, 0, 0, 1)
  # Everytime if the counter rised 10 the Relay1 will be set off
  if not connect.counter % 10:
    bB.putBit(1, 1, 0, 0, 0)
  # Read the Potentiometer on Analog Output 1
  AI_voltage = bB.getShort(1, 1, 0)
  # Read the Digital Input 1-4 on the Module 5-131
  Di1 = bB.getBit(1, 1, 4, 0)
  Di2 = bB.getBit(1, 1, 4, 1)
  Di3 = bB.getBit(1, 1, 4, 2)
  Di4 = bB.getBit(1, 1, 4, 3)
  print(f'Ai1: {AI_voltage}, Di: {Di1}{Di2}{Di3}{Di4}', end="\r")
  # In every UpdateCycle the counter raise by 1
  connect.counter += 1
  if not connect.thread: 
    connect.update()
  # Start the Thread with given UpdateRate
  threading.Timer(connect.updateRate, userinterface).start()


if __name__ == "__main__":
  # The connection will be startet and the Connection with the Nodes will be configured
  connect.start_serial()
  # Start the updateCycle to get the Buffer Inforamtion of the Bricks
  connect.update()
  # Start the thread
  userinterface()
