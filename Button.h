#include <stdint.h>

#define BUTTON_ONE_SHOT_DELAY_MAX 500

struct Button
{
  const unsigned int pin;
  const unsigned int led_pin;
  bool active;
  bool release_delaying;
  bool pressed;
  unsigned int delay_count;
  uint32_t last_press_time;
};

void ButtonInit(struct Button *);

void ButtonUpdate(struct Button *);

void for_each_button(struct Button *, const unsigned int, void (*)(struct Button *));