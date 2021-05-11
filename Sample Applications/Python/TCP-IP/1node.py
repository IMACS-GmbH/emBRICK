# Load from emBrick ethernet Module the connect class
# Here you can change with:
# connect.ipList = ['192.168.3.10','192.168.3.12']  | Add the LWC's IP Address here
# connect.emBrickPort = 7086 || Is preconfigured on 7086 you can change it if you want connected over a another Port
# connect.updateRate = 0.0 | Preconfigured on 0.0 for the fastest possible updateCycle
from emBrick.ethernet import connect
# Load from emBrick ethernet Module the bB class
# for the 3 get & 3 put Functions:
# bB.getShort(node, module, bytePos)
# bB.getByte(node, module, bytePos)
# bB.getBit(node, module, bytePos, bitPos)
# bB.putShort(node, module, bytePos, value)
# bB.putByte(node, module, bytePos, value)
# bB.putBit(node, module, bytePos, bitPos, value)
from emBrick.ethernet import bB
# Here we type the IP Address of the LWC's
connect.ipList = ['192.168.3.10']
# The connection will be startet and the Connection with the Nodes will be configured
connect.start_ethernet()
# Start the updateCycle to get the Buffer Inforamtion of the Bricks
connect.update()

##### Example Code #####
import time

while True:
  # Put the Relay 1 on Node 1 Module 1 (5-131) on
  bB.putBit(1, 1, 0, 0, 1)
  # Wait for a second
  time.sleep(1)
   # Put the Relay 1 on Node 1 Module 1 (5-131) off
  bB.putBit(1, 1, 0, 0, 0)
  # Wait for a second
  time.sleep(1)
