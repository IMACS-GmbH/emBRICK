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
connect.ipList = ['192.168.3.10','192.168.3.12']


##### Example Code #####
# Import the Python Module threading
import threading
# Variable to change the update time of the thread
updateTime = 0.1
def userinterface():
    # Put Relay 1 on Node 1 Module 1 (1-513) on
    bB.putBit(1, 1, 0, 0, 1)
    # Put Relay 1 on Node 2 Module 1 (1-513) on
    bB.putBit(2, 1, 0, 0, 1)
    # Everytime if the counter rised 10 the Relay1 will be set off
    if not connect.counter % 10:
      bB.putBit(1, 1, 0, 0, 0)
      bB.putBit(2, 1, 0, 0, 0)
    #####
    # Read the Potentiometer on Node 1Analog Output 1
    N1_AI_voltage = bB.getShort(1, 1, 0)
    # Read the Digital Input 1-4 on the Node 1 Module 5-131
    N1_Di1 = bB.getBit(1, 1, 4, 0)
    N1_Di2 = bB.getBit(1, 1, 4, 1)
    N1_Di3 = bB.getBit(1, 1, 4, 2)
    N1_Di4 = bB.getBit(1, 1, 4, 3)
    #####
    # Read the Potentiometer on Node 2 Analog Output 1
    N2_AI_voltage = bB.getShort(2, 1, 0)
    # Read the Digital Input 1-4 on the Node 2 Module 5-131
    N2_Di1 = bB.getBit(2, 1, 4, 0)
    N2_Di2 = bB.getBit(2, 1, 4, 1)
    N2_Di3 = bB.getBit(2, 1, 4, 2)
    N2_Di4 = bB.getBit(2, 1, 4, 3)
    
    print(f'Node 1 Ai1: {N1_AI_voltage}, Di: {N1_Di1}{N1_Di2}{N1_Di3}{N1_Di4}\tNode 2 Ai1: {N2_AI_voltage}, Di: {N2_Di1}{N2_Di2}{N2_Di3}{N2_Di4}', end="\r")
    # In every UpdateCycle the counter raise by 1
    connect.counter +=1
    # Start the Thread with given UpdateRate
    threading.Timer(updateTime, userinterface).start()
    
if __name__ == "__main__":
  # The connection will be startet and the Connection with the Nodes will be configured
  connect.start_ethernet()
  # Start the updateCycle to get the Buffer Inforamtion of the Bricks
  connect.update()
  # Start the thread 
  userinterface()
