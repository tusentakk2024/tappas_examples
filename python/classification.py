#!/usr/bin/env python3
import sys
import gi
import logging

gi.require_version("GLib", "2.0")
gi.require_version("GObject", "2.0")
gi.require_version("Gst", "1.0")

from gi.repository import Gst, GLib, GObject

def tutorial_main():
    Gst.init(sys.argv[1:])

    pipeline = Gst.parse_launch(" \
        filesrc location=./resources/example.mp4 name=source ! \
        qtdemux ! \
        h264parse ! \
        avdec_h264 max-threads=2 ! \
        videoscale name=source_videoscale n-threads=2 ! \
        videoconvert n-threads=3 name=source_convert qos=false ! \
        video/x-raw, format=RGB, pixel-aspect-ratio=1/1 ! \
        videoscale name=inference_videoscale n-threads=2 qos=false ! \
        video/x-raw, pixel-aspect-ratio=1/1 ! \
        videoconvert name=inference_videoconvert n-threads=2 ! \
        hailonet hef-path=./resources/resnet_v1_50.hef ! \
        hailofilter name=pose-estimation so-path=./resources/libclassification_post.so qos=false ! \
        hailooverlay ! \
        videoconvert ! \
        fpsdisplaysink name=hailo_display video-sink=xvimagesink sync=false text-overlay=false \
    ")
    ret = pipeline.set_state(Gst.State.PLAYING)
    if ret == Gst.StateChangeReturn.FAILURE:
        print("Unable to set the pipeline to the playing state.")
        sys.exit(1)

    # Wait for EOS or error
    bus = pipeline.get_bus()
    msg = bus.timed_pop_filtered(Gst.CLOCK_TIME_NONE, Gst.MessageType.ERROR | Gst.MessageType.EOS)

    # Parse message
    if msg:
        if msg.type == Gst.MessageType.ERROR:
            err, debug_info = msg.parse_error()
            print("Error received from element {:s}: {:s}".format(
                msg.src.get_name(),
                err.message))
            print(f"Debugging information: {debug_info if debug_info else 'none'}")
        elif msg.type == Gst.MessageType.EOS:
            print("End-Of-Stream reached.")
        else:
            # This should not happen as we only asked for ERRORs and EOS
            print("Unexpected message received.")

    pipeline.set_state(Gst.State.NULL)

if __name__ == "__main__":
    tutorial_main()
