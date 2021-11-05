# Load from emBrick serial Module the connect class
from emBRICK.modbus_rtu import connect
# Here you can change with:
# connect.port = "COM*" or "/dev/ttySC0" |your port
# connect.unit_id = [1,2] |your connected nodes
# connect.timeout = 0.1 | to change the Timeout for the Modbus Connection
# connect.baudrate = 460800 | to change the Baudrate
# connect.updateRate = 0.005 | for +5 ms that will be slow down the updateCycle between LWCS and RemoteMaster
# Load from emBrick serial Module the bB class
from emBRICK.modbus_rtu import bB
# for the 3 get & 3 put Functions:
# bB.getShort(node, module, bytePos)
# bB.getByte(node, module, bytePos)
# bB.getBit(node, module, bytePos, bitPos)
# bB.putShort(node, module, bytePos, value)
# bB.putByte(node, module, bytePos, value)
# bB.putBit(node, module, bytePos, bitPos, value)

# With os we can detect, which Operation system we use
import os

# nt is the Operation System "Windows"
if os.name == "nt":
    # Windows: check please your COM port in "Geräte-Manger" and change the Port if you have a another Port
    connect.port = "COM5"
# posix is the Operation System "LINUX"
elif os.name == "posix":
    # RaspberryPi: Here is the Port always the same if you use our PiBrick Driver
    connect.port = "/dev/ttySC0"
# The baudrate is preconfigured to 460800 if you want u can change it
# connect.baudrate = 460800
# Node 1
# Configure here with which Modbus Address/es the Programm should connect
connect.unit_id.append(1)
# Node 2
# connect.unit_id.append(2)


##### Example Code #####
# Import the Python Module threading
import threading
import time
from tabulate import tabulate
import numpy as np

# Variable to change the starttime of the threads
startTime_n1 = 0.1
startTime_n2 = 0.2

class create_variables:
    def __init__(self):
        # Create the variable energy_n1_l1 to save the reading Energy value from Node 1 L1 in it
        self.energy_n1_l1 = 0
        # Create the variable energy_n1_l2 to save the reading Energy value from Node 1 L2 in it
        self.energy_n1_l2 = 0
        # Create the variable energy_n1_l3 to save the reading Energy value from Node 1 L3 in it
        self.energy_n1_l3 = 0
        # Create the variable energy_n2_l1 to save the reading Energy value from Node 2 L1 in it
        self.energy_n2_l1 = 0
        # Create the variable energy_n2_l2 to save the reading Energy value from Node 2 L2 in it
        self.energy_n2_l2 = 0
        # Create the variable energy_n2_l3 to save the reading Energy value from Node 2 L3 in it
        self.energy_n2_l3 = 0
        # Create the variable counter_check to control the result counter on Node1
        self.counter_check = 0
        # Create the variable counter_check to control the result counter on Node2
        self.counter_check2 = 0
        # Create the variable table for printing the Information out
        self.table_n1 = []
        self.table_n2 = []
        # Variable to check if new sensor data is available
        self.counter_check_last_loop = 0
        self.counter_check2_last_loop = 0

var = create_variables()


