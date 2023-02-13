#!/usr/bin/evn python3
# Import Threading Module
import threading
import time
import math
import json
from datetime import datetime
# Import pymodbus for the Modbus RTU Module
from pymodbus.client.sync import ModbusSerialClient as ModbusClient
lock = threading.Lock()

filename = "error"
ERROR = open(filename, "w+")

brick_name = {2181: "CAE_G8Di8Do", 4602: "CAE_B3U4I", 4603: "CAE_B3U4I", 2470: "CAE_G2Mi2Ao", 2471: "CAE_G2Mi2Ao",
            2472: "CAE_G2Mi2Ao", 2431: "CAE_G4Ai4Tmp", 2432: "CAE_G4Ai4Tmp", 2433: "CAE_G4Ai4Tmp", 2434: "CAE_G4Ai4Tmp",
            2435: "CAE_G4Ai4Tmp", 2436: "CAE_G4Ai4Tmp"}


def splitBytes(list16bit):
    ''' Function to split a list with 16 bit elements into 8 bit element list '''
    list8bit = []
    for value in list16bit:
        a = (value >> 8) & 0xFF
        b = value & 0xFF
        list8bit.append(a)
        list8bit.append(b)
    return list8bit


def addBytes(list8bit):
    ''' Function to add two 8 bit element from a list to one 16 bit element and put them in a list '''
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


class local_master:
    ''' Sorting the information about the local master '''
    __slots__ = ['number_bricks','state','master_id','prot_ver','soft_ver', 'manu_id', 'res1', 'res2']
    def __init__(self, number_bricks, state, master_id, prot_ver, soft_ver, manu_id, res1, res2):
        self.number_bricks = number_bricks
        self.state = state
        self.master_id = master_id
        self.prot_ver = prot_ver
        self.soft_ver = soft_ver
        self.manu_id = manu_id
        self.res1 = res1
        self.res2 = res2


class slave_module:
    ''' Sorting the Information about the Bricks '''
    __slots__ = ['state', 'data_mosi', 'data_miso', 'prot_ver', 'hard_rev', 'brick_id', 'manu_id', 'offset_mosi', 'offset_miso', "name"]
    def __init__(self, state, data_mosi, data_miso, prot_ver, hard_rev, brick_id, manu_id, offset_mosi, offset_miso, name):
        self.state = state
        self.data_mosi = data_mosi
        self.data_miso = data_miso
        self.prot_ver = prot_ver
        self.hard_rev = hard_rev
        self.brick_id = brick_id
        self.manu_id = manu_id
        self.offset_mosi = offset_mosi
        self.offset_miso = offset_miso
        self.name = name


class connection:
    def __init__(self):
        self.port = ''
        self.baudrate = 460800
        self.number = 0
        self.slave_address = []
        self.unit_id = []

        self.node = ''
        self.timeout = 0.1
        self.updateRate = 0.005
        self.master = {}
        self.slave =  {}

    def start(self):
        print("---------------------------------------------------\n")
        print("emBRICK(R), Starterkit Modbus RTU Python\n")
        print("(c) 2022 by IMACS GmbH \n")
        print("---------------------------------------------------\n")
        # Connecting to the LWC's on the given Slave Adress
        self.node = ModbusClient(method='rtu', port=self.port, timeout= self.timeout, baudrate= self.baudrate)
        ERROR.write(f'Error Log\nCommunication over Modbus RTU with Python\
         Baudrate: {self.baudrate}, \tPort: {self.port}, \tTimeout: {self.timeout}\n\n')
        if self.node.is_socket_open() == True:
            self.node.close()
        connected = self.node.connect()
        if not connected:
            ERROR.write(f'{datetime.now()}\tERROR: Connection with Port {self.port} failed!')
            ERROR.close()
            exit()
        # Initialize the Functions to start the Program
        bB_update.getLWCsInfo()
        for id in self.unit_id:
            self.master["local" + str(id)] = local_master(bB_update.local_data[id][0], bB_update.local_data[id][1], ((bB_update.local_data[id][2] << 8) + bB_update.local_data[id][3]),
                        bB_update.local_data[id][4],bB_update.local_data[id][5], bB_update.local_data[id][6], bB_update.local_data[id][7], bB_update.local_data[id][8])                 
            self.slave["master" + str(id)] = {}
            for i in range(self.master["local" + str(id)].number_bricks):
                found = brick_name.get((bB_update.local_data[id][9+(i*11)+5] << 8) + bB_update.local_data[id][9+(i*11)+6], "Not found!")
                self.slave["master" + str(id)]["Brick"+ str(i+1)] = slave_module(bB_update.local_data[id][9+(i*11)+0], bB_update.local_data[id][9+(i*11)+1], bB_update.local_data[id][9+(i*11)+2],
                        bB_update.local_data[id][9+(i*11)+3], bB_update.local_data[id][9+(i*11)+4], (bB_update.local_data[id][9+(i*11)+5] << 8) + bB_update.local_data[id][9+(i*11)+6],
                        (bB_update.local_data[id][9+(i*11)+7] << 8) + bB_update.local_data[id][9+(i*11)+8],\
                        bB_update.local_data[id][9+(i*11)+9], bB_update.local_data[id][9+(i*11)+10], found )
        bB_update.update_first()
        bB.createEmptyList()
        updatcycle = threading.Thread(target=bB_update.update)
        updatcycle.daemon = True.
        updatcycle.start()
        init()
        #time.sleep(1)
        #t1.join()

