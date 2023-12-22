#!/usr/bin/env python3

from periphery import I2C
from time import sleep, time_ns
import sys

# FIXME: replace this with a proper i2c to serial implentation, and add kernel tty to module.

i2c = I2C("/dev/links/csi1_i2c")
address = 0x1c

def dprint(a):
    b=a
    #print (a)

def send_visca(msg: bytes):
    msgs = [I2C.Message(b'\x80'+msg, read=False)]
    try:
        i2c.transfer(address, msgs)
    except Exception as e:
        dprint(e)

def read_visca_resp(count=16):
    msgs = [I2C.Message([0x80]), I2C.Message(bytearray(count), read=True)]
    i2c.transfer(address, msgs)
    data = msgs[-1].data
    return data

def get_rx_count():
    msgs = [I2C.Message([0x0A]), I2C.Message([0], read=True)]
    i2c.transfer(address, msgs)
    bytes_waiting = msgs[-1].data[0]
    dprint(f"fifo has {bytes_waiting} bytes")
    return bytes_waiting


def wait_for_input(timeout_ms):
    start = time_ns()
    while time_ns() < start + (timeout_ms * 1000000):
        msgs = [I2C.Message([0x09]), I2C.Message([0], read=True)]
        i2c.transfer(address, msgs)
        uart_status = msgs[-1].data[0]
        dprint(bin(uart_status))
        busy_tx       = bool(uart_status & 0b0010_0000)
        busy_rx       = bool(uart_status & 0b0001_0000)
        data_fulltx   = bool(uart_status & 0b0000_1000)
        data_emptytx  = bool(uart_status & 0b0000_0100)
        data_fullrx   = bool(uart_status & 0b0000_0010)
        data_emptyrx  = bool(uart_status & 0b0000_0001)
        if busy_rx == 0 and data_emptyrx == 0:
            time_taken = time_ns() - start
            dprint(f"waited {time_taken/1000000} ms for data in")
            return True
    time_taken = time_ns() - start
    dprint(f"waited {time_taken/1000000} ms for data in")
    return False

def wait_for_output(timeout_ms):
    start = time_ns()
    while time_ns() < start + (timeout_ms * 1000000):
        msgs = [I2C.Message([0x09]), I2C.Message([0], read=True)]
        i2c.transfer(address, msgs)
        uart_status = msgs[-1].data[0]
        # dprint(bin(uart_status))
        busy_tx       = bool(uart_status & 0b0010_0000)
        busy_rx       = bool(uart_status & 0b0001_0000)
        data_fulltx   = bool(uart_status & 0b0000_1000)
        data_emptytx  = bool(uart_status & 0b0000_0100)
        data_fullrx   = bool(uart_status & 0b0000_0010)
        data_emptyrx  = bool(uart_status & 0b0000_0001)
        if busy_tx == 0 and data_emptytx == 1:
            time_taken = time_ns() - start
            dprint(f"waited {time_taken/1000000} ms for data out")
            return True
    time_taken = time_ns() - start
    dprint(f"waited {time_taken/1000000} ms for data out")
    return False

def send_n_wait(msg: bytes):
    send_visca(msg)
    wait_for_output(50)


def main():
    send_visca(b'hello\xff')
    # send_n_wait(b'hello\xff')
    sleep(0.1)
    wait_for_input(50)
    out = read_visca_resp()
    print("out: ", out)

if __name__ == "__main__":
    if 'send' in sys.argv[1].lower():
        data = bytearray.fromhex(sys.argv[2])
        data = data + b'\xff' if data[-1] != b'\xff' else data
        send_n_wait(data)
    elif 'recv' in sys.argv[1].lower():
        wait_for_input(50)
        count = get_rx_count()
        data = read_visca_resp(count)
        print(data.hex())
    elif 'xfer' in sys.argv[1].lower():
        # clear rx buffer
        count = get_rx_count()
        data = read_visca_resp(count)
        # tx
        data = bytearray.fromhex(sys.argv[2])
        data = data + b'\xff' if data[-1] != b'\xff' else data
        send_n_wait(data)
        # rx
        wait_for_input(50)
        count = get_rx_count()
        data = read_visca_resp(count)
        print(data.hex())
    else:
        main()
