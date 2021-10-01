/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
//#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "esp_spi_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "WM8978.h"
#include "DspFaust.h"
#include "secrets.h"


/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
/*
//Credentials from menuconfig
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY
*/
//Credentials form secrets.h
#define EXAMPLE_ESP_WIFI_SSID      SECRET_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      SECRET_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY  SECRET_ESP_MAXIMUM_RETRY





extern "C" {           //FCKX
    void app_main(void);
}


extern "C" {           //FCKX
    static esp_mqtt_client * mqtt_app_start(void);
}

extern "C" {           //FCKX
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event);
}

/*

extern "C" {
    static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);
}

extern "C" {
    void wifi_init_sta(void);
}

*/

DspFaust* DSP;
WM8978 wm8978;
int hpVol_L = 40;
int hpVol_R = 40;
float lfoFreq = 10;
float lfoDepth = 0.25;
float synthA = 10;
float synthD = 1;
float synthS = 0.01;
float synthR = 1.0;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station MQTT Faust";

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data){
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    esp_event_loop_delete_default(); //FCKX
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    /*
    wifi_config_t wifi_config = {
        .sta = {
            //.ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid = "Verhoeckx_glas",
            //.password = EXAMPLE_ESP_WIFI_PASS,
            .password = "Goofy2kmacho_99",
            //Setting a password implies station will connect to all security modes including WEP/WPA.
            // However these modes are deprecated and not advisable to be used. Incase your Access point
            // doesn't support WPA2, these mode can be enabled by commenting below line
	     .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    
    */
    
    //https://esp32.com/viewtopic.php?t=1317
    //You can init structs in C++ but all elements of the struct has to be included in the initialization in the order they are declared as you can not reference properties by name. 
    //Luckily, in C++, the struct can be initialized with {} and each property assigned separately.
    
    wifi_config_t wifi_config = {};  
    strcpy((char*)wifi_config.sta.ssid, EXAMPLE_ESP_WIFI_SSID );
    strcpy((char*)wifi_config.sta.password, EXAMPLE_ESP_WIFI_PASS);
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

void update_controls(){  //do this in a cyclic freertos task
    
  wm8978.hpVolSet(hpVol_R, hpVol_L);
  DSP->setParamValue("/WaveSynth_FX/lfoFreq",lfoFreq);
  DSP->setParamValue("/WaveSynth_FX/lfoDepth",lfoDepth); 
  DSP->setParamValue("/WaveSynth_FX/A",synthA);
  DSP->setParamValue("/WaveSynth_FX/D",synthD); 
  DSP->setParamValue("/WaveSynth_FX/S",synthS); 
  DSP->setParamValue("/WaveSynth_FX/R",synthR); 
  
  
    
}

//static const char *TAG = "MQTT_EXAMPLE";



static void call_faust_api(esp_mqtt_event_handle_t event){
    printf("HANDLING FAUST API CALL=%s\n"," /faust/api");
    if (strncmp(event->topic, "/faust/api/DspFaust",strlen("/faust/api/DspFaust")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else
    
    if (strncmp(event->topic, "/faust/api/DspFaust",strlen("/faust/api/DspFaust")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/start",strlen("/faust/api/start")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/stop",strlen("/faust/api/stop")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/isRunning",strlen("/faust/api/isRunning")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/keyOn",strlen("/faust/api/keyOn")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/keyOff",strlen("/faust/api/keyOff")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/newVoice",strlen("/faust/api/newVoice")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/deleteVoice",strlen("/faust/api/deleteVoice")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/allNotesOff",strlen("/faust/api/allNotesOff")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/propagateMidi",strlen("/faust/api/propagateMidi")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/getJSONUI",strlen("/faust/api/getJSONUI")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/getJSONMeta",strlen("/faust/api/getJSONMeta")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/buildUserInterface",strlen("/faust/api/buildUserInterface")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/getParamsCount",strlen("/faust/api/getParamsCount")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/setParamValue",strlen("/faust/api/SetParamValue")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/getParamValue",strlen("/faust/api/getParamValue")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/setVoiceParamValue",strlen("/faust/api/setVoiceParamValue")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else     
    if (strncmp(event->topic, "/faust/api/getVoiceParamValue",strlen("/faust/api/getVoiceParamValue")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/getParamAddress",strlen("/faust/api/getParamAddress")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/getVoiceParamAddress",strlen("/faust/api/getVoiceParamAddress")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else      
    if (strncmp(event->topic, "/faust/api/DspFaust",strlen("/faust/api/DspFaust")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/DspFaust",strlen("/faust/api/DspFaust")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/getParamMin",strlen("/faust/api/getParamMin")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/getParamMax",strlen("/faust/api/getParamMax")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/getParamInit",strlen("/faust/api/getParamInit")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/getMetaData",strlen("/faust/api/getMetaData")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/propagateAcc",strlen("/faust/api/propagateAcc")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/setAccConverter",strlen("/faust/api/setAccCoverter")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/propagateGyr",strlen("/faust/api/propagateGyr")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/setGyrConverter",strlen("/faust/api/setGyrCoverter")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/getCPULoad",strlen("/faust/api/getCPULoad")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else    
    if (strncmp(event->topic, "/faust/api/configureOSC",strlen("/faust/api/configureOSC")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else 
    if (strncmp(event->topic, "/faust/api/isOSCOn",strlen("/faust/api/isOSCOn")) == 0) {
             printf("HANDLING FAUST API CALL: COMMAND= %s\n",event->topic);
    } else       

    {
        printf("HANDLING FAUST API CALL: INVALID COMMAND= %s\n",event->topic);
          }            
    }


static void call_faust_api2(esp_mqtt_event_handle_t event){
    printf("HANDLING FAUST API2 CALL=%s\n"," /faust/api2");
    if (strncmp(event->topic, "/faust/api2/rtttl",strlen("/faust/api2/rtttl")) == 0) {
             printf("HANDLING FAUST API2 CALL: COMMAND= %s\n",event->topic);
    } else
    if (strncmp(event->topic, "/faust/api2/gate",strlen("/faust/api2/gate")) == 0) {
             printf("HANDLING FAUST API2 CALL: COMMAND= %s\n",event->topic);   
    } else 
    if (strncmp(event->topic, "/faust/api2/wm8978/hpVol/left",strlen("/faust/api2/wm8978/hpVol/left")) == 0)
        {
             printf("HANDLING FAUST API2 CALL: COMMAND= %s\n",event->topic);
             printf("SETTING hpVol LEFT to: %s\n",event->data);  
             hpVol_L = atoi(event->data);             

             
    } else  
    if (strncmp(event->topic, "/faust/api2/wm8978/hpVol/right",strlen("/faust/api2/wm8978/hpVol/right")) == 0)
        {
             printf("HANDLING FAUST API2 CALL: COMMAND= %s\n",event->topic);
             printf("SETTING hpVol RIGHT to: %s\n",event->data);
             hpVol_R = atoi(event->data);
             
    } else  

    if (strncmp(event->topic, "/faust/api2/DSP/synthA",strlen("/faust/api2/DSP/synthA")) == 0)
        {
             printf("HANDLING FAUST API2 CALL: COMMAND= %s\n",event->topic);
             printf("SETTING synthR to: %s\n",event->data);
             synthA = atof(event->data);
             
    } else    
    if (strncmp(event->topic, "/faust/api2/DSP/synthD",strlen("/faust/api2/DSP/synthD")) == 0)
        {
             printf("HANDLING FAUST API2 CALL: COMMAND= %s\n",event->topic);
             printf("SETTING synthD to: %s\n",event->data);
             synthD = atof(event->data);
             
    } else    
        
    if (strncmp(event->topic, "/faust/api2/DSP/synthS",strlen("/faust/api2/DSP/synthS")) == 0)
        {
             printf("HANDLING FAUST API2 CALL: COMMAND= %s\n",event->topic);
             printf("SETTING synthS to: %s\n",event->data);
             synthS = atof(event->data);
             
    } else    



    if (strncmp(event->topic, "/faust/api2/DSP/synthR",strlen("/faust/api2/DSP/synthR")) == 0)
        {
             printf("HANDLING FAUST API2 CALL: COMMAND= %s\n",event->topic);
             printf("SETTING synthR to: %s\n",event->data);
             synthR = atof(event->data);
             
    } else         

    if (strncmp(event->topic, "/faust/api2/DSP/lfoFreq",strlen("/faust/api2/DSP/lfoFreq")) == 0)
        {
             printf("HANDLING FAUST API2 CALL: COMMAND= %s\n",event->topic);
             printf("SETTING lfoFreq to: %s\n",event->data);
             lfoFreq = atof(event->data);
             
    } else    

    if (strncmp(event->topic, "/faust/api2/DSP/lfoDepth",strlen("/faust/api2/DSP/lfoDepth")) == 0)
        {
             printf("HANDLING FAUST API2 CALL: COMMAND= %s\n",event->topic);
             printf("SETTING lfoDepth to: %s\n",event->data);
             lfoDepth = atof(event->data);
             
    } else    


        
    {
             printf("HANDLING FAUST API2 CALL: INVALID COMMAND= %s\n",event->topic);
          }            
    }


static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event){  
    //ESP_LOGI(TAG, "FCKX: HANDLER_CB CALLED");  //FCKX
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    int result;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
            ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
            
            msg_id = esp_mqtt_client_subscribe(client, "/FCKX/test", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            //if topic starts with /faust/api or /faust/api2, call a command dispatcher/handler
            //code here
           
   //result =  strncmp(event->topic, "/faust/api2/gate",event->topic_len); //OK
            //printf("result=%d\n", result);
            /*
            result =  strncmp("/faust/api2/gate",event->topic,event->topic_len);  //OK
            printf("result=%d\n", result);
            result =  strcmp("/faust/api2/gate",event->topic);                    //NOK    
            printf("result=%d\n", result);
            printf("topic=%s\n", event->topic);             
            if (result == 0 ) {ESP_LOGI(TAG, "HIT TOPIC = /faust/api2/gate");};
            */
         //   if (strncmp(event->topic, "/faust/api",event->topic_len)==0) {ESP_LOGI(TAG, "HIT TOPIC = /faust/api");} 
         //   else
              //if (result == 0) {ESP_LOGI(TAG, "HIT TOPIC = /faust/api2/gate");}; 
    //if (result == 0) {printf("HIT result=%d\n",result);};
 //   if (strncmp(event->topic, "/faust/api2/gate",event->topic_len) == 0) {
     if (strncmp(event->topic, "/faust/api2",strlen("/faust/api2")) == 0) {
        printf("HIT result=%s\n"," /faust/api2/gate");
        call_faust_api2(event);
    
    }
    //  else  if (strncmp(event->topic, "/faust/api",event->topic_len) == 0) {
        else  if (strncmp(event->topic, "/faust/api",strlen("/faust/api")) == 0) {
          printf("HIT result=%s\n"," /faust/api");
          call_faust_api(event);
      };

          //else
           //      {ESP_LOGI(TAG, "other HIT TOPIC");}
            //if topic starts with .....
            //if (my_topic.rfind("/faust/api2",0) == 0 ) {ESP_LOGI(TAG, "HIT TOPIC starts with /faust/api2");};
            //else
                
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}
//throws a compilation error! May be obsolete when registering is done i a different way

/*
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {

    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}
*/

//FCKX guess
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, esp_mqtt_event_handle_t event_data) {
    
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

/*
../main/main.cpp: In function 'void mqtt_event_handler(void*, esp_event_base_t, int32_t, void*)':
../main/main.cpp:94:27: error: invalid conversion from 'void*' to 'esp_mqtt_event_handle_t' {aka 'esp_mqtt_event_t*'} [-fpermissive]
     mqtt_event_handler_cb(event_data);
                           ^~~~~~~~~~
*/

/*
//https://github.com/espressif/esp-idf/issues/5248

@mntolia This is an issue with C++ build, has been fixed in espressif/esp-mqtt@8a1e1a5, but apparently that hasn't made it yet to arduino. As a workaround you can cast the variables to the expected types or even avoid using an event loop altogether.
It is still possible to configure mqtt_event_handler_cb() as a plain callback in client config:

    mqtt_config.event_handle = mqtt_event_handler_cb;
And remove both definition of the mqtt_event_handler() and registration of the handler esp_mqtt_client_register_event()

*/

static esp_mqtt_client * mqtt_app_start(void){
//static void mqtt_app_start(void){
    /*
    esp_mqtt_client_config_t mqtt_cfg = {
    //.uri = CONFIG_BROKER_URL,
    .uri = "mqtt://goofy2knet.synology.me",
    .username = "Fred",
    .password = "Nwwnlil_12",
    .client_id = "TTGO_TAudio_1"
    };
    */
       
    //FCKX
    esp_mqtt_client_config_t mqtt_cfg = {0};
    // mqtt_cfg.uri = CONFIG_BROKER_URL;
    //mqtt_cfg.uri = "mqtt://goofy2knet.synology.me";
    //mqtt_cfg.uri = "mqtt://192.168.2.200:1883";
    mqtt_cfg.uri = SECRET_ESP_MQTT_BROKER_URI;
    //strcpy((char*)mqtt_cfg.uri, "mqtt://goofy2knet.synology.me");
    //strcpy((char*)mqtt_cfg.username, "Fred");
    //mqtt_cfg.username = "Fred";
    mqtt_cfg.username = SECRET_ESP_MQTT_BROKER_USERNAME;
    //strcpy((char*)mqtt_cfg.password, "Nwwnlil_12");
    //mqtt_cfg.password = "Nwwnlil_12";
    mqtt_cfg.password = SECRET_ESP_MQTT_BROKER_PASSWORD;
    //strcpy((char*)mqtt_cfg.client_id, "TTGO_TAudio_1");
    //mqtt_cfg.client_id = "TTGO_TAudio_1";
    mqtt_cfg.client_id = SECRET_ESP_MQTT_CLIENT_ID;
    
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    //mqtt_cfg.event_handle = mqtt_event_handler_cb;
    //esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, (esp_event_handler_t)mqtt_event_handler, client);  //trial, added 2x type casting 
  
    ESP_LOGI(TAG, "START MQTT CLIENT");  //FCKX
    esp_mqtt_client_start(client);
    ESP_LOGI(TAG, "MQTT CLIENT STARTED");  //FCKX
    
    return client;
}

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
char *song = "GoodBad:d=4,o=5,b=56:32p,32a#,32d#6,32a#,32d#6,8a#.,16f#.,16g#.,d#,32a#,32d#6,32a#,32d#6,8a#.,16f#.,16g#.,c#6,32a#,32d#6,32a#,32d#6,8a#.,16f#.,32f.,32d#.,c#,32a#,32d#6,32a#,32d#6,8a#.,16g#.,d#";
//char *song = "TopGun:d=4,o=4,b=31:32p,16c#,16g#,16g#,32f#,32f,32f#,32f,16d#,16d#,32c#,32d#,16f,32d#,32f,16f#,32f,32c#,16f,d#,16c#,16g#,16g#,32f#,32f,32f#,32f,16d#,16d#,32c#,32d#,16f,32d#,32f,16f#,32f,32c#,g#";
//char *song = "A-Team:d=8,o=5,b=125:4d#6,a#,2d#6,16p,g#,4a#,4d#.,p,16g,16a#,d#6,a#,f6,2d#6,16p,c#.6,16c6,16a#,g#.,2a#";
//char *song = "Flinstones:d=4,o=5,b=40:32p,16f6,16a#,16a#6,32g6,16f6,16a#.,16f6,32d#6,32d6,32d6,32d#6,32f6,16a#,16c6,d6,16f6,16a#.,16a#6,32g6,16f6,16a#.,32f6,32f6,32d#6,32d6,32d6,32d#6,32f6,16a#,16c6,a#,16a6,16d.6,16a#6,32a6,32a6,32g6,32f#6,32a6,8g6,16g6,16c.6,32a6,32a6,32g6,32g6,32f6,32e6,32g6,8f6,16f6,16a#.,16a#6,32g6,16f6,16a#.,16f6,32d#6,32d6,32d6,32d#6,32f6,16a#,16c.6,32d6,32d#6,32f6,16a#,16c.6,32d6,32d#6,32f6,16a#6,16c7,8a#.6";
//char *song = "Jeopardy:d=4,o=6,b=125:c,f,c,f5,c,f,2c,c,f,c,f,a.,8g,8f,8e,8d,8c#,c,f,c,f5,c,f,2c,f.,8d,c,a#5,a5,g5,f5,p,d#,g#,d#,g#5,d#,g#,2d#,d#,g#,d#,g#,c.7,8a#,8g#,8g,8f,8e,d#,g#,d#,g#5,d#,g#,2d#,g#.,8f,d#,c#,c,p,a#5,p,g#.5,d#,g#";
//char *song = "Gadget:d=16,o=5,b=50:32d#,32f,32f#,32g#,a#,f#,a,f,g#,f#,32d#,32f,32f#,32g#,a#,d#6,4d6,32d#,32f,32f#,32g#,a#,f#,a,f,g#,f#,8d#";
//char *song = "Smurfs:d=32,o=5,b=200:4c#6,16p,4f#6,p,16c#6,p,8d#6,p,8b,p,4g#,16p,4c#6,p,16a#,p,8f#,p,8a#,p,4g#,4p,g#,p,a#,p,b,p,c6,p,4c#6,16p,4f#6,p,16c#6,p,8d#6,p,8b,p,4g#,16p,4c#6,p,16a#,p,8b,p,8f,p,4f#";
//char *song = "MahnaMahna:d=16,o=6,b=125:c#,c.,b5,8a#.5,8f.,4g#,a#,g.,4d#,8p,c#,c.,b5,8a#.5,8f.,g#.,8a#.,4g,8p,c#,c.,b5,8a#.5,8f.,4g#,f,g.,8d#.,f,g.,8d#.,f,8g,8d#.,f,8g,d#,8c,a#5,8d#.,8d#.,4d#,8d#.";
//char *song = "LeisureSuit:d=16,o=6,b=56:f.5,f#.5,g.5,g#5,32a#5,f5,g#.5,a#.5,32f5,g#5,32a#5,g#5,8c#.,a#5,32c#,a5,a#.5,c#.,32a5,a#5,32c#,d#,8e,c#.,f.,f.,f.,f.,f,32e,d#,8d,a#.5,e,32f,e,32f,c#,d#.,c#";
//char *song = "MissionImp:d=16,o=6,b=95:32d,32d#,32d,32d#,32d,32d#,32d,32d#,32d,32d,32d#,32e,32f,32f#,32g,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,a#,g,2d,32p,a#,g,2c#,32p,a#,g,2c,a#5,8c,2p,32p,a#5,g5,2f#,32p,a#5,g5,2f,32p,a#5,g5,2e,d#,8d";

#define isdigit(n) (n >= '0' && n <= '9')

/*
//keyOn  keyOff not active.... as other MIDI functionality?

void play_keys(DspFaust * aDSP)
{
        for (int ii = 50; ii < 100; ii++){
        aDSP->keyOn(50,50);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        aDSP->keyOff(50);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        }  
}
*/


void play_poly_without_midi(DspFaust * aDSP) 
{
    uintptr_t voiceAddress = aDSP->newVoice(); //create main voice
        for (int ii = 50; ii < 100; ii++){

           aDSP->setVoiceParamValue("/Polyphonic/Voices/WaveSynth_FX/freq",voiceAddress,440.0);
           // aDSP->setVoiceParamValue("/WaveSynth_FX/freq",freqs[(scale-4) * 12 + note]);
           aDSP->setVoiceParamValue("/Polyphonic/Voices/WaveSynth_FX/gate",voiceAddress,1.0);
           vTaskDelay(500 / portTICK_PERIOD_MS);          
           aDSP->setVoiceParamValue("/Polyphonic/Voices/WaveSynth_FX/gate",voiceAddress,0);
           vTaskDelay(500 / portTICK_PERIOD_MS); 
           
           aDSP->setVoiceParamValue("/Polyphonic/Voices/WaveSynth_FX/freq",voiceAddress,220.1);
           // aDSP->setVoiceParamValue("/WaveSynth_FX/freq",freqs[(scale-4) * 12 + note]);
           aDSP->setVoiceParamValue("/Polyphonic/Voices/WaveSynth_FX/gate",voiceAddress,1);
           vTaskDelay(500 / portTICK_PERIOD_MS);          
           aDSP->setVoiceParamValue("/Polyphonic/Voices/WaveSynth_FX/gate",voiceAddress,0);
           vTaskDelay(500 / portTICK_PERIOD_MS);
        }
           aDSP->deleteVoice(voiceAddress); //delete main voice
};


void play_mono_rtttl(char *p, DspFaust * aDSP)
{
 // aDSP->allNotesOff();  
  aDSP->setParamValue("/WaveSynth_FX/lfoFreq",lfoFreq);
  aDSP->setParamValue("/WaveSynth_FX/lfoDepth",lfoDepth); 
  aDSP->setParamValue("/WaveSynth_FX/A",synthA);
  aDSP->setParamValue("/WaveSynth_FX/D",synthD); 
  aDSP->setParamValue("/WaveSynth_FX/S",synthS); 
  aDSP->setParamValue("/WaveSynth_FX/R",synthR); 
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
       
   //   Serial.print("Playing: ");
   //   Serial.print(scale, 10); Serial.print(' ');
   //   Serial.print(note, 10); Serial.print(" (");
   //   Serial.print(notes[(scale - 4) * 12 + note], 10);
   //   Serial.print(") ");
   //   Serial.println(duration, 10);
      /*
      aDSP->setParamValue("/elecGuitar/midi/freq",freqs[(scale-4) * 12 + note]);
      aDSP->setParamValue("/elecGuitar/gate",1);
     */
/* 
      aDSP->setParamValue("/simpleSynt_Analog/freq",freqs[(scale-4) * 12 + note]);
      aDSP->setParamValue("/simpleSynt_Analog/gate",1);     
*/

      aDSP->setParamValue("/WaveSynth_FX/freq",freqs[(scale-4) * 12 + note]);
      aDSP->setParamValue("/WaveSynth_FX/gate",1); 

      
      vTaskDelay(duration / portTICK_PERIOD_MS);
      //printf("%s \n",DSP->getJSONUI());
      //DSP->setParamValue("gain",0);
    //  aDSP->setParamValue("/simpleSynt_Analog/gate",0); 
        // aDSP->setParamValue("/elecGuitar/gate",0);
        aDSP->setParamValue("/WaveSynth_FX/gate",0); 
     // tone1.play(notes[(scale - 4) * 12 + note]);
      
      update_controls();  //later do this in a freertos task 
      
      //delay(duration);
      //tone1.stop();
      
    }
    else
    {
      //Serial.print("Pausing: ");
      //Serial.println(duration, 10);
      vTaskDelay(5*duration/ portTICK_PERIOD_MS);
    }
  }  //while (p*)
  } //song player    
 


void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

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

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    esp_mqtt_client_handle_t  mqtt_client =  mqtt_app_start();
    int msg_id;
    
    msg_id = esp_mqtt_client_publish(mqtt_client, "/faust", "init", 0, 0, 0);
    WM8978 wm8978;
    wm8978.init();
    wm8978.addaCfg(1,1); 
    wm8978.inputCfg(1,0,0);     
    wm8978.outputCfg(1,0); 
    wm8978.micGain(30);
    wm8978.auxGain(0);
    wm8978.lineinGain(0);
    wm8978.spkVolSet(0);
    wm8978.hpVolSet(hpVol_L,hpVol_R);
    wm8978.i2sCfg(2,0);
   
 //   YOU MUST USE faust2api API calls
    int SR = 48000;
    int BS = 32; //was 8
   
   // DspFaust dspFaust(SR,BS);
     // if (dspFaust.isRunning()) {printf("BEFORE START RUNNING\n");} else {printf("BEFORE START NOT RUNNING\n");} ;
   // dspFaust.start();
    
    //if (dspFaust.isRunning()) {printf("AFTER START RUNNING\n");} else {printf("AFTER START NOT RUNNING\n");} ;
    
   // printf("Hello modified CHECKIT 1 world!\n");
   // printf("BEFORE DspFaust INSTANTIATION\n");
    DSP = new DspFaust(SR,BS); 
    //printf("Hello modified 2x world!\n");
   // if (DSP->isRunning()) {printf("BEFORE START RUNNINGa\n");} else {printf("BEFORE START NOT RUNNINGb\n");} ;

    DSP->start();
    if (DSP->isRunning()) {printf("DSP is running");} else {printf("AFTER START NOT RUNNINGd\n");} ;
  /*
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
        printf("myvoice0 created\n");
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
        printf("Could not create myvoice0 \n");
        
        };

*/

/*
Main Parameters
0: /Polyphonic/Voices/Panic
1: /Polyphonic/Voices/elecGuitar/midi/freq
2: /Polyphonic/Voices/elecGuitar/midi/bend
3: /Polyphonic/Voices/elecGuitar/midi/gain
4: /Polyphonic/Voices/elecGuitar/midi/sustain
5: /Polyphonic/Voices/elecGuitar/pluckPosition
6: /Polyphonic/Voices/elecGuitar/outGain
7: /Polyphonic/Voices/elecGuitar/gate
Independent Voice
0: /elecGuitar/gate
1: /elecGuitar/midi/bend
2: /elecGuitar/midi/freq
3: /elecGuitar/midi/gain
4: /elecGuitar/midi/sustain
5: /elecGuitar/outGain
6: /elecGuitar/pluckPosition
*/


//msg_id = esp_mqtt_client_subscribe(mqtt_client, "/faust/gate", 1);
//ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
msg_id = esp_mqtt_client_subscribe(mqtt_client, "/faust/api/#", 1);
ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
msg_id = esp_mqtt_client_subscribe(mqtt_client, "/faust/api2/#", 1);
ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);


msg_id = esp_mqtt_client_publish(mqtt_client, "/faust/jsonui", DSP->getJSONUI(), 0, 0, 0);  //to be implemented: publish the UI by remote request via th
while(1){
msg_id = esp_mqtt_client_publish(mqtt_client, "/faust", "rtttl song loop started", 0, 0, 0);
play_mono_rtttl(song, DSP); //try to implement playing a song in a separate task
msg_id = esp_mqtt_client_publish(mqtt_client, "/faust", "rtttl song loop finished", 0, 0, 0);
}

/*
msg_id = esp_mqtt_client_publish(mqtt_client, "/faust", "keys song loop started", 0, 0, 0);
play_poly_keys( DSP); //try to implement playing a song in a separate task
msg_id = esp_mqtt_client_publish(mqtt_client, "/faust", "keys song loop finished", 0, 0, 0);
*/

/*
msg_id = esp_mqtt_client_publish(mqtt_client, "/faust", "poly_without_midi song loop started", 0, 0, 0);
play_poly_without_midi(DSP); 
msg_id = esp_mqtt_client_publish(mqtt_client, "/faust", "poly_without_midi song loop finished", 0, 0, 0);
*/
while(1) {
        printf(">>>>>Loop<<<<<< \n");
        //printf("%s \n",DSP->getJSONUI());
        /*
        DSP->keyOn(50,50);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        DSP->keyOff(50);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        DSP->keyOn(100,50);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        DSP->keyOff(100);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        */
        
        
     //   DSP->setParamValue("freq",rand()%(2000-50 + 1) + 50);
      //  DSP->setParamValue("gain",0.1);
        
        /*
        DSP->setParamValue("/elecGuitar/midi/freq",rand()%(2000-50 + 1) + 50);
        DSP->setParamValue("/elecGuitar/gate",1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        printf("%s \n",DSP->getJSONUI());
        //DSP->setParamValue("gain",0);
        DSP->setParamValue("/elecGuitar/gate",0);
        */
        
        vTaskDelay(500 / portTICK_PERIOD_MS);
   

  
  //play_rtttl(song, DSP);
  

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
