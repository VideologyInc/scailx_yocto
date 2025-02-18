#!/usr/bin/env python3
#
# Example usage:
# gst-launch-1.0 videotestsrc ! scailxwebsink name=test

import gi
import os, sys
from termcolor import colored, cprint
import requests, socket
from urllib.parse import quote
import uuid
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib, GObject

Gst.init(None)

HOST = 'localhost'
PORT = 1984

def create_link(socketpath):
    # Get the full file path
    name = os.path.basename(socketpath)

    # Construct the API URL with query parameter
    params = {
        "name": quote(name),
        "src": f"exec:gst-launch-1.0 -q unixfdsrc socket-path={socketpath} ! imxvideoconvert_g2d ! vpuenc_h264 qp-max=30 qp-min=20 ! fdsink"
    }
    url = f"http://{HOST}:{PORT}/api/streams"
    response = requests.put(url, params=params)

    if response.status_code == 200:
        hostname = socket.gethostname()
        ip_address = socket.gethostbyname(hostname)
        print("video up at: " + colored(f"http://{hostname}.local:{PORT}/stream.html?src={quote(name)}", 'green', attrs=['underline']) + " or " + colored(f"http://{ip_address}:{PORT}/stream.html?src={quote(name)}", 'green', attrs=['underline']))
    else:
        print(colored(f"Failed to add stream {name}. Status code: {response.status_code}", 'red'))

def del_link(socketpath):
    name = os.path.basename(socketpath)

    # Construct the API URL with query parameter
    params = { "src": quote(name) }
    url = f"http://{HOST}:{PORT}/api/streams"
    response = requests.delete(url, params=params)

    if response.status_code != 200:
        print(f"Failed to delete stream {name} from webrtc server. {response.status_code}")

class ScailxWebSink(Gst.Bin):
    '''
    A GStreamer bin that wraps unixfdsink element with automatic socket path
    configuration in /tmp directory
    '''
    GST_PLUGIN_NAME = "scailxwebsink"
    __gstmetadata__ = ("Scailx Web Sink",
                      "Video/Sink",
                      "Unix domain socket file descriptor sink element for web streaming",
                      "Kobus")

    FORMATS = "{RGB16,RGBx,RGBA,BGRA,BGRx,BGR16,ARGB,ABGR,xRGB,xBGR,I420,NV12,UYVY,YUY2,YVYU,YV12,NV16,NV21}"
    __gsttemplates__ = Gst.PadTemplate.new("sink",
                                           Gst.PadDirection.SINK,
                                           Gst.PadPresence.ALWAYS,
                                           Gst.Caps.from_string(f"video/x-raw,format={FORMATS}"))

    __protocols__ = ("scailxweb",)
    __uritype__ = Gst.URIType.SINK

    socket_dir = GObject.Property(type=str, blurb='path to folder where unixfd sockets will be created', default="/tmp/scailxwebsink/")

    def __init__(self):
        super(ScailxWebSink, self).__init__()

        # Create the unixfdsink element
        self._unixfdsink = Gst.ElementFactory.make("unixfdsink", "sink")
        if not self._unixfdsink:
            raise RuntimeError("Could not create unixfdsink element")

        # Get the sink pad template from unixfdsink
        unixfdsink_factory = Gst.ElementFactory.find("unixfdsink")
        if not unixfdsink_factory:
            raise RuntimeError("Could not find unixfdsink element factory")

        # TODO: self.name is empty during __init__. Figure out how to add unixfd entity only after pad req or something
        # Generate socket path based on element name or random UUID
        if not os.path.exists(self.socket_dir):
            os.makedirs(self.socket_dir)
        socket_path = os.path.join(self.socket_dir, uuid.uuid4().hex[:6])

        # Set the properties
        self._unixfdsink.set_property("socket-path", socket_path)
        self._unixfdsink.set_property("wait-for-connection", True)

        # Add the element to our bin
        self.add(self._unixfdsink)

        # Create ghost pad using the template
        self._sinkpad = Gst.GhostPad.new_from_template(
            "sink",
            self._unixfdsink.get_static_pad("sink"),
            self.__gsttemplates__
        )

        # Add the pad
        self.add_pad(self._sinkpad)

    def __del__(self):
        pass

    def do_get_uri(self, uri):
        return "scailxweb://"

    def do_set_uri(self, uri):
        return True

    def do_change_state(self, transition: Gst.StateChange):
        if transition == Gst.StateChange.READY_TO_PAUSED:
            try:
                self.start()
            except Exception as e:
                print(f" scailxwebsink failed to start: {e}")
                self.stop()
                return Gst.StateChange.FAILURE
        elif transition == Gst.StateChange.PAUSED_TO_READY:
            self.stop()
        return Gst.Bin.do_change_state(self, transition)

    def start(self):
        create_link(self._unixfdsink.get_property("socket-path"))

    def stop(self):
        del_link(self._unixfdsink.get_property("socket-path"))

GObject.type_register(ScailxWebSink)
__gstelementfactory__ = (ScailxWebSink.GST_PLUGIN_NAME, Gst.Rank.PRIMARY + 1, ScailxWebSink)