connect = connection()


class brickBus_communication:
    #
    def __init__(self):
        self.local_data = {}
        self.updated = {}
        self.updates = {}
        self.stopThread = False
    def getLWCsInfo(self):
        for id in connect.unit_id:
            i = 0
            # Reads in the Init Data in the address range 1000h ... 107 ch
            while(i < 5):
                responce = connect.node.read_input_registers(4096,104, unit = id)
                if not responce.isError():
                    data_16byte = responce.registers
                    self.local_data[id] = splitBytes(data_16byte)
                    break
                else:
                    ERROR.write(f'{datetime.now()}\tERROR: Connection to the Coupling-Master with Modbus Address {id} failed!\n')
                    i += 1
                if i == 5:
                    ERROR.write(f'{datetime.now()}\tERROR: Connection to the Coupling-Master with Modbus Address {id} failed!\n')
                    connect.unit_id.remove(id)
        if len(connect.unit_id) == 0:
            print("Connection failed.")
            ERROR.close()
            exit()

    def update_first(self, ):
        ''' Update the emBricks in the individual updaterate '''
        for id in connect.unit_id:
            i = 0
            while(i < 5):
                responce = connect.node.read_input_registers(0, 120, unit=id)
                time.sleep(0.005)
                if not responce.isError():
                    self.updates[id] = responce.registers
                else:
                    ERROR.write(f'{datetime.now()}\tERROR: Checksum Error or Timeout\n')
                    i += 1
                if self.updates[id] != 0:
                    self.updated[id] = splitBytes(self.updates[id])
                    break
            if i == 5:
                ERROR.write(f'{datetime.now()}\tERROR: Connection with LWCs on Modbus Address {id} not possible\n')
                connect.unit_id.remove(id)
        if len(connect.unit_id) == 0:
            ERROR.close()
            exit()


    def update(self):
        # Update the emBricks in the individual updaterate
        while True:
            if not connect.node.is_socket_open() == True:
                connect.node.connect()
            for id in connect.unit_id:
                if not bB.put[id] == []:
                    with lock:
                        bB.set[id] = addBytes(bB.put[id])
                arguments = {
                    'read_address': 0,
                    'read_count': bB.buffer_length[id],
                    'write_address': 0,
                    'write_registers': bB.set[id],
                }

                responce = connect.node.readwrite_registers(unit=id, **arguments)
                if not responce.isError():
                    self.updates[id] = responce.registers
                else:
                    ERROR.write(f'{datetime.now()}\tERROR: Checksum Error or Timeout Error\n')

                if self.updates[id] != 0:
                    lock.acquire()
                    self.updated[id] = splitBytes(self.updates[id])

                    lock.release()
                if self.stopThread:
                    ERROR.close()
                    raise SystemExit()


bB_update = brickBus_communication()



