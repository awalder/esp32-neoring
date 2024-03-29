#include "driver/rmt_tx.h"
#include "encoder.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "glm/glm.hpp"
#include "glm/trigonometric.hpp"
#include "glm/vec3.hpp"
#include "neowriter.h"
#include "types.h"
#include <cmath>
#include <random>
#include <vector>

static const char* const TAG = "neoring";

// 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
glm::vec3 palette(float t)
{
    // clang-format off
    // [[0.000 0.500 0.500] [0.000 0.500 0.500] [0.000 0.500 0.333] [0.000 0.500 0.667]]
    // [[0.938 0.328 0.718] [0.659 0.438 0.328] [0.388 0.388 0.296] [2.538 2.478 0.168]]
    //[[0.821 0.328 0.242] [0.659 0.481 0.896] [0.612 0.340 0.296] [2.820 3.026 -0.273]]
    //[[0.667 0.500 0.500] [0.500 0.667 0.500] [0.667 0.666 0.500] [0.200 0.000 0.500]]
    // [[0.500 0.500 0.000] [0.500 0.500 0.000] [0.500 0.500 0.000] [0.500 0.000 0.000]]
    // [[0.650 0.500 0.310] [-0.650 0.500 0.600] [0.333 0.278 0.278] [0.660 0.000 0.667]]
    // [[0.590 0.811 0.120] [0.410 0.392 0.590] [0.940 0.548 0.278] [-4.242 -6.611 -4.045]]
    // [[0.590 0.811 0.120] [1.128 0.392 0.868] [4.226 1.408 2.458] [-4.244 -6.613 -4.047]]
    // [[0.938 0.500 0.188] [0.098 0.168 0.438] [0.800 0.800 0.608] [0.000 0.108 0.500]]
    // [[0.938 0.488 0.188] [0.918 0.248 0.438] [0.800 1.618 0.608] [-0.452 -0.192 0.492]]
    // [[0.240 0.240 0.198] [1.000 1.000 -0.003] [2.000 2.000 2.000] [0.000 0.500 0.667]]
    // [[0.240 0.240 -0.202] [1.000 1.000 -0.002] [2.000 2.000 2.000] [0.000 0.500 0.667]]
    // [[0.610 0.610 0.088] [0.500 0.500 -0.002] [2.000 2.000 2.000] [0.000 0.500 0.667]]
    // [[1.118 0.500 0.500] [0.500 0.500 0.500] [1.000 1.000 1.000] [0.000 0.333 0.667]]
    // 0.5, 0.5, 0.5		0.5, 0.5, 0.5	1.0, 1.0, 1.0	0.00, 0.10, 0.20
    // clang-format on

    // auto a = glm::vec3(0.5, 0.5, 0.5);
    // auto b = glm::vec3(0.5, 0.5, 0.5);
    // auto c = glm::vec3(1.0, 1.0, 1.0);
    // auto d = glm::vec3(0.00, 0.10, 0.20);

    // auto a = glm::vec3(0.938, 0.500, 0.188);
    // auto b = glm::vec3(0.098, 0.168, 0.438);
    // auto c = glm::vec3(0.800, 0.800, 0.608);
    // auto d = glm::vec3(0.000, 0.108, 0.500);

    // auto a = glm::vec3(0.938, 0.500, 0.188);
    // auto b = glm::vec3(0.098, 0.168, 0.438);
    // auto c = glm::vec3(0.800, 0.800, 0.608);
    // auto d = glm::vec3(0.000, 0.108, 0.500);

    // auto a = glm::vec3(0.240, 0.240, -0.202);
    // auto b = glm::vec3(1.000, 1.000, -0.002);
    // auto c = glm::vec3(2.000, 2.000, 2.000);
    // auto d = glm::vec3(0.000, 0.500, 0.667);

    // auto a = glm::vec3(0.240, 0.240, 0.198);
    // auto b = glm::vec3(1.000, 1.000, -0.003);
    // auto c = glm::vec3(2.000, 2.000, 2.000);
    // auto d = glm::vec3(0.000, 0.500, 0.667);

    // auto a = glm::vec3(0.610, 0.610, 0.088);
    // auto b = glm::vec3(0.500, 0.500, -0.002);
    // auto c = glm::vec3(2.000, 2.000, 2.000);
    // auto d = glm::vec3(0.000, 0.500, 0.667);

    // auto a = glm::vec3(0.938, 0.488, 0.188);
    // auto b = glm::vec3(0.918, 0.248, 0.438);
    // auto c = glm::vec3(0.800, 1.618, 0.608);
    // auto d = glm::vec3(-0.452, -0.192, 0.492);

    // auto a = glm::vec3(0.938, 0.500, 0.188);
    // auto b = glm::vec3(0.098, 0.168, 0.438);
    // auto c = glm::vec3(0.800, 0.800, 0.608);
    // auto d = glm::vec3(0.000, 0.108, 0.500);

    // auto a = glm::vec3(0.590, 0.811, 0.120);
    // auto b = glm::vec3(1.128, 0.392, 0.868);
    // auto c = glm::vec3(4.226, 1.408, 2.458);
    // auto d = glm::vec3(-4.244, -6.613, -4.047);

    // pinkki aarnelle
    // auto a = glm::vec3(0.590, 0.811, 0.120);
    // auto b = glm::vec3(0.410, 0.392, 0.590);
    // auto c = glm::vec3(0.940, 0.548, 0.278);
    // auto d = glm::vec3(-4.242, -6.611, -4.045);

    // auto a = glm::vec3(0.650, 0.500, 0.310);
    // auto b = glm::vec3(-0.650, 0.500, 0.600);
    // auto c = glm::vec3(0.333, 0.278, 0.278);
    // auto d = glm::vec3(0.660, 0.000, 0.667);

    // auto a = glm::vec3(0.500, 0.500, 0.000);
    // auto b = glm::vec3(0.500, 0.500, 0.000);
    // auto c = glm::vec3(0.500, 0.500, 0.000);
    // auto d = glm::vec3(0.500, 0.000, 0.000);

    // auto a = glm::vec3(0.667, 0.500, 0.500);
    // auto b = glm::vec3(0.500, 0.667, 0.500);
    // auto c = glm::vec3(0.667, 0.666, 0.500);
    // auto d = glm::vec3(0.200, 0.000, 0.500);

    // auto a = Vec3(0.821, 0.328, 0.242);
    // auto b = Vec3(0.659, 0.481, 0.896);
    // auto c = Vec3(0.612, 0.340, 0.296);
    // auto d = Vec3(2.820, 3.026, -0.273);

    // auto a = Vec3(0.938, 0.328, 0.718);
    // auto b = Vec3(0.659, 0.438, 0.328);
    // auto c = Vec3(0.388, 0.388, 0.296);
    // auto d = Vec3(2.538, 2.478, 0.168);

    auto a = glm::vec3(0.500, 0.000, 0.500);
    auto b = glm::vec3(0.500, 0.000, 0.500);
    auto c = glm::vec3(0.500, 0.000, 0.333);
    auto d = glm::vec3(0.500, 0.000, 0.667);

    // auto a = glm::vec3(0.000, 0.500, 0.500);
    // auto b = glm::vec3(0.000, 0.500, 0.500);
    // auto c = glm::vec3(0.000, 0.500, 0.333);
    // auto d = glm::vec3(0.000, 0.500, 0.667);

    // auto a = glm::vec3(0.5, 0.5, 0.5);
    // auto b = glm::vec3(0.5, 0.5, 0.5);
    // auto c = glm::vec3(1.0, 1.0, 1.0);
    // auto d = glm::vec3(0.00, 0.333, 0.667);

    // auto a = Vec3(0.5, 0.5, 0.5);
    // auto b = Vec3(0.5, 0.5, 0.5);
    // auto c = Vec3(1.0, 0.7, 0.4);
    // auto d = Vec3(0.00, 0.15, 0.20);

    // auto a = Vec3(0.500, 0.500, 0.500);
    // auto b = Vec3(0.500, 0.500, 0.500);
    // auto c = Vec3(1.000, 1.000, 1.000);
    // auto d = Vec3(0.000, 0.333, 0.667);

    return glm::clamp(a + b * glm::cos((c * t + d) * 6.28318f), 0.0f, 1.0f);
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

    r = uint8_t(red * 63.0f);
    g = uint8_t(green * 63.0f);
    b = uint8_t(blue * 63.0f);
}

