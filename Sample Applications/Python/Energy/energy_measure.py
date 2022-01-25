# Load from emBrick serial Module the connect class
import datetime
import time
from emBRICK.bricks import findBricks
import emBRICK.bricks as eB
from emBRICK.modbus_rtu import bB
from emBRICK.modbus_rtu import connect
import scipy.interpolate
# Here you can change with:
# connect.port = "COM*" or "/dev/ttySC0" |your port
# connect.unit_id = [1,2] |your connected nodes
# connect.timeout = 0.1 | to change the Timeout for the Modbus Connection
# connect.baudrate = 460800 | to change the Baudrate
# connect.updateRate = 0.005 | for +5 ms that will be slow down the updateCycle between LWCS and RemoteMaster

# RaspberryPi: Here is the Port always the same if you use our PiBrick Driver
connect.port = "/dev/ttySC0"
# The baudrate is preconfigured to 460800 (for PiOS) use 460800
connect.baudrate = 115200
# Configure here with which Modbus Address/es the Programm should connect
connect.unit_id.append(1)
# The connection with the LWCS will be configured and the BrickBus UpdateCycle will be started
connect.start()
# Find connected bricks and preconfigured the brick to use
findBricks()

########## Programm Start ##################
backup = open("backup.txt", "w+")
try:
   ### Configuration of Bricks at the begin ###
   ## Brick1 B3U4I##
   # PreConfigured Current, Voltage, Power Correction Variables 4-602
   # eB.Brick1.cur_val_602 = 0.1873508544
   # eB.Brick1.vol_val_602 = 0.01361541109
   # eB.Brick1.pow_val_602 = 334.3461753
   # PreConfigured Current, Voltage, Power Correction Variables 4-603
   # eB.Brick1.cur_val_603 = 0.005026486336
   # eB.Brick1.vol_val_603 = 0.01361541109
   # eB.Brick1.pow_val_603 = 8.970263239
   # To set which results should be filtered in eB.Brick1.readPower()
   # eB.Brick1.filter_max = 1
   # eB.Brick1.filter_min = -1
   # Start the Energy calculation with a preconfigured Value
   # eB.Brick1.energy = [1000, 500, 250]
   #
   K_therm_power_kW = 1.143
   # Set how much measure values the brick should store and calculate the average until it begin from start
   # brick need 10 ms for one measure value. measure_store should be between 0 - 50
   # eB.Brick1.average_depth(avg_interval)
   eB.Brick1.average_depth(20)  # 20 measure values also 20 * 10 ms = 200 ms
   # Set how much the phase angle for L1 - L3 should be corrected
   # Brick1.phase_cor(phase_cor_l1, phase_cor_l2, phase_cor_l3)
   eB.Brick1.phase_cor(-1.6, -1.6, -1.6)

   ## Brick2 8Di8Do
   # No Configuration needed

   ## Brick3 8Di8Do
   # No Configuration needed

   ## Brick4 2Mi2Ao
   # Configure the Multi Inputs 1 & 2
   # mi_mode1 (1=voltage/digital_input 2=current 3=pulse_pullup)
   # mi_mode2 (1=voltage/digital_input 2=pulse_pullup)
   # Brick4.input_mode(mi_mode1, mi_mode2)
   eB.Brick4.input_mode(3, 2)
   # Set the Analog Output 1 to 5.2 V
   eB.Brick4.analog_output1(5.2)
   old_counter = 0
   # Multi Input 1
   # Here you can change the pulse valency from Multi Input 1
   pulse_valency_mi1 = 1
   # Set the Period Duration from 0 to 65545 ms
   period_duration_mi1 = 5000
   # sets the period in which the pulses are to be measured
   eB.Brick4.pulse_input_attribute1(period_duration_mi1)
   # Multi Input 2
   # Here you can change the pulse valency from Multi Input 2
   pulse_valency_mi2 = 1
   # Set the Period Duration from 0 to 65545 ms
   period_duration_mi2 = 5000
   # sets the period in which the pulses are to be measured
   eB.Brick4.pulse_input_attribute2(period_duration_mi2)

   ## Brick5 4Ai4Tmp ##
   # For the bricks with the PT1000 temperature sensor, an interpolation calculation is performed to determine the value.
   if eB.Brick8.brick_id in [2433, 2436]:
      eB.Brick8.y_interp = scipy.interpolate.interp1d(eB.Brick8.digit_pt1000, eB.Brick8.temp_pt1000)

   while True:
      ## Brick1 3U4I
      power = eB.Brick1.readPower()
      phase = eB.Brick1.readPhase()
      current = eB.Brick1.readCurrent()
      voltage = eB.Brick1.readVoltage()
      energy = eB.Brick1.calcEnergy()  # in Ws
      ## Brick 2 8Di8Do
      di1 = eB.Brick2.digital_input(1)
      eB.Brick2.digital_output(1, 1) if di1 else eB.Brick2.digital_output(1, 0)

      ## Brick3 8Di8Do
      di2 = eB.Brick3.digital_input(2)
      eB.Brick3.digital_output(2, 1) if di2 else eB.Brick3.digital_output(2, 0)

      ## Brick4 2Mi2Ao
      # Multi Input 1
      ai1 = eB.Brick4.analog_input1()
      impuls_sum_mi1 = eB.Brick4.pulse_input_sum1()
      amount_mi1 = impuls_sum_mi1 * pulse_valency_mi1
      diff_mi1 = eB.Brick4.pulse_input_dif1()
      flow_input_mi1 = (diff_mi1 / (period_duration_mi1 /1000)) * pulse_valency_mi1
      counter = eB.Brick4.pulse_input_sum1_counter()

      # Multi Input 2
      impuls_sum_mi2 = eB.Brick4.pulse_input_sum2()
      amount_mi2 = impuls_sum_mi2 * pulse_valency_mi2
      diff_mi2 = eB.Brick4.pulse_input_dif2()
      flow_input_mi2 = (diff_mi2 / (period_duration_mi2 /1000)) * pulse_valency_mi2
      #counter = eB.Brick4.pulse_input_sum2_counter()

      ## Brick8 4Ai4Tmp
      current_input1 = eB.Brick8.analog_input1()
      temp_forward = eB.Brick8.temp_input1()
      temp_return = eB.Brick8.temp_input2()

      # Wärmeleistung Berechnung
      therm_power = flow_input_mi2 * (temp_forward - temp_return) * K_therm_power_kW

      if counter != old_counter:
         old_counter = counter
         print("\n")
         print("Brick1 3U4I measurement results:")
         print(f"Power [L1, L2, L3]: {power}")
         print(f"Phase [IA, IB, IC]: {phase}")
         print(f"Current [IA, IB, IC, IN] : {current}")
         print(f"Voltage [UA, UB, UC] : {voltage}")
         print(f"Energy [L1, L2, L3]: {energy}")
         print("\n")

         ## Brick2 8Di8Do
         print("Brick2 8Di8Do measurement results:")
         print(f'Digital Input 1: {di1}')
         print("\n")

         ## Brick3 8Di8Do
         print("Brick3 8Di8Do measurement results:")
         print(f'Digital Input 2: {di2}')
         print("\n")

         ## Brick4 2Mi2Ao
         print("Brick4 2Mi2Ao measurement results:")
         # Impuls Input 1
         print("Multi Input 1")
         print(f"Impuls_Amount Mi1: {amount_mi1} nl")
         print(f"Impuls Frequenz Mi1: {flow_input_mi1} l/s")
         #print("Analog Input: ", ai1)
         #print("Counter: ", counter)

         # Impuls Input 2
         print("Multi Input 2")
         print(f"Impuls_Amount Mi2: {amount_mi2} nl")
         print(f"Impuls Frequenz Mi2: {flow_input_mi2} l/s")

         # Brick8 4Ai4Tmp
         print(f'Brick5 4Ai4Tmp measurement results: \n')
         print(f'Current Input 1: {current_input1} mA')
         print(f'Temperatur Vorlauf: {temp_forward} °C')
         print(f'Temperatur Rücklauf: {temp_return} °C')

         # Wärmeleistung
         print(f'Thermal Power: {therm_power} kW')