class functions:
    ''' Implement the 6 Main Function to Control the Bricks '''
    def __init__(self, ):
        self.gS = 0
        self.gByte = 0
        self.gBit = 0
        self.pShort = []
        self.pByte = []
        self.pBit = []
        self.put = {}
        self.set = {}
        self.buffer_length = {}
        self.printed_gBit = False
        self.printed_gB = False
        self.printed_gS = False
        self.printed_pBit = False
        self.printed_pB = False
        self.printed_pS = False

    def createEmptyList(self):
        for id in connect.unit_id:
            output = 0
            input = 0
            for j in range(connect.master["local" + str(id)].number_bricks):
                output += connect.slave["master" + str(id)]["Brick" + str(j+1)].data_mosi
                input += connect.slave["master" + str(id)]["Brick" + str(j+1)].data_miso
            input = math.ceil(input / 2)
            self.put[id] = [0] * output
            self.buffer_length[id] = input

    def getShort(self, node, module, bytePos):
        ''' Return the value in Size of 2 bytes(16bit) of the desired Byte Position '''
        if  "Brick" + str(module) in connect.slave["master" + str(node)]:
            lock.acquire()
            byte1 = (bB_update.updated[node][connect.slave["master" + str(node)]["Brick" + str(module)].offset_miso + bytePos + 1]) << 8
            byte2 = bB_update.updated[node][connect.slave["master" + str(node)]["Brick" + str(module)].offset_miso + bytePos + 2]
            lock.release()
            self.gS = byte1 + byte2
        else:
            self.gS = None
            if self.printed_gS == False:
                ERROR.write(
                    f'{datetime.now()}\tERROR: getShort(), ID: {node} Brick: {module} BytePos: {bytePos}\n')
                self.printed_gS = True
        return self.gS

    def getByte(self, node, module, bytePos):
        ''' Return the value in Size of 1 byte(8bit) of the desired Byte Position '''
        if  "Brick" + str(module) in connect.slave["master" + str(node)]:
            byte = connect.slave["master" + str(node)]["Brick" + str(module)].offset_miso + bytePos + 1
            lock.acquire()
            self.gByte = bB_update.updated[node][byte]
            lock.release()
        else:
            self.gByte = None
            if self.printed_gB == False:
                ERROR.write(f'{datetime.now()}\tERROR: getByte(), ID: {node} Brick: {module} BytePos: {bytePos}\n')
                self.printed_gB = True
        return self.gByte

    def getBit(self, node, module, bytePos, bitPos):
        ''' Return the value in Size of a Bit of the desired Byte -> Bit Position '''
        if  "Brick" + str(module) in connect.slave["master" + str(node)]:
            lock.acquire()
            byte = bB_update.updated[node][connect.slave["master" + str(node)]["Brick" + str(module)].offset_miso + bytePos + 1]
            lock.release()
            self.gBit = 1 if (byte & (1 << bitPos)) else 0
        else:
            self.gBit = None
            if self.printed_gBit == False:
                ERROR.write(
                    f'{datetime.now()}\tERROR: getBit(), ID {node} Brick: {module} BytePos: {bytePos} BitPos: {bitPos}\n')
                self.printed_gBit = True
        return self.gBit


    def putShort(self, node, module, bytePos, value):
        ''' Put the desired value in Size 2 bytes to the desired Byte Position '''
        try:
            output = connect.slave["master" + str(node)]["Brick" + str(module)].offset_mosi + bytePos
            output1 = (value >> 8) & 0xFF
            output2 = value & 0xFF
            self.pShort = [output1] + [output2]
            self.put[node][output] = self.pShort[0]
            self.put[node][(output + 1)] = self.pShort[1]
        except:
            if self.printed_pS == False:
                ERROR.write(f'{datetime.now()}\tERROR: putShort(), ID: {node} Brick: {module} BytePos: {bytePos}\n')
                self.printed_pS = True


    def putByte(self, node, module, bytePos, value):
        ''' Put the desired value in Size 1 bytes to the desired Byte Position '''
        try:
            output = connect.slave["master" + str(node)]["Brick" + str(module)].offset_mosi + bytePos
            self.pByte = value
            self.put[node][output] = self.pByte
        except:
            if self.printed_pB == False:
                ERROR.write(f'{datetime.now()}\tERROR: putByte(), ID: {node} Module: {module} BytePos: {bytePos}\n')
                self.printed_pB = True

    def putBit(self, node, module, bytePos, bitPos, value):
        ''' Put the desired value in Size 1 bit to the desired Byte -> Bit Position '''
        try:
            output = connect.slave["master" + str(node)]["Brick" + str(module)].offset_mosi + bytePos
            self.pBit = 1 << bitPos
            val0 = 2 ** (bitPos + 1) - 1
            val = ((self.put[node][output]) & val0) >> bitPos
            if value:
                if not val:
                    self.put[node][output] += self.pBit
            else:
                if val >= 1:
                    self.put[node][output] -= self.pBit
        except:
            if self.printed_pBit == False:
                ERROR.write(f'Node: {node}, Module: {module}, BytePos: {bytePos}, BitPos: {bitPos} not found!\n')
                self.printed_pBit = True


bB = functions()

def init():
    ''' Prints the Information about the Local Master and the connected Bricks '''
    info_master = {}
    #Build the struct of the json data

    #Prints the Information about the Local Master and the connected Bricks
    for id in connect.unit_id:
        info_master["Node" + str(id)] = {"ID": connect.master["local" + str(id)].master_id} 
        info_master["Node" + str(id)]["Software Version"] = connect.master["local" + str(id)].soft_ver
        info_master["Node" + str(id)] ["Number of connected Bricks"] = connect.master["local" + str(id)].number_bricks
        for i in range(connect.master["local" + str(id)].number_bricks):
            info_master["Node" + str(id)] ["Brick" + str(i+1)] = {'Name': connect.slave["master" + str(id)]["Brick" + str(i+1)].name}
            info_master["Node" + str(id)] ["Brick" + str(i+1)] ['ID'] = connect.slave["master" + str(id)]["Brick" + str(i+1)].brick_id
            info_master["Node" + str(id)] ["Brick" + str(i+1)] ['Hardware Revision'] = connect.slave["master" + str(id)]["Brick" + str(i+1)].hard_rev
            info_master["Node" + str(id)] ["Brick" + str(i+1)] ['Protocol Version'] = connect.slave["master" + str(id)]["Brick" + str(i+1)].prot_ver
    json_object = json.dumps(info_master,indent=4)
    parsed = json.loads(json_object)
    print(json.dumps(parsed, indent=4, sort_keys=False))
    with open("info.json", "w") as outfile:
        json.dump(info_master, outfile)

    

    