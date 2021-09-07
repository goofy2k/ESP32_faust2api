#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "WM8978.h"
#include "FaustSawtooth.h"

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

    int SR = 48000;
    int BS = 8;
    FaustSawtooth faustSawtooth(SR,BS);  
    faustSawtooth.start();

    while(1) {
        faustSawtooth.setParamValue("freq",rand()%(2000-50 + 1) + 50);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}