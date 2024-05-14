DESCRIPTION = "CasaOS-UserService"
SECTION = "casa-os"
HOMEPAGE = "https://https://casaos.io/"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRC_URI = "git://${GO_IMPORT};branch=main;protocol=https"
SRCREV = "aaaaa"
UPSTREAM_CHECK_COMMITS = "1"

GO_IMPORT = "github.com/IceWhaleTech/CasaOS-UserService"

# CGO_ENABLED = "0"

inherit go-mod