# Creating a Function for node 1 to start it later as a thread
def node1():
    while True:
        ##### Node 1  #####
        # Get the Result Counter from Brick 4-602 to check how much measurements the brick has make
        # The Result Counter goes from 0 to 255. After 255 the counter begins from 0 again.
        node1_res_counter = bB.getByte(1, 1, 2)
        # Get the Information back in which Command (page) we are currently.
        node1_page = bB.getByte(1, 1, 0)
        # Every time when the result counter goes up  1 we get a new mean result of the power.
        # The Average Depth is preconfigured on 10.
        # If you want a another average depth. You muss change it on Page 4 on Byte 11. You can change it between 1 and 50.

        # Check if a new mean result is coming
        if node1_res_counter != var.counter_check:
            # Read out Voltage U_A eff - U_C eff
            node1_UA_eff = bB.getShort(1, 1, 22)
            node1_UB_eff = bB.getShort(1, 1, 24)
            node1_UC_eff = bB.getShort(1, 1, 26)
            # Calculate Digits to V
            node1_UA_in_V = (node1_UA_eff * 13.603 / 1000)
            node1_UB_in_V = (node1_UB_eff * 13.603 / 1000)
            node1_UC_in_V = (node1_UC_eff * 13.603 / 1000)

            # Read out the Phase I_A - I_C
            node1_phase_IA = bB.getShort(1, 1, 10)
            node1_phase_IB = bB.getShort(1, 1, 12)
            node1_phase_IC = bB.getShort(1, 1, 14)
            # Calculate Digits in Grad
            node1_phase_IA_ = 0.017578125 * node1_phase_IA
            node1_phase_IB_ = 0.017578125 * node1_phase_IB
            node1_phase_IC_ = 0.017578125 * node1_phase_IC
            # Read out the current Power on L1-L3
            node1_power_L1 = bB.getShort(1, 1, 4)
            node1_power_L1 = np.int16(node1_power_L1)
            node1_power_L2 = bB.getShort(1, 1, 6)
            node1_power_L2 = np.int16(node1_power_L2)
            node1_power_L3 = bB.getShort(1, 1, 8)
            node1_power_L3 = np.int16(node1_power_L3)
            # Calculate Digits to W
            node1_power_L1_W = (node1_power_L1 * 11.1449)
            node1_power_L2_W = (node1_power_L2 * 11.1449)
            node1_power_L3_W = (node1_power_L3 * 11.1449)
            # Read out the current I_A eff - I_C eff
            node1_IA_eff = bB.getShort(1, 1, 16)
            node1_IB_eff = bB.getShort(1, 1, 18)
            node1_IC_eff = bB.getShort(1, 1, 20)
            # Calculate Digits to mA
            node1_IA_A = (node1_IA_eff * 6.24503 / 1000)
            node1_IB_A = (node1_IB_eff * 6.24503 / 1000)
            node1_IC_A = (node1_IC_eff * 6.24503 / 1000)

            # update the var.counter_check
            var.counter_check = node1_res_counter

            if var.counter_check_last_loop != var.counter_check:
                var.counter_check_last_loop = var.counter_check
                # Add the Power Value to Variable energy_n1_l1-l3
                var.energy_n1_l1 += node1_power_L1_W * 0.1
                var.energy_n1_l2 += node1_power_L2_W * 0.1
                var.energy_n1_l3 += node1_power_L3_W * 0.1

            if var.counter_check % 10:
                var.table_n1.clear()
                # Print out the current state of Voltage, Power and the energy in Watt pro seconds
                headers = ["Spannung in V", "Leistung in W", "Energie Ws", "Strom in A", "Phase in ° ", "Counter"]
                var.table_n1.append(["Node1 L1", "{:.2f}".format(node1_UA_in_V), "{:.2f}".format(node1_power_L1_W),
                                     "{:.2f}".format(var.energy_n1_l1),
                                     "{:.2f}".format(node1_IA_A), "{:.2f}".format(node1_phase_IA_), node1_res_counter])
                var.table_n1.append(["Node1 L2", "{:.2f}".format(node1_UB_in_V), "{:.2f}".format(node1_power_L2_W),
                                     "{:.2f}".format(var.energy_n1_l2),
                                     "{:.2f}".format(node1_IB_A), "{:.2f}".format(node1_phase_IB_), node1_res_counter])
                var.table_n1.append(["Node1 L3", "{:.2f}".format(node1_UC_in_V), "{:.2f}".format(node1_power_L3_W),
                                     "{:.2f}".format(var.energy_n1_l3),
                                     "{:.2f}".format(node1_IC_A), "{:.2f}".format(node1_phase_IC_), node1_res_counter])
                print(tabulate(var.table_n1, headers, tablefmt="grid"))


