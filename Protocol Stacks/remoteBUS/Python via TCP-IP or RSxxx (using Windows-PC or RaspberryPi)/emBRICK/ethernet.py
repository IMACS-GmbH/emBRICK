# Import socket from Module Sockets
import socket
# Import threading from Module Threaded
import threading
# to lock the buffer
lock = threading.RLock()
# For Current Date
from datetime import datetime
filename = 'error.txt'
import time

# Creat the class emBrickTelegram to configure the Commands for the LWC's
class emBrickTelegram:
    def __init__(self):
        # EB_ALIVE gives a "Hello World" as Responce
        self.EB_ALIVE = bytes([6, 0, 1, 0, 0, 0])
        # To Checking if the responce is correctly
        self.RD_ALIVE = 'Hello World'
        # EB_NODEINFO gives all Node Information we needed
        self.EB_NODEINFO = bytes([6, 0, 2, 0, 0, 0])
        # Reset the LWC's
        self.EB_RESETBUS = bytes([6, 0, 3, 0, 0, 0])
        # To Close the Connection with the LWC's
        self.EB_CLOSECONNECTION = bytes([6, 0, 254, 0, 0, 0])
        # Creating empty Masteroffset list
        self.EB_EXCHANGEDATA = []
        self.putData = ''
        # Create a dictionary to save the Responce Header from Remote Master's
        self.rxHeader = {}
        # Create a dictionary to save the Responce Data from Remote Master's
        self.rxData = {}

    def alive(self, node):
        n = connect.nodes[node]
        n.setblocking(True)
        # Send the Command EB_ALIVE to check if the connection with the Remote Master is correctly
        n.send(self.EB_ALIVE)
        # Responce the Header File and save it in rxHeader Dictionary
        self.rxHeader[node] = n.recv(6)
        # Responce the Data and save it in the rxData dictionary
        self.rxData[node] = n.recv(1024)
        # Checking if the Responce is "Hello World"
        if self.rxData[node].decode("utf-8") == self.RD_ALIVE:
            n.setblocking(False)
            return 1
        else:
            n.setblocking(False)
            return 2

    def info(self, node):
        n = connect.nodes[node]
        # Set the Socket Non-Blocking off
        n.setblocking(True)
        # Send the Command to get the Information of RemoteMaster and the Modules
        n.send(self.EB_NODEINFO)
        self.rxHeader[node] = n.recv(6)
        self.rxData[node] = n.recv(1024)
        # Gives the Information to the class local to sorted the Data
        local.info(self.rxData[node])
        # Gives the Information to the class slave to sorted the Data
        slave.info(node, self.rxData[node])

    def close(self, node):
        n = connect.nodes[node]
        n.setblocking(True)
        # Send the Command to close the connection
        n.send(self.EB_CLOSECONNECTION)
        self.rxHeader[node] = n.recv(6)

    def update(self):
        # Configure the Header File for the Update Cycle
        for n in range(len(connect.ipList)):
            num_brick = local.number_of_bricks[n]
            # Get the Data Offset Master Input Slave Output from Last Brick
            offset_miso = slave.offset_miso[n, num_brick]
            # Get the Data Offset Master Output Slave Input for the Last Brick
            offset_mosi = slave.offset_mosi[n, num_brick]
            header1 = (local.number_of_bricks[n] + 6) % 256
            header2 = (local.number_of_bricks[n] + 6) // 256
            if offset_miso <= num_brick:
                offset_miso = 3
            self.EB_EXCHANGEDATA.append(bytes([header1, header2, 16, 0, 0, 0, offset_mosi, offset_miso]))

    def put(self, node, put):
        # Configure the Header File for the Functions putShort, putByte, putBit
        self.putData = self.EB_EXCHANGEDATA[node] + put

    def exChange(self, node, cmd, first):
        # Sends the commands to the local master and receive the answer
        with lock:
            n = connect.nodes[node]
            n.setblocking(True)
            # Checking if it is the first updateCycle
            if first:
                n.send(cmd + bytes(0))
            else:
                n.send(cmd)
            # Receive Header
            self.rxHeader[node] = n.recv(6)
            # Receive Brick Input Information
            self.rxData[node] = n.recv(1024)
            n.setblocking(False)


command = emBrickTelegram()


