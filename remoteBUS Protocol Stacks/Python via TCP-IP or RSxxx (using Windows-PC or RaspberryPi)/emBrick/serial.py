#!/usr/bin/evn python3
# Import pymodbus a Modbus RTU Serial Module
from pymodbus.client.sync import ModbusSerialClient as ModbusClient
# Import Threading Module
import threading

import time
lock = threading.RLock()

def splitBytes(list16bit):
    # Function to split a list with 16 bit elements into 8 bit element list
    list8bit = []
    with lock:
        for value in list16bit:
            a = value // 256
            b = value % 256
            list8bit.append(a)
            list8bit.append(b)
        return list8bit

def addBytes(list8bit):
    list16bit = []
    if not len(list8bit) // 2:
        list8bit.append(0)
    with lock:
        for num in range(len(list8bit)):
            if not num % 2:
                byte0 = list8bit[num] << 8
            else:
                byte1 = list8bit[num]
                byte = byte0 + byte1
                list16bit.append(byte)
        return list16bit

class data_from_local_master:
    # Reading the init Data from Local Master and split the 16 bit elements into 8 bit
    def __init__(self):
        #self.datain16byte = []
        self.data16 = {}
        self.data8 = {}
    def getData(self):
        for n in connect.slave_adress:
            # Reads in the Init Data in the address range 1000h ... 107 ch
            responce = connect.node.read_input_registers(4096,104, unit = n)
            self.data16[n] = responce.registers
            self.data8[n] = splitBytes(self.data16[n])
getInfo = data_from_local_master()

class local_master:
    # Sorting the information about the local master
    def __init__(self):
        self.number_of_bricks = []
        self.status = []
        self.id = []
        self.pro_vers = []
        self.soft_ver = []
        self.manu_id = []
        self.reserved1 = []
        self.reserved2 = []
    def info(self):
        for n in connect.slave_adress:
            self.number_of_bricks.append(getInfo.data8[n][0])
            self.status.append(getInfo.data8[n][1])
            self.id.append((getInfo.data8[n][2] << 8) + getInfo.data8[n][3] )
            self.pro_vers.append(getInfo.data8[n][4])
            self.soft_ver.append(getInfo.data8[n][5])
            self.manu_id.append(getInfo.data8[n][6])
            self.reserved1.append(getInfo.data8[n][7])
            self.reserved2.append(getInfo.data8[n][8])
local = local_master()

class slave_module:
    # Sorting the Information about the emBricks
    def __init__(self):
        self.status = {}
        self.data_length_mosi = {}
        self.data_length_miso = {}
        self.pro_ver = {}
        self.hard_rev = {}
        self.id = {}
        self.manu_id = {}
        self.offset_mosi = {}
        self.offset_miso = {}
    def info(self):
        for n in connect.slave_adress:
            for i in range(local.number_of_bricks[n-1]):
                self.status[n, i+1] = getInfo.data8[n][9+(i*11)+0]
                self.data_length_mosi[n, i+1] = getInfo.data8[n][9+(i*11)+1]
                self.data_length_miso[n, i+1] = getInfo.data8[n][9+(i*11)+2]
                self.pro_ver[n, i+1] = getInfo.data8[n][9+(i*11)+3]
                self.hard_rev[n, i+1] = getInfo.data8[n][9+(i*11)+4]
                self.id[n, i+1] = (getInfo.data8[n][9+(i*11)+5] << 8) + getInfo.data8[n][9+(i*11)+6]
                self.manu_id[n, i+1] = (getInfo.data8[n][9+(i*11)+7] << 8) + getInfo.data8[n][9+(i*11)+8]
                self.offset_mosi[n, i+1] = getInfo.data8[n][9+(i*11)+9]
                self.offset_miso[n, i+1] = getInfo.data8[n][9+(i*11)+10]
slave = slave_module()

class connection:
    def __init__(self):
        self.port = ''
        self.number = 0
        self.slave_adress = []
        self.updated = {}
        self.updates = {}
        self.node = ''
        self.updateRate = 1
        self.counter = 0
        self.timeout = 1
        self.baudrate = 460800
        self.timeout_between_msg = 0.005
        self.thread = False

    def start_serial(self):
        print("---------------------------------------------------\n")
        print("emBRICK(R), Starterkit Serial Python\n")
        print("(c) 2021 by IMACS GmbH \n")
        print("---------------------------------------------------\n")
        # Connecting to the LWC's on the given Slave Adress
        node = ModbusClient(method='rtu', port=self.port, timeout= self.timeout, baudrate= self.baudrate)
        if node.is_socket_open() == True:
            node.close()
        node.connect()
        self.node = node

        for i in range(self.number):
            test = self.node.read_input_registers(4096, 104, unit=i+1)
            try:
                test = test.registers
                self.slave_adress.append(i + 1)
            except:
                print(f'Connection failed to the Remote Master on {i+1} failed !')
        # Initialize the Functions to start the Program
        getInfo.getData()
        local.info()
        slave.info()
        connect.update_first()
        bB.createEmptyList()
        init()

    def update_first(self,):
        # Update the emBricks in the individual updaterate
        with lock:
            for n in self.slave_adress:
                responce = self.node.read_input_registers(0,120, unit= n)
                time.sleep(0.005)
                try:
                    self.updates[n] = responce.registers
                except:
                    continue
                if self.updates[n] != 0:
                    self.updated[n]= splitBytes(self.updates[n])
            bB.createEmptyList()

    def update(self):
        # Update the emBricks in the individual updaterate
        with lock:
            #bB.set.clear()
            for n in self.slave_adress:
                num_bricks = local.number_of_bricks[n - 1]
                if not bB.put[n] == []:
                    bB.set[n] = addBytes(bB.put[n])
                    #print('Set:', bB.set[n])
                    arguments = {
                        'read_address': 0,
                        'read_count': bB.buffer_length[n],
                        'write_address': 0,
                        'write_registers': bB.set[n],
                    }
                    responce = self.node.readwrite_registers(unit=n, **arguments)
                    try:
                        self.updates[n] = responce.registers
                    except:
                        continue
                    if self.updates[n] != 0:
                        self.updated[n]= splitBytes(self.updates[n])
                        time.sleep(connect.timeout_between_msg)
                else:
                    responce = self.node.read_input_registers(0, bB.buffer_length, unit=n)
                    try:
                        self.updates[n] = responce.registers
                    except:
                        continue
                    if self.updates[n] != 0:
                        self.updated[n] = splitBytes(self.updates[n])
                        time.sleep(connect.timeout_between_msg)
            if self.updateRate > 0.5:
                self.thread = True
            if self.thread:
                threading.Timer(0.0, connect.update).start()