finally:
   # Brick 1 3U4I
   backup.write(f'Measurement Results {datetime.datetime.now()}')
   backup.write(f'Brick Brick1 3U4I:\n'
                f'Power [L1, L2, L3]: {power}\n'
                f'Phase [IA, IB, IC]: {phase}\n'
                f'Current [IA, IB, IC, IN] : {current}\n'
                f'Voltage [UA, UB, UC] : {voltage}\n'
                f'Energy [L1, L2, L3]: {energy}\n\n')
   # Brick4 2Mi2Ao
   backup.write(f'Brick Brick4 2Mi2Ao: \n'
                f'Multi Input 1\n'
                f'Impuls_Amount Mi1: {amount_mi1} nl\n'
                f'Impuls Frequenz Mi1: {flow_input_mi1} l/s\n'
                f'Impuls_Amount Mi2: {amount_mi2} nl\n\n'
                f'Multi Input 2\n'
                f'Impuls_Amount Mi2: {amount_mi2} nl\n'
                f'Impuls Frequenz Mi2: {flow_input_mi2} l/s')
   # Brick8 4Ai4Tmp
   backup.write(f'Brick5 4Ai4Tmp: \n'
                f'Current Input 1: {current_input1} mA'
                f'Temperatur Input 1: {temp_forward} °C'
                f'Temperatur Input 2: {temp_return} °C')
   backup.write(f'Thermal Power: {therm_power} kW')

   backup.close()
