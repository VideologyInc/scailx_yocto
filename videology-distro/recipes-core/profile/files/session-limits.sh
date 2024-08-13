#!/bin/bash

# Get the current session ID using PAM_SESSION_ID or fallback to loginctl
session_id=$(loginctl session-status | head -n 1 | awk '{print $1}')
cpu_quota=$(( $(nproc) * 75))
if [ -n "$session_id" ]; then
    # echo "Setting MemoryMax=70% CPUQuota=${cpu_quota}% for session-${session_id}.scope"
    systemctl set-property "session-${session_id}.scope" MemoryHigh=60% MemoryMax=70% MemoryAccounting=true CPUAccounting=true CPUQuota=${cpu_quota}%
else
    echo "Could not find session ID for user $PAM_USER shell"
fi
