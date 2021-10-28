/*  Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
//#define MIDICTRL 1

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h" //for using software timers, NOT required for the nbDelay (?)

#include "freertos/event_groups.h"

#include "esp_spi_flash.h"


#include "esp_log.h"


#include "WM8978.h"
#include "DspFaust.h"




#define MAX_RTTL_LENGTH 1024

#define DSP_MACHINE "sawtooth_synth"


extern "C" {           //FCKX
    void app_main(void);
}


//software timer parameters. See example in: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html#timer-api
#define NUM_TIMERS 1

// An array to hold handles to the created timers.
TimerHandle_t xTimers[ NUM_TIMERS ];

// An array to hold a count of the number of times each timer expires.
int32_t lExpireCounters[ NUM_TIMERS ] = { 0 };

//timer callback to be defined just before main (OR should it be just before the sequencer procedure that uses it?

int msg_id;

//audio codec parameters WM8978 
WM8978 wm8978;
int hpVol_L = 20;
int hpVol_R = 20;
int micGain = 30;
int lineinGain = 0;    //check wm8978 docs for min/max and type (float vs int)

bool micen = false;
bool lineinen = false;
bool auxen = false;

bool dacen = true;
bool bpen = false;


//initial values in app for DSP parameters (may not be compliant with DspFaust.cpp defaults)
DspFaust* DSP;
float lfoFreq = 0.1;     //
float lfoDepth = 0;
float gain = 0.5;
int gate = 0;
float synthA = 0.01;
float synthD = 0.6;
float synthS = 0.2;
float synthR = 0.9;
bool incoming_updates = false;
float bend = 0;
float synthFreq = 440;
float waveTravel = 0;

int songlength = 0;
char * songbuffer = "Bond:d=4,o=5,b=80:32p,16c#6,32d#6,32d#6,16d#6,8d#6,16c#6,16c#6,16c#6,16c#6,32e6,32e6,16e6,8e6,16d#6,16d#6,16d#6,16c#6,32d#6,32d#6,16d#6,8d#6,16c#6,16c#6,16c#6,16c#6,32e6,32e6,16e6,8e6,16d#6,16d6,16c#6,16c#7,c.7,16g#6,16f#6,g#.6";
;

bool play_flag = true;
int poly = 0; //ofset for 


//parameter base ID's (WaveSynth FX), taken from API README.md
//add 1 in case of polyphony
int synthABaseId = 0;
int synthDBaseId = 1;
int synthRBaseId = 2;
int synthSBaseId = 3;
int bendBaseId = 4;
int synthFreqBaseId = 5; //for DSP_MACHINE poly
int gainBaseId = 6; //for DSP_MACHINE poly
int gateBaseId = 7; //for DSP_MACHINE poly
int lfoDepthBaseId = 8;
int lfoFreqBaseId = 9;
int waveTravelBaseId = 10;

static const char *TAG = "MQTT_FAUST";




void execute_single_midi_command(DspFaust * aDSP, int mididata){  //uses propagateMidi
static const char *TAG = "EXECUTE_SINGLE_COMMAND";
             int data2 = mididata & 0x000000ff;
             int data1 = (mididata & 0x0000ff00)>>8;
             int status = (mididata & 0x00ff0000)>>16;
             ESP_LOGI(TAG,"NUMERICAL VALUE:%u ", mididata);
             ESP_LOGI(TAG,"STATUS: %u (0x%X)", status, status);
             ESP_LOGI(TAG,"DATA1: %u (0x%X)", data1, data1);
             ESP_LOGI(TAG,"DATA2: %u (0x%X)", data2, data2) ; //integers are 32 bits!!!!             
             ESP_LOGI(TAG,"...to be implemented...(store msg and) call mqtt_midi"); 
             int type = status & 0xf0;
             int channel = status & 0x0f;
             int count = 3;
             int time = 0;
             aDSP->propagateMidi(count, time, type, channel, data1, data2);

};




/*
* update DSP parameters, based on the values set in the input storage
* input storage is updated asynchronously in MQTT callbacks (API , API2)
* update_controls is called at strategically suitable moments in e.g. playing sequences
*/

