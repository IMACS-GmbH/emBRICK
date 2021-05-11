Voraussetzungen f�r SPI0 auf dem BeagleBone Black:

- Im Linux-Verzeichns /lib/firmware/ sollten folgende Dateien existieren:
	* "BB-SPIDEV0-00A0.dtbo"
	* "BB-SPIDEV0-00A0.dts"

Falls nicht, dann die Dateien aus diesem Repository dorthin kopieren und damit versuchen.
(Oder "Tutorial API Beaglebone Black � GNUBLIN" dazu lesen.)

- In der Datei "uEnv.txt", die �ber den "Wechseldatentr�ger/Massenspeicher" �ber Windows zu finden ist
  oder unter Linux "/media/BEAGLEBONE/uEnv.txt", muss am Ende der "optargs"-Zeile noch folgendes angeh�ngt werden:
  "capemgr.enable_partno=BB-SPIDEV0"
  
  Bsp. f�r uEnv.txt:
   optargs=quiet drm.debug=7 capemgr.enable_partno=BB-SPIDEV0
