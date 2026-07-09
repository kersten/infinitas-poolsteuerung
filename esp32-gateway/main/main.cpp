#include <string.h>
#include <stdlib.h>

#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

#include <esp_matter.h>

#include "Protocol.h"

namespace {

constexpr char kLogTag[] = "pool_matter";
constexpr int kUartBaud = 115200;
constexpr uart_port_t kControllerUart = UART_NUM_2;
constexpr gpio_num_t kControllerUartRxPin = GPIO_NUM_16;
constexpr gpio_num_t kControllerUartTxPin = GPIO_NUM_17;
constexpr size_t kUartLineCapacity = 160;

uint16_t leftEndpointId = 0;
uint16_t rightEndpointId = 0;
char uartLine[kUartLineCapacity];
size_t uartLineLength = 0;

using namespace chip::app::Clusters;
using namespace esp_matter;
using namespace esp_matter::endpoint;

void initializeNvs() {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
}

void initializeControllerUart() {
  const uart_config_t config = {
      .baud_rate = kUartBaud,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };
  ESP_ERROR_CHECK(uart_param_config(kControllerUart, &config));
  ESP_ERROR_CHECK(uart_set_pin(kControllerUart, kControllerUartTxPin,
                               kControllerUartRxPin, UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE));
  ESP_ERROR_CHECK(uart_driver_install(kControllerUart, 512, 0, 0, nullptr, 0));
}

bool sendCommand(pool::protocol::Command command) {
  const char *const text = pool::protocol::commandName(command);
  const size_t length = strlen(text);
  if (uart_write_bytes(kControllerUart, text, length) !=
          static_cast<int>(length) ||
      uart_write_bytes(kControllerUart, "\n", 1) != 1) {
    ESP_LOGE(kLogTag, "Could not send controller command: %s", text);
    return false;
  }
  return true;
}

void requestStatus() { (void)sendCommand(pool::protocol::Command::Status); }

void publishEndpointState(uint16_t endpointId, bool on) {
  if (endpointId == 0) {
    return;
  }

  // report() bypasses the app's PRE_UPDATE callback, which is essential here:
  // this is a confirmed controller state, not a new request to the Mega.
  esp_matter_attr_val_t value = esp_matter_bool(on);
  const esp_err_t err = attribute::report(
      endpointId, OnOff::Id, OnOff::Attributes::OnOff::Id, &value);
  if (err != ESP_OK) {
    ESP_LOGE(kLogTag, "Could not report endpoint %u state: %s", endpointId,
             esp_err_to_name(err));
  }
}

void applyState(const pool::protocol::StateSnapshot &state) {
  publishEndpointState(leftEndpointId, pool::isChannelActive(state.left));
  publishEndpointState(rightEndpointId, pool::isChannelActive(state.right));
}

void handleControllerLine(const char *line) {
  if (strcmp(line, "READY") == 0) {
    requestStatus();
    return;
  }

  pool::protocol::StateSnapshot state;
  if (pool::protocol::parseState(line, state)) {
    applyState(state);
    return;
  }

  // Unknown or malformed UART input is never reflected into the Matter data
  // model. Keep it visible in the ESP-IDF monitor for diagnostics.
  ESP_LOGW(kLogTag, "Controller UART: %s", line);
}

void consumeControllerByte(char character) {
  if (character == '\r') {
    return;
  }
  if (character == '\n') {
    uartLine[uartLineLength] = '\0';
    if (uartLineLength > 0) {
      handleControllerLine(uartLine);
    }
    uartLineLength = 0;
    return;
  }
  if (uartLineLength < sizeof(uartLine) - 1) {
    uartLine[uartLineLength++] = character;
  } else {
    // Discard one overlong line without changing a Matter endpoint.
    uartLineLength = 0;
  }
}

void pollControllerUart() {
  uint8_t buffer[32];
  const int received = uart_read_bytes(kControllerUart, buffer, sizeof(buffer),
                                       pdMS_TO_TICKS(100));
  if (received < 0) {
    ESP_LOGW(kLogTag, "Controller UART read failed: %d", received);
    return;
  }
  for (int index = 0; index < received; ++index) {
    consumeControllerByte(static_cast<char>(buffer[index]));
  }
}

pool::protocol::Command commandForEndpoint(uint16_t endpointId, bool requestedOn) {
  if (endpointId == leftEndpointId) {
    return requestedOn ? pool::protocol::Command::StartLeft
                       : pool::protocol::Command::StopLeft;
  }
  if (endpointId == rightEndpointId) {
    return requestedOn ? pool::protocol::Command::StartRight
                       : pool::protocol::Command::StopRight;
  }
  return pool::protocol::Command::Unknown;
}

esp_err_t attributeUpdateCallback(attribute::callback_type_t type,
                                  uint16_t endpointId, uint32_t clusterId,
                                  uint32_t attributeId,
                                  esp_matter_attr_val_t *value,
                                  void *) {
  if (type != attribute::PRE_UPDATE || clusterId != OnOff::Id ||
      attributeId != OnOff::Attributes::OnOff::Id) {
    return ESP_OK;
  }

  const pool::protocol::Command command =
      commandForEndpoint(endpointId, value->val.b);
  if (command == pool::protocol::Command::Unknown) {
    return ESP_OK;
  }

  if (!sendCommand(command)) {
    return ESP_FAIL;
  }

  // Do not accept a speculative Matter attribute write. A subsequent STATE
  // line from the Mega is the only path that publishes a new endpoint value.
  return ESP_ERR_INVALID_STATE;
}

esp_err_t identificationCallback(identification::callback_type_t type,
                                 uint16_t endpointId, uint8_t effectId,
                                 uint8_t effectVariant, void *) {
  ESP_LOGI(kLogTag,
           "Identify request: type=%u endpoint=%u effect=%u variant=%u", type,
           endpointId, effectId, effectVariant);
  return ESP_OK;
}

void createMatterEndpoints() {
  node::config_t nodeConfig;
  node_t *const node = node::create(&nodeConfig, attributeUpdateCallback,
                                    identificationCallback);
  if (node == nullptr) {
    ESP_LOGE(kLogTag, "Failed to create Matter node");
    abort();
  }

  pump::config_t leftConfig;
  leftConfig.on_off.on_off = false;
  endpoint_t *const leftEndpoint =
      pump::create(node, &leftConfig, ENDPOINT_FLAG_NONE, nullptr);
  if (leftEndpoint == nullptr) {
    ESP_LOGE(kLogTag, "Failed to create left pump endpoint");
    abort();
  }
  leftEndpointId = endpoint::get_id(leftEndpoint);

  pump::config_t rightConfig;
  rightConfig.on_off.on_off = false;
  endpoint_t *const rightEndpoint =
      pump::create(node, &rightConfig, ENDPOINT_FLAG_NONE, nullptr);
  if (rightEndpoint == nullptr) {
    ESP_LOGE(kLogTag, "Failed to create right pump endpoint");
    abort();
  }
  rightEndpointId = endpoint::get_id(rightEndpoint);

  ESP_LOGI(kLogTag, "Matter endpoints: left=%u right=%u", leftEndpointId,
           rightEndpointId);
}

}  // namespace

extern "C" void app_main() {
  initializeNvs();
  initializeControllerUart();
  createMatterEndpoints();

  ESP_ERROR_CHECK(esp_matter::start(nullptr));

  // The Mega might already be running when the ESP32 starts.
  requestStatus();
  while (true) {
    pollControllerUart();
  }
}
