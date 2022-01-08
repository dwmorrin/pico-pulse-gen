#include <stdio.h>
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "Button.h"

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

uint tempo_delay_ms = 0;

// map of the GPIO pins
enum PIN_ASSIGN
{
    PROG_BTN_1,
    PROG_LED_1,
    PROG_BTN_2,
    PROG_LED_2,
    PROG_BTN_3,
    PROG_LED_3,
    PROG_BTN_4,
    PROG_LED_4,
    PROG_BTN_5,
    PROG_LED_5,
    PROG_BTN_6,
    PROG_LED_6,
    PROG_BTN_7,
    PROG_LED_7,
    PROG_BTN_8,
    PROG_LED_8,
    CLK_LED,
    PATT_OUT_1,
    PATT_BTN_1,
    PATT_OUT_2,
    PATT_BTN_2,
    PATT_OUT_3,
    PATT_BTN_3,
    NOT_USED,
    NOT_USED_,
    BUILT_IN_LED,
    READ_WRITE,
    TEMPO_POT,
};

#define PATTERN_LENGTH 8

struct Pattern
{
    uint8_t pin_out;            // output pin
    struct Button load;         // load button
    bool state[PATTERN_LENGTH]; // state of the pattern
};

struct Pattern_Container
{
    uint8_t index;
    struct Pattern clock;
    struct Pattern progammable[3];
};

uint8_t pattern_index = 0;

struct Pattern patterns[] = {
    {
        .pin_out = PATT_OUT_1,
        .load = {
            .pin = PATT_BTN_1,
            .no_led = true,
        },
        .state = {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        .pin_out = PATT_OUT_2,
        .load = {
            .pin = PATT_BTN_2,
            .no_led = true,
        },
        .state = {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        .pin_out = PATT_OUT_3,
        .load = {
            .pin = PATT_BTN_3,
            .no_led = true,
        },
        .state = {0, 0, 0, 0, 0, 0, 0, 0},
    }};

// no load button for clock; always on
const struct Pattern clock = {
    .pin_out = CLK_LED,
    .state = {1, 1, 1, 1, 1, 1, 1, 1}};

int64_t onBeat(alarm_id_t, void *);
int64_t onHalfBeat(alarm_id_t id, void *data)
{
    gpio_clr_mask(
        (1 << patterns[0].pin_out) |
        (1 << patterns[1].pin_out) |
        (1 << patterns[2].pin_out) |
        (1 << clock.pin_out));
    add_alarm_in_ms(tempo_delay_ms, onBeat, 0, true);
    return 0;
}

int64_t onBeat(alarm_id_t id, void *data)
{
    uint32_t mask = (1 << clock.pin_out);
    // unrolled loop to be as fast as possible, untested if necessary
    if (patterns[0].state[pattern_index])
        mask |= (1 << patterns[0].pin_out);
    if (patterns[1].state[pattern_index])
        mask |= (1 << patterns[1].pin_out);
    if (patterns[2].state[pattern_index])
        mask |= (1 << patterns[2].pin_out);
    gpio_set_mask(mask);
    pattern_index = (pattern_index + 1) % PATTERN_LENGTH;
    add_alarm_in_ms(tempo_delay_ms, onHalfBeat, 0, true);
    return 0;
}

void Pattern_Update(struct Button *, const unsigned int);
void Pattern_Setup()
{
    // clock is just an LED out
    gpio_init(clock.pin_out);
    gpio_set_dir(clock.pin_out, GPIO_OUT);

    // patterns have a button input and an output
    for (int i = 0; i < 3; ++i)
    {
        gpio_init(patterns[i].pin_out);
        ButtonInit(&patterns[i].load);
        gpio_set_dir(patterns[i].pin_out, GPIO_OUT);
    }

    // READ/WRITE could be slide switch or button that is held when writing
    gpio_init(READ_WRITE);
    gpio_set_dir(READ_WRITE, GPIO_IN);
}

#define MIN_DELAY_MS 50.0
#define MAX_DELAY_MS 1000.0

uint tempo_map(uint16_t adc_value)
{
    return (MIN_DELAY_MS-MAX_DELAY_MS)/4095.0 * adc_value + MAX_DELAY_MS;
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

    Pattern_Setup();

    adc_init();
    adc_gpio_init(TEMPO_POT);
    adc_select_input(1);
    tempo_delay_ms = tempo_map(adc_read());

    alarm_id_t alarm_id = add_alarm_in_ms(0, onBeat, 0, true);

    while (1)
    {
        for_each_button(program_btns, ARRAY_LENGTH(program_btns), ButtonUpdate);
        Pattern_Update(program_btns, ARRAY_LENGTH(program_btns));
        tempo_delay_ms = tempo_map(adc_read());
    }
}

void Pattern_Update(struct Button *program_btns, const uint length)
{
    for (int i = 0; i < 3; ++i)
    {
        ButtonUpdate(&patterns[i].load);
        if (patterns[i].load.pressed_flag)
        {
            patterns[i].load.pressed_flag = false;
            // high is a write
            if (gpio_get(READ_WRITE))
                for (int index = 0; index < length; ++index)
                    patterns[i].state[index] = program_btns[index].active;
            // low is a read
            else
                for (int index = 0; index < length; ++index)
                {
                    // load pattern into the buttons and button LEDs
                    program_btns[index].active = patterns[i].state[index];
                    gpio_put(program_btns[index].led_pin, patterns[i].state[index]);
                }
        }
    }

}
