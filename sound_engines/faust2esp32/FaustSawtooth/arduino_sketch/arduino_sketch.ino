#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TONE_DURATION 500

// Relative paths DO NOT work
// #include "../hello_world/main/WM8978.h"
// #include "../hello_world/main/FaustSawtooth.h"

// Relative paths DO NOT work
//#include "..\hello_world\main\WM8978.h"    //relative path in sound_engine project     
//#include "..\hello_world\main\FaustSawtooth.h"

//This works: an absolute path to a flexible location. But absolute paths are not very conventient
//#include "C:\Users\Fred\esp_projects\ESP32-with-Faust\sound_engines\faust2esp32\FaustSawtooth\hello_world\main\WM8978.h"
//#include "C:\Users\Fred\esp_projects\ESP32-with-Faust\sound_engines\faust2esp32\FaustSawtooth\hello_world\main\FaustSawtooth.h"

/*
perhaps useful to create a solution where you calculate absolute paths from relative ones
https://arduino.stackexchange.com/questions/8651/loading-local-libraries
#define PROJECT_ROOT C:\path\to\your\project\folder

#define TO_STRING(s) #s
#define ABSOLUTE_PATH(root, relative_path) TO_STRING(root\relative_path)
#define RELATIVE_PATH(library) ABSOLUTE_PATH(PROJECT_ROOT, library)

#include RELATIVE_PATH(some\file\relative\path.h)
#include RELATIVE_PATH(another\file\relative\path.h)
*/

//define the variable project root
//IMPORTANT:  path should NOT contain minus signs, such as in ESP32-with-Faust   i.s.o. ESP32_Faust
#define PROJECT_ROOT C:\Users\Fred\esp_projects\ESP32_Faust\sound_engines\faust2esp32\FaustSawtooth

//define fixed conversion methods
#define TO_STRING(s) #s
#define ABSOLUTE_PATH(root, relative_path) TO_STRING(root\relative_path)
#define RELATIVE_PATH(library) ABSOLUTE_PATH(PROJECT_ROOT, library)

//define includes with fixed path derived from relative paths
//#include RELATIVE_PATH(hello_world\main\WM8978.h)
//#include RELATIVE_PATH(hello_world\main\FaustSawtooth.h)

// This works provided that libs are in the same folder as .ino
#include "WM8978.h"                 
#include "FaustSawtooth.h"

FaustSawtooth faustSawtooth(48000,8);

void setup() {
  WM8978 wm8978;
  wm8978.init();
  wm8978.addaCfg(1,1); 
  wm8978.inputCfg(1,0,0);     
  wm8978.outputCfg(1,0); 
  wm8978.micGain(30);
  wm8978.auxGain(0);
  wm8978.lineinGain(0);
  wm8978.spkVolSet(0);
  wm8978.hpVolSet(40,40);
  wm8978.i2sCfg(2,0);

 faustSawtooth.start();
}

void loop() {
  //faustSawtooth.setParamValue("freq",rand()%(2000-50 + 1) + 50);
  //delay(1000);
/*
 int numTones = 10; // the number of tones in the scale
int tones[] = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440}; // the frequency for each tone
// mid C C# D D# E F F# G G# A
*/
float pitch_shift = 1;
//faustSawtooth.start();
faustSawtooth.setParamValue("freq",pitch_shift*220);
delay(TONE_DURATION);
faustSawtooth.setParamValue("freq",pitch_shift*261);
delay(TONE_DURATION);
faustSawtooth.setParamValue("freq",pitch_shift*277);
delay(TONE_DURATION);
faustSawtooth.setParamValue("freq",pitch_shift*294);
delay(TONE_DURATION);
faustSawtooth.setParamValue("freq",pitch_shift*311);
delay(TONE_DURATION);
faustSawtooth.setParamValue("freq",pitch_shift*330);
delay(TONE_DURATION);
faustSawtooth.setParamValue("freq",pitch_shift*349);
delay(TONE_DURATION);
faustSawtooth.setParamValue("freq",pitch_shift*370);
delay(TONE_DURATION);
faustSawtooth.setParamValue("freq",pitch_shift*392);
delay(TONE_DURATION);
faustSawtooth.setParamValue("freq",pitch_shift*415);
delay(TONE_DURATION);
faustSawtooth.setParamValue("freq",pitch_shift*440);
delay(TONE_DURATION);
//faustSawtooth.stop();
faustSawtooth.setParamValue("freq",0.01);
delay(TONE_DURATION);
delay(2000);




  
}
