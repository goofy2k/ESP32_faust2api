## DSP engines

This folder contains C++ engines obtained by compilation of a corresponding Faust .dsp file with the faust2api script with the options -ESP32 -nvoices 12 enginename.dsp

A line with `throw std::bad_alloc()` has been commented out of the code.

Copy the DspFaust files into the `main` folder of the project. Before building the firmware, remove old build results with `idf.py fullclean`.

