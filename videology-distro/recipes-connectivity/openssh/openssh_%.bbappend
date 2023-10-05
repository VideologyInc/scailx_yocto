do_install:append () {
	# to allow vscode-remote
	sed -i -e '/AllowTcpForwarding/c\AllowTcpForwarding yes' ${D}${sysconfdir}/ssh/sshd_config
	sed -i -e '/AllowAgentForwarding/c\AllowAgentForwarding yes' ${D}${sysconfdir}/ssh/sshd_config
}
