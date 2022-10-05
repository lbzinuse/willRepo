#!/usr/bin/python3

import serial
import io


class carCAN:

    ser = 0
    def __init__(self):
        print("Hello World")
        self.ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=0)
        self.ser.bytesize = 8
        self.ser.parity = 'N'
        self.ser.stopbits = 1
        self.ser.open()
        self.ser.write('root')
        self.ser.flush()
        line = self.ser.readline()
        print(line)

    """
    this will request the engine coolant temprature.    "01 05"
    The response will be of the form:
    41 05 7B
    The 41 05 shows that this is a response to a
    mode 1 request for PID 05, while the 7B is the desired
    data. Converting the hexadecimal 7B to decimal, one
    gets 7 x 16 + 11 = 123. This represents the current
    temperature in degrees Celsius, but with the zero
    offset to allow for subzero temperatures. To convert to
    the actual coolant temperature, you need to subtract
    40 from the value obtained. In this case, then, the
    coolant temperature is 123 - 40 or 83°C.
    """
    def engine_coolant_temerature(self):
        response = self.transmit("0105")
        if response[0:4] == "4105": # verify if response is for transmitted command
            temprature = ((int(response[4:6], 16) -40) * 9/5) + 32 # this is in fahrenheit
            return temprature


    def engine_rpm(self):
        response = self.transmit("010C")
        if response[0:4] == "410C":  # verify if response is for transmitted command
            rpm = int(response[4:8], 16)   # this is changing from hex to decimal
            return rpm/4 # the rpm number is sent in increments of 1/4.  To convert to actual engine speed, device by 4

    def diagnostic_trouble_codes(self):
        response = self.transmit("010C")
        if response[0:4] == "4101":  # verify if response is for transmitted command
            num_of_errors = int(response[4:6], 16)  # this is changing from hex to decimal
            num_of_errors = num_of_errors - 128 # actual number of errors
        response = self.transmit("03")
        if response[0:2] == "43":  # verify if response is for transmitted command
            for x in range(2, num_of_errors*4, 4)
                error1 = response[x:x+4]
                print(error1)

    def transmit(self, message):
        return self.ser.write(message)



if __name__ == '__main__':
    response = "43013300000000"
    num_of_errors = 1


    obj = carCAN()
    print(obj.engine_coolant_temerature())


