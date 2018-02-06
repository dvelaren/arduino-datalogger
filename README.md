# arduino-datalogger
This program allows an Arduino to acquire system response to an injected setpoint and store it on a microSD

# Arduino datalogger for System Identification
![alt text](https://raw.githubusercontent.com/tidusdavid/arduino-datalogger/master/resources/architecture.png)
This program allows an Arduino to acquire system response to an injected setpoint and store it on a microSD. It uses Recursive Moving Average for filtering the measurement and uses as trigger a switch, which is debounced internally to avoid problems in system identification
