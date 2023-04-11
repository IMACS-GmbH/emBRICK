
from emBRICK.modbus_rtu import bB_update
from emBRICK.modbus_rtu import bB
from emBRICK.modbus_rtu import connect
import threading
import math
import time
import sys
import numpy as np
from datetime import datetime
import scipy.interpolate

lock = threading.Lock()

brick_info = ''
lock.acquire()
def findBricks():
    '''
    Check connected Bricks and configure the corresponding objects
    '''
    
    # TODO: braucht man das global hier? 
    global brick_info

    bricks = {2181: CAE_G8Di8Do, 4602: CAE_B3U4I, 4603: CAE_B3U4I, 2470: CAE_G2Mi2Ao, 2471: CAE_G2Mi2Ao,
              2472: CAE_G2Mi2Ao, 2431: CAE_G4Ai4Tmp, 2432: CAE_G4Ai4Tmp, 2433: CAE_G4Ai4Tmp, 2434: CAE_G4Ai4Tmp,
              2435: CAE_G4Ai4Tmp, 2436: CAE_G4Ai4Tmp}
    '''
    bricks = {2181: G8Di8Do, 4602: B3U4I, 4603: B3U4I, 2470: G2Mi2Ao, 2471: G2Mi2Ao, 2472: G2Mi2Ao, 2431: G4Ai4Tmp,
              2432: G4Ai4Tmp, 2433: G4Ai4Tmp, 2434: G4Ai4Tmp, 2435: G4Ai4Tmp, 2436: G4Ai4Tmp}
    '''
    for node in connect.unit_id:
        for i in range(connect.master["local" + str(node)].number_bricks):
            id = connect.slave["master" + str(node)]["Brick" + str(i + 1)].brick_id
            found = bricks.get(id, "None") # find brick name in dictionary or otherwise return "None"
            if found == "None":
                error = open("error.txt", "a+")
                error.write(f'{datetime.now()} ERROR: No Brick with ID {id} found.\n')
                error.close()
                brick_info += f'NOT FOUND! Brick {i+1}: {connect.slave["master" + str(node)]["Brick" + str(i+1)].name} '\
                f'ID: {connect.slave["master" + str(node)]["Brick" + str(i+1)].brick_id // 1000}-{connect.slave["master" + str(node)]["Brick" + str(i+1)].brick_id % 1000} '\
                f'Hardware Revision: {connect.slave["master" + str(node)]["Brick" + str(i+1)].hard_rev}\n'
            else:
                globals()[f"Brick{i + 1}"] = found(node, bricks[id], i+1, \
                connect.slave["master" + str(node)]["Brick" + str(i+1)].brick_id,\
                connect.slave["master" + str(node)]["Brick" + str(i+1)].data_miso,\
                connect.slave["master" + str(node)]["Brick" + str(i+1)].data_mosi,\
                connect.slave["master" + str(node)]["Brick" + str(i+1)].offset_miso,\
                connect.slave["master" + str(node)]["Brick" + str(i+1)].offset_mosi)

                found = "Brick"+str(i+1)

                brick_info += f'{found}: {connect.slave["master" + str(node)]["Brick" + str(i+1)].name} '\
                f'ID: {connect.slave["master" + str(node)]["Brick" + str(i+1)].brick_id // 1000}-{connect.slave["master" + str(node)]["Brick" + str(i+1)].brick_id % 1000} '\
                f'Hardware Revision: {connect.slave["master" + str(node)]["Brick" + str(i+1)].hard_rev}\n'
    print(brick_info)
    print("To abort the program please press STRG + C\n\n")


def start():
    return lock.release()



class CAE_G8Di8Do:
    '''
    Brick Information can be found in eB_products.pdf under Chapter 1.6.1 G-8Di8Do-01
    Integrated Functions:
    digital_input(di_pos)
    digital_output(do_pos, value)
    '''
    def __init__(self, node, name, brick_no, brick_id, data_length_miso, data_length_mosi, offset_miso, offset_mosi):
        self.node = node
        self.name = name
        self.id = 2181
        self.brick_no = brick_no
        self.brick_id = brick_id
        # Data Lenght
        self.data_length_miso = data_length_miso
        self.data_length_mosi = data_length_mosi
        # Offset
        self.offset_miso = offset_miso
        self.offset_mosi = offset_mosi

    def digital_input(self, di_pos):
        '''
        Check the Digital Input value from di_pos (1 - 8) and return the value.
        '''
        di = ''
        if di_pos < 1 or di_pos > 8:
            error = open("error.txt", "a")
            error.write(f'{datetime.now()} ERROR: Digital Input {di_pos} is out of range. Only between 1 and 8\n')
            error.close()
        else:
            # Check if Brick is found, if not give a Error as Msg in "error.txt"
            if self.brick_no:
                lock.acquire()
                byte = bB_update.updated[self.node][self.offset_miso + 1]
                lock.release()
                di = 1 if (byte & (1 << (di_pos - 1))) else 0
            else:
                error = open("error.txt", "a")
                error.write(f'{datetime.now()} ERROR: No Brick with ID {self.id} found.\n')
                error.close()
        return di

    def digital_output(self, do_pos, value):
        '''
        Put Digital Output with value (1 = on or 0 = off) on Position do_pos (1 - 8)
        '''
        if do_pos < 1 or do_pos > 8:
            error = open("error.txt", "a")
            error.write(
                f'{datetime.now()} ERROR: Digital Input {do_pos} is out of range. Only between 1 and 8\n')
            error.close()
        lock.acquire()
        # Check if Brick is found, if not give a Error as Msg in "error.txt"
        if self.brick_no:
            if ((bB.put[self.node][self.offset_mosi] >> do_pos) & 1) != value:
                if value:
                    bB.put[self.node][self.offset_mosi] += (1 << do_pos)
                else:
                    bB.put[self.node][self.offset_mosi] -= (1 << do_pos)
        else:
            error = open("error.txt", "a")
            error.write(f'{datetime.now()} ERROR: No Brick with ID {self.id} found.\n')
            error.close()
        lock.release()