void update_all_controls (DspFaust * aDSP){
 static const char *TAG = "update_all_controls"; 
  if (incoming_updates) {
  
  wm8978.hpVolSet(hpVol_R, hpVol_L);
  ESP_LOGI(TAG, "poly: %d", poly); 
  //if (poly == 1){
    //found out that the BaseId is sufficient, poly = 0
    poly = 1;
    
    //later, use a struct, better: re-use an existing struct
    //and or use MACROS and a loop
 /*   
        aDSP->setParamValue(synthFreqBaseId+poly, synthFreq);
    ESP_LOGI(TAG, "synthFreqBaseId %d, set gain %6.2f", synthFreqBaseId, synthFreq);  //FCKX  
         aDSP->setParamValue(gateBaseId+poly, gate);
  //  ESP_LOGI(TAG, "gateBaseId %d, set gate %6.2f", gateBaseId, gate);  //FCKX    
    
           aDSP->setParamValue(waveTravelBaseId+poly, waveTravel);
    ESP_LOGI(TAG, "waveTravelBaseId %d, set waveTravel %6.2f", waveTravelBaseId, waveTravel);  //FCKX    
    
  */  
    aDSP->setParamValue(bendBaseId+poly, bend);
    ESP_LOGI(TAG, "bendBaseId %d, set bend %6.2f", bendBaseId, bend);  //FCKX   
    aDSP->setParamValue(gainBaseId+poly, gain);
    ESP_LOGI(TAG, "gainBaseId %d, set gain %6.2f", gainBaseId, gain);  //FCKX
    
    
    aDSP->setParamValue(lfoFreqBaseId+poly, lfoFreq);
    ESP_LOGI(TAG, "lfoFreqBaseId %d, set lfoFreq %6.2f", lfoFreqBaseId, lfoFreq);  //FCKX
    aDSP->setParamValue(lfoDepthBaseId+poly, lfoDepth);
    ESP_LOGI(TAG, "lfoDepthBaseId %d, set lfoDepth %6.2f", lfoDepthBaseId, lfoDepth);  //FCKX  
    aDSP->setParamValue(synthABaseId+poly, synthA);
    ESP_LOGI(TAG, "synthABaseId %d, set synthA %6.2f", synthABaseId, synthA);  //FCKX
    aDSP->setParamValue(synthDBaseId+poly, synthD);
    ESP_LOGI(TAG, "synthDBaseId %d, set synthD %6.2f", synthDBaseId, synthD);  //FCKX
    aDSP->setParamValue(synthSBaseId+poly, synthS);
    ESP_LOGI(TAG, "synthSBaseId %d, set synthS %6.2f", synthSBaseId, synthS);  //FCKX
    aDSP->setParamValue(synthRBaseId+poly, synthR);
    ESP_LOGI(TAG, "synthRBaseId %d, set synthR %6.2f", synthRBaseId, synthR);  //FCKX
/* 
 } else { //poly == 0
    DSP->setParamValue(lfoFreqBaseId+poly, lfoFreq);
    ESP_LOGI(TAG, "lfoFreqBaseId %d, set lfoFreq %6.2f", lfoFreqBaseId, lfoFreq);  //FCKX
    aDSP->setParamValue(lfoDepthBaseId+poly, lfoDepth); 
    aDSP->setParamValue(synthABaseId+poly, synthA);
    aDSP->setParamValue(synthDBaseId+poly, synthD);
    aDSP->setParamValue(synthSBaseId+poly, synthS);
    aDSP->setParamValue(synthRBaseId+poly, synthR);    
      
  }
*/    
  }
  incoming_updates = false;

}


void update_controls_all(DspFaust * aDSP){  //maybe do this in a cyclic freertos timer task
  static const char *TAG = "update_controls"; 
  if (incoming_updates) {
  
  wm8978.hpVolSet(hpVol_R, hpVol_L);
  ESP_LOGI(TAG, "poly: %d", poly); 
  //if (poly == 1){
    //found out that the BaseId is sufficient, poly = 0
    poly = 0;
    
    //later, use a struct, better: re-use an existing struct
    //and or use MACROS and a loop
        aDSP->setParamValue(synthFreqBaseId+poly, synthFreq);
    ESP_LOGI(TAG, "synthFreqBaseId %d, set gain %6.2f", synthFreqBaseId, synthFreq);  //FCKX  
         aDSP->setParamValue(gateBaseId+poly, gate);
  //  ESP_LOGI(TAG, "gateBaseId %d, set gate %6.2f", gateBaseId, gate);  //FCKX    
    
           aDSP->setParamValue(waveTravelBaseId+poly, waveTravel);
    ESP_LOGI(TAG, "waveTravelBaseId %d, set waveTravel %6.2f", waveTravelBaseId, waveTravel);  //FCKX    
    
    
    aDSP->setParamValue(bendBaseId+poly, bend);
    ESP_LOGI(TAG, "bendBaseId %d, set bend %6.2f", bendBaseId, bend);  //FCKX   
    aDSP->setParamValue(gainBaseId+poly, gain);
    ESP_LOGI(TAG, "gainBaseId %d, set gain %6.2f", gainBaseId, gain);  //FCKX
    
    
    aDSP->setParamValue(lfoFreqBaseId+poly, lfoFreq);
    ESP_LOGI(TAG, "lfoFreqBaseId %d, set lfoFreq %6.2f", lfoFreqBaseId, lfoFreq);  //FCKX
    aDSP->setParamValue(lfoDepthBaseId+poly, lfoDepth);
    ESP_LOGI(TAG, "lfoDepthBaseId %d, set lfoDepth %6.2f", lfoDepthBaseId, lfoDepth);  //FCKX  
    aDSP->setParamValue(synthABaseId+poly, synthA);
    ESP_LOGI(TAG, "synthABaseId %d, set synthA %6.2f", synthABaseId, synthA);  //FCKX
    aDSP->setParamValue(synthDBaseId+poly, synthD);
    ESP_LOGI(TAG, "synthDBaseId %d, set synthD %6.2f", synthDBaseId, synthD);  //FCKX
    aDSP->setParamValue(synthSBaseId+poly, synthS);
    ESP_LOGI(TAG, "synthSBaseId %d, set synthS %6.2f", synthSBaseId, synthS);  //FCKX
    aDSP->setParamValue(synthRBaseId+poly, synthR);
    ESP_LOGI(TAG, "synthRBaseId %d, set synthR %6.2f", synthRBaseId, synthR);  //FCKX
/* 
 } else { //poly == 0
    DSP->setParamValue(lfoFreqBaseId+poly, lfoFreq);
    ESP_LOGI(TAG, "lfoFreqBaseId %d, set lfoFreq %6.2f", lfoFreqBaseId, lfoFreq);  //FCKX
    aDSP->setParamValue(lfoDepthBaseId+poly, lfoDepth); 
    aDSP->setParamValue(synthABaseId+poly, synthA);
    aDSP->setParamValue(synthDBaseId+poly, synthD);
    aDSP->setParamValue(synthSBaseId+poly, synthS);
    aDSP->setParamValue(synthRBaseId+poly, synthR);    
      
  }
*/    
  }
  incoming_updates = false;

}



