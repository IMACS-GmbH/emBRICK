# Load from emBrick serial Module the connect class
from emBRICK.modbus_tcp import connect
# Here you can change with:
# connect.ip_address = ['192.168.3.10','192.168.3.12'] | the Ip Addresses of the LWCS
# connect.ipPort = [502, 6965] | the Ip Port of the LWCS
# connect.timeout = 0.001 | to change the Timeout for the Modbus Connection
# connect.unit_id = 1 | is preconfigured to the default value 1, is the Modbus Address of the LWCS
# connect.updateRate = 0.0 | Preconfigured on 0.0 for the fastest possible updateCycle

# Load from emBrick serial Module the bB class
from emBRICK.modbus_tcp import bB
# for the 3 get & 3 put Functions:
# bB.getShort(node, module, bytePos)
# bB.getByte(node, module, bytePos)
# bB.getBit(node, module, bytePos, bitPos)
# bB.putShort(node, module, bytePos, value)
# bB.putByte(node, module, bytePos, value)
# bB.putBit(node, module, bytePos, bitPos, value)

#Config for Node
#LWCS IP Address
connect.ip_address = ["192.168.3.10"]
#Ip Port
connect.ipPort = [502]
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
