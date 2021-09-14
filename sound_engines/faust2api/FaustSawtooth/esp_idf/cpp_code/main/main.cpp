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
    //DspFaust dspFaust(SR,BS);
     //dspFaust.start();
    printf("Hello modified CHECK 1 world!\n");
    DspFaust* DSP = new DspFaust(SR,BS); 
    //printf("Hello modified 2x world!\n");
    DSP->start();
    printf("Hello modified 3x world!\n");
    while(1) {
        //printf("Hello modified 4x world!\n");
       // dspFaust.setParamValue("freq",rand()%(2000-50 + 1) + 50);
       DSP->setParamValue("freq",440);
     //   YOU MUST USE faust2api API calls
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}