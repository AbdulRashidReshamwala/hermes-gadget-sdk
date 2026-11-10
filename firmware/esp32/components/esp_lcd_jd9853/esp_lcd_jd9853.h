/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Modified for Hermes Gadget: adapted from firmware/hardware_test/components/
 * esp_lcd_jd9853 in https://github.com/ALTIO-AI-PRIVATE-LIMITED/cheeko-gotchi,
 * commit 813fb6d221cd09e2f492e7cb169bf183685ddeea (unchanged at d6bf1f8), itself
 * an Espressif esp_lcd panel-driver template; the Espressif release it was
 * copied from is not recorded there. Changes: removed the hal/spi_ll.h include and the two SPI
 * configuration macros (the port builds its own bus and panel IO), and the
 * init table is now required. See NOTICE and LICENSES/Apache-2.0.txt.
 */
/**
 * @file
 * @brief ESP LCD: jd9853
 */

#pragma once

#include "esp_lcd_panel_vendor.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LCD panel initialization commands.
 *
 */
typedef struct {
    int cmd;                /*<! The specific LCD command */
    const void *data;       /*<! Buffer that holds the command specific data */
    size_t data_bytes;      /*<! Size of `data` in memory, in bytes */
    unsigned int delay_ms;  /*<! Delay in milliseconds after this command */
} jd9853_lcd_init_cmd_t;

/**
 * @brief LCD panel vendor configuration.
 *
 * @note  This structure needs to be passed to the `vendor_config` field in `esp_lcd_panel_dev_config_t`.
 *
 */
typedef struct {
    const jd9853_lcd_init_cmd_t *init_cmds;     /*!< Pointer to the initialization commands array. Required: this component has no
                                                 *   default table. Declare it `static const`, outside any function.
                                                 */
    uint16_t init_cmds_size;                    /*<! Number of commands in above array */
} jd9853_vendor_config_t;

/**
 * @brief Create LCD panel for model jd9853
 *
 * @note  Vendor specific initialization can be different between manufacturers, should consult the LCD supplier for initialization sequence code.
 *        `panel_dev_config->vendor_config` must point to a `jd9853_vendor_config_t` with a non-empty table.
 *
 * @param[in] io LCD panel IO handle
 * @param[in] panel_dev_config general panel device configuration
 * @param[out] ret_panel Returned LCD panel handle
 * @return
 *          - ESP_ERR_INVALID_ARG   if a parameter is invalid or the init table is missing
 *          - ESP_ERR_NO_MEM        if out of memory
 *          - ESP_OK                on success
 */
esp_err_t esp_lcd_new_panel_jd9853(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel);

#ifdef __cplusplus
}
#endif