class CAE_B3U4I:
    ''' Brick Information can find in eB_products.pdf unter Chapter 5.3.7 B-3U4I-400-##-RB
    Integrated Functions:
    phase_cor(phase_cor_angle_l1, phase_cor_angle_l2, phase_cor_angle_l3)
    average_depth(avg_interval)
    resultCnt()
    readPower()
    calcEnergy()
    readPhase()
    readCurrent()
    readVoltage()
    '''
    def __init__(self, node, name, brick_no, brick_id, data_length_miso, data_length_mosi, offset_miso, offset_mosi):
        self.node = node
        self.name = name
        self.id = [4602, 4603]
        self.brick_id = brick_id
        self.brick_no = brick_no
        # Data Lenght
        self.data_length_miso = data_length_miso
        self.data_length_mosi = data_length_mosi
        # Offset
        self.offset_miso = offset_miso
        self.offset_mosi = offset_mosi
        self.avg_interval = 10
        # PreConfigured Current, Voltage, Power Correction Variables 4-602
        self.cur_val_602 = 0.1873508544
        self.vol_val_602 = 0.01361541109
        self.pow_val_602 = 334.3461753
        # PreConfigured Current, Voltage, Power Correction Variables 4-603
        self.cur_val_603 = 0.006245028479
        self.vol_val_603 = 0.01361541109
        self.pow_val_603 = 11.14487251
        #
        self.filter_max = 1
        self.filter_min = -1
        self.result_cnt = ''
        self.old_result_cnt = 0
        self.power = [0] * 3
        self.phase = [0] * 3
        self.current = [0] * 4
        self.voltage = [0] * 3
        self.energy = [0] * 3
        self.maxsize = sys.maxsize
        self.run = False
        self.raw_power = [0] * 3

    def start(self):
        self.run = True
    
    def stop(self):
        self.run = False

    def phase_cor(self, phase_cor_angle_l1, phase_cor_angle_l2, phase_cor_angle_l3):
        '''
        Set how much the phase angle for L1 - L3 should be corrected.
        '''
        if self.brick_no:
            lock.acquire()
            # Set Page to 1
            bB.put[self.node][self.offset_mosi] = 1
            # Read out Acknowledge Counter
            ackn_cnt = bB_update.updated[self.node][self.offset_miso + 2]
            ### Phase L1 Correction Calculation  ###
            phi = phase_cor_angle_l1 / 180 * math.pi
            omega = 2 * math.pi * (50 / 8000)
            phase_correction_val = ((math.sin(phi - omega) + math.sin(omega)) / (math.sin(2 * omega - phi))) * (2 ** 27)
            phase_val = math.ceil(phase_correction_val)
            # Put the calculated value for Phase L1 to brick
            bB.put[self.node][self.offset_mosi + 4] = (phase_val >> 24) & 0xFF
            bB.put[self.node][self.offset_mosi + 5] = (phase_val >> 16) & 0xFF
            bB.put[self.node][self.offset_mosi + 6] = (phase_val >> 8) & 0xFF
            bB.put[self.node][self.offset_mosi + 7] = phase_val & 0xFF
            ### Phase L2 Correction Calculation  ###
            phi = phase_cor_angle_l2 / 180 * math.pi
            phase_correction_val = ((math.sin(phi - omega) + math.sin(omega)) / (math.sin(2 * omega - phi))) * (2 ** 27)
            phase_val = math.ceil(phase_correction_val)
            # Put the calculated value for Phase L2 to brick
            bB.put[self.node][self.offset_mosi + 8] = (phase_val >> 24) & 0xFF
            bB.put[self.node][self.offset_mosi + 9] = (phase_val >> 16) & 0xFF
            bB.put[self.node][self.offset_mosi + 10] = (phase_val >> 8) & 0xFF
            bB.put[self.node][self.offset_mosi + 11] = phase_val & 0xFF
            ### Phase L3 Correction Calculation  ###
            phi = phase_cor_angle_l3 / 180 * math.pi
            phase_correction_val = ((math.sin(phi - omega) + math.sin(omega)) / (math.sin(2 * omega - phi))) * (2 ** 27)
            phase_val = math.ceil(phase_correction_val)
            # Put the calculated value for Phase L3 to brick
            bB.put[self.node][self.offset_mosi + 12] = (phase_val >> 24) & 0xFF
            bB.put[self.node][self.offset_mosi + 13] = (phase_val >> 16) & 0xFF
            bB.put[self.node][self.offset_mosi + 14] = (phase_val >> 8) & 0xFF
            bB.put[self.node][self.offset_mosi + 15] = phase_val & 0xFF
            # Trigger To Sub hochzählen um die Einstellung zu speichern
            bB.put[self.node][self.offset_mosi + 1] = ackn_cnt + 1
            # Check if the brick has saved the Configuration
            check_ackn_cnt = bB_update.updated[self.node][self.offset_miso + 2]
            lock.release()
            time.sleep(0.05)
            lock.acquire()
            retry = 5
            # If the Acknowledge Counter not count up, retry it 5 times
            while (check_ackn_cnt == ackn_cnt) and retry < 0:
                bB.put[self.node][self.offset_mosi + 1] = ackn_cnt + 1
                time.sleep(0.1)
                check_ackn_cnt = bB_update.updated[self.node][self.offset_miso + 2]
                retry -= 1
            if not retry:
                error = open("error.txt", "a")
                error.write(f'{datetime.now()} ERROR: phase_cor(), Acknowledge Counter do not count up.\n')
                error.close()
            lock.release()
        else:
            error = open("error.txt", "a")
            error.write(f'{datetime.now()} ERROR: phase_cor(), no Brick with ID {self.id} found.\n')
            error.close()

    def average_depth(self, avg_interval):
        '''
        Set in which interval the brick should store and calculate the averages of voltage, current, angle, power.
        Brick need 10 ms for one measure value (corresponds to avg_interval = 1). avg_interval should be between 1 - 50
        '''
        if self.brick_no:
            lock.acquire()
            self.avg_interval = avg_interval
            # Set Page to 4
            bB.put[self.node][self.offset_mosi] = 4
            # Read out Acknowledge Counter
            ackn_cnt = bB_update.updated[self.node][self.offset_miso + 2]
            # Set in which interval the average will be calculated (0 - 50)
            bB.put[self.node][self.offset_mosi + 11] = avg_interval
            # Count up Trigger To Sub to save the configuration
            bB.put[self.node][self.offset_mosi + 1] = ackn_cnt + 1
            lock.release()
            time.sleep(0.05)
            lock.acquire()
            check_ackn_cnt = bB_update.updated[self.node][self.offset_miso + 2]
            retry = 5
            # If the Acknowledge Counter not count up, retry it 5 times
            while (check_ackn_cnt == ackn_cnt) and retry < 0:
                bB.put[self.node][self.offset_mosi + 1] = ackn_cnt + 1
                time.sleep(0.1)
                check_ackn_cnt = bB_update.updated[self.node][self.offset_miso + 2]
                retry -= 1
            if not retry:
                error = open("error.txt", "a")
                error.write(
                    f'{datetime.now()} ERROR: average_depth(), Acknowledge Counter dont count up.\n')
                error.close()
            lock.release()
        else:
            error = open("error.txt", "a")
            error.write(f'{datetime.now()} ERROR: average_depth(), no Brick with ID {self.id} found.\n')
            error.close()

    def resultCnt(self):
        '''
        Return the current state of Result Counter
        '''
        page = bB_update.updated[self.node][self.offset_miso + 1]
        if not page == 1:
            lock.acquire()
            # Set the page to 1
            bB.put[self.node][self.offset_mosi] = 1
            # Read out the Acknowledge Counter
            ackn_cnt = bB_update.updated[self.node][self.offset_miso + 2]
            # Count up Trigger To Sub to save the configuration
            bB.put[self.node][self.offset_mosi + 1] = ackn_cnt + 1
            lock.release()
            lock.acquire()
            check_ackn_cnt = bB_update.updated[self.node][self.offset_miso + 2]
            retry = 5
            # If the Acknowledge Counter not count up, retry it 5 times
            while (check_ackn_cnt == ackn_cnt) and retry < 0:
                time.sleep(0.1)
                check_ackn_cnt = bB_update.updated[self.node][self.offset_miso + 2]
                retry -= 1
            if not retry:
                error = open("error.txt", "a")
                error.write(
                    f'{datetime.now()} ERROR: Acknowledge Counter dont count up, switch to Page 1 may failed.\n')
                error.close()
            lock.release()
        return bB_update.updated[self.node][self.offset_miso + 3]

    def readPower(self):
        '''
        Return Power[L1, L2, L3] in Watt as measured by brick.
        In 4-602 the Digits will be multiplied with self.pow_val_602
        In 4-603 the Digits will be multiplied with self.pow_val_603
        '''
        page = bB_update.updated[self.node][self.offset_miso + 1]
        if not page == 1:
            bB.put[self.node][self.offset_mosi] = 1
            # CommandCounter auslesen
            commandcounter = bB_update.updated[self.node][self.offset_miso + 2]
            newcounter = commandcounter
            # Trigger To Sub hochzählen um die Einstellung zu speichern
            bB.put[self.node][self.offset_mosi + 1] = commandcounter + 1
            while commandcounter == newcounter:
                newcounter = bB_update.updated[self.node][self.offset_miso + 2]

        power_l1 = (bB_update.updated[self.node][self.offset_miso + 5] << 8) + bB_update.updated[self.node][
            self.offset_miso + 6]
        power_l2 = (bB_update.updated[self.node][self.offset_miso + 7] << 8) + bB_update.updated[self.node][
            self.offset_miso + 8]
        power_l3 = (bB_update.updated[self.node][self.offset_miso + 9] << 8) + bB_update.updated[self.node][
            self.offset_miso + 10]
        self.raw_power = [power_l1, power_l2, power_l3]
        if self.brick_id == 4602:
            self.power = [i * self.pow_val_602 if i > self.filter_max or i < self.filter_min else 0 for i in np.int16(self.raw_power)]
        else:
            self.power = [i * self.pow_val_603 if i > self.filter_max or i < self.filter_min else 0 for i in np.int16(self.raw_power)]

        return self.power

    def calcEnergy(self):
        '''
        Calculate and Return the Energy[L1, L2, L3] in Ws from Power[L1, L2, L3]
        '''
        if self.run:
            energy_val = self.avg_interval / 100
            self.result_cnt = self.resultCnt()
            if self.result_cnt != self.old_result_cnt:
                self.old_result_cnt = self.result_cnt
                self.energy[0] += energy_val * self.power[0]
                if self.energy[0] > self.maxsize:
                    self.energy[0] = 0
                self.energy[1] += energy_val * self.power[1]
                if self.energy[1] > self.maxsize:
                    self.energy[1] = 0
                self.energy[2] += energy_val * self.power[2]
                if self.energy[2] > self.maxsize:
                    self.energy[2] = 0
        return self.energy

    def readPhase(self):
        '''
        Return the Phase[L1, L2, L3]  as degree
        '''
        page = bB_update.updated[self.node][self.offset_miso + 1]
        if not page == 1:
            bB.put[self.node][self.offset_mosi] = 1
            # CommandCounter auslesen
            commandcounter = bB_update.updated[self.node][self.offset_miso + 2]
            newcounter = commandcounter
            # Trigger To Sub hochzählen um die Einstellung zu speichern
            bB.put[self.node][self.offset_mosi + 1] = commandcounter + 1
            while commandcounter == newcounter:
                newcounter = bB_update.updated[self.node][self.offset_miso + 2]

        self.phase[0] = (bB_update.updated[self.node][self.offset_miso + 11] << 8) + bB_update.updated[self.node][
            self.offset_miso + 12]
        self.phase[1] = (bB_update.updated[self.node][self.offset_miso + 13] << 8) + bB_update.updated[self.node][
            self.offset_miso + 14]
        self.phase[2] = (bB_update.updated[self.node][self.offset_miso + 15] << 8) + bB_update.updated[self.node][
            self.offset_miso + 16]
        self.phase = [i / 20480 * 360 for i in self.phase]
        # Check if Phase is over 180 degree.
        x = range(3)
        for i in x:
            if self.phase[i] > 180:
                self.phase[i] -= 360

        return self.phase

    def readCurrent(self):
        '''
        Return the Current[L1, L2, L3] in Ampere
        '''
        page = bB_update.updated[self.node][self.offset_miso + 1]
        if not page == 1:
            bB.put[self.node][self.offset_mosi] = 1
            # CommandCounter auslesen
            commandcounter = bB_update.updated[self.node][self.offset_miso + 2]
            newcounter = commandcounter
            # Trigger To Sub hochzählen um die Einstellung zu speichern
            bB.put[self.node][self.offset_mosi + 1] = commandcounter + 1
            while commandcounter == newcounter:
                newcounter = bB_update.updated[self.node][self.offset_miso + 2]
        self.current[0] = (bB_update.updated[self.node][self.offset_miso + 17] << 8) + bB_update.updated[self.node][
            self.offset_miso + 18]
        self.current[1] = (bB_update.updated[self.node][self.offset_miso + 19] << 8) + bB_update.updated[self.node][
            self.offset_miso + 20]
        self.current[2] = (bB_update.updated[self.node][self.offset_miso + 21] << 8) + bB_update.updated[self.node][
            self.offset_miso + 22]
        self.current[3] = (bB_update.updated[self.node][self.offset_miso + 29] << 8) + bB_update.updated[self.node][
            self.offset_miso + 30]
        if self.brick_id == 4602:
            self.current = [i * self.cur_val_602 for i in self.current]
        else:
            self.current = [i * self.cur_val_603 for i in self.current]
        return self.current

    def readVoltage(self):
        '''
        Return the Voltage[L1, L2, L3] in Volt
        '''
        page = bB_update.updated[self.node][self.offset_miso + 1]
        if not page == 1:
            bB.put[self.node][self.offset_mosi] = 1
            # CommandCounter auslesen
            commandcounter = bB_update.updated[self.node][self.offset_miso + 2]
            newcounter = commandcounter
            # Trigger To Sub hochzählen um die Einstellung zu speichern
            bB.put[self.node][self.offset_mosi + 1] = commandcounter + 1
            while commandcounter == newcounter:
                newcounter = bB_update.updated[self.node][self.offset_miso + 2]
        self.voltage[0] = (bB_update.updated[self.node][self.offset_miso + 23] << 8) + bB_update.updated[self.node][
            self.offset_miso + 24]
        self.voltage[1] = (bB_update.updated[self.node][self.offset_miso + 25] << 8) + bB_update.updated[self.node][
            self.offset_miso + 26]
        self.voltage[2] = (bB_update.updated[self.node][self.offset_miso + 27] << 8) + bB_update.updated[self.node][
            self.offset_miso + 28]
        if self.brick_id == 4602:
            self.voltage = [i * self.vol_val_602 for i in self.voltage]
        else:
            self.voltage = [i * self.vol_val_603 for i in self.voltage]
        return self.voltage


