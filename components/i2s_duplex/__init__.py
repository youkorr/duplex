import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import esp32
from esphome.const import (
    CONF_ID,
    CONF_MODE,
    CONF_SAMPLE_RATE,
    CONF_BITS_PER_SAMPLE,
    CONF_CHANNEL,
)
from esphome.core import CORE

CODEOWNERS = ["@your_username"]
DEPENDENCIES = ["esp32"]

# Configuration keys
CONF_I2S_LRCLK_PIN = "i2s_lrclk_pin"
CONF_I2S_BCLK_PIN = "i2s_bclk_pin"
CONF_I2S_MCLK_PIN = "i2s_mclk_pin"
CONF_I2S_DIN_PIN = "i2s_din_pin"
CONF_I2S_DOUT_PIN = "i2s_dout_pin"
CONF_I2S_AUDIO_SAMPLE_RATE = "sample_rate"
CONF_I2S_AUDIO_BITS_PER_SAMPLE = "bits_per_sample"
CONF_I2S_AUDIO_CHANNEL = "channel"

# Namespace
i2s_duplex_ns = cg.esphome_ns.namespace("i2s_duplex")
I2SDuplexComponent = i2s_duplex_ns.class_("I2SDuplexComponent", cg.Component)

# Audio channel options
I2S_CHANNEL_LEFT = "left"
I2S_CHANNEL_RIGHT = "right"
I2S_CHANNEL_STEREO = "stereo"

I2S_CHANNELS = {
    I2S_CHANNEL_LEFT: cg.global_ns.I2S_CHANNEL_FMT_ONLY_LEFT,
    I2S_CHANNEL_RIGHT: cg.global_ns.I2S_CHANNEL_FMT_ONLY_RIGHT,
    I2S_CHANNEL_STEREO: cg.global_ns.I2S_CHANNEL_FMT_RIGHT_LEFT,
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(I2SDuplexComponent),
        cv.Required(CONF_I2S_LRCLK_PIN): cv.int_range(min=0, max=39),
        cv.Required(CONF_I2S_BCLK_PIN): cv.int_range(min=0, max=39),
        cv.Optional(CONF_I2S_MCLK_PIN): cv.int_range(min=0, max=39),
        cv.Required(CONF_I2S_DIN_PIN): cv.int_range(min=0, max=39),
        cv.Required(CONF_I2S_DOUT_PIN): cv.int_range(min=0, max=39),
        cv.Optional(CONF_I2S_AUDIO_SAMPLE_RATE, default=16000): cv.int_range(min=8000, max=96000),
        cv.Optional(CONF_I2S_AUDIO_BITS_PER_SAMPLE, default=16): cv.one_of(16, 24, 32),
        cv.Optional(CONF_I2S_AUDIO_CHANNEL, default=I2S_CHANNEL_STEREO): cv.enum(I2S_CHANNELS),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Set GPIO pins
    cg.add(var.set_lrclk_pin(config[CONF_I2S_LRCLK_PIN]))
    cg.add(var.set_bclk_pin(config[CONF_I2S_BCLK_PIN]))
    cg.add(var.set_din_pin(config[CONF_I2S_DIN_PIN]))
    cg.add(var.set_dout_pin(config[CONF_I2S_DOUT_PIN]))
    
    if CONF_I2S_MCLK_PIN in config:
        cg.add(var.set_mclk_pin(config[CONF_I2S_MCLK_PIN]))
    
    # Set audio parameters
    cg.add(var.set_sample_rate(config[CONF_I2S_AUDIO_SAMPLE_RATE]))
    cg.add(var.set_bits_per_sample(config[CONF_I2S_AUDIO_BITS_PER_SAMPLE]))
    cg.add(var.set_channel(config[CONF_I2S_AUDIO_CHANNEL]))

    # Add required includes
    cg.add_library("ESP32", None)