class emBrickConnection:
    def __init__(self, ):
        self.ipList = []
        self.nodes = {}
        self.emBrickPort = 7086
        self.updateRate = 0.0
        self.counter = 0
        self.Brick_Input = {}

    def start_ethernet(self):
        # Print the Program Title
        print("---------------------------------------------------\n")
        print("emBRICK(R), Starterkit Ethernet Python\n")
        print("(c) 2021 by IMACS GmbH \n")
        print("---------------------------------------------------\n")
        # Create a Error File
        error = open(filename, 'w')
        error.write(f'Error Log\nCommunication over Python Ethernet\t Date: {datetime.now()}\n')
        error.close
        for i in range(len(self.ipList)):
            try:
                # Trying to connected to the given Ip Adress
                self.nodes[i] = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.nodes[i].setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                try:
                    self.nodes[i].connect((self.ipList[i], self.emBrickPort))
                except:
                    print('Already Connected')
                check = command.alive(i)
                if check == 1:
                    command.info(i)
                elif check == 2:
                    print("Error connecting with the LWC's failed.")
            except socket.error:
                print('Connecting failed, Retrying')
                error = open(filename, 'at')
                error.write(f'Connection to the Coupling-Master on {self.ipList[i]} failed !')
                error.close
                self.close()
                time.sleep(10)
                #self.nodes[i].connect((self.ipList[i], self.emBrickPort))
                self.start_ethernet()

        # Initialize the Functions to start the Program
        init()
        bB.createEmptyList()
        command.update()
        update = threading.Timer(0.1, connect.update)
        update.start()

    def close(self):
        # Send the function to close the connection with the emBrick Node
        for i in range(len(self.ipList)):
            command.close(i)
            self.nodes[i].shutdown(SHUT_WR)
            self.nodes[i].close()

    def update_first(self):
        # Update the emBricks in the individual updaterate
        with lock:
            for i in range(len(self.ipList)):
                command.exChange(i, command.EB_EXCHANGEDATA[i], 1)

    @staticmethod
    def update():
        # Sending the "update" Command to the LWC's
        while True:
            put = {}
            for keys, i in zip(bB.put, bB.put.values()):
                put[keys] = bytes(i)
                command.put(keys - 1, put[keys])
                command.exChange(keys - 1, command.putData, 0)
            connect.brick_input()
            time.sleep(connect.updateRate)


    def brick_input(self, ):
        # Create a input list for every connected brick
        with lock:
            for i in range(len(self.ipList)):
                num_brick = local.number_of_bricks[i]
                offset_miso = slave.offset_miso
                data_miso = slave.data_length_miso
                for n in range(num_brick):
                    begin = 2 + offset_miso[i, (n + 1)]
                    end = begin + data_miso[i, (n + 1)]
                    self.Brick_Input[i, (n + 1)] = command.rxData[i][begin:end]


connect = emBrickConnection()


class local_master:
    # Sorting the information about the remote master
    def __init__(self):
        # Creating variable as empty list
        self.number_of_bricks = []
        self.status = []
        self.id = []
        self.pro_vers = []
        self.soft_ver = []
        self.manu_id = []
        self.reserved1 = []
        self.reserved2 = []

    def info(self, data):
        # adding the information from remote master in the empty list
        self.number_of_bricks.append(data[0])
        self.status.append(data[1])
        self.id.append((data[2] << 8) + data[3])
        self.pro_vers.append(data[4])
        self.soft_ver.append(data[5])
        self.manu_id.append(data[6])
        self.reserved1.append(data[7])
        self.reserved2.append(data[8])


local = local_master()


class slave_module:
    # Sorting the Information about the emBricks
    def __init__(self):
        # Creating variable as empty dictionary
        self.status = {}
        self.data_length_mosi = {}
        self.data_length_miso = {}
        self.pro_ver = {}
        self.hard_rev = {}
        self.id = {}
        self.manu_id = {}
        self.offset_mosi = {}
        self.offset_miso = {}

    def info(self, n, data):
        for i in range(local.number_of_bricks[n]):
            # Adding the embrick module information in the dictionary
            self.status[n, i + 1] = data[9 + (i * 11) + 0]
            self.data_length_mosi[n, i + 1] = data[9 + (i * 11) + 1]
            self.data_length_miso[n, i + 1] = data[9 + (i * 11) + 2]
            self.pro_ver[n, i + 1] = data[9 + (i * 11) + 3]
            self.hard_rev[n, i + 1] = data[9 + (i * 11) + 4]
            self.id[n, i + 1] = (data[9 + (i * 11) + 5] << 8) + data[9 + (i * 11) + 6]
            self.manu_id[n, i + 1] = (data[9 + (i * 11) + 7] << 8) + data[9 + (i * 11) + 8]
            self.offset_mosi[n, i + 1] = data[9 + (i * 11) + 9]
            self.offset_miso[n, i + 1] = data[9 + (i * 11) + 10]


