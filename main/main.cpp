#include "driver/rmt_tx.h"
#include "encoder.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cmath>
#include <vector>

static const char* const TAG = "neoring";

// 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_LED_STRIP_RESOLUTION_HZ 10000000

struct Led
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

auto setup()
{
}

extern "C" auto app_main() -> void
{
    ESP_LOGI(TAG, "Create RMT TX channel");
    rmt_channel_handle_t led_chan = nullptr;
    rmt_tx_channel_config_t tx_chan_config = {
            .gpio_num = GPIO_NUM_3,
            .clk_src = RMT_CLK_SRC_DEFAULT,
            .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
            .mem_block_symbols = 64, // increase the block size can make the LED
                                     // less flickering
            .trans_queue_depth = 4,  // set the number of transactions that can
                                     // be pending in the background
    };

    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    ESP_LOGI(TAG, "Install led strip encoder");
    rmt_encoder_handle_t led_encoder = nullptr;
    led_strip_encoder_config_t encoder_config = {
            .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    ESP_LOGI(TAG, "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(led_chan));

    ESP_LOGI(TAG, "Start LED rainbow chase");
    rmt_transmit_config_t tx_config = {
            .loop_count = 0, // no transfer loop
    };

    const int numLeds = 20 + 16; // Assume you have 16 LEDs
    auto data = std::vector<Led>(numLeds);

    double waveWidth = 3.0;  // Width of the wave in terms of LED count
    double waveSpeed = 0.50; // Speed of the wave movement

    while(true) // Infinite loop to keep the wave going
    {
        double t = esp_timer_get_time() / 1000000.0; // Current time in seconds

        // Calculate the position of the wave center
        double waveCenter = fmod(t * waveSpeed * numLeds, double(numLeds));

        for(int i = 0; i < numLeds; ++i)
        {
            double distance = fabs(i - waveCenter);
            if(distance <= waveWidth / 2)
            {
                // LED is within the wave
                double intensity = (waveWidth / 2 - distance) / (waveWidth / 2) * 0.6;
                data[i].r = uint8_t(intensity * 64);
                data[i].g = uint8_t(intensity * 255);
                data[i].b = uint8_t(intensity * 32);
            }
            else
            {
                // LED is outside the wave
                data[i].r = 0;
                data[i].g = 0;
                data[i].b = 0;
            }
        }

        // Flush RGB values to LEDs
        ESP_ERROR_CHECK(rmt_transmit(
                led_chan,
                led_encoder,
                data.data(),
                data.size() * sizeof(Led),
                &tx_config));

        ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));

        vTaskDelay(pdMS_TO_TICKS(50)); // Delay to control the speed of the wave
    }
}
