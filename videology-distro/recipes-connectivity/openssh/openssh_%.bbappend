do_install:append () {
	# to allow vscode-remote
	sed -i -e '/AllowTcpForwarding/c\AllowTcpForwarding yes' ${D}${sysconfdir}/ssh/sshd_config
	sed -i -e '/AllowAgentForwarding/c\AllowAgentForwarding yes' ${D}${sysconfdir}/ssh/sshd_config

	# disable DNS lookups
	sed -i -e '/UseDNS/c\UseDNS no' ${D}${sysconfdir}/ssh/sshd_config

	# use scailx ssh key-select script
	sed -i -e '/AuthorizedKeysCommand /c\AuthorizedKeysCommand /etc/ssh/scailx-keys.sh' ${D}${sysconfdir}/ssh/sshd_config
	sed -i -e '/AuthorizedKeysCommandUser /c\AuthorizedKeysCommandUser root' ${D}${sysconfdir}/ssh/sshd_config

	# allow more auth tries
	sed -i -e '/MaxAuthTries/c\MaxAuthTries 20' ${D}${sysconfdir}/ssh/sshd_config
}

RDEPEND:${PN} += "scailx-ssh-keys"