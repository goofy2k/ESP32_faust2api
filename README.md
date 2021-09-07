# ESP32 with Faust
 Exploring Digital Sound Processing with Faust on a TTGO TAudio board


## Objective

[Faust]() is a system for Digital Sound Processing (creating digitally synthesized audio). 
 
It takes a sound engine (.dsp) file generated in an GUI or with a text editor and generates C++ code that can be included in a user's project for creation of firmware for sound genrating application.

![workflow_partial](/images/workflow_general_cropped.png)    

 Recently Faust has been used for an ESP32 based board: [Lilygo TTGO TAudio](). See this [article](/documents/smc20_faust_esp32.pdf) and this [tutorial.](https://faustdoc.grame.fr/tutorials/esp32/)

 More:  
 - [this](https://ccrma.stanford.edu/~rmichon/faustMicro/)
 - [Michon site to be found]()
 - https://github.com/grame-cncm/faust/tree/master-dev/architecture/esp32

 The tutorial contains walktrough examples using either the ESP-IDF environment (cli) or the Arduino IDE.
 
 The **objective of this project** is to learn explore the workflows for creating applications for the ESP32 TTGO TAudio board. The ultimate goal is to:  
 1. contribute to the Faust project in making the faust2api script suitable for ESP32 applications 
 2. create polyphonic sound engines

## Project flow

The faust2api script is a bash script and **requires a Faust environment under Linux**. Setting up this environment is considered treated as a separate project, see the [linux4faust repo](https://github.com/goofy2k/linux4faust).

For maximum flexibility in firmware generation (using the external RAM for large size firmware) the ESP-IDF environment is preferred over the Arduino IDE. Current potential solutions in my linux4faust project do not allow usage of serial ports. So until now firmware flashing **requires ESP-IDF under Windows**.
 
For validation and testing of the Linux environment for Faust preferably in combination with the ESP-IDF environment , a proven set with sound engine (.dsp) and ESP32 user app, workflow must be available. 

### Milestones
1. Faust IDE under Windows: step through the walkthrough in the Faust for ESP32 tutorial  activity OBSOLETE, we can use Faust scripts now  
  a. ESP-IDF  
  b. external RAM with ESP-IDF  
  c. Arduino IDE  
  d. check if external RAM with Arduino IDE is possible
2. faust2esp32 script under Linux (use master-dev branch with ESP32 entry in faust2api, also requires success in linux4faust)
  a. Arduino IDE  
  b. ESP-IDF  
  c. external RAM with ESP-IDF  
  d. ONLY if necessary use Arduino IDE for testing  
3. faust2api script under Linux (use master-dev branch with ESP32 entry in faust2api, also requires success in linux4faust)  
  a. ESP-IDF  
  b. external RAM with ESP-IDF  
  c. ONLY if necessary use Arduino IDE for testing  
4. define the roadmap to polyphony on ESP32 

As we can run Linux scripts now, milestone 1 does not add anything. The Faust IDE is useful for checking audio output of .dsp files. 

### TODO

|  #  | milestone | desrciption                                                             | depends on | status |
|-----|-----------|-------------------------------------------------------------------------|------------|--------|
|  1  |     *     | design and report command line structure for smooth dev workflow        |            |started |
|  2  |     *     | communicate first impression, questions remarks (see issues)            |            |started |
|  3  |     2a    | create main.cpp for Arduino IDE (FaustSawtooth, faust2esp32)            |            |finished|
|  4  |     2b    | create main.cpp for ESP-IDF     (FaustSawtooth, faust2esp32)            |     3      |finished|
|  5  |     2c    | create main.cpp for ESP-IDF     (large dsp/external RAM, faust2esp32)   |     4      |   w    |
|  6  |     3a    | study and evaluate API documentation (faust2api)                        |            |   w    |
|  7  |     3b    | create main.cpp for using API        (faust2api)                        |    5,6     |started |   
|  8  |     3b    | run sound generation tests with API   (faust2api)                       |     7      |   w    |   

Activities 6,7,8 are the core and will run in improvement cycles
User experience vs architecture

**Milestone 2a/b**
Follow the tutorial workflow, see:  
- https://faustdoc.grame.fr/tutorials/esp32/
also see:
- main_starter.jpg  
- https://github.com/grame-cncm/faust/blob/master-dev/architecture/esp32/esp32.cpp#L159  replace lines .... and .... 
- look at https://github.com/grame-cncm/faust/tree/master-dev/architecture/esp32 
 
 ## Tutorial workflow
 
 to be done: show final folder structure first
 
  #### Sound engine files
 - Create a .dsp file, put it in the root of the sound_engine folder (so in the parent folder of hello_world
 - Compile the .dsp file with the selected Faust script (faust2esp32 in this example)
   - Open a Powershell window in Windows Terminal
   - Move to the folder containing the .dsp file
   - PS wsl ~/faust/tools/faust2appls/faust2esp32 -h > faustesp32_help.txt    to generate the help for the selected script
   - PS wsl ~/faust/tools/faust2appls/faust2esp32 -lib FaustSawtooth.dsp
   - Copy the 4 files (a .cpp and .h for the sound-engine lib and for the codec driver WM8978) lib to:
     - the main folder of the C++ code
     - the folder containing the Arduino .ino file  
 
 #### C++ code
 - copy the ESP-IDF hello_world example integrally to the sound_engine folder, just keep it's name unchanged for now
 - follow the instructions for adaptation of main and CMake files
 - Make sure that the code still compiles
   - Open an ESP-IDF 4.2 Powershell window    (can this be done in WIndows Terminal?)
   - move to the folder containing the folder "main" (so to the parent of folder main) 
   - make sure that the build folder is absent in the parent folder of the main folder
   - PS  idf.py set-target esp32
   - PS idf.py menuconfig (exit without changes for now)
   - PS idf.py build
   - PS idf.py -p COM10 flash
   - PS idf.py -p COM10 monitor 
   

 **REMARK:** create a .gitignore file to prevent that the build folder is uploaded to Github  
 
 - Complete the main.cpp app with the required functionality (as shown in the example):  
   - Include the header files in the top of the file
   - Configure the audio codec
   - Instantiate the Faust DSP Engine
   - Start the Faust DSP Engine
   - Change the sound frequency (dynamically) with the setParamValue function
 - Compile the app again (see above)  
   - FAIL: undefined references to libs.  Change CMakeLists.txt in the ... folder to contain 
     - **idf_component_register(SRCS "main.cpp" "FaustSawtooth.cpp" "WM8978.cpp"**  or
     - **idf_component_register(SRCS "main.cpp" "DspFaust.cpp" "WM8978.cpp"**
   - FAIL:    
     - use **idf.py --no-ccache build**
   - SUCCESS: with warnings about deprecated items (create an issue), but first flash
   - FAIL: started to build again (why?, because of previous no-ccache?)
     - start over again and use no-cache also in the flash command: PS idf.py --no-ccache -p COM10 flash
   - SUCCES ! 
   - Make fimware recognizable to detect if indeed new firmware was flashed: with a version nr printed to the monitor, or better, a sound fingerprint 
     - changed duration of the tones   
    - SUCCES !!!!! new firmware runs OK    
 - Debug until success  **NOT YET** 
 
 #### Arduino code 
 - Full Arduino folder (.ino and libs in one folder works)
 - This implies double storage (and maintenance!!) for the libs
 - find a way to include the libraries with in a different folder than the .ino file. Use relative path in the include statement.

**Milestone 7**  
FaustSawtooth and faust2api  
doesn't compile 
may have something to do with long path lengths, see: https://www.esp32.com/viewtopic.php?t=14651  
and informative link therein: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html#idf-py-options
possible workaround: One way to workaround it is to set env variable ..... IDF_CCACHE_ENABLED='' .... before launching idf.py.
SET  
$Env:<variable-name> = "<new-value>"  
GET  
 
NO SOLUTION !!!
 
 READ THIS:  https://www.cxyzjd.com/article/zhangjingxun12/117095349
 
 has something to do with paths > 90
 
 idf.py --no-ccache build
 
 Now the error becomes:  undefined reference to Libname::function name
 
 In: https://stackoverflow.com/questions/1517138/trying-to-include-a-library-but-keep-getting-undefined-reference-to-messages
 
 The trick here is to put the library AFTER the module you are compiling. The problem is a reference thing. The linker resolves references in order, so when the library is BEFORE the module being compiled, the linker gets confused and does not think that any of the functions in the library are needed. By putting the library AFTER the module, the references to the library in the module are resolved by the linker.

 BUT HOW AND WHERE?
 
 https://stackoverflow.com/questions/67039814/linker-error-in-esp-idf-framework-undefined-reference
 UPDATE THE CMAKE fil
 
 SO ESP-IDF: set env variable CCACH + updated CMAKELists and use nocach in idf.py -nocahce build
 
 Arduino:  'dynamic_cast' not permitted with -fno-rtti  TRY TO DETECT WHERE THE ERROR OCCURS BY COMMENTING OUT CALLS TO THE LIB
   
#### THERE IS A PROBLEM IN ARDUINO WITH RELATIVE PATHS FOR LIB INCLUDES
#### PUT THE LIBS IN THE SAME FOLDER AS THE .INO FILE AND INCLUDE BETWEEN "   "
#### TRY TO USE RELATIVE INCLUDES IN THE C++ CODE 
#### THIS PREVENTS THAT YOU HAVE TO MAINTAIN TWO VERSIONS FOR EACH LIB SYNCED
#### DRAW THE FOLDER STRUCTURE BEFOR YOU IMPLEMENT A SOLUTION


## Further tooling

**logging of script output for easier of line study of results

**script command lines**  
create batch files?  
In target folder:  
wsl ~/faust/tools/faust2appls/faust2esp32 -h > faust2esp32_help.txt
 
PS C:\Users\Fred\esp_projects\ESP32-with-Faust\sound_engines\FaustSawtooth\cpp\faust2esp32> wsl ~/faust/tools/faust2appls/faust2esp32 ../../FaustSawtooth.dsp  
PS C:\Users\Fred\esp_projects\ESP32-with-Faust\sound_engines\FaustSawtooth\cpp\faust2esp32> wsl ~/faust/tools/faust2appls/faust2esp32 - > faustesp32_help.txt

 ** folder structure generator  https://ascii-tree-generator.com/
 
 
 ![folder_structure](images/folder%20structure.jpg)  
 
 sound_engines/                       a sound engine's configuration is defined by: Faust script, .dsp file   
├─ faust2api/                         each sound engine is implemented in a cpp_code and an arduino_sketch  
│  ├─ DSPTemplate/  
│  ├─ FaustSawtooth/   
│  │  ├─ FaustSawtooth.dsp  
│  │  ├─ arduino_sketch/  
│  │  │  ├─ arduino_sketch.ino  
│  │  │  ├─ DspFaust.cpp             lib files have to be stored in 2 places (Arduino does not accept relative paths)  
│  │  │  ├─ DspFaust.h               
│  │  │  ├─ WM8978.cpp  
│  │  │  ├─ WM8978.h  
│  │  ├─ cpp_code/  
│  │  │  ├─ main/  
│  │  │  │  ├─ main.cpp  
│  │  │  │  ├─ CMake_stuff  
│  │  │  │  ├─ DspFaust.cpp         must implement relative path to arduino_sketch folder for the libs to prevent double maintenance (risky version control)   
│  │  │  │  ├─ DspFaust.h  
│  │  │  │  ├─ WM8978.cpp  
│  │  │  │  ├─ WM8978.h  
│  │  │  ├─ CMake_stuff    
├─ faust2esp32/  
│  ├─ DSPTemplate/  
│  ├─ FaustSawtooth/  
│  ├─ another_dsp/  
│  │  ├─ another_dsp.dsp  

 
 
 
 
 
 

## MELODIES IMPLEMENT IN ARDUINO SAWTOOTH
 
https://www.browncountylibrary.org/wp-content/uploads/2018/03/arduino_piezo.pdf
 e.g.  
 int numTones = 10; // the number of tones in the scale  
int tones[] = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440}; // the frequency for each tone  
// mid C C# D D# E F F# G G# A  


