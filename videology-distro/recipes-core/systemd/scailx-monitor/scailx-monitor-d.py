#! /usr/bin/env python3

import time
import os, psutil
import socket
import logging

logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

led_path = "/sys/devices/platform/scailx_leds/leds/scailx_status_led/brightness"

class LEDDaemon:
    def __init__(self, led_path, interval=1.6, debug=False, services='swupdate'):
        self.led_path = led_path
        self.debug = debug
        self.ready = False
        self.services = [s for s in services.split()]
        self.address = os.getenv("NOTIFY_SOCKET")
        try:
            self.socket = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
            self.address = os.getenv("NOTIFY_SOCKET")
            logger.info(f"got socket {self.socket} addr:{self.address}")
        except:
            logger.info("Failed to create socket")

        self.interval =	interval
        self.led_state = 0

    def send(self, message):
        if self.address and self.socket:
            self.socket.sendto(message.encode(), self.address)
        else:
            logger.info(f"Cant send to notify socket:{self.socket} addr:{self.address}")

    def toggle_led(self):
        with open(self.led_path, 'w') as led_file:
            self.led_state = 0 if self.led_state else 1;
            led_file.write("%d" % self.led_state)

    def all_services_running(self):
        processes = [ n.name() for n in psutil.process_iter() ]
        services = { s: s in processes for s in self.services }
        ret = all(services.values())
        if not ret:
            logger.info(f"Services not running: {services}")
        else:
            logger.debug(f"All services running: {services}")
        return ret

    def run(self):
        div=4
        self.send("READY=1\n")
        while True:
            if not self.all_services_running():
                if psutil.boot_time() < 35:         # allow a few seconds for the services to start.
                    logger.info("Services not started. Waiting a few seconds.")
                else:
                    exit(1)
            else:
                # emit watchdog notification
                logger.debug("emit notification")
                self.send("WATCHDOG=1\n")
                if self.ready == False:
                    self.ready = True
                    self.send("READY=1\n")

            for i in range (div):
                self.toggle_led()
                time.sleep(self.interval/div/2)

if __name__ == "__main__":
    # get watchdog environment variables
    logger.info("running scailx monitor deamon")
    watchdog_interval = os.getenv("WATCHDOG_USEC")
    watchdog_interval = int(watchdog_interval) / 1000000 if watchdog_interval else 1.6
    logger.info(f"using watchdog interval {watchdog_interval}")
    led_daemon = LEDDaemon(led_path, watchdog_interval)
    # with DaemonContext():
    led_daemon.run()
