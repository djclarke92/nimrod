Nimrod Home Automation

NodeMCU notes for ESP8266 devices

See https://nodemcu.readthedocs.io/en/master/ for ducomentation


0.	 Build your NodeMCU firmward in the cloud

https://nodemcu-build.com/index.php

Use the 1.5.4.1-final branch as the safe option but other should work as long as you add in the following
modules: adc crypto enduser_setup file gpio http net node pwm tmr uart websocket wifi + TLS


1.  Flash the nodemcu device

Put device into flash mode (hold flash button, press and release reset button, then release flash button)

./esptool.py --port /dev/ttyUSB2 erase_flash
./esptool.py --port /dev/ttyUSB2 write_flash -fm dio 0x00000 ../nodemcu-1.5.4.1-final-8-modules-2018-02-13-00-51-36-integer.bin

there is a delay after the first reboot while the file system is formatted by the new firmware.


2.  Upload Nimrod init.lua file using ESPlorer

Read up on the enduser_setup module to see how to configure the NodeMCU WiFi for your home network.

2.1 Example log from ESPlorer once the init.lua code is running

NodeMCU 2.1.0+djc build unspecified powered by Lua 5.1.4 on SDK 2.1.0(116b762)
node init
enduser_setup is connected
loaded
wifi is disconnected, state 0
IP unavaiable, Waiting...
IP is 192.168.1.26
RSSI is -63
connect skt
skt connected
skt send: MCU000CID1328586....
skt msg sent
skt recv: PG000000
skt recv: NNMCU001
New name MCU001
skt send: MCU001PG............
skt msg sent
skt recv: OK000000
skt send: MCU001PG............
skt msg sent
skt recv: OK000000
skt send: MCU001PG............
skt msg sent
skt recv: OK000000
skt send: MCU001PG............

2.2	Example log file from the nimrod host

20190923-181020 INFO : new client connection (2 clients)
20190923-181020 INFO : socket is blocking
20190923-181024 INFO : SSL_accept returned 1
20190923-181024 INFO : SSL connection using ECDHE-RSA-AES256-GCM-SHA384
20190923-181024 INFO : No certificates
20190923-181024 INFO : Sent ping msg type E_MT_MCU_MSG to 192.168.1.26, socket 1
20190923-181024 INFO : Got NodeMCU msg 'MCU000CID1328586....'
20190923-181024 INFO : Got msg type E_MT_MCU_MSG (segment 0 of 0)
20190923-181024 INFO : MCU chip id from 'MCU000'
20190923-181024 INFO : Found matching MCU chip id for MCU001
20190923-181024 INFO : New MCU device name is 'MCU001'
20190923-181024 INFO : New MCU name set to 'MCU001'
20190923-181024 WARN : Extra socket 0 for mcu device MCU001, ping scheduled
20190923-181024 INFO : Sent reply msg type E_MT_MCU_MSG to 192.168.1.26, socket 1
20190923-181027 INFO : Sent ping msg type E_MT_MCU_MSG to 192.168.1.26, socket 0


