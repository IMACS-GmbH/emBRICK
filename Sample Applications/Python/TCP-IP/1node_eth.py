# Load from emBrick ethernet Module the connect class
# Here you can change with:
# connect.ipList = ['192.168.3.10','192.168.3.12']  | Add the LWC's IP Address here
# connect.emBrickPort = 7086 || Is preconfigured on 7086 you can change it if you want connected over a another Port
# connect.updateRate = 0.0 | Preconfigured on 0.0 for the fastest possible updateCycle
from emBRICK.ethernet import connect
# Load from emBrick ethernet Module the bB class
# for the 3 get & 3 put Functions:
# bB.getShort(node, module, bytePos)
# bB.getByte(node, module, bytePos)
# bB.getBit(node, module, bytePos, bitPos)
# bB.putShort(node, module, bytePos, value)
# bB.putByte(node, module, bytePos, value)
# bB.putBit(node, module, bytePos, bitPos, value)
from emBRICK.ethernet import bB
# Here we type the IP Address of the LWCS
connect.ipList = ['192.168.3.10']
# The connection with the LWCS will be configured and the BrickBus UpdateCycle will be started
connect.start_ethernet()


##### Example Code #####
import time

while True:
  # Get the current State of Digial-Input1 on Bit0
  Di1 = bB.getBit(1, 1, 4, 0)
  # If the Digital-Input get 1 (High)
  if Di1:
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

