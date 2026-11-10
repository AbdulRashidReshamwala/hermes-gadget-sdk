// Serial console: every line goes to hg::App::console() on the app task, so the
// same commands work here, in the simulator and from `hermes-gadget provision`.
#include "port.hpp"  // first: pulls in FreeRTOS.h ahead of task.h/queue.h

#include <cstdio>
#include <cstring>
#include <string>

#include "esp_console.h"
#include "esp_log.h"
#include "hg/console_args.hpp"
#include "sdkconfig.h"
#if CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG
#include "driver/usb_serial_jtag.h"
#endif

namespace hgp::console {
namespace {

const char* TAG = "hg.console";

int run(int argc, char** argv) {
  // ESP-IDF has already split the line and removed its quotes. `wifi` takes several
  // values that may hold spaces, so its arguments are quoted again.
  const bool quote = argc && std::strcmp(argv[0], "wifi") == 0;
  std::string line;
  for (int i = 0; i < argc; ++i) {
    if (i) line.push_back(' ');
    line += quote && i ? hg::quote_arg(argv[i]) : std::string(argv[i]);
  }
  // The app task answers on its own time. The slot outlives this function
  // if that answer comes after the wait below gave up.
  ConsoleRequest* req = ConsoleRequest::create();
  if (!events::post(EventType::Console, line.data(), line.size(), 0, req)) {
    req->release();  // nobody will answer: drop the app task's share too
    std::printf("@error device busy\n");
  } else if (!req->wait(3000)) {
    std::printf("@error device busy\n");
  } else {
    std::printf("%s\n", req->reply().c_str());
  }
  req->release();
  return 0;
}

void start() {
  static const char* const kCommands[] = {"help",      "status", "diag",   "get",         "set",       "say",       "camera",
                                          "talk",      "release", "cancel", "new-session", "reconnect", "forget-key",
                                          "factory-reset", "settings", "wifi-setup", "wifi", "phone", "perf"};
  for (const char* name : kCommands) {
    esp_console_cmd_t cmd = {};
    cmd.command = name;
    cmd.help = "Hermes Gadget command (see: help)";
    cmd.func = &run;
    esp_console_cmd_register(&cmd);
  }
  esp_console_repl_t* repl = nullptr;
  esp_console_repl_config_t repl_cfg = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
  repl_cfg.prompt = "gadget>";
  repl_cfg.max_cmdline_length = 256;
#if CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG
  esp_console_dev_usb_serial_jtag_config_t dev = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&dev, &repl_cfg, &repl));
#elif CONFIG_ESP_CONSOLE_USB_CDC
  esp_console_dev_usb_cdc_config_t dev = ESP_CONSOLE_DEV_CDC_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_console_new_repl_usb_cdc(&dev, &repl_cfg, &repl));
#else
  esp_console_dev_uart_config_t dev = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_console_new_repl_uart(&dev, &repl_cfg, &repl));
#endif
  ESP_ERROR_CHECK(esp_console_start_repl(repl));
  ESP_LOGI(TAG, "console ready (type: help)");
}

}  // namespace

void begin() {
#if CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG
  // Setting up this console stalls until a USB host is attached, and on battery there is
  // none. Wait for one on a task of its own so the app loop, and Wi-Fi with it, still run.
  auto wait_for_host = [](void*) {
    while (!usb_serial_jtag_is_connected()) vTaskDelay(pdMS_TO_TICKS(250));
    start();
    vTaskDelete(nullptr);
  };
  if (xTaskCreate(wait_for_host, "hg-console-up", 6144, nullptr, 1, nullptr) != pdPASS)
    ESP_LOGE(TAG, "console task not started");
#else
  start();
#endif
}

}  // namespace hgp::console
