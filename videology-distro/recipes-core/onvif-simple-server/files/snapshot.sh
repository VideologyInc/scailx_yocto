#! /usr/bin/env bash

# Don't edit below this line

CONF_PATH=/usr/local/etc/onvif/onvif_server.conf

URL=$(grep "^url=" $CONF_PATH | sed 's/.*=//' | sed "s/%s/127.0.0.1/")

ffmpeg -y -i $URL -vframes 1 snapshot.jpeg > /dev/null 2>&1

CONTENT_LENGTH=$(stat -c '%s' snapshot.jpeg)

echo "Content-Length: $CONTENT_LENGTH"
echo "Content-Type: image/jpg"
echo ""
cat snapshot.jpeg

exit 0