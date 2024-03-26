#! /usr/bin/env python3

import time
import os, psutil
from daemon import DaemonContext
import sd_notify

led_path = "/sys/devices/platform/scailx_leds/leds/scailx_status_led/brightness"

class LEDDaemon:
    def __init__(self, led_path, interval=0.8):
        self.led_path = led_path
        self.notify = sd_notify.Notifier()
        self.interval =	interval

    def toggle_led(self):
        with open(self.led_path, 'r') as led_file:
            state = led_file.read().strip()
        new_state = '0' if state == '1' else '1'
        with open(self.led_path, 'w') as led_file:
            led_file.write(new_state)

    def all_services_running(self):
        services = {}
        services["swupdate"] = "swupdate" not in (n.name() for n in psutil.process_iter())
        ret = all(services.values())    
        if not ret:
            print("Services not running: ", services)
        return ret
            
    def run(self):
        counter = 0
        while True:
            self.toggle_led()
            time.sleep(self.interval)
            # check services every fourth loop
            if counter >= 3 == 0:
                counter = 0
                if self.all_services_running():
                    if psutil.boot_time() > 35:         # allow a few seconds for the services to start.
                        break;  
                    else:
                        print("Services not started. Waiting a few seconds.")
            # emit watchdog notification
            if self.notify.enabled():   
                self.notify.notify() 

if __name__ == "__main__":
    # get watchdog environment variables
    watchdog_interval = os.getenv("WATCHDOG_USEC")
    watchdog_interval = int(watchdog_interval) / 1000000 if watchdog_interval else 1.6
    led_daemon = LEDDaemon(led_path, watchdog_interval/2)
    # with DaemonContext():
    led_daemon.run()
