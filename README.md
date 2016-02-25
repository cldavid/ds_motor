ds_motor<br>
========<br>
<br>
Aquarium Dosing Unit<br>
<br>
What do you need? <br>
	- Arduino uno yun <br>
	- Adafruit motor shield v2 <br>
<br>
Bridge console: <br>
Available via: <br>
	ssh root@arduino.lan <br>
	telnet localhost 6571 <br>
<br>
Commands: <br>
help			- Help function <br>
setUnixTime		- Set Unix Time <br>
getUnixTime		- Get System Time in seconds
getDate			- Get Human system time
drive_motor_1		- drive motor 1 for x ms
drive_motor_2		- drive motor 2 for x ms
drive_motor_3		- drive motor 3 for x ms
drive_motor_4		- drive motor 4 for x ms
set_motor_1		- program motor 1
set_motor_2		- program motor 2
set_motor_3		- program motor 3
set_motor_4		- program motor 4
get_motor_1		- get motor 1 info
get_motor_2		- get motor 2 info
get_motor_3		- get motor 3 info
get_motor_4		- get motor 4 info
disable_motor_1		- disable motor 1
disable_motor_2		- disable motor 2
disable_motor_3		- disable motor 3
disable_motor_4		- disable motor 4
system_info		- print system info
debug			- enable extra debug traces
save			- save current config