void update_controls(uintptr_t voiceAddress, DspFaust * aDSP){  //maybe do this in a cyclic freertos timer task
  static const char *TAG = "update_controls"; 
  if (incoming_updates) {
  
  wm8978.hpVolSet(hpVol_R, hpVol_L);
  ESP_LOGI(TAG, "poly: %d", poly); 
  //if (poly == 1){
    //found out that the BaseId is sufficient, poly = 0
    poly = 0;
    
    //later, use a struct, better: re-use an existing struct
    //and or use MACROS and a loop
        aDSP->setVoiceParamValue(synthFreqBaseId+poly,voiceAddress, synthFreq);
    ESP_LOGI(TAG, "synthFreqBaseId %d, set gain %6.2f", synthFreqBaseId, synthFreq);  //FCKX  
         aDSP->setVoiceParamValue(gateBaseId+poly,voiceAddress, gate);
  //  ESP_LOGI(TAG, "gateBaseId %d, set gate %6.2f", gateBaseId, gate);  //FCKX    
    
           aDSP->setVoiceParamValue(waveTravelBaseId+poly,voiceAddress, waveTravel);
    ESP_LOGI(TAG, "waveTravelBaseId %d, set waveTravel %6.2f", waveTravelBaseId, waveTravel);  //FCKX    
    
    
    aDSP->setVoiceParamValue(bendBaseId+poly,voiceAddress, bend);
    ESP_LOGI(TAG, "bendBaseId %d, set bend %6.2f", bendBaseId, bend);  //FCKX   
    aDSP->setVoiceParamValue(gainBaseId+poly,voiceAddress, gain);
    ESP_LOGI(TAG, "gainBaseId %d, set gain %6.2f", gainBaseId, gain);  //FCKX
    
    
    aDSP->setVoiceParamValue(lfoFreqBaseId+poly,voiceAddress, lfoFreq);
    ESP_LOGI(TAG, "lfoFreqBaseId %d, set lfoFreq %6.2f", lfoFreqBaseId, lfoFreq);  //FCKX
    aDSP->setVoiceParamValue(lfoDepthBaseId+poly,voiceAddress, lfoDepth);
    ESP_LOGI(TAG, "lfoDepthBaseId %d, set lfoDepth %6.2f", lfoDepthBaseId, lfoDepth);  //FCKX  
    aDSP->setVoiceParamValue(synthABaseId+poly,voiceAddress, synthA);
    ESP_LOGI(TAG, "synthABaseId %d, set synthA %6.2f", synthABaseId, synthA);  //FCKX
    aDSP->setVoiceParamValue(synthDBaseId+poly,voiceAddress, synthD);
    ESP_LOGI(TAG, "synthDBaseId %d, set synthD %6.2f", synthDBaseId, synthD);  //FCKX
    aDSP->setVoiceParamValue(synthSBaseId+poly,voiceAddress, synthS);
    ESP_LOGI(TAG, "synthSBaseId %d, set synthS %6.2f", synthSBaseId, synthS);  //FCKX
    aDSP->setVoiceParamValue(synthRBaseId+poly,voiceAddress, synthR);
    ESP_LOGI(TAG, "synthRBaseId %d, set synthR %6.2f", synthRBaseId, synthR);  //FCKX
/* 
 } else { //poly == 0
    DSP->setParamValue(lfoFreqBaseId+poly, lfoFreq);
    ESP_LOGI(TAG, "lfoFreqBaseId %d, set lfoFreq %6.2f", lfoFreqBaseId, lfoFreq);  //FCKX
    aDSP->setParamValue(lfoDepthBaseId+poly, lfoDepth); 
    aDSP->setParamValue(synthABaseId+poly, synthA);
    aDSP->setParamValue(synthDBaseId+poly, synthD);
    aDSP->setParamValue(synthSBaseId+poly, synthS);
    aDSP->setParamValue(synthRBaseId+poly, synthR);    
      
  }
*/    
  }
  incoming_updates = false;

}



