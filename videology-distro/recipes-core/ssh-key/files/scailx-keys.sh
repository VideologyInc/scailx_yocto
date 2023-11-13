#!/bin/bash

if [ -s '/etc/ssh/authorized_keys' ]; then
    cat '/etc/ssh/authorized_keys'
else
    cat '/etc/ssh/authorized_keys_scailx_dev'
fi