class CAE_G2Mi2Ao:
    '''
    Brick Information can find in eB_products.pdf unter Chapter 1.6.11 G-2Mi2Ao-0#
    Integrated Functions:
    analog_output1(value)   # Set Output by Brick ID 2-470, 2-471 0..10 V or by Brick ID 2-472 0..20 mA
    analog_output2(value)   # Set Output by Brick ID 2-470 to 0..V or by Brick ID 2-471, 2-472 0..20 mA
    pulse_input_control1(self, period_duration, filter_period)
    pulse_input_attribute1(period_duration)
    pulse_input_control2(period_duration, filter_period)
    pulse_input_attribute2(period_duration)
    input_mode(mi1_mode, mi2_mode)     # mi1_mode 1: Voltage 2: Current 3: Pulse Pullup
                                       # mi2_mode 1: Voltage 2: Pulse Pullup
    analog_input1()    # if mi1_mode: 1 convert digits in voltage or mi1_mode: 2 convert digits in current
    analog_input2()    # only digits in voltage possible
    pulse_input_sum1()   # Count the input pulse and give them back as value
    pulse_input_sum1_counter()  # Count from 0 to 3 in 1 second steps
    pulse_input_dif1()
    pulse_input_sum2() # Count the input pulse and give them back as value
    pulse_input_sum2_counter() # Count from 0 to 3 in 1 second steps
    pulse_input_dif2()
    '''
    def __init__(self, node, name, brick_no, brick_id, data_length_miso, data_length_mosi, offset_miso, offset_mosi):
        self.node = node
        self.name = name
        #self.id = [2470, 2471, 2472]
        self.brick_id = brick_id
        self.brick_no = brick_no
        # Data Length
        self.data_length_miso = data_length_miso
        self.data_length_mosi = data_length_mosi
        # Offset
        self.offset_miso = offset_miso
        self.offset_mosi = offset_mosi
        ##
        self.mi1_mode = ''
        self.mi2_mode = ''
        self.ai1 = ''
        self.ai2 = ''
        #
        self.oldPulse1 = 0
        self.oldPulse2 = 0
        self.pulse_sum1 = 0
        self.pulse_sum2 = 0
        self.maxsize = sys.maxsize
        self.run = False

    def start(self):
        self.run = True
    
    def stop(self):
        self.run = False

    def analog_output1(self, value):
        '''
        Set the analog output 1 to the value
        Brick ID 2-470, 2-471 set a value between 0..10 A
        Brick ID 2-472 set a value between 0..20 mA
        '''
        lock.acquire()
        # To set the analog output 1 to the value of the variable value
        if self.brick_id == 2470 or self.brick_id == 2471:
            # To set the analog output 1 to the value of the variable value
            # Analog Output Voltage 0...10 V
            val_in_digit = 15100 / 10 * value if value <= 10 else 15100
            bB.put[self.node][self.offset_mosi] = (int(val_in_digit) >> 8) & 0xFF
            bB.put[self.node][self.offset_mosi + 1] = int(val_in_digit) & 0xFF
        elif self.brick_id == 2472:
            # Analog Output Current 0...20 mA
            val_in_digit = 15000 / 20 * value if value <= 20 else 15000
            bB.put[self.node][self.offset_mosi] = (int(val_in_digit) >> 8) & 0xFF
            bB.put[self.node][self.offset_mosi + 1] = int(val_in_digit) & 0xFF
        lock.release()

    def analog_output2(self, value):
        '''
        Set the analog output 2 to the value
        Brick ID 2-470 set a value between 0..10 A
        Brick ID 2-471, 2-472 set a value between 0..20 mA
        '''
        lock.acquire()
        # To set the analog output 2 to the value of the variable value
        if self.brick_id == 2470:
            # Analog Output Voltage 0...10 V
            # Convert value in digits (0... 15100)
            val_in_digit = 15100 / 10 * value if value <= 10 else 15100
            # Put the Value in the update cycle
            bB.put[self.node][self.offset_mosi] = (int(val_in_digit) >> 8) & 0xFF
            bB.put[self.node][self.offset_mosi + 1] = int(val_in_digit) & 0xFF
        elif self.brick_id == 2471 or self.brick_id == 2472:
            # Analog Output Current 0...20 mA
            val_in_digit = 15000 / 20 * value if value <= 20 else 15000
            bB.put[self.node][self.offset_mosi] = (int(val_in_digit) >> 8) & 0xFF
            bB.put[self.node][self.offset_mosi + 1] = int(val_in_digit) & 0xFF
        lock.release()

    def pulse_input_control1(self, period_duration, filter_period):
        '''

        '''
        lock.acquire()
        part_a = (period_duration << 10) & 0x3F
        part_b = filter_period & 0x3FF
        total = part_a + part_b
        bB.put[self.node][self.offset_mosi + 4] = (total >> 8) & 0xFF
        bB.put[self.node][self.offset_mosi + 5] = total & 0xFF
        lock.release()

    def pulse_input_attribute1(self, period_duration):
        '''
        By setting the period from 0 to 65535 ms the function
        pulse_input_diff1() returns the pulses in the period
        '''
        lock.acquire()
        bB.put[self.node][self.offset_mosi + 6] = (period_duration >> 8) & 0xFF
        bB.put[self.node][self.offset_mosi + 7] = period_duration & 0xFF
        lock.release()

    def pulse_input_control2(self, period_duration, filter_period):
        '''

        '''
        lock.acquire()
        part_a = (period_duration << 10) & 0x3F
        part_b = filter_period & 0x3FF
        total = part_a + part_b
        bB.put[self.node][self.offset_mosi + 8] = (total >> 8) & 0xFF
        bB.put[self.node][self.offset_mosi + 9] = total & 0xFF
        lock.release()

    def pulse_input_attribute2(self, period_duration):
        '''
        By setting the period from 0 to 65535 ms the function
        pulse_input_diff1() returns the pulses in the period
        '''
        lock.acquire()
        bB.put[self.node][self.offset_mosi + 10] = (period_duration >> 8) & 0xFF
        bB.put[self.node][self.offset_mosi + 11] = period_duration & 0xFF
        lock.release()

    def input_mode(self, mi1_mode, mi2_mode):
        '''
        To set the multi inputs 1 and 2 in the different mode.
        mi1_mode 1: Voltage, Pulse or Digital-Input
                 2: Current
                 3: Pulse or Digital-Input + PullUp
        mi2_mode 1: Voltage, Pulse or Digital-Input
                 2: Pulse or Digital-Input + PullUp
        '''
        self.mi1_mode = mi1_mode
        self.mi2_mode = mi2_mode
        lock.acquire()
        # Mi1 Config1
        bit0 = bB.put[self.node][self.offset_mosi + 12] >> 0
        # Mi1 Config2
        bit1 = bB.put[self.node][self.offset_mosi + 12] >> 1
        # Mi2 Config1
        bit2 = bB.put[self.node][self.offset_mosi + 12] >> 2

        # voltage or digital input
        if self.mi1_mode == 1:
            if (bit0 & 1) != 0: bB.put[self.node][self.offset_mosi + 12] -= (1 << 0)
            if (bit1 & 1) != 0: bB.put[self.node][self.offset_mosi + 12] -= (1 << 1)

        # Current
        elif self.mi1_mode == 2:
            if (bit0 & 1) != 0: bB.put[self.node][self.offset_mosi + 12] -= (1 << 0)
            if (bit1 & 1) != 1: bB.put[self.node][self.offset_mosi + 12] += (1 << 1)

        # Pulse Pullup
        elif self.mi1_mode == 3:
            if (bit0 & 1) != 1: bB.put[self.node][self.offset_mosi + 12] += (1 << 0)
            if (bit1 & 1) != 0: bB.put[self.node][self.offset_mosi + 12] -= (1 << 1)

        if self.mi2_mode == 1 and (bit2 & 1) != 0:
            bB.put[self.node][self.offset_mosi + 12] -= (1 << 2)

        if self.mi2_mode == 2 and (bit2 & 1) != 1:
            bB.put[self.node][self.offset_mosi + 12] += (1 << 2)
        lock.release()

    ## Inputs ###
    def analog_input1(self):
        '''
        Return the value of the analog input 1 in voltage (mi1_mode = 1) or current (mi1_mode = 2)
        '''
        # To read the analog input 1 and gives the value back
        lock.acquire()
        short = (bB_update.updated[self.node][self.offset_miso + 1] << 8) + bB_update.updated[self.node][
            self.offset_miso + 2]
        if self.mi1_mode == 2:  # Current
            self.ai1 = (20 / 894) * short
        elif self.mi1_mode == 1:  # Voltage
            self.ai1 = (10 / 933) * short
        else:
            self.ai1 = short
        lock.release()
        return self.ai1

    def analog_input2(self, ):
        '''
        Return the value of the analog input 1 in voltage
        '''
        lock.acquire()
        short = (bB_update.updated[self.node][self.offset_miso + 3] << 8) + bB_update.updated[self.node][
            self.offset_miso + 4]
        self.ai2 = (10 / 933) * short
        lock.release()
        return self.ai2

    def pulse_input_sum1(self, ):
        '''
        Calculate and Return the pulse sum of Multi Input 1
        '''
        if self.run:
            lock.acquire()
            pulse = ((bB_update.updated[self.node][self.offset_miso + 5] & 0x3F) << 8) + bB_update.updated[self.node][
                self.offset_miso + 6]
            impuls_freq = (bB_update.updated[self.node][self.offset_miso + 7] << 8) + bB_update.updated[self.node][
                self.offset_miso + 8]
            if self.oldPulse1 != pulse:
                self.pulse_sum1 += 1 if impuls_freq == 0 else impuls_freq
                self.oldPulse1 = pulse
            lock.release()
        if self.pulse_sum1 >= self.maxsize:
            self.pulse_sum1 = 0
        return self.pulse_sum1

    def pulse_input_sum1_counter(self, ):
        '''
        Counts up from 0 to 3 every second and returns the value
        '''
        lock.acquire()
        bytes = bB_update.updated[self.node][self.offset_miso + 5] >> 6
        lock.release()
        return bytes

    def pulse_input_dif1(self, ):
        '''
        Calculate how many Impulse get Input 1 in the Period Duration (0..65535ms)
        Period Duration will be set in the function pulse_input_attribute1(period_duration)
        '''
        lock.acquire()
        short = (bB_update.updated[self.node][self.offset_miso + 7] << 8) + bB_update.updated[self.node][
            self.offset_miso + 8]
        lock.release()
        return short

    def pulse_input_sum2(self, ):
        '''
        Calculate and Return the pulse sum of Multi Input 2
        '''
        if self.run:
            lock.acquire()
            pulse = ((bB_update.updated[self.node][self.offset_miso + 9] & 0x3F) << 8) + bB_update.updated[self.node][
                self.offset_miso + 10]
            impuls_freq = (bB_update.updated[self.node][self.offset_miso + 11] << 8) + bB_update.updated[self.node][
                self.offset_miso + 12]
            if self.oldPulse2 != pulse:
                self.pulse_sum2 += 1 if impuls_freq == 0 else impuls_freq
                self.oldPulse2 = pulse
            lock.release()
        if self.pulse_sum2 >= self.maxsize:
            self.pulse_sum2 = 0
        return self.pulse_sum2

    def pulse_input_dif2(self, ):
        '''
        Calculate how many Impulse get Input 2 in the Period Duration (0..65535ms)
        Period Duration will be set in the function pulse_input_attribute2(period_duration)
        '''
        lock.acquire()
        short = (bB_update.updated[self.node][self.offset_miso + 11] << 8) + bB_update.updated[self.node][
            self.offset_miso + 12]
        lock.release()
        return short

    def input_image(self):
        '''

        '''
        ai_state = []
        lock.acquire()
        bytes = bB_update.updated[self.node][self.offset_miso + 13] & 0x03
        lock.release()
        ai_state.append(bytes & 1)
        ai_state.append((bytes >> 1) & 1)
        return ai_state