void nbDelay(int delayTicks) {        
        TickType_t startTick = xTaskGetTickCount();
        while( (xTaskGetTickCount()-startTick) < pdMS_TO_TICKS(delayTicks)){
          //…
        } 
    };

/* note playing routines using the Faust API commands
*
*/

/* definitions for use in rtttl sequencers 
*/
#define OCTAVE_OFFSET 0

float freqs[] = { 0,
65.40639133,69.29565774,73.41619198,77.78174593,82.40688923,87.30705786,92.49860568,97.998859,103.8261744,110,116.5409404,123.4708253,
130.8127827,138.5913155,146.832384,155.5634919,164.8137785,174.6141157,184.9972114,195.997718,207.6523488,220,233.0818808,246.9416506,
261.6255653,277.182631,293.6647679,311.1269837,329.6275569,349.2282314,369.9944227,391.995436,415.3046976,440,466.1637615,493.8833013,
523.2511306,554.365262,587.3295358,622.2539674,659.2551138,698.4564629,739.9888454,783.990872,830.6093952,880,932.327523,987.7666025
};

/*
int notes[] = { 0,
NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4,
NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5,
NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6,
NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7
};
*/

//char *song = "The Simpsons:d=4,o=5,b=160:c.6,e6,f#6,8a6,g.6,e6,c6,8a,8f#,8f#,8f#,2g,8p,8p,8f#,8f#,8f#,8g,a#.,8c6,8c6,8c6,c6";
//char *song = "Indiana:d=4,o=5,b=250:e,8p,8f,8g,8p,1c6,8p.,d,8p,8e,1f,p.,g,8p,8a,8b,8p,1f6,p,a,8p,8b,2c6,2d6,2e6,e,8p,8f,8g,8p,1c6,p,d6,8p,8e6,1f.6,g,8p,8g,e.6,8p,d6,8p,8g,e.6,8p,d6,8p,8g,f.6,8p,e6,8p,8d6,2c6";
//char *song = "TakeOnMe:d=4,o=4,b=160:8f#5,8f#5,8f#5,8d5,8p,8b,8p,8e5,8p,8e5,8p,8e5,8g#5,8g#5,8a5,8b5,8a5,8a5,8a5,8e5,8p,8d5,8p,8f#5,8p,8f#5,8p,8f#5,8e5,8e5,8f#5,8e5,8f#5,8f#5,8f#5,8d5,8p,8b,8p,8e5,8p,8e5,8p,8e5,8g#5,8g#5,8a5,8b5,8a5,8a5,8a5,8e5,8p,8d5,8p,8f#5,8p,8f#5,8p,8f#5,8e5,8e5";
//char *song = "Entertainer:d=4,o=5,b=140:8d,8d#,8e,c6,8e,c6,8e,2c.6,8c6,8d6,8d#6,8e6,8c6,8d6,e6,8b,d6,2c6,p,8d,8d#,8e,c6,8e,c6,8e,2c.6,8p,8a,8g,8f#,8a,8c6,e6,8d6,8c6,8a,2d6";
//char *song = "Muppets:d=4,o=5,b=250:c6,c6,a,b,8a,b,g,p,c6,c6,a,8b,8a,8p,g.,p,e,e,g,f,8e,f,8c6,8c,8d,e,8e,8e,8p,8e,g,2p,c6,c6,a,b,8a,b,g,p,c6,c6,a,8b,a,g.,p,e,e,g,f,8e,f,8c6,8c,8d,e,8e,d,8d,c";
//char *song = "Xfiles:d=4,o=5,b=125:e,b,a,b,d6,2b.,1p,e,b,a,b,e6,2b.,1p,g6,f#6,e6,d6,e6,2b.,1p,g6,f#6,e6,d6,f#6,2b.,1p,e,b,a,b,d6,2b.,1p,e,b,a,b,e6,2b.,1p,e6,2b.";
//char *song = "Looney:d=4,o=5,b=140:32p,c6,8f6,8e6,8d6,8c6,a.,8c6,8f6,8e6,8d6,8d#6,e.6,8e6,8e6,8c6,8d6,8c6,8e6,8c6,8d6,8a,8c6,8g,8a#,8a,8f";
//char *song = "20thCenFox:d=16,o=5,b=140:b,8p,b,b,2b,p,c6,32p,b,32p,c6,32p,b,32p,c6,32p,b,8p,b,b,b,32p,b,32p,b,32p,b,32p,b,32p,b,32p,b,32p,g#,32p,a,32p,b,8p,b,b,2b,4p,8e,8g#,8b,1c#6,8f#,8a,8c#6,1e6,8a,8c#6,8e6,1e6,8b,8g#,8a,2b";
//char *song = "Bond:d=4,o=5,b=80:32p,16c#6,32d#6,32d#6,16d#6,8d#6,16c#6,16c#6,16c#6,16c#6,32e6,32e6,16e6,8e6,16d#6,16d#6,16d#6,16c#6,32d#6,32d#6,16d#6,8d#6,16c#6,16c#6,16c#6,16c#6,32e6,32e6,16e6,8e6,16d#6,16d6,16c#6,16c#7,c.7,16g#6,16f#6,g#.6";
//char *song = "MASH:d=8,o=5,b=140:4a,4g,f#,g,p,f#,p,g,p,f#,p,2e.,p,f#,e,4f#,e,f#,p,e,p,4d.,p,f#,4e,d,e,p,d,p,e,p,d,p,2c#.,p,d,c#,4d,c#,d,p,e,p,4f#,p,a,p,4b,a,b,p,a,p,b,p,2a.,4p,a,b,a,4b,a,b,p,2a.,a,4f#,a,b,p,d6,p,4e.6,d6,b,p,a,p,2b";
//char *song = "StarWars:d=4,o=5,b=45:32p,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#.6,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#6";
//char *song = "TopGun:d=4,o=4,b=31:32p,16c#,16g#,16g#,32f#,32f,32f#,32f,16d#,16d#,32c#,32d#,16f,32d#,32f,16f#,32f,32c#,16f,d#,16c#,16g#,16g#,32f#,32f,32f#,32f,16d#,16d#,32c#,32d#,16f,32d#,32f,16f#,32f,32c#,g#";
//char *song = "A-Team:d=8,o=5,b=125:4d#6,a#,2d#6,16p,g#,4a#,4d#.,p,16g,16a#,d#6,a#,f6,2d#6,16p,c#.6,16c6,16a#,g#.,2a#";
//char *song = "Flinstones:d=4,o=5,b=40:32p,16f6,16a#,16a#6,32g6,16f6,16a#.,16f6,32d#6,32d6,32d6,32d#6,32f6,16a#,16c6,d6,16f6,16a#.,16a#6,32g6,16f6,16a#.,32f6,32f6,32d#6,32d6,32d6,32d#6,32f6,16a#,16c6,a#,16a6,16d.6,16a#6,32a6,32a6,32g6,32f#6,32a6,8g6,16g6,16c.6,32a6,32a6,32g6,32g6,32f6,32e6,32g6,8f6,16f6,16a#.,16a#6,32g6,16f6,16a#.,16f6,32d#6,32d6,32d6,32d#6,32f6,16a#,16c.6,32d6,32d#6,32f6,16a#,16c.6,32d6,32d#6,32f6,16a#6,16c7,8a#.6";
//char *song = "Jeopardy:d=4,o=6,b=125:c,f,c,f5,c,f,2c,c,f,c,f,a.,8g,8f,8e,8d,8c#,c,f,c,f5,c,f,2c,f.,8d,c,a#5,a5,g5,f5,p,d#,g#,d#,g#5,d#,g#,2d#,d#,g#,d#,g#,c.7,8a#,8g#,8g,8f,8e,d#,g#,d#,g#5,d#,g#,2d#,g#.,8f,d#,c#,c,p,a#5,p,g#.5,d#,g#";
//char *song = "Gadget:d=16,o=5,b=50:32d#,32f,32f#,32g#,a#,f#,a,f,g#,f#,32d#,32f,32f#,32g#,a#,d#6,4d6,32d#,32f,32f#,32g#,a#,f#,a,f,g#,f#,8d#";
//char *song = "Smurfs:d=32,o=5,b=200:4c#6,16p,4f#6,p,16c#6,p,8d#6,p,8b,p,4g#,16p,4c#6,p,16a#,p,8f#,p,8a#,p,4g#,4p,g#,p,a#,p,b,p,c6,p,4c#6,16p,4f#6,p,16c#6,p,8d#6,p,8b,p,4g#,16p,4c#6,p,16a#,p,8b,p,8f,p,4f#";
//char *song = "MahnaMahna:d=16,o=6,b=125:c#,c.,b5,8a#.5,8f.,4g#,a#,g.,4d#,8p,c#,c.,b5,8a#.5,8f.,g#.,8a#.,4g,8p,c#,c.,b5,8a#.5,8f.,4g#,f,g.,8d#.,f,g.,8d#.,f,8g,8d#.,f,8g,d#,8c,a#5,8d#.,8d#.,4d#,8d#.";
//char *song = "LeisureSuit:d=16,o=6,b=56:f.5,f#.5,g.5,g#5,32a#5,f5,g#.5,a#.5,32f5,g#5,32a#5,g#5,8c#.,a#5,32c#,a5,a#.5,c#.,32a5,a#5,32c#,d#,8e,c#.,f.,f.,f.,f.,f,32e,d#,8d,a#.5,e,32f,e,32f,c#,d#.,c#";
//char *song = "MissionImp:d=16,o=6,b=95:32d,32d#,32d,32d#,32d,32d#,32d,32d#,32d,32d,32d#,32e,32f,32f#,32g,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,a#,g,2d,32p,a#,g,2c#,32p,a#,g,2c,a#5,8c,2p,32p,a#5,g5,2f#,32p,a#5,g5,2f,32p,a#5,g5,2e,d#,8d";
char *song = "GoodBad:d=4,o=5,b=56:32p,32a#,32d#6,32a#,32d#6,8a#.,16f#.,16g#.,d#,32a#,32d#6,32a#,32d#6,8a#.,16f#.,16g#.,c#6,32a#,32d#6,32a#,32d#6,8a#.,16f#.,32f.,32d#.,c#,32a#,32d#6,32a#,32d#6,8a#.,16g#.,d#";

