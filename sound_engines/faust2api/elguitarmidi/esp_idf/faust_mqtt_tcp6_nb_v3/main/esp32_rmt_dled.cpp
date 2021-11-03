#ifdef __cplusplus
extern "C" {
#endif

#include "esp32_rmt_dled.h"

#include <stdint.h>
#include "esp_log.h"
#include "driver/rmt.h"
#include "soc/rmt_struct.h"
#include "driver/gpio.h"

static const char *LOG_TAG  = "rmt_dled";

/*
 * Standard APB clock (as needed by WiFi and BT to work) is 80 MHz.
 * This gives a 12.5 ns * rmt_config_t::clk_div for computing RMT durations, like this:
 *         duration = required_duration_in_ns / (12.5 * rmt_config_t::clk_div)
 */
const uint8_t  rmt_clk_divider  = 4;
const uint16_t rmt_clk_duration = 50; // ns,  12.5 * 4(=config.clk_div in rmt_config_for_digital_led_strip)

esp_err_t rmt_dled_init(rmt_pixel_strip_t *rps)
{
    if (rps == NULL) { return ESP_ERR_INVALID_ARG; }

    rps->strip = NULL;
    //rps->gpio = ;
    //rps->channel = ;
    rps->rmtLO.val = 0; rps->rmtHI.val = 0;
    rps->rmtLR.val = 0; rps->rmtHR.val = 0;
    rps->ugly_buffer = NULL;

    return ESP_OK;
}

esp_err_t rmt_dled_create(rmt_pixel_strip_t *rps, pixel_strip_t *strip)
{
	if (rps == NULL) {
		ESP_LOGE(LOG_TAG, "init: Argument is NULL");
		return ESP_ERR_INVALID_ARG;
	}
	if (strip == NULL) {
		ESP_LOGE(LOG_TAG, "init: Strip is NULL");
		return ESP_ERR_INVALID_ARG;
	}
	if (strip->length == 0) {
		ESP_LOGE(LOG_TAG, "init: Strip length is 0");
		return ESP_ERR_INVALID_SIZE;
	}

    rps->strip = strip;

    /* for every pixel are needed `8 * rps->strip->bytes_per_led` bits
     * for every bit is needed a `rmt_item32_t` */
    uint32_t req_length = rps->strip->length * 8 * rps->strip->bytes_per_led * sizeof(rmt_item32_t);
	rps->ugly_buffer = (rmt_item32_t*)malloc(req_length);
	if (rps->ugly_buffer == NULL){
		ESP_LOGE(LOG_TAG, "Failed to allocate memory for ugly buffer");
		return ESP_ERR_NO_MEM;
	}
    else {
        ESP_LOGI(LOG_TAG, "Allocated %d bytes for ugly_buffer", req_length);
    }

	rps->rmtLO.level0 = 1;
	rps->rmtLO.level1 = 0;
	rps->rmtLO.duration0 = strip->T0H / rmt_clk_duration;
	rps->rmtLO.duration1 = strip->T0L / rmt_clk_duration;

	rps->rmtHI.level0 = 1;
	rps->rmtHI.level1 = 0;
	rps->rmtHI.duration0 = strip->T1H / rmt_clk_duration;
	rps->rmtHI.duration1 = strip->T1L / rmt_clk_duration;

	rps->rmtLR.level0 = 1;
	rps->rmtLR.level1 = 0;
	rps->rmtLR.duration0 = strip->T0H / rmt_clk_duration;
	rps->rmtLR.duration1 = strip->TRS / rmt_clk_duration;

	rps->rmtHR.level0 = 1;
	rps->rmtHR.level1 = 0;
	rps->rmtHR.duration0 = strip->T1H / rmt_clk_duration;
	rps->rmtHR.duration1 = strip->TRS / rmt_clk_duration;

    return ESP_OK;
}

void rmt_dled_set_gpio(rmt_pixel_strip_t *rps)
{
    gpio_pad_select_gpio(rps->gpio_number);
    gpio_set_direction(  rps->gpio_number, GPIO_MODE_OUTPUT);
    gpio_set_level(      rps->gpio_number, 0);
}

esp_err_t rmt_dled_config(rmt_pixel_strip_t *rps, gpio_num_t gpio_number_in, rmt_channel_t channel_in)
{
	if (rps == NULL) {
		ESP_LOGE(LOG_TAG, "argument is NULL");
		return ESP_ERR_INVALID_ARG;
	}

    rps->gpio_number = gpio_number_in;
    rps->channel = channel_in;

    rmt_dled_set_gpio(rps);

    rmt_config_t config;

    config.rmt_mode = rmt_mode_t::RMT_MODE_TX;
    config.channel  = rps->channel;
    config.clk_div  = rmt_clk_divider;
    config.gpio_num = rps->gpio_number;

    /* One memory block is 64 words * 32 bits each; the type is rmt_item32_t (defined in rmt_struct.h).
    A channel can use more memory blocks by taking from the next channels, so channel 0 can have 8
    memory blocks and channel 7 just one. */
    config.mem_block_num = 1;

    config.tx_config.loop_en              = false;
    config.tx_config.carrier_en           = false;
    config.tx_config.carrier_freq_hz      = 0;
    config.tx_config.carrier_duty_percent = 0;
    config.tx_config.carrier_level        = rmt_carrier_level_t::RMT_CARRIER_LEVEL_LOW;
    config.tx_config.idle_level           = rmt_idle_level_t::RMT_IDLE_LEVEL_LOW;
    config.tx_config.idle_output_en       = true;

    // stop this rmt channel
    esp_err_t ret_val;

	ret_val = rmt_rx_stop(rps->channel);
    if(ret_val != ESP_OK) {
    	ESP_LOGE(LOG_TAG, "[0x%x] rmt_rx_stop failed", ret_val);
    	return ret_val;
    }
    ret_val = rmt_tx_stop(rps->channel);
    if(ret_val != ESP_OK) {
    	ESP_LOGE(LOG_TAG, "[0x%x] rmt_tx_stop failed", ret_val);
    	return ret_val;
    }

    // disable rmt interrupts for this channel
    rmt_set_rx_intr_en(    rps->channel, 0);
    rmt_set_err_intr_en(   rps->channel, 0);
    rmt_set_tx_intr_en(    rps->channel, 0);
    rmt_set_tx_thr_intr_en(rps->channel, 0, 0xffff);
    // set rmt memory to normal (not power-down) mode
    ret_val = rmt_set_mem_pd(rps->channel, false);
    if(ret_val != ESP_OK) {
    	ESP_LOGE(LOG_TAG, "[0x%x] rmt_set_mem_pd failed", ret_val);
    	return ret_val;
    }

    /* The rmt_config function internally:
     * - enables the RMT module by calling periph_module_enable(PERIPH_RMT_MODULE);
     * - sets data mode with rmt_set_data_mode(RMT_DATA_MODE_MEM);
     * - associates the gpio pin with the rmt channel using rmt_set_pin(channel, mode, gpio_num);
     */
    ret_val = rmt_config(&config);
    if(ret_val != ESP_OK) {
    	ESP_LOGE(LOG_TAG, "[0x%x] rmt_config failed", ret_val);
    	return ret_val;
    }

    ret_val = rmt_driver_install(rps->channel, 0, 0);
    if(ret_val != ESP_OK) {
    	ESP_LOGE(LOG_TAG, "[0x%x] rmt_driver_install failed", ret_val);
    	return ret_val;
    }

    return ESP_OK;
}

void rmt_dled_byte_to_rmtitem(rmt_pixel_strip_t *rps, uint8_t data, uint16_t idx_in)
{
	uint8_t mask = 0x80;
	uint16_t idx = idx_in;

	while (mask != 0){
		rps->ugly_buffer[idx++] =
			((data & mask) != 0) ? rps->rmtHI : rps->rmtLO;
		mask = mask >> 1;
	}
}

esp_err_t rmt_dled_send(rmt_pixel_strip_t *rps)
{
	if (rps == NULL) {
		ESP_LOGE(LOG_TAG, "argument is NULL");
		return ESP_ERR_INVALID_ARG;
	}
	if (rps->ugly_buffer == NULL) {
		ESP_LOGE(LOG_TAG, "ugly buffer is NULL");
		return ESP_ERR_INVALID_ARG;
	}
	if (rps->strip == NULL) {
		ESP_LOGE(LOG_TAG, "buffer is NULL");
		return ESP_ERR_INVALID_ARG;
	}
	if (rps->strip->buffer == NULL) {
		ESP_LOGE(LOG_TAG, "buffer is NULL");
		return ESP_ERR_INVALID_ARG;
	}
	if (rps->strip->buffer_length == 0) {
		ESP_LOGE(LOG_TAG, "buffer length is 0");
		return ESP_ERR_INVALID_ARG;
	}

	uint16_t didx = 0;
	for (uint16_t i = 0; i < rps->strip->buffer_length; i++) {
		rmt_dled_byte_to_rmtitem(rps, rps->strip->buffer[i], didx);
		didx += 8;
	}

	// change last bit to include reset time
	didx--;
	if (rps->ugly_buffer[didx].val == rps->rmtHI.val) {
		rps->ugly_buffer[didx] = rps->rmtHR;
	}
	else {
		rps->ugly_buffer[didx] = rps->rmtLR;
	}

	esp_err_t ret_val = rmt_write_items(rps->channel, rps->ugly_buffer, rps->strip->buffer_length * 8, true);
    if(ret_val != ESP_OK) {
    	ESP_LOGE(LOG_TAG, "[0x%x] rmt_write_items failed", ret_val);
    	return ret_val;
    }

    return ESP_OK;
}

#ifdef __cplusplus
}
#endif