connect = connection()

class functions:
    # Implement the 6 Main Function to Control the emBricks
    def __init__(self,):
        self.gS = 0
        self.gByte = 0
        self.gBit = 0
        self.pShort = []
        self.pByte = []
        self.pBit = []
        self.put = {}
        self.set = {}
        self.buffer_length = []

    def createEmptyList(self):
        for i in connect.slave_adress:
            output = 0
            input = 0
            for j in range(local.number_of_bricks[i-1]):
                output += slave.data_length_mosi[i, j+1]
                input += slave.data_length_miso[i, j+1]
                
            self.put[i] = [0] * (output)
            self.buffer_length.append(input)

    def getShort(self, node, module, bytePos):
        # Return the value in Size of 2 bytes(16bit) of the desired Byte Position
        with lock:
            try:
                byte1 = (connect.updated[node][slave.offset_miso[node, module] + bytePos +1]) <<8
                byte2 = connect.updated[node][slave.offset_miso[node, module] + bytePos +2]
                self.gS = byte1 + byte2
            except:
                self.gS = "Node or Brick not found!"
            return self.gS

    def getByte(self,node,module,bytePos):
        # Return the value in Size of 1 byte(8bit) of the desired Byte Position
        with lock:
            try:
                byte = slave.offset_miso[node, module] + bytePos +1
                self.gByte = connect.updated[node][byte]
            except:
                self.gByte = "Node or Brick not found!"
            return self.gByte

    def getBit(self, node, module, bytePos, bitPos):
        # Return the value in Size of a Bit of the desired Byte -> Bit Position
        with lock:
            try:
                bit = slave.offset_miso[node, module] + bytePos +1
                try:
                    byte = connect.updated[node][bit]
                    self.gBit = 1 if (byte & (1 << bitPos)) else 0
                except:
                    self.gBit = "Byte Position not found!"
            except:
                self.gBit = "Node or Brick not found!"
            return self.gBit

    def putShort(self, node, module, bytePos, value):
        # Put the desired value in Size 2 bytes to the desired Byte Position
        with lock:
            try:
                output = slave.offset_mosi[node, module] + bytePos
                output1 = value // 256
                output2 = value % 256
                self.pShort = [output2] + [output1]
                self.put[node][output] = self.pShort[0]
                self.put[node][(output+1)] = self.pShort[1]
            except:
                print("Node: ", node, ", Module: ", module, ", BytePos: ", bytePos, " not found!",
                      end="\r")


    def putByte(self, node, module, bytePos, value):
        # Put the desired value in Size 1 bytes to the desired Byte Position
        with lock:
            try:
                output = slave.offset_mosi[node, module] + bytePos
                self.pByte = value
                self.put[node][output] = self.pByte
            except:
                print("Node: ", node, ", Module: ", module, ", BytePos: ", bytePos, " not found!",
                      end="\r")

    def putBit(self, node, module, bytePos, bitPos, value):
        # Put the desired value in Size 1 bit to the desired Byte -> Bit Position
        with lock:
            try:
                output = slave.offset_mosi[node, module] + bytePos
                self.pBit = 1 << bitPos
                val = self.put[node][output] >> bitPos
                if value:
                    if not val:
                        self.put[node][output] += self.pBit
                else:
                    if val >= 1:
                        self.put[node][output] -= self.pBit
            except:
                print("Node: ", node, ", Module: ", module, ", BytePos: ", bytePos, ", BitPos: ", bitPos, " not found!",
                      end="\r")
bB = functions()

def init():
    # Prints the Information about the Local Master and the connected Bricks to him
    for n in connect.slave_adress:
        print(f'\nNode {n} is connected with the local master: {local.id[n-1]//1000}-{local.id[n-1]%1000}')
        print(f'With following modules:')
        for i in range(local.number_of_bricks[n - 1]):
            print(f'{i+1}: {slave.id[n, i + 1] // 1000}-{slave.id[n, i + 1] % 1000}')
    print("To abort the program please press STRG + C\n\n")
