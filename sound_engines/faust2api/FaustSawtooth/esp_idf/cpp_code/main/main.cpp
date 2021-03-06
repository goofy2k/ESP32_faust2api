#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "WM8978.h"
#include "DspFaust.h"

extern "C" {
    void app_main(void);
}

void app_main(void)
{

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
   
 //   YOU MUST USE faust2api API calls
    int SR = 48000;
    int BS = 32; //was 8
    printf("BEFORE DspFaust INSTANTIATION\n");
   // DspFaust dspFaust(SR,BS);
     // if (dspFaust.isRunning()) {printf("BEFORE START RUNNING\n");} else {printf("BEFORE START NOT RUNNING\n");} ;
   // dspFaust.start();
    
    //if (dspFaust.isRunning()) {printf("AFTER START RUNNING\n");} else {printf("AFTER START NOT RUNNING\n");} ;
    
    printf("Hello modified CHECKIT 1 world!\n");
    DspFaust* DSP = new DspFaust(SR,BS); 
    //printf("Hello modified 2x world!\n");
    if (DSP->isRunning()) {printf("BEFORE START RUNNING\n");} else {printf("BEFORE START NOT RUNNING\n");} ;

    DSP->start();
    if (DSP->isRunning()) {printf("AFTER START RUNNING\n");} else {printf("AFTER START NOT RUNNING\n");} ;
  
    //printf("Hello modified 3x world!\n");
    //DSP->setParamValue("freq",220);
    //DSP->keyOn(50,50);
    DSP->allNotesOff();



    
     float CPULoad = DSP->getCPULoad();
     printf("CPULoad %7.5f \n",CPULoad);
     
     float min0 = DSP->getParamMin(0);
     float max0 = DSP->getParamMax(0);
     float init0 = DSP->getParamInit(0);
     float min1 = DSP->getParamMin(1);
     float max1 = DSP->getParamMax(1);
     float init1 = DSP->getParamInit(1); 
     printf("min0 = %7.5f \n",min0);
     printf("max0 = %7.5f \n",max0);
     printf("init0 = %7.5f \n",init0);
     printf("min1 = %7.5f \n",min1);
     printf("max1 = %7.5f \n",max1);
     printf("init1 = %7.5f \n",init1);

    uintptr_t myvoice0 = DSP->newVoice();
    printf("myvoice0 = %u \n",myvoice0);
    
    //printf("myvoice0: %" PRIxPTR "\n", myvoice0);
    
    uintptr_t myvoice1 = DSP->newVoice();
    printf("myvoice1 = %u \n",myvoice1);
    //printf("myvoice1: %" PRIxPTR "\n", myvoice1);
    //printf("myvoice1: %PRIxPTR \n", myvoice1);
    //printf("The address of i is 0x%lx.\n", myvoice1);
    if (myvoice0 != 0) {
    DSP->setVoiceParamValue(0,myvoice0,220);  
    float freq0 = DSP-> getVoiceParamValue(0, myvoice0);
    printf("freq0 = %7.5f \n",freq0);
    float gain0 = DSP-> getVoiceParamValue(1, myvoice0);
    printf("gain0 = %7.5f \n",gain0);
    float freq1 = DSP-> getVoiceParamValue(0, myvoice1);
    printf("freq1 = %7.5f \n",freq1);
    float gain1 = DSP-> getVoiceParamValue(1, myvoice1);
    printf("gain1 = %7.5f \n",gain1);
    
    } else {
        printf("NO POLYPHONY \n");
        
        };

//DSP->setAccConverter(0, 0, 0, 0.1, 0.3, 1.1);


while(1) {
        printf("Loop \n");
  //      DSP->propagateAcc(0, rand()*1.0);
        DSP->setParamValue("freq",rand()%(2000-50 + 1) + 50);
        DSP->setParamValue("gain",0.1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        DSP->setParamValue("gain",0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
};



    /*
    while(1) {
        //printf("Hello modified 4x world!\n");
       // dspFaust.setParamValue("freq",rand()%(2000-50 + 1) + 50);
       DSP->setParamValue("freq",440);
     //   YOU MUST USE faust2api API calls
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    */
    // Waiting forever
    vTaskSuspend(nullptr);
    
}


/*
void propagateAcc(int acc, float v)
Propagate the RAW value of a specific accelerometer

axis to the Faust object.

Arguments
acc: the accelerometer axis (0: x, 1: y, 2: z)

v: the RAW acceleromter value in m/s

void setAccConverter(int p, int acc, int curve, float amin, float amid, float amax)
Set the conversion curve for the accelerometer. https://ccrma.stanford.edu/~rmichon/faustTutorials/

Arguments
id: the UI parameter id

acc: the accelerometer axis (0: x, 1: y, 2: z)

curve: the curve (0: up, 1: down, 2: up and down)

amin: mapping min point

amid: mapping middle point

amax: mapping max point

void propagateGyr(int gyr, float v)
Propagate the RAW value of a specific gyroscope

axis to the Faust object.

Arguments
gyr: the gyroscope axis (0: x, 1: y, 2: z)

v: the RAW accelerometer value in m/s

void setGyrConverter(int p, int gyr, int curve, float amin, float amid, float amax)
Set the conversion curve for the gyroscope.

Arguments
id: the UI parameter id

gyr: the gyroscope axis (0: x, 1: y, 2: z)

curve: the curve (0: up, 1: down, 2: up and down)

amin: mapping min point

amid: mapping middle point

amax: mapping max point
*/