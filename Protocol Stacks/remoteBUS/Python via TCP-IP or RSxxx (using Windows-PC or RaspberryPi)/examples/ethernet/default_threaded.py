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

# Import the Python Module threading
import threading
# Variable to change the update time of the thread
updateTime = 0.1
def userinterface():
    ### Write your own Application ####
    
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