#define isdigit(n) (n >= '0' && n <= '9')

/* 
* keyOn / keyOff players
*/



void play_keys_nb(DspFaust * aDSP){  //uses keyOn / keyOff
    // start continuous background voice for testing polyphony
    static const char *TAG = "PLAY_KEYS";
    int res;
    uintptr_t voiceAddress;
    ESP_LOGW(TAG, "starting play_keys_nb ");

    nbDelay(3000); 
    int vel1 = 126;
    
    voiceAddress = aDSP->keyOn(38,vel1);
    nbDelay(5000); 
    for (int pitch = 48; pitch < 69; pitch++){

    //printf("counter ii %d \n",ii);  

    nbDelay(100);        
       //C major 0,2,6
       //D minor 0, -2, 2
       //A minor 0, 2, 4
    ESP_LOGI(TAG, "keyOn pitch %d velocity % d", pitch,vel1);      
    voiceAddress = aDSP->keyOn(pitch+0,vel1);
    voiceAddress = aDSP->keyOn(pitch -2,vel1);
    voiceAddress = aDSP->keyOn(pitch +2,vel1);
    voiceAddress = aDSP->keyOn(pitch+9,vel1);
    //update_controls(voiceAddress,aDSP);
    ESP_LOGI(TAG, "after keyOn");  

        

    ESP_LOGI(TAG, "keyOn pitch %d velocity % d", pitch,vel1); 

    nbDelay(100);         

     res = aDSP->keyOff(pitch+0);
     res = aDSP->keyOff(pitch-2);
      res = aDSP->keyOff(pitch+2);
      res = aDSP->keyOff(pitch+9);
    } 
     ESP_LOGW(TAG, "end of sequence");     
    nbDelay(5000); 
   
     //release continuous background voice, that has long ago already been stolen :-)
     res = aDSP->keyOff(38);
     
     aDSP->allNotesOff(false); //delete main voice

    }