slave = slave_module()


class functions:
    # Implement the 6 Main Function to Control the emBricks
    def __init__(self, ):
        # Creating the variables
        self.gS = 0
        self.gByte = 0
        self.gBit = 0
        self.pShort = []
        self.pByte = []
        self.pBit = {}
        self.put = {}
        self.test = []
        self.printed_pBit = False
        self.printed_pB = False
        self.printed_pS = False

    def createEmptyList(self):
        # Creating empty output list
        for i in connect.nodes:
            output = 0
            for j in range(local.number_of_bricks[i]):
                output += slave.data_length_mosi[i, j + 1]
                self.put[i + 1] = [0] * output

    def getShort(self, node, module, bytePos):
        # Return the value in Size of 2 bytes(16bit) of the desired Byte Position
        with lock:
            try:
                input = connect.Brick_Input[(node - 1), module]
                try:
                    byte1 = input[(bytePos + 1)]
                    byte2 = input[(bytePos + 2)]
                    byte = bytes([byte1, byte2])
                    self.gS = int.from_bytes(byte, "big")
                except:
                    self.gS = None
            except:
                self.gS = None
            return self.gS

    def getByte(self, node, module, bytePos):
        # Return the value in Size of 1 byte(8bit) of the desired Byte Position
        with lock:
            try:
                input = connect.Brick_Input[(node - 1), module]
                self.gByte = input[(bytePos + 1)]
            except:
                self.gByte = None
            return self.gByte

    def getBit(self, node, module, bytePos, bitPos):
        # Return the value in Size of a Bit of the desired Byte -> Bit Position
        with lock:
            try:
                input = connect.Brick_Input[(node - 1), module]
                try:
                    byte = input[(bytePos + 1)]
                    self.gBit = 1 if (byte & (1 << bitPos)) else 0
                except:
                    self.gBit = None
            except:
                self.gBit = None
            return self.gBit

    def putShort(self, node, module, bytePos, value):
        # Put the desired value in Size 2 bytes to the desired Byte Position
        with lock:
            try:
                output = slave.offset_mosi[node - 1, module] + bytePos
                val1 = value // (1 << 8)
                val2 = value % (1 << 8)
                self.pShort = [val2] + [val1]
                self.put[node][output] = self.pShort[0]
                self.put[node][(output+1)] = self.pShort[1]
            except:
                if self.printed_pS == False:
                    error = open(filename, 'at')
                    error.write(f'Node: {node}, Module: {module}, BytePos: {bytePos} not found!\n')
                    error.close
                    self.printed_pS = True

    def putByte(self, node, module, bytePos, value):
        # Put the desired value in Size 1 bytes to the desired Byte Position
        with lock:
            try:
                output = slave.offset_mosi[node - 1, module] + bytePos
                self.pByte = value
                self.put[node][output] = self.pByte
            except:
                if self.printed_pB == False:
                    error = open(filename, 'at')
                    error.write(f'Node: {node}, Module: {module}, BytePos: {bytePos} not found!\n')
                    error.close
                    self.printed_pB = True

    def putBit(self, node, module, bytePos, bitPos, value):
        # Put the desired value in Size 1 bit to the desired Byte -> Bit Position
        with lock:
            try:
                output = slave.offset_mosi[node -1, module] + bytePos
                self.pBit = 1 << bitPos
                val0 = 2**(bitPos+1) - 1
                val = ((self.put[node][output])&val0) >> bitPos
                if value:
                    if not val:
                        self.put[node][output] += self.pBit
                else:
                    if val:
                        self.put[node][output] -= self.pBit
            except:
                if self.printed_pBit == False:
                    error = open(filename, 'at')
                    error.write(f'Node: {node}, Module: {module}, BytePos: {bytePos}, BitPos: {bitPos} not found!\n')
                    error.close
                    self.printed_pBit = True


bB = functions()


def init():
    # Prints the Information about the Local Master and the connected Bricks to him
    for n in connect.nodes:
        print(f'\nNode {n+1} connected over the IP: {connect.ipList[n]} \n'
        f'with the local master: {local.id[n - 1] // 1000}-{local.id[n - 1] % 1000} Software Version {local.soft_ver[n - 1]}')
        print(f'With following modules:')
        for i in range(local.number_of_bricks[n]):
            print(f'{i + 1}: {slave.id[n, i + 1] // 1000}-{slave.id[n, i + 1] % 1000} with Hardware Revision {slave.hard_rev[n, i + 1]}')
    print("To abort the program please press STRG + C\n\n")
