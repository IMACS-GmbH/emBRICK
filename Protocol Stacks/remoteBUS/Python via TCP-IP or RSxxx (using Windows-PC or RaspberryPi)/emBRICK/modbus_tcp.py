#!/usr/bin/evn python3
# Import Threading Module
import threading
import time
# Import pymodbus for the Modbus TCP Module
from pymodbus.client.sync import ModbusTcpClient as ModbusClient
# For Current Date
from datetime import datetime
lock = threading.RLock()
filename = 'error.txt'

def splitBytes(list16bit):
    # Function to split a list with 16 bit elements into 8 bit element list
    list8bit = []
    for value in list16bit:
        a = (value >> 8) & 0xff
        b = value & 0xff
        list8bit.append(a)
        list8bit.append(b)
    return list8bit

def addBytes(list8bit):
    list16bit = []
    if not len(list8bit) // 2:
        list8bit.append(0)
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
        for n in range(len(connect.node)):
            # Reads in the Init Data in the address range 1000h ... 107 ch
            responce = connect.node[n].read_input_registers(4096,104, unit = connect.unit_id)
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
        for n in range(len(connect.node)):
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
    # Sorting the Information about the emBRICKs
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
        for n in range(len(connect.node)):
            for i in range(local.number_of_bricks[n]):
                self.status[n, i] = getInfo.data8[n][9+(i*11)+0]
                self.data_length_mosi[n, i] = getInfo.data8[n][9+(i*11)+1]
                self.data_length_miso[n, i] = getInfo.data8[n][9+(i*11)+2]
                self.pro_ver[n, i] = getInfo.data8[n][9+(i*11)+3]
                self.hard_rev[n, i] = getInfo.data8[n][9+(i*11)+4]
                self.id[n, i] = (getInfo.data8[n][9+(i*11)+5] << 8) + getInfo.data8[n][9+(i*11)+6]
                self.manu_id[n, i] = (getInfo.data8[n][9+(i*11)+7] << 8) + getInfo.data8[n][9+(i*11)+8]
                self.offset_mosi[n, i] = getInfo.data8[n][9+(i*11)+9]
                self.offset_miso[n, i] = getInfo.data8[n][9+(i*11)+10]
slave = slave_module()

class connection:
    def __init__(self):
        self.ip_address = []
        self.ipPort = []
        self.number = 0
        self.unit_id = 1
        self.slave_address = []
        self.updated = {}
        self.updates = {}
        self.node = []
        self.timeout = 0.1
        self.updateRate = 0.0


    def start(self):
        print("---------------------------------------------------\n")
        print("emBRICK(R), Starterkit Modbus TCP Python\n")
        print("(c) 2021 by IMACS GmbH \n")
        print("---------------------------------------------------\n")
        # Create a Error File
        error = open(filename, 'w')
        error.write(f'Error Log\n Communication over Python Modbus TCP \t Date: {datetime.now()}\n')
        error.close
        for ip, port in zip(self.ip_address,self.ipPort):
            node = ModbusClient(ip, port=port, stimeout=self.timeout)
            if node.is_socket_open() == True:
                node.close()
            connected = node.connect()
            if connected:
                self.node.append(node)
                self.number += 1
            else:
                error = open(filename, 'a+')
                error.write(f'Connection to the Coupling-Master on {ip} failed !')
                error.close

        # Initialize the Functions to start the Program
        getInfo.getData()
        local.info()
        slave.info()
        connect.update_first()
        bB.createEmptyList()
        init()
        update = threading.Timer(0.1, connect.update)
        update.start()

    def update_first(self, ):
        # Update the emBricks in the individual updaterate
        for n in range(len(self.node)):
            responce = self.node[n].read_input_registers(0, 120, unit=self.unit_id)
            time.sleep(0.005)
            try:
                self.updates[n] = responce.registers
            except:
                continue
            if self.updates[n] != 0:
                self.updated[n] = splitBytes(self.updates[n])
        #bB.createEmptyList()

    def update(self):
        # Update the emBricks in the individual updaterate
        while True:
            for n in range(len(self.node)):
                num_bricks = local.number_of_bricks[n]
                if not bB.put[n] == []:
                    bB.set[n] = addBytes(bB.put[n])
                    # print('Set:', bB.set[n])
                    arguments = {
                        'read_address': 0,
                        'read_count': bB.buffer_length[n],
                        'write_address': 0,
                        'write_registers': bB.set[n],
                    }
                    responce = self.node[n].readwrite_registers(unit=self.unit_id, **arguments)
                    try:
                        self.updates[n] = responce.registers
                    except:
                        continue
                    if self.updates[n] != 0:
                        self.updated[n] = splitBytes(self.updates[n])
                else:
                    responce = self.node.read_input_registers(0, bB.buffer_length, unit=n)
                    try:
                        self.updates[n] = responce.registers
                    except:
                        continue
                    if self.updates[n] != 0:
                        self.updated[n] = splitBytes(self.updates[n])
            time.sleep(connect.updateRate)


