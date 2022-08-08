import serial
import io

if __name__ == '__main__':
    ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=0)
    ser.bytesize = 8
    ser.parity='N'
    ser.stopbits=1
    # ser.open()
    ser.write(b'ATZ')
    ser.flush()
    line = ser.readline()
    print(line == unicode("hello\n"))
    ser.close()