#include "libswitch.h"

#define SWITCH_GAIN 0.2f

void sw_calc_gain(switch_t *sw) {
	if (sw->pos == 0) {
		sw->gain = SWITCH_GAIN;
	}
	else {
		sw->gain = -SWITCH_GAIN;
	}

	sw->pos = !sw->pos;
}

void sw_button_cb(switch_t *sw) {
	if (sw->gain != 0) {
		sw->anim_pos += sw->gain;

		if ((sw->gain < 0) && (sw->anim_pos < sw->pos)) {
			sw->anim_pos = sw->pos;
			sw->gain     = 0;
		}
		else if ((sw->gain > 0) && (sw->anim_pos > sw->pos)) {
			sw->anim_pos = sw->pos;
			sw->gain     = 0;
		}
	}
	else if (sw->anim_pos != sw->pos) {
		sw->anim_pos = sw->pos;
	}
}