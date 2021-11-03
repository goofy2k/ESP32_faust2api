/************************************************************************
 ************************************************************************
 FCKX_SEQUENCER API 
 Copyright (C) 2021 FCKX
 ---------------------------------------------------------------------
Sequencer functions for application with TTGO TAudio V1.6 board and
Faust DSP engines
 ************************************************************************
 ************************************************************************/

//have a look in DspFaust.cpp: class esp32_midi : public midi_handler 
//can you use this class or jdksmid???

#ifndef __fckx_sequencer_api__
#define __fckx_sequencer_api__
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "esp_log.h"
//#include "AdaFruit_NeoPixel.h"




/*
#include "jdksmidi/sequencer.h"
#include "jdksmidi/track.h"
#include "jdksmidi/multitrack.h"
#include "jdksmidi/tempo.h"
#include "jdksmidi/matrix.h"
#include "jdksmidi/process.h"

#include "jdksmidi/world.h"
#include "jdksmidi/midi.h"
#include "jdksmidi/msg.h"
#include "jdksmidi/sysex.h"
#include "jdksmidi/parser.h"
*/



static const char *TAG = "FCKX_SEQUENCER";

/*
 Design considerations: see repo https://github.com/goofy2k/Faust-GUI


*/

//struct/class sequence()
/*
* a sequence of timestamped MIDI events
* has methods to:
*  - add an event
*  - remove an event
*  - play the sequence
*  - loop over part of the sequence
*  -  
* makes use of freeRTOS timers/tasks
* 
*/





void turnLEDon(){};
void turnLEDof();


void test_fckx_sequencer_lib() {
      ESP_LOGI(TAG, "LIB CALL SUCCEEDED");
      //introduce basic functionality here
      //later, split of functions and implement JSONUI/GUI
       
      //start a timer to wait for a period and then turn led on and off

 



            
      
    
};

#endif

