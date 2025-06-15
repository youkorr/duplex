#include "i2s_duplex.h"

#ifdef USE_ESP32

namespace esphome {
namespace i2s_duplex {

void I2SDuplexComponent::setup() {
  ESP_LOGD(TAG, "Setting up I2S Duplex...");
  
  if (!this->configure_i2s()) {
    ESP_LOGE(TAG, "Failed to configure I2S");
    this->mark_failed();
    return;
  }
  
  this->initialized_ = true;
  this->setup_complete_ = true;
  ESP_LOGI(TAG, "I2S Duplex setup completed successfully");
}

void I2SDuplexComponent::loop() {
  if (!this->initialized_) {
    return;
  }
  
  // Cette méthode peut être utilisée pour des tâches périodiques
  // si nécessaire dans le futur
}

void I2SDuplexComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "I2S Duplex Configuration:");
  ESP_LOGCONFIG(TAG, "  LRCLK Pin: GPIO%d", this->lrclk_pin_);
  ESP_LOGCONFIG(TAG, "  BCLK Pin: GPIO%d", this->bclk_pin_);
  ESP_LOGCONFIG(TAG, "  MCLK Pin: GPIO%d", this->mclk_pin_);
  ESP_LOGCONFIG(TAG, "  DIN Pin (Microphone): GPIO%d", this->din_pin_);
  ESP_LOGCONFIG(TAG, "  DOUT Pin (Speaker): GPIO%d", this->dout_pin_);
  ESP_LOGCONFIG(TAG, "  Sample Rate: %d Hz", this->sample_rate_);
  ESP_LOGCONFIG(TAG, "  Bits per Sample: %d", this->bits_per_sample_);
  ESP_LOGCONFIG(TAG, "  Channel Format: %s", 
                this->channel_format_ == I2S_CHANNEL_FMT_RIGHT_LEFT ? "Stereo" :
                this->channel_format_ == I2S_CHANNEL_FMT_ONLY_LEFT ? "Left" : "Right");
  ESP_LOGCONFIG(TAG, "  Status: %s", this->initialized_ ? "Ready" : "Failed");
}

bool I2SDuplexComponent::configure_i2s() {
  // Configuration I2S
  i2s_config_t i2s_config = this->get_i2s_config();
  
  esp_err_t err = i2s_driver_install(I2S_NUM, &i2s_config, 0, nullptr);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to install I2S driver: %s", esp_err_to_name(err));
    return false;
  }
  
  // Configuration des pins
  i2s_pin_config_t pin_config = this->get_pin_config();
  
  err = i2s_set_pin(I2S_NUM, &pin_config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set I2S pins: %s", esp_err_to_name(err));
    i2s_driver_uninstall(I2S_NUM);
    return false;
  }
  
  // Démarrer I2S
  err = i2s_start(I2S_NUM);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start I2S: %s", esp_err_to_name(err));
    i2s_driver_uninstall(I2S_NUM);
    return false;
  }
  
  ESP_LOGI(TAG, "I2S configured successfully");
  return true;
}

void I2SDuplexComponent::cleanup_i2s() {
  if (this->initialized_) {
    i2s_stop(I2S_NUM);
    i2s_driver_uninstall(I2S_NUM);
    this->initialized_ = false;
  }
}

i2s_config_t I2SDuplexComponent::get_i2s_config() {
  i2s_config_t config = {
      .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX),
      .sample_rate = this->sample_rate_,
      .bits_per_sample = static_cast<i2s_bits_per_sample_t>(this->bits_per_sample_),
      .channel_format = this->channel_format_,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = DMA_BUF_COUNT,
      .dma_buf_len = DMA_BUF_LEN,
      .use_apll = false,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 0
  };
  
  return config;
}

i2s_pin_config_t I2SDuplexComponent::get_pin_config() {
  i2s_pin_config_t pin_config = {
      .bck_io_num = this->bclk_pin_,
      .ws_io_num = this->lrclk_pin_,
      .data_out_num = this->dout_pin_,
      .data_in_num = this->din_pin_
  };
  
  // MCLK est optionnel
  if (this->mclk_pin_ != 255) {
    pin_config.mck_io_num = this->mclk_pin_;
  } else {
    pin_config.mck_io_num = I2S_PIN_NO_CHANGE;
  }
  
  return pin_config;
}

bool I2SDuplexComponent::read_microphone(std::vector<int16_t> &buffer, size_t samples) {
  if (!this->initialized_) {
    ESP_LOGW(TAG, "I2S not initialized, cannot read microphone");
    return false;
  }
  
  buffer.resize(samples);
  size_t bytes_to_read = samples * sizeof(int16_t);
  size_t bytes_read = 0;
  
  esp_err_t err = i2s_read(I2S_NUM, buffer.data(), bytes_to_read, &bytes_read, portMAX_DELAY);
  
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read from microphone: %s", esp_err_to_name(err));
    return false;
  }
  
  if (bytes_read != bytes_to_read) {
    ESP_LOGW(TAG, "Read fewer bytes than expected: %d/%d", bytes_read, bytes_to_read);
    buffer.resize(bytes_read / sizeof(int16_t));
  }
  
  return true;
}

bool I2SDuplexComponent::write_speaker(const std::vector<int16_t> &buffer) {
  if (!this->initialized_) {
    ESP_LOGW(TAG, "I2S not initialized, cannot write to speaker");
    return false;
  }
  
  if (buffer.empty()) {
    return true;
  }
  
  size_t bytes_to_write = buffer.size() * sizeof(int16_t);
  size_t bytes_written = 0;
  
  esp_err_t err = i2s_write(I2S_NUM, buffer.data(), bytes_to_write, &bytes_written, portMAX_DELAY);
  
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to write to speaker: %s", esp_err_to_name(err));
    return false;
  }
  
  if (bytes_written != bytes_to_write) {
    ESP_LOGW(TAG, "Wrote fewer bytes than expected: %d/%d", bytes_written, bytes_to_write);
    return false;
  }
  
  return true;
}

}  // namespace i2s_duplex
}  // namespace esphome

#endif  // USE_ESP32