# Creating a Function for node 2 to start it later as a thread
def node2():
    while True:
        #### Node 2 #######
        # Get the Result Counter from Brick 4-602 to check how much measurement the brick make
        # The Result Counter goes from 0 to 255. After 255 the counter begins from 0 again.
        node2_res_counter = bB.getByte(2, 1, 2)
        # Get the Information back in which Command (page) we are currently.
        node2_page = bB.getByte(2, 1, 0)
        # Every time when the result counter goes up  1 we get a new mean result of the power.
        # The Average Depth is preconfigured on 10.
        # If you want a another average depth. You muss change it on Page 4 on Byte 11. You can change it between 1 and 50.

        # Check if a new mean result is coming
        if node2_res_counter != var.counter_check2:
            # Read out Voltage U_A eff - U_C eff
            node2_UA_eff = bB.getShort(2, 1, 22)
            node2_UB_eff = bB.getShort(2, 1, 24)
            node2_UC_eff = bB.getShort(2, 1, 26)
            # Calculate Digits in V
            node2_UA_in_V = (node2_UA_eff * 13.603 / 1000)
            node2_UB_in_V = (node2_UB_eff * 13.603 / 1000)
            node2_UC_in_V = (node2_UC_eff * 13.603 / 1000)

            # Read out the Phase I_A - I_C
            node2_phase_IA = bB.getShort(2, 1, 10)
            node2_phase_IB = bB.getShort(2, 1, 12)
            node2_phase_IC = bB.getShort(2, 1, 14)
            # Calculate Digits in °
            node2_phase_IA_ = 0.017578125 * node2_phase_IA
            node2_phase_IB_ = 0.017578125 * node2_phase_IB
            node2_phase_IC_ = 0.017578125 * node2_phase_IC
            # Read out the current Power on L1-L3
            node2_power_L1 = bB.getShort(2, 1, 4)
            node2_power_L1 = np.int16(node2_power_L1)
            node2_power_L2 = bB.getShort(2, 1, 6)
            node2_power_L2 = np.int16(node2_power_L2)
            node2_power_L3 = bB.getShort(2, 1, 8)
            node2_power_L3 = np.int16(node2_power_L3)
            # Calculate Digits to W
            node2_power_L1_W = (node2_power_L1 * 11.1449)
            node2_power_L2_W = (node2_power_L2 * 11.1449)
            node2_power_L3_W = (node2_power_L3 * 11.1449)
            # Read out the current I_A eff - I_C eff
            node2_IA_eff = bB.getShort(2, 1, 16)
            node2_IB_eff = bB.getShort(2, 1, 18)
            node2_IC_eff = bB.getShort(2, 1, 20)
            # Calculate Digits in mA
            node2_IA_A = (node2_IA_eff * 6.24503 / 1000)
            node2_IB_A = (node2_IB_eff * 6.24503 / 1000)
            node2_IC_A = (node2_IC_eff * 6.24503 / 1000)

            # update the var.counter_check2
            var.counter_check2 = node2_res_counter

            if var.counter_check2_last_loop != var.counter_check2:
                var.counter_check2_last_loop = var.counter_check2
                # Add the Power Value to Variable energy_n2_l1-l3
                var.energy_n2_l1 += node2_power_L1_W * 0.1
                var.energy_n2_l2 += node2_power_L2_W * 0.1
                var.energy_n2_l3 += node2_power_L3_W * 0.1

            if var.counter_check2 == 255:
                var.table_n2.clear()
                # Print out the current state of Voltage, Power and the energy in Watt pro seconds
                headers = ["Spannung in V", "Leistung in W", "Energie Ws", "Strom in A", "Phase in ° ", "Counter"]
                var.table_n2.append(["Node2 L1", "{:.2f}".format(node2_UA_in_V), "{:.2f}".format(node2_power_L1_W),
                                     "{:.2f}".format(var.energy_n2_l1),
                                     "{:.2f}".format(node2_IA_A), "{:.2f}".format(node2_phase_IA_), node2_res_counter])
                var.table_n2.append(["Node2 L2", "{:.2f}".format(node2_UB_in_V), "{:.2f}".format(node2_power_L2_W),
                                     "{:.2f}".format(var.energy_n2_l2),
                                     "{:.2f}".format(node2_IB_A), "{:.2f}".format(node2_phase_IB_), node2_res_counter])
                var.table_n2.append(["Node2 L3", "{:.2f}".format(node2_UC_in_V), "{:.2f}".format(node2_power_L3_W),
                                     "{:.2f}".format(var.energy_n2_l3),
                                     "{:.2f}".format(node2_IC_A), "{:.2f}".format(node2_phase_IC_), node2_res_counter])

                print(tabulate(var.table_n2, headers, tablefmt="grid"))


if __name__ == "__main__":
    # The connection with the LWCS will be configured and the BrickBus UpdateCycle will be started
    connect.start()
    # Config the threads node1 & node2
    n1 = threading.Timer(startTime_n1, node1)
    n2 = threading.Timer(startTime_n2, node2)
    # Start the threads
    n1.start()
    n2.start()
