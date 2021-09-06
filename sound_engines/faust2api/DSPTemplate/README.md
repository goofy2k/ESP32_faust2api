# Configuration

**This folder contains sound engines compiled using a Faust script (CLI):**  
- Platform: 
- Architecture:  

By default the faust2api script is used.  
If the folder contains a file named faust2api_* that file will be used for compilation

This folder contains the following elements:
### Faust input
- template.dsp
- faust2esp32_V0.1 option: Faust script for conversion of template.dsp to cpp source code. If not present, the script with the name of the parent (parent-parent?) folder will be used 
### Faust output   
- template_cpp   folder with Faust output, the .cpp/.h files for this configuration (faust script + template.dsp file)
### Sources for ESP32 firmware
- main.cpp       ESP-IDF main app 
- template.ino   option: Arduino IDE sketch  

**NOTES:** for proper compilation of the controler firmware, the files in the template_cpp folder must be in the same folder as main.cpp
This will be adapted when OR 1) scripting is in place 2) CMake files are adapted.
Draw folder structure here, or somewhere in the upper level of the tree
