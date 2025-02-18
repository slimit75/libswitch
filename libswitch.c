#include "libswitch.h"

#include <stdlib.h>
#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>

#define SWITCH_GAIN 0.2f

switch_t *all_sw;

int sw_button_cb(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon) {
	int i = (int)inRefcon;

	if (all_sw[i].state == 0) {
		all_sw[i].act_gain = SWITCH_GAIN;
	}
	else {
		all_sw[i].act_gain = -SWITCH_GAIN;
	}

	all_sw[i].state = !all_sw[i].state;

	return 1;
}

void sw_ref() {
	for (int i = 0; i < (sizeof(&all_sw) / sizeof(all_sw[0])); i++) {
		if (all_sw[i].act_gain != 0) {
			all_sw[i].anim_pos += all_sw[i].act_gain;

			// TODO: holy if statement
			if (((all_sw[i].act_gain < 0) && (all_sw[i].anim_pos < (float)all_sw[i].state)) || (
					(all_sw[i].act_gain > 0) && (all_sw[i].anim_pos > (float)all_sw[i].state))) {
				all_sw[i].anim_pos = (float)all_sw[i].state;
				all_sw[i].act_gain = 0;
			}
		}
		else if (all_sw[i].anim_pos != (float)all_sw[i].state) {
			all_sw[i].anim_pos = (float)all_sw[i].state;
		}
	}
}

int sw_dr_fetch_state(void *inRefcon) {
	return all_sw[(int)inRefcon].state;
}

void sw_dr_write_state(void *inRefcon, int inValue) {
	all_sw[(int)inRefcon].state = inValue;
}

float sw_dr_fetch_anim(void *inRefcon) {
	return all_sw[(int)inRefcon].anim_pos;
}

switch_t *sw_basic_init(const char *dr_name, const char *dr_anim_name, const char *cmd_name) {
	int idx;
	if (!all_sw) {
		all_sw = malloc(sizeof(switch_t));
		idx    = 0;
	}
	else {
		idx = (sizeof(&all_sw) / sizeof(all_sw[0]));
	}

	all_sw[idx].state    = 0;
	all_sw[idx].anim_pos = 0;
	all_sw[idx].act_gain = 0;

	XPLMRegisterCommandHandler(all_sw[idx].cmd_toggle, sw_button_cb, 1, (void *)idx);
	XPLMRegisterDataAccessor(dr_name, xplmType_Int, true, sw_dr_fetch_state, sw_dr_write_state, NULL, NULL, NULL, NULL,
							 NULL, NULL,
							 NULL, NULL, NULL, NULL, (void *)idx, (void *)idx);
	XPLMRegisterDataAccessor(dr_anim_name, xplmType_Float, false, NULL, NULL, sw_dr_fetch_anim, NULL, NULL, NULL, NULL,
							 NULL,
							 NULL, NULL, NULL, NULL, (void *)idx, NULL);

	return &all_sw[idx];
};

void sw_destroy_all() {
	for (int i = 0; i < (sizeof(&all_sw) / sizeof(all_sw[0])); i++) {
		XPLMUnregisterCommandHandler(all_sw[i].cmd_toggle, sw_button_cb, 1, (void *)i);
	}

	free(all_sw);
}
