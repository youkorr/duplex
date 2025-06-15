#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#ifdef USE_ESP32
#include "driver/i2s.h"
#include "driver/gpio.h"
#include <vector>

namespace esphome {
namespace i2s_duplex {

static const char *const TAG = "i2s_duplex";

class I2SDuplexComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  // Pin configuration setters
  void set_lrclk_pin(uint8_t pin) { this->lrclk_pin_ = pin; }
  void set_bclk_pin(uint8_t pin) { this->bclk_pin_ = pin; }
  void set_mclk_pin(uint8_t pin) { this->mclk_pin_ = pin; }
  void set_din_pin(uint8_t pin) { this->din_pin_ = pin; }
  void set_dout_pin(uint8_t pin) { this->dout_pin_ = pin; }

  // Audio configuration setters
  void set_sample_rate(uint32_t sample_rate) { this->sample_rate_ = sample_rate; }
  void set_bits_per_sample(uint8_t bits_per_sample) { this->bits_per_sample_ = bits_per_sample; }
  void set_channel(i2s_channel_fmt_t channel) { this->channel_format_ = channel; }

  // Audio data methods
  bool read_microphone(std::vector<int16_t> &buffer, size_t samples);
  bool write_speaker(const std::vector<int16_t> &buffer);
  
  // Utility methods
  bool is_ready() const { return this->initialized_; }
  uint32_t get_sample_rate() const { return this->sample_rate_; }
  
 protected:
  // GPIO pins
  uint8_t lrclk_pin_{45};  // Word Select / Frame Clock
  uint8_t bclk_pin_{17};   // Bit Clock
  uint8_t mclk_pin_{2};    // Master Clock (optional)
  uint8_t din_pin_{16};    // Data Input (microphone)
  uint8_t dout_pin_{15};   // Data Output (speaker)

  // Audio configuration
  uint32_t sample_rate_{16000};
  uint8_t bits_per_sample_{16};
  i2s_channel_fmt_t channel_format_{I2S_CHANNEL_FMT_RIGHT_LEFT};

  // I2S configuration
  static const i2s_port_t I2S_NUM = I2S_NUM_0;
  static const int DMA_BUF_COUNT = 8;
  static const int DMA_BUF_LEN = 1024;

  // Internal state
  bool initialized_{false};
  bool setup_complete_{false};

  // Internal methods
  bool configure_i2s();
  void cleanup_i2s();
  i2s_config_t get_i2s_config();
  i2s_pin_config_t get_pin_config();
};

}  // namespace i2s_duplex
}  // namespace esphome

#endif  // USE_ESP32
