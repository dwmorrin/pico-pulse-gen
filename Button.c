#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "Button.h"

#define NOW() (to_us_since_boot(get_absolute_time()))

void ButtonInit(struct Button *b)
{
    gpio_init(b->pin);
    gpio_set_dir(b->pin, GPIO_IN);
    if (!b->no_led)
    {
        gpio_init(b->led_pin);
        gpio_set_dir(b->led_pin, GPIO_OUT);
    }
}

void ButtonUpdate(struct Button *b)
{
    bool pin_is_high = gpio_get(b->pin);
    if (!b->pressed && b->release_delaying)
    {
        if (b->delay_count < BUTTON_ONE_SHOT_DELAY_MAX)
            ++b->delay_count;
        if (b->delay_count == BUTTON_ONE_SHOT_DELAY_MAX)
            b->release_delaying = false;
    }
    else if (!b->pressed && pin_is_high)
    {
        b->pressed = true;
        b->active = !b->active;
        b->release_delaying = true;
        b->delay_count = 0;
        if (!b->no_led)
            gpio_put(b->led_pin, b->active);
        b->last_press_time = NOW();
        b->pressed_flag = true;
        printf("Button %d pressed\n", b->pin);
    }
    else if (b->pressed)
    {
        if (b->release_delaying && b->delay_count < BUTTON_ONE_SHOT_DELAY_MAX)
            ++b->delay_count;
        if (b->delay_count == BUTTON_ONE_SHOT_DELAY_MAX)
        {
            if (b->release_delaying)
            {
                b->release_delaying = false;
                printf("Button %d delay done [%d]\n", b->pin, NOW() - b->last_press_time);
            }
            if (!pin_is_high)
            {
                b->pressed = false;
                b->pressed_flag = false;
                b->delay_count = 0;
                b->release_delaying = true;
                printf("Button %d released\n", b->pin);
            }
        }
    }
}

void for_each_button(struct Button *buttons, const uint length, void (*cb)(struct Button *))
{
    for (uint i = 0; i < length; ++i)
        cb(&buttons[i]);
}