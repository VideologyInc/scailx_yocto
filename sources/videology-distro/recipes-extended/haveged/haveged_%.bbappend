FILES_${PN}_append = " /run/lock/subsys"

do_install_append() {
        install -d -m 777 ${D}/run/lock/subsys
} 
