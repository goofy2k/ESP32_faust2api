# Basic ESP32 faust2api example

We now have a working basic example app (faust_mqtt_tcp4_v3_KEEP). This document contains a walkthrough on how to:  

  1. use this example
  2. create a project based on this example
 
Any changes implemented during write up of this consolidation are done in faust_mqtt_tcp5_v1

The example contains a main C++ app. It uses the the output of the faust2api script. The WM8978 audio driver files are used unmodified. The DspFaust.cpp file has been modified to get things working on the TTGO TAudio board. These modifications may have to be implemented in the faust2api script for ESP32.

General description of the features in the example...

  

