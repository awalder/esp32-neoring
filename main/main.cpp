#include "driver/rmt_tx.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <vector>

// oneBitHighTime = 48;
// oneBitLowTime = 48;
// zeroBitHighTime = 24;
// zeroBitLowTime = 72;

// static const int treset = 1000;

struct PulseWidths
{ // Data transfer times
        static const int t0h = 32;
        static const int t0l = 68;
        static const int t1h = 64;
        static const int t1l = 36;
    // static const int t0h = 24;
    // static const int t0l = 72;
    // static const int t1h = 48;
    // static const int t1l = 48;
    static const int treset = 1000;
};

struct Vertex
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

auto setup() -> void
{
    auto channel = rmt_channel_t::RMT_CHANNEL_0;

    auto tx_config = rmt_tx_config_t{
            .idle_level = RMT_IDLE_LEVEL_LOW,
            .carrier_en = false,
            .loop_en = false,
            .idle_output_en = true,
    };

    auto rmt_rx = rmt_config_t{
            .rmt_mode = RMT_MODE_TX,
            .channel = channel,
            .gpio_num = GPIO_NUM_3,
            .clk_div = 1,
            .mem_block_num = 3,
            .tx_config = tx_config,
    };

    ESP_ERROR_CHECK(rmt_config(&rmt_rx));
    ESP_ERROR_CHECK(rmt_driver_install(channel, 0, 0));
}

extern "C" auto app_main() -> void
{
    const int numLeds = 4;
    const int bitsPerLed = 24;
    const int numBits = numLeds * bitsPerLed;

    setup();
    auto channel = rmt_channel_t::RMT_CHANNEL_0;
    rmt_item32_t items[numBits];
    // auto items = std::vector<rmt_item32_t>(numBits);
    auto data = std::vector<Vertex>(numLeds);

    data.at(0).g = 1;
    data.at(0).r = 0;
    data.at(0).b = 0;

    ESP_LOGI("main", "bitsPerLed: %d", bitsPerLed);
    ESP_LOGI("main", "numLeds: %d", numLeds);
    ESP_LOGI("main", "numBits: %d", numBits);

    for(int i = 0; i < numBits; i++)
    {
        auto& item = items[i];
        item.level0 = 1;
        item.duration0 = PulseWidths::t0h;
        item.level1 = 0;
        item.duration1 = PulseWidths::t0l;
    }

    for(int i = 0; i < numLeds; i++)
    {
        auto& vertex = data[i];
        auto offset = i * bitsPerLed;
        auto& item = items[offset];

        for(int j = 0; j < 8; j++)
        {
        // if(widePixelData & mask)
        // {
        //     this->rmtItems[i].duration0 = oneBitHighTime;
        //     this->rmtItems[i].duration1 = oneBitLowTime;
        // }
        // else
        // {
        //     this->rmtItems[i].duration0 = zeroBitHighTime;
        //     this->rmtItems[i].duration1 = zeroBitLowTime;
        // }
            uint8_t mask = 1 << (7 - j);
            uint8_t bit = (vertex.g & mask) != 0;
            rmt_item32_t& item = items[offset + j];
            item.level0 = !bit;
            item.duration0 = bit ? PulseWidths::t1h : PulseWidths::t0h;
            item.level1 = bit;
            item.duration1 = bit ? PulseWidths::t1l : PulseWidths::t0l;
        }

        for(int j = 0; j < 8; j++)
        {
            // uint8_t mask = 1 << j;
            uint8_t mask = 1 << (7 - j);
            uint8_t bit = (vertex.r & mask) != 0;
            rmt_item32_t& item = items[offset + 8 + j];
            item.level0 = !bit;
            item.duration0 = bit ? PulseWidths::t1h : PulseWidths::t0h;
            item.level1 = bit;
            item.duration1 = bit ? PulseWidths::t1l : PulseWidths::t0l;
        }

        for(int j = 0; j < 8; j++)
        {
            // uint8_t mask = 1 << j;
            uint8_t mask = 1 << (7 - j);
            uint8_t bit = (vertex.b & mask) != 0;
            rmt_item32_t& item = items[offset + 16 + j];
            item.level0 = !bit;
            item.duration0 = bit ? PulseWidths::t1h : PulseWidths::t0h;
            item.level1 = bit;
            item.duration1 = bit ? PulseWidths::t1l : PulseWidths::t0l;
        }
    }

    for(int i = 0; i < bitsPerLed; ++i)
    {
        rmt_item32_t& item = items[i];
        printf("%d", item.level1);
        if((i + 1) % 8 == 0)
        {
            printf(" ");
        }
    }
    printf("\n");

    // {
    //     static const int tres = 50000; // Assuming a 1MHz RMT clock
    //
    //     // Create a reset pulse item
    //     rmt_item32_t resetItem;
    //     resetItem.level0 = 0;
    //     resetItem.duration0 = tres;
    //     resetItem.level1 = 0;
    //     resetItem.duration1 = 0; // No second level for the reset item
    //
    //     // Insert the reset item at the beginning of the items vector
    //     items.insert(items.begin(), resetItem);
    // }

    rmt_wait_tx_done(channel, portMAX_DELAY);
    // rmt_write_items(channel, items.data(), items.size(), true);
    esp_err_t ret = rmt_write_items(channel, items, numBits, true);
    if(ret != ESP_OK)
    {
        printf("Error: %s\n", esp_err_to_name(ret));
    }
    rmt_wait_tx_done(channel, portMAX_DELAY);
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
}
