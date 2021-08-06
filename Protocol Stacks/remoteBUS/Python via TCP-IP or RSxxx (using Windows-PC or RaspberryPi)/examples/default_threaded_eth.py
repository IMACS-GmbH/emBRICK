# Load from emBrick ethernet Module the connect class
# Here you can change with:
# connect.ipList = ['192.168.3.10','192.168.3.12']  | Add the LWCS IP Address here
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
# Node 1
# Here we type the IP Address of the first LWCS
connect.ipList.append('192.168.3.10')
# Node 2
# Here we type the IP Address of the second LWCS
connect.ipList.append('192.168.3.12')


# Import the Python Module threading
import threading
import time

# Create your functions to call it later in threads, like
#def node1():
    #..... (bB.putByte(...) or Ai = bB.getShort(...))


if __name__ == "__main__":
  # The connection with the LWCS will be configured and the BrickBus UpdateCycle will be started
  connect.start_ethernet()
  # Here configure your function as threads
  # n1 = threading.Timer(0.1, node1)  | 0.1 is the beginning time of the thread

  # Now start the threads
  #n1.start()
