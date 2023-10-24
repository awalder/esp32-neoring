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

// Function to convert a hue value to RGB.
void hueToRgb(float hue, uint8_t& r, uint8_t& g, uint8_t& b)
{
    float red, green, blue;
    float f = (hue / 60.0f) - floor(hue / 60.0f);
    float p = 0.0f;
    float q = 1.0f - f;
    float t = f;

    if(hue < 60.0f)
    {
        red = 1.0f;
        green = t;
        blue = p;
    }
    else if(hue < 120.0f)
    {
        red = q;
        green = 1.0f;
        blue = p;
    }
    else if(hue < 180.0f)
    {
        red = p;
        green = 1.0f;
        blue = t;
    }
    else if(hue < 240.0f)
    {
        red = p;
        green = q;
        blue = 1.0f;
    }
    else if(hue < 300.0f)
    {
        red = t;
        green = p;
        blue = 1.0f;
    }
    else
    {
        red = 1.0f;
        green = p;
        blue = q;
    }

    r = uint8_t(red * 255.0f);
    g = uint8_t(green * 255.0f);
    b = uint8_t(blue * 255.0f);
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

    const int numLeds = 20;
    auto data = std::vector<Led>(numLeds);

    float cycleSpeed = 0.12f; // Speed of the color cycling

    while(true) // Infinite loop to keep the effect going
    {
        double t = esp_timer_get_time() / 1000000.0; // Current time in seconds

        for(int i = 0; i < numLeds; ++i)
        {
            float hue = fmod(
                    (i * 360.0f / numLeds) + (t * cycleSpeed * 360.0f), 360.0f);
            hueToRgb(hue, data[i].r, data[i].g, data[i].b);
        }

        // Flush RGB values to LEDs
        ESP_ERROR_CHECK(rmt_transmit(
                led_chan,
                led_encoder,
                data.data(),
                data.size() * sizeof(Led),
                &tx_config));

        ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));

        vTaskDelay(pdMS_TO_TICKS(
                50)); // Delay to control the speed of the color cycling
    }
}