class CAE_G4Ai4Tmp:
    '''
    Brick Information can find in eB_products.pdf unter Chapter 1.6.8 G-4Ai4Tmp-0#
    Integrated Functions:
    analog_input1()
    analog_input2()
    analog_input3()
    analog_input4()
    temp_input1()
    temp_input2()
    temp_input3()
    temp_input4()
    '''
    def __init__(self, node, name, brick_no, brick_id, data_length_miso, data_length_mosi, offset_miso, offset_mosi):
        self.node = node
        self.name = name
        self.id = [2431, 2432, 2433, 2434, 2435, 2436]
        self.brick_id = brick_id
        self.brick_no = brick_no
        # Data Length
        self.data_length_miso = data_length_miso
        self.data_length_mosi = data_length_mosi
        # Offset
        self.offset_miso = offset_miso
        self.offset_mosi = offset_mosi
        self.digit_pt1000 = [0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 1000]
        self.temp_pt1000 = [-78.276, -56.207, -33.929, -10.714, 13.6, 38.8, 65.217, 92.609, 121.364, 151.429, 183.0, 215.789,
                250.556, 286.667, 325.0, 378.571]
        self.y_interp = ''


    def analog_input1(self):
        '''
        Return the value in mini Ampere (0 - 20) by Brick ID 2-431, 2-432, 2-433)
                         in Volt (0 - 30) by Brick ID 2-434, 2-435, 2-436)
        '''
        lock.acquire()
        short = (bB_update.updated[self.node][self.offset_miso + 1] << 8) + bB_update.updated[self.node][
            self.offset_miso + 2]
        if self.brick_id in [2431, 2432, 2433]:
            ai1 = (20 / 894) * short
        elif self.brick_id in [2434, 2435, 2436]:
            ai1 = (30 / 959) * short
        lock.release()
        return ai1

    def analog_input2(self):
        '''
        Return the value in mini Ampere (0 - 20) by Brick ID 2-431, 2-432, 2-433)
                         in Volt (0 - 30) by Brick ID 2-434, 2-435, 2-436)
        '''
        lock.acquire()
        short = (bB_update.updated[self.node][self.offset_miso + 3] << 8) + bB_update.updated[self.node][
            self.offset_miso + 4]
        if self.brick_id in [2431, 2432, 2433]:
            ai2 = (20 / 894) * short
        elif self.brick_id in [2434, 2435, 2436]:
            ai2 = (30 / 959) * short
        lock.release()
        return ai2

    def analog_input3(self):
        '''
        Return the value in mini Ampere (0 - 20) by Brick ID 2-431, 2-432, 2-433)
                         in Volt (0 - 30) by Brick ID 2-434, 2-435, 2-436)
        '''
        lock.acquire()
        short = (bB_update.updated[self.node][self.offset_miso + 5] << 8) + bB_update.updated[self.node][
            self.offset_miso + 6]
        if self.brick_id in [2431, 2432, 2433]:
            ai3 = (20 / 894) * short
        elif self.brick_id in [2434, 2435, 2436]:
            ai3 = (30 / 959) * short
        lock.release()
        return ai3

    def analog_input4(self):
        '''
        Return the value in mini Ampere (0 - 20) by Brick ID 2-431, 2-432, 2-433)
                         in Volt (0 - 30) by Brick ID 2-434, 2-435, 2-436)
        '''
        lock.acquire()
        short = (bB_update.updated[self.node][self.offset_miso + 7] << 8) + bB_update.updated[self.node][
            self.offset_miso + 8]
        if self.brick_id in [2431, 2432, 2433]:
            ai4 = (20 / 894) * short
        elif self.brick_id in [2434, 2435, 2436]:
            ai4 = (30 / 959) * short
        lock.release()
        return ai4

    def temp_input1(self):
        '''
        Return the temperature value from -10°C to 50°C by Brick ID 2-431, 2-434
                                     from 0°C to 110°C by Brick ID 2-432, 2-435
                                     from -50 to 350°C by Brick ID 2-434, 2-436
        '''
        lock.acquire()
        short = (bB_update.updated[self.node][self.offset_miso + 9] << 8) + bB_update.updated[self.node][
            self.offset_miso + 10]
        if self.brick_id in [2431, 2434]:
            ti = -10 + (60 / 948) * short
        elif self.brick_id in [2432, 2435]:
            ti = (110 / 901) * short
        elif self.brick_id in [2433, 2436]:
            ti = self.y_interp(short) if short > 0 and short < 1000 else None
        lock.release()
        return ti

    def temp_input2(self):
        '''
        Return the temperature value from -10°C to 50°C by Brick ID 2-431, 2-434
                                     from 0°C to 110°C by Brick ID 2-432, 2-435
                                     from -50 to 350°C by Brick ID 2-434, 2-436
        '''
        lock.acquire()
        short = (bB_update.updated[self.node][self.offset_miso + 11] << 8) + bB_update.updated[self.node][
            self.offset_miso + 12]
        if self.brick_id in [2431, 2434]:
            ti = -10 + (60 / 948) * short
        elif self.brick_id in [2432, 2435]:
            ti = (110 / 901) * short
        elif self.brick_id in [2433, 2436]:
            ti = self.y_interp(short) if short > 0 and short < 1000 else None
        lock.release()
        return ti

    def temp_input3(self):
        '''
        Return the temperature value from -10°C to 50°C by Brick ID 2-431, 2-434
                                     from 0°C to 110°C by Brick ID 2-432, 2-435
                                     from -50 to 350°C by Brick ID 2-434, 2-436
        '''
        lock.acquire()
        short = (bB_update.updated[self.node][self.offset_miso + 13] << 8) + bB_update.updated[self.node][
            self.offset_miso + 14]
        if self.brick_id in [2431, 2434]:
            ti = -10 + (60 / 948) * short
        elif self.brick_id in [2432, 2435]:
            ti = (110 / 901) * short
        elif self.brick_id in [2433, 2436]:
            ti = self.y_interp(short) if short > 0 and short < 1000 else None
        lock.release()
        return ti

    def temp_input4(self):
        '''
        Return the temperature value from -10°C to 50°C by Brick ID 2-431, 2-434
                                     from 0°C to 110°C by Brick ID 2-432, 2-435
                                     from -50 to 350°C by Brick ID 2-434, 2-436
        '''
        lock.acquire()
        short = (bB_update.updated[self.node][self.offset_miso + 15] << 8) + bB_update.updated[self.node][
            self.offset_miso + 16]
        if self.brick_id in [2431, 2434]:
            ti = -10 + (60 / 948) * short
        elif self.brick_id in [2432, 2435]:
            ti = (110 / 901) * short
        elif self.brick_id in [2433, 2436]:
            ti = self.y_interp(short) if short > 0 and short < 1000 else None
        lock.release()
        return ti