void play_poly_rtttl_chords(char *p, DspFaust * aDSP){
 // aDSP->allNotesOff(); 
 static const char *TAG = "PLAY_POLY_RTTL_CHORDS";
 ESP_LOGW(TAG,"playing songbuffer");

 int chord[] = {1,2,4, 9};
 uintptr_t voiceAddress [4];
 


 ESP_LOGW(TAG,"size of chord %d ",sizeof(chord)/4 );
  for (unsigned int chordNote = 0; chordNote < sizeof(chord)/4; chordNote++){
      
      ESP_LOGW(TAG,"chordNote %d, chord: %d  ", chordNote, chord[chordNote] );
  };

/*
aDSP->allNotesOff(false); //delete main voice
  for (unsigned int chordNote = 0; chordNote < sizeof(chord)/4; chordNote++){       
      voiceAddress[chordNote] = aDSP->newVoice(); //create main voice};  
      aDSP->setVoiceParamValue("/sawtooth_synth/gain",voiceAddress[chordNote],1);
        };
*/
  
  // Absolutely no error checking in here
  unsigned char default_dur = 4;
  unsigned char default_oct = 6;
  int bpm = 63;
  int num;
  long wholenote;
  long duration;
  unsigned char note;
  unsigned char scale;

  // format: d=N,o=N,b=NNN:
  // find the start (skip name, etc)

  //char * p = song;

  while(*p != ':') p++;    // ignore name
  p++;                     // skip ':'

  // get default duration
  if(*p == 'd')
  {
    p++; p++;              // skip "d="
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    if(num > 0) default_dur = num;
    p++;                   // skip comma
  }

 //Serial.print("ddur: "); Serial.println(default_dur, 10);

  // get default octave
  if(*p == 'o')
  {
    p++; p++;              // skip "o="
    num = *p++ - '0';
    if(num >= 3 && num <=7) default_oct = num;
    p++;                   // skip comma
  }

  //Serial.print("doct: "); Serial.println(default_oct, 10);

  // get BPM
  if(*p == 'b')
  {
    p++; p++;              // skip "b="
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    bpm = num;
    p++;                   // skip colon
  }

  //Serial.print("bpm: "); Serial.println(bpm, 10);

  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole note (in milliseconds)

  //Serial.print("wn: "); Serial.println(wholenote, 10);


  // now begin note loop
  while(*p)
  {
    // first, get note duration, if available
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    
    if(num) duration = wholenote / num;
    else duration = wholenote / default_dur;  // we will need to check if we are a dotted note after

    // now get the note
    note = 0;

    switch(*p)
    {
      case 'c':
        note = 1;
        break;
      case 'd':
        note = 3;
        break;
      case 'e':
        note = 5;
        break;
      case 'f':
        note = 6;
        break;
      case 'g':
        note = 8;
        break;
      case 'a':
        note = 10;
        break;
      case 'b':
        note = 12;
        break;
      case 'p':
      default:
        note = 0;
    }
    p++;

    // now, get optional '#' sharp
    if(*p == '#')
    {
      note++;
      p++;
    }

    // now, get optional '.' dotted note
    if(*p == '.')
    {
      duration += duration/2;
      p++;
    }
  
    // now, get scale
    if(isdigit(*p))
    {
      scale = *p - '0';
      p++;
    }
    else
    {
      scale = default_oct;
    }

    scale += OCTAVE_OFFSET;

    if(*p == ',')
      p++;       // skip comma for next note (or we may be at the end)

    // now play the note

    if(note)
    {
       
      printf("Playing: note %c\n", note);

      /*
      aDSP->setParamValue("/elecGuitar/midi/freq",freqs[(scale-4) * 12 + note]);
      aDSP->setParamValue("/elecGuitar/gate",1);
     */
/* 
      aDSP->setParamValue("/simpleSynt_Analog/freq",freqs[(scale-4) * 12 + note]);
      aDSP->setParamValue("/simpleSynt_Analog/gate",1);     
*/


       for (unsigned int chordNote = 0; chordNote < sizeof(chord)/4; chordNote++){
          ESP_LOGW(TAG,"create  %d ",sizeof(chord)/4 ); 
      voiceAddress[chordNote] = aDSP->newVoice(); //create main voice};  
 
  aDSP->setVoiceParamValue("/sawtooth_synth/freq",voiceAddress[chordNote],freqs[(scale-4) * 12 + note+ chord[chordNote] ]);

       aDSP->setVoiceParamValue("/sawtooth_synth/gain",voiceAddress[chordNote],1);   
           
           aDSP->setVoiceParamValue("/sawtooth_synth/gate",voiceAddress[chordNote],1); 
           
       };
/*

      aDSP->setVoiceParamValue("/sawtooth_synth/freq",voiceAddress,freqs[(scale-4) * 12 + note]);
      aDSP->setVoiceParamValue("/sawtooth_synth/gate",voiceAddress,1); 

      aDSP->setVoiceParamValue("/sawtooth_synth/freq",voiceAddress2,freqs[(scale-4) * 12 + note+chord1]);
      aDSP->setVoiceParamValue("/sawtooth_synth/gate",voiceAddress2,1); 
      
      aDSP->setVoiceParamValue("/sawtooth_synth/freq",voiceAddress3,freqs[(scale-4) * 12 + note+chord2]);
      aDSP->setVoiceParamValue("/sawtooth_synth/gate",voiceAddress3,1); 
  */    
      nbDelay(duration);
      //vTaskDelay(duration / portTICK_PERIOD_MS);
      //printf("%s \n",DSP->getJSONUI());
      //DSP->setParamValue("gain",0);
      //aDSP->setParamValue("/simpleSynt_Analog/gate",0); 
      // aDSP->setParamValue("/elecGuitar/gate",0);
      
            for (unsigned int chordNote = 0; chordNote < sizeof(chord)/4; chordNote++){
                aDSP->setVoiceParamValue("/sawtooth_synth/gate",voiceAddress[chordNote],0);
                
            };
      
      /*
      aDSP->setVoiceParamValue("/sawtooth_synth/gate",voiceAddress,0);
      aDSP->setVoiceParamValue("/sawtooth_synth/gate",voiceAddress2,0); 
      aDSP->setVoiceParamValue("/sawtooth_synth/gate",voiceAddress3,0);   
      */  
  // tone1.play(notes[(scale - 4) * 12 + note]);
      
     //FCKXFCKX update_controls(voiceAddress,aDSP);  //later do this in a freertos task 
      
      //delay(duration);
      //tone1.stop();
      
    }
    else
    {
      //Serial.print("Pausing: ");
      //Serial.println(duration, 10);
      nbDelay(5*duration);
      //vTaskDelay(5*duration/ portTICK_PERIOD_MS);
    }
    

     
  }  //while (p*)
        printf("End of song: \n");
      //set gain to zero for gently swithing off this voice.....
      
      
      aDSP->allNotesOff(false); //delete main voice
      /*
       for (unsigned int chordNote = 0; chordNote < sizeof(chord)/4; chordNote++){
           
           aDSP->setVoiceParamValue("/sawtooth_synth/gain",voiceAddress[chordNote],0);
             aDSP->deleteVoice(voiceAddress[chordNote]); //delete main voice
           
       };
      */
      
      /*
      aDSP->setVoiceParamValue("/sawtooth_synth/gain",voiceAddress,0);
          aDSP->setVoiceParamValue("/sawtooth_synth/gain",voiceAddress2,0);
                aDSP->setVoiceParamValue("/sawtooth_synth/gain",voiceAddress3,0);
        aDSP->deleteVoice(voiceAddress); //delete main voice
        aDSP->deleteVoice(voiceAddress2); //delete voice2
       aDSP->deleteVoice(voiceAddress3); //delete voice3
       
       */
  } //song player    
 







