#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/select.h>
#include <getopt.h>
#include <sys/ioctl.h>


#define DEFAULT_TIMEOUT 200 // Default timeout in milliseconds

static int serial_baudrate_to_bits(int baudrate) {
    switch (baudrate) {
        case 1200: return B1200;
        case 1800: return B1800;
        case 2400: return B2400;
        case 4800: return B4800;
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        case 460800: return B460800;
        case 500000: return B500000;
        case 576000: return B576000;
        case 921600: return B921600;
        case 1000000: return B1000000;
        default: return -1;
    }
}

// Function to configure the serial port
int setup_serial(int fd, int speed) {
    struct termios tty;

    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        return -1;
    }

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    tty.c_cflag &= ~PARENB; // No parity
    tty.c_cflag &= ~CSTOPB; // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8; // 8 data bits
    tty.c_cflag &= ~CRTSCTS; // No hardware flow control
    tty.c_cflag |= CREAD | CLOCAL; // Enable receiver, ignore modem control lines

    tty.c_lflag &= ~ICANON; // Non-canonical mode
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off software flow control
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    tty.c_cc[VTIME] = 0; // No blocking read
    tty.c_cc[VMIN] = 0; // Read doesn't block

    // Save tty settings, also checking for error
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return -1;
    }

    return 0;
}

// Function to send data
int send_data(int fd, char *hex_string) {
    // convert to uppercase
    for (char *p = hex_string; *p; ++p) *p = toupper(*p);
    // Append 0xFF if not present
    if (hex_string[strlen(hex_string) - 2] != 'F' || hex_string[strlen(hex_string) - 1] != 'F') {
        strcat(hex_string, "FF");
    }

    int len = strlen(hex_string) / 2;
    unsigned char *data = (unsigned char *)malloc(len);

    // Convert hex string to binary data
    for (int i = 0; i < len; ++i) {
        sscanf(hex_string + 2*i, "%2hhx", &data[i]);
    }

    // Write data to serial port
    int ret = write(fd, data, len);
    free(data);
    return ret;
}

// Function to get the number of characters in the RX buffer
int get_rx_buffer_count(int fd) {
    int count = 0;
    if (ioctl(fd, TIOCINQ, &count) == -1) {
        perror("ioctl TIOCINQ");
        return -1; // Return -1 in case of error
    }
    return count;
}

// Function to flush the RX buffer
void flush_rx_buffer(int fd) {
    if (tcflush(fd, TCIFLUSH) == -1) {
        perror("tcflush");
    }
}

// Function to receive data
int recv_data(int fd, int timeout_ms, int wait_for_bytes) {
    fd_set read_fds;
    struct timeval timeout;
    int retval;
    unsigned char buf[256];
    unsigned char *p = buf;
    int n;

    // Set up the timeout structure
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    // Loop to read data
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);

        // Wait for data to be available for reading
        retval = select(fd + 1, &read_fds, NULL, NULL, &timeout);

        if (retval == -1) {
            perror("select()");
            break;
        } else if (retval) {
            // Data is available to read
            n = read(fd, p, sizeof(buf) - (p - buf));
            unsigned char end = p[n - 1];
            p += n;
            if (wait_for_bytes && (p - buf) >= wait_for_bytes)
                break;
            else if (end == 0xFF) // Check for end of packet (0xFF)
                break;
        } else {
            fprintf(stderr, "No data within timeout period.\n");
            break;
        }
    }
    for (int i = 0; i < (p-buf); i++)
        printf("%02X", buf[i]);
    return p-buf;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s baud device hex-data [timeout] [wait for n bytes]\n", argv[0]);
        fprintf(stderr, "writes hex-data to serial dev, waits for a response and writes hex to stdio.\n" );
        fprintf(stderr, "Designed for strings ending in 0xFF.\n" );
        fprintf(stderr, "Example: %s 9600 /dev/ttyUSB0 100\n", argv[0]);
        return 1;
    }

    int fd = open(argv[2], O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror(argv[2]);
        return 1;
    }
    flush_rx_buffer(fd);
    // get baud from 2nd argument
    int baud = atoi(argv[1]);
    if (setup_serial(fd, serial_baudrate_to_bits(baud)) != 0) {
        close(fd);
        return 1;
    }

    int ret = 1;
    int timeout = (argc >= 5) ? atoi(argv[4]) : DEFAULT_TIMEOUT;
    int wait_for_bytes = (argc >= 6) ? atoi(argv[5]) : 0;
    if ( send_data(fd, argv[3]) > 0 ) {
        //printf("send");
        tcdrain(fd);                // Wait until all data is sent
        //printf("drain");
        int r=recv_data(fd, timeout, wait_for_bytes);
        if ((r) > 0)  // Now receive the response
            ret = 0;
        //printf("recv %d", r);
    }
    //printf("close");
    close(fd);
    return ret;
}

