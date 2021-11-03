#ifndef MAIN_ESP32_RMT_DLED_H_
#define MAIN_ESP32_RMT_DLED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "driver/rmt.h"
#include "soc/rmt_struct.h"

#include "dled_strip.h"

/**
 * @brief Structure to control a LED strip using the RMT peripheral
 *
 */
typedef struct {
    pixel_strip_t *strip;       /*!< The pixels associated to the LED strip */

	gpio_num_t    gpio_number;  /*!< The number of GPIO connected to the LED strip */
	rmt_channel_t channel;      /*!< The RMT channel to control the LED strip */
	rmt_item32_t  rmtLO, rmtHI; /*!< Values required to send 0 and 1 */
    rmt_item32_t  rmtLR, rmtHR; /*!< Values required to send 0 and 1 including reset */

	rmt_item32_t  *ugly_buffer; /*!< The buffer to be passed to the RMT driver for sending */
} rmt_pixel_strip_t;

/**
 * @brief Initialize a rmt_pixel_strip_t structure.
 *
 * @param[in,out] strip The structure to be initialized.
 *
 * @return
 *    - ESP_OK success
 *    - ESP_ERR_INVALID_ARG if the `rps` argument is NULL
 */
esp_err_t rmt_dled_init(rmt_pixel_strip_t *rps);

/**
 * @brief Creates the buffer and set the `rmt_item32_t` members of a rmt_pixel_strip_t structure.
 *
 * Creates the buffer to be passed to the RMT driver for sending.
 * Based on `strip` sets rmtLO, rmtHI, rmtLR and rmtHR members.
 *
 * @param[in,out] rps   The structure to work with.
 * @param[in]     strip The strip of pixels.
 *
 * @return
 *    - ESP_OK success
 *    - ESP_ERR_INVALID_ARG if the `rps` or `strip` arguments are NULL
 *    - ESP_ERR_INVALID_SIZE if `strip->length` is zero
 *    - ESP_ERR_NO_MEM if failed to allocate memory for buffer
 */
esp_err_t rmt_dled_create(rmt_pixel_strip_t *rps, pixel_strip_t *strip);

/**
 * @brief Configures the RMT peripheral
 *
 * Configures the GPIO `gpio_number`
 * Configure the RMT peripheral using the RMT driver.
 * Sets `gpio_number` and `channel` members.
 *
 * @param[in,out] rps         The structure to work with.
 * @param[in]     gpio_number The number of GPIO connected to the LED strip.
 * @param[in]     channel     The RMT channel to control the LED strip.
 *
 * @return
 *    - ESP_OK success
 *    - ESP_ERR_INVALID_ARG if the `rps` argument is NULL
 *    - the error codes returned by the RMT driver, if error
 */
esp_err_t rmt_dled_config(rmt_pixel_strip_t *rps, gpio_num_t gpio_number, rmt_channel_t channel);

/**
 * @brief Send data to RMT driver
 *
 * @attention: Call dled_strip_fill_buffer(rps->strip) before calling this function
 * because it will transfer data from strip->pixels to strip->buffer !
 *
 * @param[in,out] rps The structure to work with.
 *
 * @return
 *    - ESP_OK success
 *    - ESP_ERR_INVALID_ARG if the `rps` argument is NULL
 *    - ESP_ERR_INVALID_ARG if `rps->strip` or a buffer is NULL
 *    - ESP_ERR_INVALID_ARG if the strip's output buffer length is zero
 *    - the error codes returned by the RMT driver, if error
 */
esp_err_t rmt_dled_send(rmt_pixel_strip_t *rps);

#ifdef __cplusplus
}
#endif

#endif