/* Use of freeRTOS software timers for non-blocking code in sequencing routines (experimental)
*
* Taken from freeRTOS part in ESP-IDF documentation

// Define a callback function that will be used by multiple timer instances.
// The callback function does nothing but count the number of times the
// associated timer expires, and stop the timer once the timer has expired
// 10 times.
*/
void vTimerCallback( TimerHandle_t pxTimer )
{
ESP_LOGI(TAG, "TIMER_CALLBACK");    
int32_t lArrayIndex;
const int32_t xMaxExpiryCountBeforeStopping = 10;
     
    // Optionally do something if the pxTimer parameter is NULL.
    configASSERT( pxTimer );
    // Which timer expired?
    lArrayIndex = ( int32_t ) pvTimerGetTimerID( pxTimer );
    // Increment the number of times that pxTimer has expired.
    lExpireCounters[ lArrayIndex ] += 1;
    ESP_LOGI(TAG, "TIMER %d EXPIRED %d times", lArrayIndex, lExpireCounters[ lArrayIndex ] ); 
    // If the timer has expired 10 times then stop it from running.
    if( lExpireCounters[ lArrayIndex ] == xMaxExpiryCountBeforeStopping )
    {
        // Do not use a block time if calling a timer API function from a
        // timer callback function, as doing so could cause a deadlock!
         xTimerStop( pxTimer, 0 );
         ESP_LOGI(TAG, "TIMER %d stopped because nr of expirations reached %d", lArrayIndex , xMaxExpiryCountBeforeStopping); 
    }
 }

