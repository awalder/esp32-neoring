#pragma once

#include "driver/gpio.h"
#include "encoder.h"
#include "types.h"
#include <vector>

class NeoWriter
{
public:
    NeoWriter(gpio_num_t pin);
    auto setup() -> void;
    auto write(const std::vector<Led>& data) -> void;

private:
    gpio_num_t pin;
    rmt_channel_handle_t led_chan = nullptr;
    rmt_encoder_handle_t led_encoder = nullptr;
};
