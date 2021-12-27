#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "Button.h"

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))
#define PATTERN_LENGTH 8
#define DELAY_MS 50

struct Pattern
{
    uint8_t pin_out;            // output pin
    struct Button load;         // load button
    uint8_t index;              // index of the pattern
    bool state[PATTERN_LENGTH]; // state of the pattern
};

struct Pattern pattern = {
    .pin_out = 16,
    .load = {
        .pin = 17,
        .no_led = true,
    },
    .index = 0,
    .state = {0, 0, 0, 0, 0, 0, 0, 0}};

int64_t onBeat(alarm_id_t, void *);
int64_t onHalfBeat(alarm_id_t id, void *data)
{
    gpio_put(pattern.pin_out, false);
    add_alarm_in_ms(DELAY_MS, onBeat, 0, true);
    return 0;
}

int64_t onBeat(alarm_id_t id, void *data)
{
    gpio_put(pattern.pin_out, pattern.state[pattern.index]);
    pattern.index = (pattern.index + 1) % PATTERN_LENGTH;
    add_alarm_in_ms(DELAY_MS, onHalfBeat, 0, true);
    return 0;
}

int main()
{
    stdio_init_all();

    struct Button program_btns[] = {
        {.pin = 0, .led_pin = 1},
        {.pin = 2, .led_pin = 3},
        {.pin = 4, .led_pin = 5},
        {.pin = 6, .led_pin = 7},
        {.pin = 8, .led_pin = 9},
        {.pin = 10, .led_pin = 11},
        {.pin = 12, .led_pin = 13},
        {.pin = 14, .led_pin = 15},
    };
    for_each_button(program_btns, ARRAY_LENGTH(program_btns), ButtonInit);

    gpio_init(pattern.pin_out);
    ButtonInit(&pattern.load);
    gpio_set_dir(pattern.pin_out, GPIO_OUT);
    alarm_id_t alarm_id = add_alarm_in_ms(0, onBeat, 0, true);

    while (1)
    {
        for_each_button(program_btns, ARRAY_LENGTH(program_btns), ButtonUpdate);
        ButtonUpdate(&pattern.load);
        if (pattern.load.pressed_flag)
        {
            pattern.load.pressed_flag = false;
            for (int i = 0; i < 8; ++i)
                pattern.state[i] = program_btns[i].active;
        }
    }
}