connect = connection()


class functions:
    # Implement the 6 Main Function to Control the emBricks
    def __init__(self, ):
        self.gS = 0
        self.gByte = 0
        self.gBit = 0
        self.pShort = []
        self.pByte = []
        self.pBit = []
        self.put = {}
        self.set = {}
        self.buffer_length = []
        self.printed_pBit = False
        self.printed_pB = False
        self.printed_pS = False

    def createEmptyList(self):
        for i in range(len(connect.node)):
            output = 0
            input = 0
            for j in range(local.number_of_bricks[i]):
                output += slave.data_length_mosi[i, j]
                input += slave.data_length_miso[i, j]

            self.put[i] = [0] * (output)
            self.buffer_length.append(input)

    def getShort(self, node, module, bytePos):
        # Return the value in Size of 2 bytes(16bit) of the desired Byte Position
        try:
            byte1 = (connect.updated[node-1][slave.offset_miso[node-1, module-1] + bytePos + 1]) << 8
            byte2 = connect.updated[node-1][slave.offset_miso[node-1, module-1] + bytePos + 2]
            self.gS = byte1 + byte2
        except:
            self.gS = None
        return self.gS

    def getByte(self, node, module, bytePos):
        # Return the value in Size of 1 byte(8bit) of the desired Byte Position
        try:
            byte = slave.offset_miso[node-1, module-1] + bytePos + 1
            self.gByte = connect.updated[node-1][byte]
        except:
            self.gByte = None
        return self.gByte

    def getBit(self, node, module, bytePos, bitPos):
        # Return the value in Size of a Bit of the desired Byte -> Bit Position
        try:
            bit = slave.offset_miso[node-1, module-1] + bytePos + 1
            byte = connect.updated[node-1][bit]
            self.gBit = 1 if (byte & (1 << bitPos)) else 0
        except:
            self.gBit = None
        return self.gBit

    def putShort(self, node, module, bytePos, value):
        #Put the desired value in Size 2 bytes to the desired Byte Position
        try:
            output = slave.offset_mosi[node-1, module-1] + bytePos
            output1 = (value >> 8) & 0xff
            output2 = value & 0xff
            self.pShort = [output2] + [output1]
            self.put[node-1][output] = self.pShort[0]
            self.put[node-1][(output + 1)] = self.pShort[1]
        except:
            if self.printed_pS == False:
                error = open(filename, 'a+')
                error.write(f'Node: {node}, Module: {module}, BytePos: {bytePos} not found!\n')
                error.close
                self.printed_pS = True

    def putByte(self, node, module, bytePos, value):
        # Put the desired value in Size 1 bytes to the desired Byte Position

        try:
            output = slave.offset_mosi[node-1, module-1] + bytePos
            self.pByte = value
            self.put[node-1][output] = self.pByte
        except:
            if self.printed_pB == False:
                error = open(filename, 'a+')
                error.write(f'Node: {node}, Module: {module}, BytePos: {bytePos} not found!\n')
                error.close
                self.printed_pB = True

    def putBit(self, node, module, bytePos, bitPos, value):
        # Put the desired value in Size 1 bit to the desired Byte -> Bit Position
        try:
            output = slave.offset_mosi[node-1, module-1] + bytePos
            self.pBit = 1 << bitPos
            val0 = 2**(bitPos+1) - 1
            val = ((self.put[node-1][output])&val0) >> bitPos
            if value:
                if not val:
                    self.put[node-1][output] += self.pBit
            else:
                if val:
                    self.put[node-1][output] -= self.pBit
        except:
            if self.printed_pBit == False:
                error = open(filename, 'a+')
                error.write(f'Node: {node}, Module: {module}, BytePos: {bytePos}, BitPos: {bitPos} not found!\n')
                error.close
                self.printed_pBit = True


bB = functions()


def init():
    # Prints the Information about the Local Master and the connected Bricks to him

    for n in range(len(connect.node)):
        print(f'\nNode {n + 1} connected over the IP: {connect.ip_address[n]} \nwith the local master: {local.id[n] // 1000}-{local.id[n] % 1000}' Software Version: {local.soft_ver[n]})
        print(f'With following modules:')
        for i in range(local.number_of_bricks[n]):
            print(f'{i + 1}: {slave.id[n, i ] // 1000}-{slave.id[n, i] % 1000} with Hardware Revision {slave.hard_rev[n, i]}')
    print("To abort the program please press STRG + C\n\n")
