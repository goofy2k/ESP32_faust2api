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
    uintptr_t myvoice0 = DSP->newVoice();
     uintptr_t myvoice1 = DSP->newVoice();
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

    float freq0 = DSP-> getVoiceParamValue(0, 0);
    float gain0 = DSP-> getVoiceParamValue(1, 0);
    float freq1 = DSP-> getVoiceParamValue(0, 1);
    float gain1 = DSP-> getVoiceParamValue(1, 1);
    printf("freq0 = %7.5f \n",freq0);
    printf("gain0 = %7.5f \n",gain0);
    printf("freq1 = %7.5f \n",freq1);
    printf("gain1 = %7.5f \n",gain1);


    //DSP->setVoiceParamValue(0,myvoice0,220);  //RUNTIME ERROR
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