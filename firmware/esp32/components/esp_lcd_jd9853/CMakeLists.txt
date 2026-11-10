# Espressif-template JD9853 panel driver (see NOTICE). Compiled only for the Cheeko Gotchi
# profile; main/port_display.cpp includes and calls it only under the same option. The
# requirements stay unconditional: ESP-IDF resolves them before the sdkconfig exists.
set(SRCS)
if(CONFIG_HG_BOARD_CHEEKO_GOTCHI)
    list(APPEND SRCS "esp_lcd_jd9853.c")
endif()
idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "."
    REQUIRES esp_lcd driver
)
