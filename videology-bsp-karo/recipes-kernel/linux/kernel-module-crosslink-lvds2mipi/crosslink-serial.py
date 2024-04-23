from periphery import Serial
from time import sleep, time_ns
import argparse
import fcntl
import ctypes

# Define the struct
class CrosslinkIoctlSerial(ctypes.Structure):
    _fields_ = [
        ("len", ctypes.c_uint32),
        ("data", ctypes.c_uint8 * 64),
    ]

# # define UART status bits
# CROSSLINK_UART_STAT_BUSY_TX       = 0b0010_0000
# CROSSLINK_UART_STAT_BUSY_RX       = 0b0001_0000
# CROSSLINK_UART_STAT_DATA_FULLTX   = 0b0000_1000
# CROSSLINK_UART_STAT_DATA_EMPTYTX  = 0b0000_0100
# CROSSLINK_UART_STAT_DATA_FULLRX   = 0b0000_0010
# CROSSLINK_UART_STAT_DATA_EMPTYRX  = 0b0000_0001

# Define the IOCTL commands
CROSSLINK_CMD_SERIAL_SEND_TX    = 0x7601
CROSSLINK_CMD_SERIAL_RECV_RX    = 0x7602
CROSSLINK_CMD_SERIAL_RX_CNT     = 0x7603
CROSSLINK_CMD_SERIAL_BAUD       = 0x7604

# Open the device file
def send(dev_path, data: bytes):
    ioctl_serial = CrosslinkIoctlSerial()
    ioctl_serial.len = len(data)
    ioctl_serial.data = (ctypes.c_uint8 * 64)(*data)
    # Call an IOCTL
    with open(dev_path) as f:
        fcntl.ioctl(f, CROSSLINK_CMD_SERIAL_SEND_TX, ioctl_serial)

def recv(dev_path, count:int=0):
    ioctl_serial = CrosslinkIoctlSerial()
    ioctl_serial.len = count
    with open(dev_path) as f:
        fcntl.ioctl(f, CROSSLINK_CMD_SERIAL_RECV_RX, ioctl_serial)
    return bytes(ioctl_serial.data[:ioctl_serial.len])

def get_rx_count(dev_path):
    ioctl_serial = CrosslinkIoctlSerial()
    with open(dev_path) as f:
        fcntl.ioctl(f, CROSSLINK_CMD_SERIAL_RX_CNT, ioctl_serial)
    return ioctl_serial.len

def get_baud(dev_path):
    ioctl_serial = CrosslinkIoctlSerial()
    with open(dev_path) as f:
        fcntl.ioctl(f, CROSSLINK_CMD_SERIAL_BAUD, ioctl_serial)
    return ioctl_serial.len

def set_baud(dev_path, baud:int):
    ioctl_serial = CrosslinkIoctlSerial()
    ioctl_serial.len = baud
    with open(dev_path) as f:
        fcntl.ioctl(f, CROSSLINK_CMD_SERIAL_BAUD, ioctl_serial)

def wait_for_rx_stable(dev_path, start_wait_ms, stop_wait_ms):
    start = time_ns()
    while get_rx_count(dev_path) == 0:
        sleep(0.002)
        if time_ns() - start > start_wait_ms*1e6:
            return False
    byte_count = get_rx_count(dev_path)
    start = time_ns()
    while time_ns() - start < stop_wait_ms*1e6:
        sleep(0.002)
        cnt = get_rx_count(dev_path)
        if cnt != byte_count:
            byte_count = cnt
            start = time_ns()
    return True

def main():
    # get argparse for dev, baudrate, data
    parser = argparse.ArgumentParser(prog='Crosslink_Visca')
    parser.add_argument('-d', '--dev', type=str, default="/dev/v4l-subdev2", help='device path')
    parser.add_argument('-t', '--timeout', type=int, default=100, help='read timeout to wait for RX data')
    parser.add_argument('data', type=str, help='data to send, as hex string')
    args = parser.parse_args()
    data = bytearray.fromhex(args.data)
    recv(args.dev)        # clear RX fifo
    send(args.dev, data)
    wait_for_rx_stable(args.dev, args.timeout, args.timeout/4)
    count = get_rx_count(args.dev)
    data = recv(args.dev)
    print(data.hex())

if __name__ == "__main__":
    main()
