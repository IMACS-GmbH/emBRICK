# Importing the function threading from Module Threaded
import threading
# Importing the Driver File emBrick_ser
import emBrick.serial
# Import the class connect and all his functions
from emBrick.serial import connect
# Import the class connect and all his functions (bB.putBit, getBit,...)
from emBrick.serial import bB

from tabulate import tabulate

# The Port is always the same if you use the Raspberry PI emBrick
connect.port = '/dev/ttySC0'
# This is the number of Remote Master
connect.number = 2

# To Change the Speed of the Updatecylce 0.1 for 100 ms
# The Maximum Speed the Module can reached is 0.03 also 30 ms
updateRate = 0.01

# Here you can create variables to use them for example in userinterface()
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
var = create_variables()

def userinterface():
    """
    ### Example how to change the Command (Page) to 4 and configure the average depth
    if connect.counter == 0:
        ## Node 1
        # Get the current state of the Communic Counter
        node1_communic_counter = bB.getByte(1, 1, 0)
        # Add 1 to the current Communic Counter to confirm your changes
        node1_communic_counter += 1
        # Put Value 4 to the Byte0. To read out the Information on Page4 of Brick 4-602
        # If you want go on a another Page change the Value for the Page you want.
        bB.putByte(1, 1, 0, 4)
        # To save the configuration we send the communic-counter+1 back to the brick
        bB.putByte(1, 1, 1, node1_communic_counter)
        ## Node 2
        # Get the current state of the Result Counter
        node2_communic_counter = bB.getByte(2, 1, 0)
        # Add 1 to the current Communic Counter to confirm your changes
        node2_communic_counter += 1
        # Put Value 4 to the Byte0. To read out the Information on Page4 of Brick 4-602
        # If you want go on a another Page change the Value for the Page you want.
        bB.putByte(2, 1, 0, 4)
        # To save the configuration we send the communic-counter+1 back to the brick
        bB.putByte(2, 1, 1, node2_communic_counter)

    elif connect.counter = 1:
        ## Node 1
        # Get the current state of the Communic Counter
        node1_communic_counter = bB.getByte(1, 1, 0)
        # Add 1 to the current Communic Counter to confirm your changes
        node1_communic_counter += 1
        # Put Value 20 to the Byte11. To change the average depth from 10 to 20.
        # If you want a another average depth measure you can change it between 1-50
        bB.putByte(1, 1, 11, 20)
        # To save the configuration we send the communic-counter+1 back to the brick
        bB.putByte(1, 1, 1, node1_communic_counter)
        ## Node 2
        # Get the current state of the Result Counter
        node2_communic_counter = bB.getByte(2, 1, 0)
        # Add 1 to the current Communic Counter to confirm your changes
        node2_communic_counter += 1
        # Put Value 20 to the Byte11. To change the average depth from 10 to 20.
        # If you want a another average depth measure you can change it between 1-50
        bB.putByte(2, 1, 11, 20)
        # To save the configuration we send the communic-counter+1 back to the brick
        bB.putByte(2, 1, 1, node2_communic_counter)

    elif connect.counter = 2:
        ## Node 1
        # Get the current state of the Communic Counter
        node1_communic_counter = bB.getByte(1, 1, 0)
        # Add 1 to the current Communic Counter to confirm your changes
        node1_communic_counter += 1
        # Put Value 1 to the Byte0. To read out the Information on Page1 of Brick 4-602
        # If you want go on a another Page change the Value for the Page you want.
        bB.putByte(1, 1, 0, 1)
        # To save the configuration we send the communic-counter+1 back to the brick
        bB.putByte(1, 1, 1, node1_communic_counter)

        # Node 2
        # Get the current state of the Result Counter
        node2_communic_counter = bB.getByte(2, 1, 0)
        # Add 1 to the current Communic Counter to confirm your changes
        node2_communic_counter += 1
        # Put Value 1 to the Byte0. To read out the Information on Page1 of Brick 4-602
        # If you want go on a another Page change the Value for the Page you want.
        bB.putByte(2, 1, 0, 1)
        # To save the configuration we send the communic-counter+1 back to the brick
        bB.putByte(2, 1, 1, node2_communic_counter)
    else:
    """
    ##### Node 1  #####
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
        # Read out the current Power on L1-L3
        node1_power_L1 = bB.getShort(1, 1, 4)
        node1_power_L2 = bB.getShort(1, 1, 6)
        node1_power_L3 = bB.getShort(1, 1, 8)
        # Calculate Digits to W
        node1_power_L1_W = (node1_power_L1 * 334.346)
        node1_power_L2_W = (node1_power_L2 * 334.346)
        node1_power_L3_W = (node1_power_L3 * 334.346)
        # Read out the current I_A eff - I_C eff
        node1_IA_eff = bB.getShort(1, 1, 16)
        node1_IB_eff = bB.getShort(1, 1, 18)
        node1_IC_eff = bB.getShort(1, 1, 20)
        # Calculate Digits to mA
        node1_IA_mA = (node1_IA_eff * 187.351)
        node1_IB_mA = (node1_IB_eff * 187.351)
        node1_IC_mA = (node1_IC_eff * 187.351)
        # Add the Power Value to Variable energy_n1_l1-l3
        var.energy_n1_l1 += node1_power_L1_W
        var.energy_n1_l2 += node1_power_L2_W
        var.energy_n1_l3 += node1_power_L3_W
        # update the var.counter_check
        var.counter_check = node1_res_counter
        # Everytime when the result counter goes 15(1.5 seconds) up we print out the state
        if not var.counter_check % 15:
            var.table_n1.clear()
            # Print out the current state of Voltage, Power and the energy in Watt pro seconds
            headers = ["Spannung in V", "Leistung in W", "Energie W pro s", "Strom in mA", "Phase in ° ", "Counter"]
            var.table_n1.append(["Node1 L1", "{:.2f}".format(node1_UA_in_V), "{:.2f}".format(node1_power_L1_W),
                              "{:.2f}".format(var.energy_n1_l1 / 10),
                              "{:.2f}".format(node1_IA_mA), "{:.2f}".format(node1_phase_IA_),node1_res_counter])
            var.table_n1.append(["Node1 L2", "{:.2f}".format(node1_UB_in_V), "{:.2f}".format(node1_power_L2_W),
                              "{:.2f}".format(var.energy_n1_l2 / 10),
                              "{:.2f}".format(node1_IB_mA), "{:.2f}".format(node1_phase_IB_),node1_res_counter])
            var.table_n1.append(["Node1 L3", "{:.2f}".format(node1_UC_in_V), "{:.2f}".format(node1_power_L3_W),
                              "{:.2f}".format(var.energy_n1_l3 / 10),
                              "{:.2f}".format(node1_IC_mA), "{:.2f}".format(node1_phase_IC_),node1_res_counter])
            print(tabulate(var.table_n1, headers, tablefmt="grid"))

    #### Node 2 #######
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

    # Get the Result Counter from Brick 4-602 to check how much measurement the brick make
    # The Result Counter goes from 0 to 255. After 255 the counter begins from 0 again.
    node2_res_counter = bB.getByte(2, 1, 2)
    # Get the Information back in which Command (page) we are currently.
    node2_page = bB.getByte(2,1,0)
    # Every time when the result counter goes up  1 we get a new mean result of the power.
    # The Average Depth is preconfigured on 10.
    # If you want a another average depth. You muss change it on Page 4 on Byte 11. You can change it between 1 and 50.

    # Check if a new mean result is coming
    if node2_res_counter != var.counter_check2:
        # Read out the current Power on L1-L3
        node2_power_L1 = bB.getShort(2, 1, 4)
        node2_power_L2 = bB.getShort(2, 1, 6)
        node2_power_L3 = bB.getShort(2, 1, 8)
        # Calculate Digits to W
        node2_power_L1_W = (node2_power_L1 * 334.346)
        node2_power_L2_W = (node2_power_L2 * 334.346)
        node2_power_L3_W = (node2_power_L3 * 334.346)
        # Read out the current I_A eff - I_C eff
        node2_IA_eff = bB.getShort(2, 1, 16)
        node2_IB_eff = bB.getShort(2, 1, 18)
        node2_IC_eff = bB.getShort(2, 1, 20)
        # Calculate Digits in mA
        node2_IA_mA = (node2_IA_eff * 187.351)
        node2_IB_mA = (node2_IB_eff * 187.351)
        node2_IC_mA = (node2_IC_eff * 187.351)
        # Add the Power Value to Variable energy_n2_l1-l3
        var.energy_n2_l1 += node2_power_L1_W
        var.energy_n2_l2 += node2_power_L2_W
        var.energy_n2_l3 += node2_power_L3_W
        # update the var.counter_check2
        var.counter_check2 = node2_res_counter
        # Everytime when the result counter goes 15(1.5 seconds) up we print out the state
        if not var.counter_check2 % 15:
            var.table_n2.clear()
            # Print out the current state of Voltage, Power and the energy in Watt pro seconds
            headers = ["Spannung in V", "Leistung in W", "Energie W pro s", "Strom in mA", "Phase in ° ", "Counter"]
            var.table_n2.append(["Node2 L1", "{:.2f}".format(node2_UA_in_V), "{:.2f}".format(node2_power_L1_W),
                              "{:.2f}".format(var.energy_n2_l1 / 10),
                              "{:.2f}".format(node2_IA_mA), "{:.2f}".format(node2_phase_IA_),node2_res_counter])
            var.table_n2.append(["Node2 L2", "{:.2f}".format(node2_UB_in_V), "{:.2f}".format(node2_power_L2_W),
                              "{:.2f}".format(var.energy_n2_l2 / 10),
                              "{:.2f}".format(node2_IB_mA), "{:.2f}".format(node2_phase_IB_),node2_res_counter])
            var.table_n2.append(["Node2 L3", "{:.2f}".format(node2_UC_in_V), "{:.2f}".format(node2_power_L3_W),
                              "{:.2f}".format(var.energy_n2_l3 / 10),
                              "{:.2f}".format(node2_IC_mA), "{:.2f}".format(node2_phase_IC_),node2_res_counter])

            print(tabulate(var.table_n2, headers, tablefmt="grid"))

    # In every UpdateCycle the counter raise by 1
    connect.counter +=1
    # Send the commands on the Remote Master (bB.getBit, bB.putBit, ...)
    if not self.thread:
      connect.update()
    # Start the Thread with given UpdateRate
    threading.Timer(connect.updateRate, userinterface).start()
if __name__ == "__main__":
# Initialize the Functions to start the Program
    connect.updateRate = updateRate
    connect.start_serial()
    userinterface()