// Configuration for the twinkling effect
// struct TwinkleConfig
// {
//     float riseTime;
//     float maxBrightness;
// };

// Function to update the brightness of an LED for the twinkling effect
// void updateTwinkle(Led& led, TwinkleConfig& config, float deltaTime)
// {
//     static std::default_random_engine generator;
//     static std::uniform_real_distribution<float> distribution(0.0, 1.0);
//
//     float targetBrightness = 1.0f;
//     // Randomly decide if the LED should start/stop twinkling
//     if(distribution(generator) < 0.05)
//     { // 5% chance to change state
//         // Randomly assign a new target brightness
//         targetBrightness = distribution(generator) * config.maxBrightness;
//     }
//
//     // Adjust current brightness towards target brightness
//     if(led.brightness < targetBrightness)
//     {
//         led.brightness += deltaTime / config.riseTime;
//         if(led.brightness > targetBrightness)
//         {
//             led.brightness = targetBrightness;
//         }
//     }
//     else if(led.brightness > targetBrightness)
//     {
//         led.brightness -= deltaTime / config.riseTime;
//         if(led.brightness < targetBrightness)
//         {
//             led.brightness = targetBrightness;
//         }
//     }
// }

extern "C" auto app_main() -> void
{
    auto writer = NeoWriter(GPIO_NUM_3);
    const int numLeds = 24;
    auto data = std::vector<Led>(numLeds);
    float brightness = 0.10f;
    while(true)
    {
        float t = float(esp_timer_get_time()) / 0'050'000.0f;
        float scalar = 0.12;

        for(int i = 0; i < numLeds; ++i)
        {
            auto color = palette((t + (float)i) * scalar);
            data[i].r = uint8_t(color.y * brightness * 255);
            data[i].g = uint8_t(color.x * brightness * 255);
            data[i].b = uint8_t(color.z * brightness * 255);
        }

        writer.write(data);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