/*
* experimental code for using software timers
*/
bool expired;
 
void myDelayTimerCallback( TimerHandle_t pxTimer )
{
ESP_LOGI(TAG, "delayTimer_CALLBACK: expired!");    
xTimerStop( pxTimer, 0 );
expired = true;    

 }

void non_blocking_Delay( int myTicks){
//non_blocking delay based on freertos timer
 TimerHandle_t delayTimer;

//create timer
        ESP_LOGI(TAG, "Creating delay timer");
        expired = false;         
        delayTimer = xTimerCreate(    "Timer",       // Just a text name, not used by the kernel.
                                         myTicks  ,   // The timer period in ticks.
                                         pdFALSE,    // The timers will auto-reload themselves when they expire.
                                         ( void * ) 1,  // Assign each timer a unique id equal to its array index.
                                         myDelayTimerCallback // Each timer calls the same callback when it expires.
                                     );

         if( delayTimer == NULL )
         {
             // The timer was not created.
             ESP_LOGI(TAG, "Timer was not created!");
         }
         else
         {
             // Start the timer.  No block time is specified, and even if one was
             // it would be ignored because the scheduler has not yet been
             // started.
             ESP_LOGI(TAG, "delayTimer to be started!");
             if( xTimerStart( delayTimer, 0 ) != pdPASS )
             {
                 // The timer could not be set into the Active state.
                 ESP_LOGI(TAG, "delayTimer could not be set into the Active state");
             }
         }
//stay in non-blocking delay loop until expired
while(!expired) {
    vTaskDelay(1);
}
} //




void app_main(void)
{
    
    static const char *TAG = "APP_MAIN";


    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("APP_MAIN", ESP_LOG_INFO);
    

    esp_log_level_set("DSPFAUST", ESP_LOG_VERBOSE);
    

    
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
    
    

    
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    //ESP_ERROR_CHECK(example_connect());
       
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    WM8978 wm8978;
    wm8978.init();
    wm8978.addaCfg(1,1); 
    wm8978.inputCfg(1,0,0);     
    wm8978.outputCfg(1,0); //da enabled ,  mic/linein/aux disabled
    wm8978.micGain(50);
    wm8978.auxGain(0);
    wm8978.lineinGain(0);
    wm8978.spkVolSet(0);
    wm8978.hpVolSet(hpVol_L,hpVol_R);
    wm8978.i2sCfg(2,0);
    ESP_LOGW(TAG,"WM8978 audio codec initialized");
    
 //   YOU MUST USE faust2api API calls
    int SR = 48000;
    int BS = 32; //was 8
    nbDelay(100);
    DSP = new DspFaust(SR,BS); 
    //nbDelay(100); //doesn't help
    DSP->start();
    //nbDelay(100); //doesn't help
    if (DSP->isRunning()) {
        ESP_LOGW(TAG,"DSP is running");            
            
    } else {
        ESP_LOGE(TAG,"DSP not running");

        } ;



    bool even = true;
    printf("                    ");
    ESP_LOGI(TAG,"--------------------"); 
         char TaskListBuffer[400];
   ESP_LOGI(TAG,     "Name	State	Priority	High Water	Stack Number");
       vTaskList( TaskListBuffer);
       ESP_LOGI(TAG,"\n\n%s", TaskListBuffer); 
    ESP_LOGI(TAG,"--------------------");   
    while(1) {
            if (even) {
                printf("\r                       ");
                printf("\r<<<<<Loop TIME>>>>>"); 
                } 
                else {
                 printf("\r                   ");
                 printf("\r>>>>>Loop TIME<<<<<");}; 
  
       
       while(play_flag){               
               play_poly_rtttl_chords(song,DSP); //OK
               //play_keys_nb(DSP);              // OK 
             
                }
                
                
   
            even = !even;
            vTaskDelay(500 / portTICK_PERIOD_MS);

        }; //while(play_flag)

    // Waiting forever, but code is never reached
    vTaskSuspend(nullptr);   
    
}
