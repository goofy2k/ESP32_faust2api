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
 - [Digital Larry's work](https://github.com/HolyCityAudio/ESP32/tree/master/faust)  

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

|  #  | milestone | desrciption                            |    DSP           | Faust script| depends on | status |  
|-----|-----------|----------------------------------------|------------------|-------------|------------|--------|  
|  1  |     *     | design command line scripts            |                  |             |            |started |  
|  2  |     2a    | Arduino sketch                         | FaustSawtooth.dsp| faust2esp32 |            |finished|  
|  3  |     2b    | ESP-IDF tutorial_app1                  | FaustSawtooth.dsp| faust2esp32 |     3      |finished|  
|  4  |     2c    | ESP-IDF tutorial_app2                  | osc.dsp/ext RAM  | faust2esp32 |     4      |   w    | 
|  4  |     2c    | ESP-IDF tutorial_app2                  | FaustSawtooth.dsp| faust2api   |            |build07 | 
|  5  |     3     | ESP-IDF [proposed app](https://github.com/grame-cncm/faust/blob/master-dev/architecture/esp32/esp32.cpp#L159)                | osc.dsp/ext RAM  | faust2esp32 |     4      |   w    |  
|  6  |     4a    | study and evaluate API documentation   |                  | faust2api   |            |   w    |  
|  8  |     4b    | ESP-IDF app with implemented API       | FaustSawtooth.dsp| faust2api   |            |   w    | 
|  8  |     5a    | ESP-IDF app with implemented API       | osc.dsp/ext RAM  | faust2api   |    5,6     |   w    |   
|  9  |     5b    | run sound generation tests with API    | osc.dsp/ext RAM  | faust2api   |     7      |   w    |   

Activities 6,7,8 are the core and will run in improvement cycles  
proposed app:  
- change lines 178  and 179 to use DspFaust object  
- use extern "C" void app_main()  to convert to cpp app  

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

 - Compile the .dsp file with the selected Faust script (faust2esp32 in this example)
From the home (~) in a Linux Window: 
~$ faust2api -esp32 /mnt/c/Users/Fred/esp_projects/ESP32_faust2api/sound_engines/faust2api/FaustSawtooth/FaustSawtooth.dsp
Your dsp-faust.zip API package for ESP32 audio is being created
Package sndfile was not found in the pkg-config search path.
Perhaps you should add the directory containing `sndfile.pc'
to the PKG_CONFIG_PATH environment variable
No package 'sndfile' found

~$ faust2esp32 -lib /mnt/c/Users/Fred/esp_projects/ESP32_faust2api/sound_engines/faust2api/FaustSawtooth/FaustSawtooth.dsp
no further output on screen

 
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
   - CHECK REPAIR BY SLETZ: FILES ALREADY IMPORTED  but not deployed OK  see below  
 - Debug until success  **NOT YET** 
 
 #### Arduino code 
 - Full Arduino folder (.ino and libs in one folder works)
 - This implies double storage (and maintenance!!) for the libs
 - find a way to include the libraries with in a different folder than the .ino file. Use relative path in the include statement.  

### Tool learnings

#### ESP-IDF  

Must add .cpp lib files to CMakeLists.txt in main folder

ESP-IDF 4.2 Powershell
from project folder (parent folder of main folder)  
PS idf.py set-target -esp32  
PS idf.py menuconfig  
PS idf.py **--no-ccache** build  
PS idf.py **--no-ccache** -p COM10 flash  
PS idf.py **--no-ccache** -p COM10 monitor  

Use --no-cchache option to prevent build errors with long paths

#### Arduino IDE

- IDE cannot directly handle relative paths for includes, so  
lib files must be in same folder as sketch (.ino)
- IDE cannot handle minus sign in paths. Name all folders accordingly.  
- To make versioning easier, prevent double copies of libs in project tree.  
- Lett ESP-IDF include libs from Arduino sketch folder using relative path and environment variable
for path to root of project  


### Lab log  

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
 
 SO ESP-IDF: set env variable CCACH + updated CMAKELists and use nocach in idf.py -nocahce build  (IS THE ENV VAR REQUIRED?  )
 
 Arduino:  'dynamic_cast' not permitted with -fno-rtti  TRY TO DETECT WHERE THE ERROR OCCURS BY COMMENTING OUT CALLS TO THE LIB
 
 It is still there!
 
 sletz: needs to remove the compilation flag -fno-rtti and add -fexception, but I'm not sure you can do that
 
 the other error should be solved:  sletz: This error "error: 'cerr' is not a member of 'std'" should be fixed in this commit https://github.com/grame-cncm/faust/commit/6275eabbde7bc736c69bf44278bd343d27e90f94  
 I imported the active files.  Hope that everything compiles well CHECK CHECK
 
 CHeck this by comparing the uploaded buildlog.txt  with the newly generated buildlog2.txt
 
 The source must be on the windows side!
 
 It is now (?)  .... output in buildlog3.txt
 
 NOTE: sletz repaired the output file of the script file manually. Doe it mean that a script-generated file is also OK????
 
 THe cerr is still there! Do you use the correct source? No have rebuilt faust (Linux)
 
 ### Now get for Arduino:
 ---------------------  
 C:\Users\Fred\esp_projects\ESP32_faust2api\sound_engines\faust2api\FaustSawtooth\arduino\DspFaust.cpp: In member function 'void dsp_voice_group::buildUserInterface(UI*)':
DspFaust.cpp:10886:79: error: 'dynamic_cast' not permitted with -fno-rtti
             if (!fGroupControl || dynamic_cast<SoundUIInterface*>(ui_interface)) {
                                                                               ^
C:\Users\Fred\esp_projects\ESP32_faust2api\sound_engines\faust2api\FaustSawtooth\arduino\DspFaust.cpp: In member function 'virtual void mydsp_poly::buildUserInterface(UI*)':
DspFaust.cpp:11230:59: error: 'dynamic_cast' not permitted with -fno-rtti
             if (dynamic_cast<midi_interface*>(ui_interface)) {
                                                           ^
DspFaust.cpp:11231:74: error: 'dynamic_cast' not permitted with -fno-rtti
                 fMidiHandler = dynamic_cast<midi_interface*>(ui_interface);
                                                                          ^
exit status 1
'dynamic_cast' not permitted with -fno-rtti
---------------------  
 
### Now get for ESP-IDF (see buildlog4.txt):
 ---------------------  
 926/933] Building CXX object esp-idf/main/CMakeFiles/__idf_main.dir/main.cpp.obj
../main/main.cpp: In function 'void app_main()':
../main/main.cpp:28:9: warning: unused variable 'SR' [-Wunused-variable]
     int SR = 48000;
         ^~
../main/main.cpp:29:9: warning: unused variable 'BS' [-Wunused-variable]
     int BS = 8;
         ^~

[928/933] Building CXX object esp-idf/main/CMakeFiles/__idf_main.dir/DspFaust.cpp.obj
FAILED: esp-idf/main/CMakeFiles/__idf_main.dir/DspFaust.cpp.obj 
C:\Users\Fred\.espressif\tools\xtensa-esp32-elf\esp-2020r3-8.4.0\xtensa-esp32-elf\bin\xtensa-esp32-elf-g++.exe  -DHAVE_CONFIG_H -DMBEDTLS_CONFIG_FILE=\"mbedtls/esp_config.h\" -DUNITY_INCLUDE_CONFIG_H -DWITH_POSIX -Iconfig -IC:/Users/Fred/esp-idf/components/newlib/platform_include -IC:/Users/Fred/esp-idf/components/freertos/include -IC:/Users/Fred/esp-idf/components/freertos/xtensa/include -IC:/Users/Fred/esp-idf/components/heap/include -IC:/Users/Fred/esp-idf/components/log/include -IC:/Users/Fred/esp-idf/components/lwip/include/apps -IC:/Users/Fred/esp-idf/components/lwip/include/apps/sntp -IC:/Users/Fred/esp-idf/components/lwip/lwip/src/include -IC:/Users/Fred/esp-idf/components/lwip/port/esp32/include -IC:/Users/Fred/esp-idf/components/lwip/port/esp32/include/arch -IC:/Users/Fred/esp-idf/components/soc/src/esp32/. -IC:/Users/Fred/esp-idf/components/soc/src/esp32/include -IC:/Users/Fred/esp-idf/components/soc/include -IC:/Users/Fred/esp-idf/components/esp_rom/include -IC:/Users/Fred/esp-idf/components/esp_common/include -IC:/Users/Fred/esp-idf/components/esp_system/include -IC:/Users/Fred/esp-idf/components/xtensa/include -IC:/Users/Fred/esp-idf/components/xtensa/esp32/include -IC:/Users/Fred/esp-idf/components/esp32/include -IC:/Users/Fred/esp-idf/components/driver/include -IC:/Users/Fred/esp-idf/components/driver/esp32/include -IC:/Users/Fred/esp-idf/components/esp_ringbuf/include -IC:/Users/Fred/esp-idf/components/efuse/include -IC:/Users/Fred/esp-idf/components/efuse/esp32/include -IC:/Users/Fred/esp-idf/components/espcoredump/include -IC:/Users/Fred/esp-idf/components/esp_timer/include -IC:/Users/Fred/esp-idf/components/esp_ipc/include -IC:/Users/Fred/esp-idf/components/soc/soc/esp32/include -IC:/Users/Fred/esp-idf/components/soc/soc/esp32/../include -IC:/Users/Fred/esp-idf/components/soc/soc/esp32/private_include -IC:/Users/Fred/esp-idf/components/vfs/include -IC:/Users/Fred/esp-idf/components/esp_wifi/include -IC:/Users/Fred/esp-idf/components/esp_wifi/esp32/include -IC:/Users/Fred/esp-idf/components/esp_event/include -IC:/Users/Fred/esp-idf/components/esp_netif/include -IC:/Users/Fred/esp-idf/components/esp_eth/include -IC:/Users/Fred/esp-idf/components/tcpip_adapter/include -IC:/Users/Fred/esp-idf/components/app_trace/include -IC:/Users/Fred/esp-idf/components/mbedtls/port/include -IC:/Users/Fred/esp-idf/components/mbedtls/mbedtls/include -IC:/Users/Fred/esp-idf/components/mbedtls/esp_crt_bundle/include -IC:/Users/Fred/esp-idf/components/bootloader_support/include -IC:/Users/Fred/esp-idf/components/app_update/include -IC:/Users/Fred/esp-idf/components/spi_flash/include -IC:/Users/Fred/esp-idf/components/wpa_supplicant/include -IC:/Users/Fred/esp-idf/components/wpa_supplicant/port/include -IC:/Users/Fred/esp-idf/components/wpa_supplicant/include/esp_supplicant -IC:/Users/Fred/esp-idf/components/nvs_flash/include -IC:/Users/Fred/esp-idf/components/pthread/include -IC:/Users/Fred/esp-idf/components/perfmon/include -IC:/Users/Fred/esp-idf/components/asio/asio/asio/include -IC:/Users/Fred/esp-idf/components/asio/port/include -IC:/Users/Fred/esp-idf/components/cbor/port/include -IC:/Users/Fred/esp-idf/components/coap/port/include -IC:/Users/Fred/esp-idf/components/coap/port/include/coap -IC:/Users/Fred/esp-idf/components/coap/libcoap/include -IC:/Users/Fred/esp-idf/components/coap/libcoap/include/coap2 -IC:/Users/Fred/esp-idf/components/console -IC:/Users/Fred/esp-idf/components/nghttp/port/include -IC:/Users/Fred/esp-idf/components/nghttp/nghttp2/lib/includes -IC:/Users/Fred/esp-idf/components/esp-tls -IC:/Users/Fred/esp-idf/components/esp_adc_cal/include -IC:/Users/Fred/esp-idf/components/esp_gdbstub/include -IC:/Users/Fred/esp-idf/components/esp_hid/include -IC:/Users/Fred/esp-idf/components/tcp_transport/include -IC:/Users/Fred/esp-idf/components/esp_http_client/include -IC:/Users/Fred/esp-idf/components/esp_http_server/include -IC:/Users/Fred/esp-idf/components/esp_https_ota/include -IC:/Users/Fred/esp-idf/components/protobuf-c/protobuf-c -IC:/Users/Fred/esp-idf/components/protocomm/include/common -IC:/Users/Fred/esp-idf/components/protocomm/include/security -IC:/Users/Fred/esp-idf/components/protocomm/include/transports -IC:/Users/Fred/esp-idf/components/mdns/include -IC:/Users/Fred/esp-idf/components/esp_local_ctrl/include -IC:/Users/Fred/esp-idf/components/sdmmc/include -IC:/Users/Fred/esp-idf/components/esp_serial_slave_link/include -IC:/Users/Fred/esp-idf/components/esp_websocket_client/include -IC:/Users/Fred/esp-idf/components/expat/expat/expat/lib -IC:/Users/Fred/esp-idf/components/expat/port/include -IC:/Users/Fred/esp-idf/components/wear_levelling/include -IC:/Users/Fred/esp-idf/components/fatfs/diskio -IC:/Users/Fred/esp-idf/components/fatfs/vfs -IC:/Users/Fred/esp-idf/components/fatfs/src -IC:/Users/Fred/esp-idf/components/freemodbus/common/include -IC:/Users/Fred/esp-idf/components/idf_test/include -IC:/Users/Fred/esp-idf/components/idf_test/include/esp32 -IC:/Users/Fred/esp-idf/components/jsmn/include -IC:/Users/Fred/esp-idf/components/json/cJSON -IC:/Users/Fred/esp-idf/components/libsodium/libsodium/src/libsodium/include -IC:/Users/Fred/esp-idf/components/libsodium/port_include -IC:/Users/Fred/esp-idf/components/mqtt/esp-mqtt/include -IC:/Users/Fred/esp-idf/components/openssl/include -IC:/Users/Fred/esp-idf/components/spiffs/include -IC:/Users/Fred/esp-idf/components/ulp/include -IC:/Users/Fred/esp-idf/components/unity/include -IC:/Users/Fred/esp-idf/components/unity/unity/src -IC:/Users/Fred/esp-idf/components/wifi_provisioning/include -mlongcalls -Wno-frame-address   -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -Wall -Werror=all -Wno-error=unused-function -Wno-error=unused-but-set-variable -Wno-error=unused-variable -Wno-error=deprecated-declarations -Wextra -Wno-unused-parameter -Wno-sign-compare -ggdb -Og -std=gnu++11 -fno-exceptions -fno-rtti -D_GNU_SOURCE -DIDF_VER=\"v4.2-dirty\" -DESP_PLATFORM -MD -MT esp-idf/main/CMakeFiles/__idf_main.dir/DspFaust.cpp.obj -MF esp-idf\main\CMakeFiles\__idf_main.dir\DspFaust.cpp.obj.d -o esp-idf/main/CMakeFiles/__idf_main.dir/DspFaust.cpp.obj -c ../main/DspFaust.cpp
../main/DspFaust.cpp: In function 'void buildUIGlue(UIGlue*, UI*, bool)':
../main/DspFaust.cpp:1062:101: warning: cast between incompatible function types from 'void (*)(void*, const char*, double*, double, double, double, double)' to 'addVerticalSliderFun' {aka 'void (*)(void*, const char*, float*, float, float, float, float)'} [-Wcast-function-type]
         glue->addVerticalSlider = reinterpret_cast<addVerticalSliderFun>(addVerticalSliderGlueDouble);
                                                                                                     ^
../main/DspFaust.cpp:1063:107: warning: cast between incompatible function types from 'void (*)(void*, const char*, double*, double, double, double, double)' to 'addHorizontalSliderFun' {aka 'void (*)(void*, const char*, float*, float, float, float, float)'} [-Wcast-function-type]
         glue->addHorizontalSlider = reinterpret_cast<addHorizontalSliderFun>(addHorizontalSliderGlueDouble);
                                                                                                           ^
../main/DspFaust.cpp:1064:83: warning: cast between incompatible function types from 'void (*)(void*, const char*, double*, double, double, double, double)' to 'addNumEntryFun' {aka 'void (*)(void*, const char*, float*, float, float, float, float)'} [-Wcast-function-type]
         glue->addNumEntry = reinterpret_cast<addNumEntryFun>(addNumEntryGlueDouble);
                                                                                   ^
../main/DspFaust.cpp:1065:113: warning: cast between incompatible function types from 'void (*)(void*, const char*, double*, double, double)' to 'addHorizontalBargraphFun' {aka 'void (*)(void*, const char*, float*, float, float)'} [-Wcast-function-type]
         glue->addHorizontalBargraph = reinterpret_cast<addHorizontalBargraphFun>(addHorizontalBargraphGlueDouble);
                                                                                                                 ^
../main/DspFaust.cpp:1066:107: warning: cast between incompatible function types from 'void (*)(void*, const char*, double*, double, double)' to 'addVerticalBargraphFun' {aka 'void (*)(void*, const char*, float*, float, float)'} [-Wcast-function-type]
         glue->addVerticalBargraph = reinterpret_cast<addVerticalBargraphFun>(addVerticalBargraphGlueDouble);
                                                                                                           ^
../main/DspFaust.cpp: In member function 'void dsp_voice_group::buildUserInterface(UI*)':
../main/DspFaust.cpp:10886:79: error: 'dynamic_cast' not permitted with -fno-rtti
             if (!fGroupControl || dynamic_cast<SoundUIInterface*>(ui_interface)) {
                                                                               ^
../main/DspFaust.cpp: In member function 'virtual void mydsp_poly::buildUserInterface(UI*)':
../main/DspFaust.cpp:11230:59: error: 'dynamic_cast' not permitted with -fno-rtti
             if (dynamic_cast<midi_interface*>(ui_interface)) {
                                                           ^
../main/DspFaust.cpp:11231:74: error: 'dynamic_cast' not permitted with -fno-rtti
                 fMidiHandler = dynamic_cast<midi_interface*>(ui_interface);
                                                                          ^
../main/DspFaust.cpp: In constructor 'esp32audio::esp32audio(int, int)':
../main/DspFaust.cpp:17974:61: warning: 'I2S_COMM_FORMAT_I2S' is deprecated [-Wdeprecated-declarations]
                 .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
                                                             ^~~~~~~~~~~~~~~~~~~
In file included from C:/Users/Fred/esp-idf/components/soc/src/esp32/include/hal/i2s_ll.h:30,
                 from C:/Users/Fred/esp-idf/components/soc/include/hal/i2s_hal.h:28,
                 from C:/Users/Fred/esp-idf/components/driver/include/driver/i2s.h:24,
                 from ../main/DspFaust.cpp:17804:
C:/Users/Fred/esp-idf/components/soc/include/hal/i2s_types.h:70:5: note: declared here
     I2S_COMM_FORMAT_I2S       __attribute__((deprecated)) = 0x01, /*!< I2S communication format I2S, correspond to `I2S_COMM_FORMAT_STAND_I2S`*/
     ^~~~~~~~~~~~~~~~~~~
../main/DspFaust.cpp:17974:83: warning: 'I2S_COMM_FORMAT_I2S_MSB' is deprecated [-Wdeprecated-declarations]
                 .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
                                                                                   ^~~~~~~~~~~~~~~~~~~~~~~
In file included from C:/Users/Fred/esp-idf/components/soc/src/esp32/include/hal/i2s_ll.h:30,
                 from C:/Users/Fred/esp-idf/components/soc/include/hal/i2s_hal.h:28,
                 from C:/Users/Fred/esp-idf/components/driver/include/driver/i2s.h:24,
                 from ../main/DspFaust.cpp:17804:
C:/Users/Fred/esp-idf/components/soc/include/hal/i2s_types.h:71:5: note: declared here
     I2S_COMM_FORMAT_I2S_MSB   __attribute__((deprecated)) = 0x01, /*!< I2S format MSB, (I2S_COMM_FORMAT_I2S |I2S_COMM_FORMAT_I2S_MSB) correspond to `I2S_COMM_FORMAT_STAND_I2S`*/
     ^~~~~~~~~~~~~~~~~~~~~~~
../main/DspFaust.cpp:17979:13: warning: missing initializer for member 'i2s_config_t::tx_desc_auto_clear' [-Wmissing-field-initializers]
             };
             ^
../main/DspFaust.cpp:17979:13: warning: missing initializer for member 'i2s_config_t::fixed_mclk' [-Wmissing-field-initializers]
../main/DspFaust.cpp: In constructor 'DspFaust::DspFaust(bool)':
../main/DspFaust.cpp:24963:26: error: exception handling disabled, use -fexceptions to enable
     throw std::bad_alloc();
                          ^
../main/DspFaust.cpp: At global scope:
../main/DspFaust.cpp:1248:13: warning: 'void buildManagerGlue(MemoryManagerGlue*, dsp_memory_manager*)' defined but not used [-Wunused-function]
 static void buildManagerGlue(MemoryManagerGlue* glue, dsp_memory_manager* manager)
             ^~~~~~~~~~~~~~~~
../main/DspFaust.cpp:1226:13: warning: 'void buildMetaGlue(MetaGlue*, Meta*)' defined but not used [-Wunused-function]
 static void buildMetaGlue(MetaGlue* glue, Meta* meta)
             ^~~~~~~~~~~~~
../main/DspFaust.cpp:1051:13: warning: 'void buildUIGlue(UIGlue*, UI*, bool)' defined but not used [-Wunused-function]
 static void buildUIGlue(UIGlue* glue, UI* ui_interface, bool is_double)
             ^~~~~~~~~~~
../main/DspFaust.cpp:162:20: warning: 'std::__cxx11::string pathToContent(const string&)' defined but not used [-Wunused-function]
 static std::string pathToContent(const std::string& path)
                    ^~~~~~~~~~~~~
../main/DspFaust.cpp:156:13: warning: 'bool isopt(char**, const char*)' defined but not used [-Wunused-function]
 static bool isopt(char* argv[], const char* name)
             ^~~~~
../main/DspFaust.cpp:146:20: warning: 'const char* lopts1(int, char**, const char*, const char*, const char*)' defined but not used [-Wunused-function]
 static const char* lopts1(int argc, char* argv[], const char* longname, const char* shortname, const char* def)
                    ^~~~~~
../main/DspFaust.cpp:140:20: warning: 'const char* lopts(char**, const char*, const char*)' defined but not used [-Wunused-function]
 static const char* lopts(char* argv[], const char* name, const char* def)
                    ^~~~~
../main/DspFaust.cpp:130:13: warning: 'long int lopt1(int, char**, const char*, const char*, long int)' defined but not used [-Wunused-function]
 static long lopt1(int argc, char* argv[], const char* longname, const char* shortname, long def)
             ^~~~~
../main/DspFaust.cpp:124:13: warning: 'long int lopt(char**, const char*, long int)' defined but not used [-Wunused-function]
 static long lopt(char* argv[], const char* name, long def)
             ^~~~
../main/DspFaust.cpp:122:12: warning: 'int int2pow2(int)' defined but not used [-Wunused-function]
 static int int2pow2(int x) { int r = 0; while ((1<<r) < x) r++; return r; }
            ^~~~~~~~
../main/DspFaust.cpp:120:12: warning: 'int lsr(int, int)' defined but not used [-Wunused-function]
 static int lsr(int x, int n) { return int(((unsigned int)x) >> n); }
            ^~~
 
 
 
 
  ---------------------  
 
 Both platforms have a 'dynamic_cast' not permitted with -fno-rtti error
 Arduino  
 DspFaust.cpp:10886:79:  
 DspFaust.cpp:11230:59:  
 DspFaust.cpp:11231:74:   
 
 https://stackoverflow.com/questions/8469900/cant-downcast-because-class-is-not-polymorphic  
 RTTI = run-time type information (RTTI)  
 https://stackoverflow.com/questions/4486609/when-can-compiling-c-without-rtti-cause-problems  !!!  
 
 COMPILER OPTION FOR RTTI:    CONFIG_COMPILER_CXX_RTTI where to use ?  
 https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/kconfig.html#config-compiler-cxx-rtti
 in sdkconfig  
 set CONFIG_COMPILER_CXX_RTTI=y    
 
 NOTE this is removed again after running idf.py set-target esp32
 enable it again before running build
 buildlog5.txt
 
 CONFIG_CXX_EXCEPTIONS=y in sdkconfig
 https://esp32.com/viewtopic.php?t=8575
 
  
**make it "hard" with menuconfig?**  
 https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/kconfig.html#config-cxx-exceptions
 
 success buildlog7.txt success!! 
 
 instantiated AND started lib >runtime error heap error
 
 instantiate only:  buildlog8.txt
 buildlog8.txt
 command line now looks:
 ![as this.](images/command_line_buildlog8.jpg)
 
 ## Toggle logging run-time output with ctrl L
 
 functions:  
 - dynamic_cast    
 - throw  
 require? compiler options:
 - CONFIG_CXX_EXCEPTIONS=y
 - CONFIG_COMPILER_CXX_RTTI=y  
 lead to runtime errors: ?    
 - run time error at only instantiating DspFaust:  CORRUPT HEAP: multi_heap.c:439 detected at 0x3ffb4960  
 # RESULT
 OK.  When I toggle the exception handling (without consequences for the throw call, because it is commented out,  the runtime HEAP ERROR stays. Conclusion: we can safely enable exception handling (CONFIG_COMPILER_CXX_EXCEPTIONS=y in sdkconfig) and use the throw call in line 24963 of DspFaust.cpp.
#COMMENTED OUT THE DYNAMIC_CASTS
 
 ....
   
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
 
 sound_engines/..................a sound engine's configuration is defined by: Faust script, .dsp file   
├─ faust2api/....................each sound engine is implemented in a cpp_code and an arduino_sketch  
│  ├─ DSPTemplate/  
│  ├─ FaustSawtooth/   
│  │  ├─ FaustSawtooth.dsp  
│  │  ├─ arduino_sketch/  
│  │  │  ├─ arduino_sketch.ino  
│  │  │  ├─ DspFaust.cpp........lib files have to be stored in 2 places (Arduino does not accept relative paths)  
│  │  │  ├─ DspFaust.h               
│  │  │  ├─ WM8978.cpp  
│  │  │  ├─ WM8978.h  
│  │  ├─ cpp_code/  
│  │  │  ├─ main/  
│  │  │  │  ├─ main.cpp  
│  │  │  │  ├─ CMake_stuff  
│  │  │  │  ├─ DspFaust.cpp....must implement relative path to arduino_sketch folder for the libs to prevent double maintenance (risky version control)   
│  │  │  │  ├─ DspFaust.h  
│  │  │  │  ├─ WM8978.cpp  
│  │  │  │  ├─ WM8978.h  
│  │  │  ├─ CMake_stuff    
├─ faust2esp32/  
│  ├─ DSPTemplate/  
│  ├─ FaustSawtooth/  
│  ├─ another_dsp/  
│  │  ├─ another_dsp.dsp  

#### Also check:  
- https://www.makeuseof.com/tag/5-ways-to-print-folder-and-directory-contents-in-windows/  
- https://www.thewindowsclub.com/how-to-create-a-folder-tree-in-windows-10  
 
 
 
 
 
 

## MELODIES IMPLEMENT IN ARDUINO SAWTOOTH
 
https://www.browncountylibrary.org/wp-content/uploads/2018/03/arduino_piezo.pdf
 e.g.  
 int numTones = 10; // the number of tones in the scale  
int tones[] = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440}; // the frequency for each tone  
// mid C C# D D# E F F# G G# A  


