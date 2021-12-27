#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "Button.h"

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

#define PIN_OUT 16

int64_t onBeat(alarm_id_t, void *);
int64_t onHalfBeat(alarm_id_t id, void *data)
{
    gpio_put(PIN_OUT, false);
    add_alarm_in_ms(500, onBeat, 0, true);
    return 0;
}

int64_t onBeat(alarm_id_t id, void *data)
{
    gpio_put(PIN_OUT, true);
    add_alarm_in_ms(500, onHalfBeat, 0, true);
    return 0;
}

int main()
{
    stdio_init_all();

    struct Button buttons[] = {
        {.pin = 0, .led_pin = 1},
        {.pin = 2, .led_pin = 3},
        {.pin = 4, .led_pin = 5},
        {.pin = 6, .led_pin = 7},
        {.pin = 8, .led_pin = 9},
        {.pin = 10, .led_pin = 11},
        {.pin = 12, .led_pin = 13},
        {.pin = 14, .led_pin = 15},
    };
    for_each_button(buttons, ARRAY_LENGTH(buttons), ButtonInit);

    gpio_init(PIN_OUT);
    gpio_set_dir(PIN_OUT, GPIO_OUT);
    alarm_id_t alarm_id = add_alarm_in_ms(0, onBeat, 0, true);

    while (1)
    {
        for_each_button(buttons, ARRAY_LENGTH(buttons), ButtonUpdate);
    }
}
