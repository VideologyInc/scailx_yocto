DESCRIPTION = "Package group for Qt5"

inherit packagegroup

# Install fonts
QT5_FONTS = "ttf-dejavu-common ttf-dejavu-sans ttf-dejavu-sans-mono ttf-dejavu-serif"

# Install qtquick3d
QT5_QTQUICK3D = "qtquick3d"

QT5_IMAGE_INSTALL = " \
    ${QT5_FONTS} \
    ${QT5_QTQUICK3D} \
    ${@bb.utils.contains('DISTRO_FEATURES', 'x11', 'libxkbcommon', '', d)} \
    ${@bb.utils.contains('DISTRO_FEATURES', 'wayland', 'qtwayland qtwayland-plugins', '', d)} \
    qtbase \
    qtmultimedia \
    qtbase-plugins \
    qtquickcontrols2 \
"

IMAGE_INSTALL += " \
    ${QT5_FONTS} \ 
"

#    gstreamer1.0-plugins-good-qt 

RDEPENDS_${PN} += " \
    ${QT5_IMAGE_INSTALL} \
"
