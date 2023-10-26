#include "neowriter.h"
#include "driver/rmt_tx.h"
#include "encoder.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* const TAG = "NeoWriter";
static const uint32_t RMT_LED_STRIP_RESOLUTION_HZ = 10'000'000;

NeoWriter::NeoWriter(gpio_num_t pin) : pin(pin)
{
    setup();
}

auto NeoWriter::setup() -> void
{
    ESP_LOGI(TAG, "Create RMT TX channel");
    rmt_tx_channel_config_t tx_chan_config = {
            .gpio_num = GPIO_NUM_3,
            .clk_src = RMT_CLK_SRC_DEFAULT,
            .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
            .mem_block_symbols = 64, // increase the block size can make the
                                     // LED less flickering
            .trans_queue_depth = 4,  // set the number of transactions that
                                     // can be pending in the background
    };

    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    ESP_LOGI(TAG, "Install led strip encoder");
    led_strip_encoder_config_t encoder_config = {
            .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    ESP_LOGI(TAG, "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(led_chan));

    ESP_LOGI(TAG, "Start LED rainbow chase");
}

auto NeoWriter::write(const std::vector<Led>& data) -> void
{
    rmt_transmit_config_t tx_config = {
            .loop_count = 0, // no transfer loop
    };

    ESP_ERROR_CHECK(rmt_transmit(
            led_chan,
            led_encoder,
            data.data(),
            data.size() * sizeof(Led),
            &tx_config));

    ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
}
