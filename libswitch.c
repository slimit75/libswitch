#include "libswitch.h"

#include <assert.h>
#include <stdlib.h>
#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>

#define SWITCH_GAIN 0.2

struct {
	// Internal
	sw_type_t type;

	// Data for basic switches
	int state;
	double act_gain;
	double anim_pos;
	XPLMDataRef dr_state;
	XPLMDataRef dr_anim;
	XPLMCommandRef cmd_toggle;

	// Data for multi-position switches
	int min;
	int max;
	bool starter;
	XPLMCommandRef cmd_toggle_l;
	XPLMCommandRef cmd_toggle_r;
} typedef sw_t;

sw_t *all_sw;
int all_sw_size = 0;

// Switch Callbacks
int sw_basic_cb(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon) {
	int i = (int)inRefcon;

	if (inPhase == 0) {
		if (all_sw[i].state == 0) {
			all_sw[i].act_gain = SWITCH_GAIN;
		}
		else {
			all_sw[i].act_gain = -SWITCH_GAIN;
		}

		all_sw[i].state = !all_sw[i].state;
	}

	return xplm_CommandContinue;
}

int sw_spring_cb(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon) {
	int i = (int)inRefcon;

	if (inPhase == 0) {
		all_sw[i].act_gain = SWITCH_GAIN;
		all_sw[i].state = 1;
	}
	else if (inPhase == 2) {
		all_sw[i].act_gain = -SWITCH_GAIN;
		all_sw[i].state = 0;
	}
	return xplm_CommandContinue;
}

int sw_multi_l_cb(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon) {
	int i = (int)inRefcon;

	if ((inPhase == 0) && (all_sw[i].state > all_sw[i].min)) {
		all_sw[i].act_gain = -SWITCH_GAIN;
		all_sw[i].state -= 1;
	}

	return xplm_CommandContinue;
}

int sw_multi_r_cb(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon) {
	int i = (int)inRefcon;

	if ((inPhase == 0) && (all_sw[i].state < all_sw[i].max)) {
		all_sw[i].act_gain = SWITCH_GAIN;
		all_sw[i].state += 1;
	}
	else if ((all_sw[i].starter == true) && (all_sw[i].state == all_sw[i].max) && (inPhase == 2)) {
		all_sw[i].act_gain = -SWITCH_GAIN;
		all_sw[i].state -= 1;
	}

	return xplm_CommandContinue;
}

// Refresh all animation datarefs
void sw_ref() {
	for (int i = 0; i < all_sw_size; i++) {
		if (all_sw[i].act_gain != 0) {
			all_sw[i].anim_pos += all_sw[i].act_gain;

			// We don't want to overshoot the actual current state of the switch
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

// Callbacks to update the state of the switch. Used by X-Plane and other stakeholders
int sw_basic_get_state(void *inRefcon) {
	return all_sw[(int)inRefcon].state;
}

void sw_basic_write_state(void *inRefcon, int inValue) {
	all_sw[(int)inRefcon].state = inValue;
}

double sw_basic_get_anim(void *inRefcon) {
	return all_sw[(int)inRefcon].anim_pos;
}

int sw_spring_get_state(void *inRefcon) {
	return all_sw[(int)inRefcon].state;
}

void sw_spring_write_state(void *inRefcon, int inValue) {
	all_sw[(int)inRefcon].state = inValue;
}

double sw_spring_get_anim(void *inRefcon) {
	return all_sw[(int)inRefcon].anim_pos;
}

int sw_multi_get_state(void *inRefcon) {
	return all_sw[(int)inRefcon].state;
}

void sw_multi_write_state(void *inRefcon, int inValue) {
	all_sw[(int)inRefcon].state = inValue;
}

double sw_multi_get_anim(void *inRefcon) {
	return all_sw[(int)inRefcon].anim_pos;
}

switch_t sw_init() {
	int idx;
	if (all_sw == NULL) {
		// Allocate initial memory if the array doesn't already exist
		all_sw = malloc(sizeof(sw_t));
		all_sw_size = 1;
		idx = 0;
	}
	else {
		// Expand the array to support our new switch
		all_sw_size++;
		all_sw = realloc(all_sw, sizeof(sw_t) * all_sw_size);
		idx = all_sw_size - 1;
	}

	all_sw[idx].state = 0;
	all_sw[idx].anim_pos = 0;
	all_sw[idx].act_gain = 0;

	return idx;
}

switch_t sw_basic_init(const char *dr_name, const char *dr_anim_name, const char *cmd_name, const char *cmd_desc) {
	// Initialize the switch
	int idx = sw_init();
	all_sw[idx].type = SW_BASIC;

	// Register commands
	if (cmd_desc == NULL) {
		cmd_desc = "";
	}

	if (XPLMFindCommand(cmd_name) == NULL) {
		all_sw[idx].cmd_toggle = XPLMCreateCommand(cmd_name, cmd_desc);
	}
	else {
		all_sw[idx].cmd_toggle = XPLMFindCommand(cmd_name);
	}
	XPLMRegisterCommandHandler(all_sw[idx].cmd_toggle, sw_basic_cb, 1, (void *)idx);

	// Register datarefs
	if (dr_name != NULL) {
		XPLMRegisterDataAccessor(
			dr_name,
			xplmType_Int,
			true,
			sw_basic_get_state,
			sw_basic_write_state,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			(void *)idx,
			(void *)idx
		);
	}

	if (dr_anim_name != NULL) {
		XPLMRegisterDataAccessor(
			dr_anim_name,
			xplmType_Double,
			false,
			NULL,
			NULL,
			NULL,
			NULL,
			sw_basic_get_anim,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			(void *)idx,
			NULL
		);
	}

	return idx;
}

switch_t sw_spring_init(const char *dr_name, const char *dr_anim_name, const char *cmd_name, const char *cmd_desc) {
	// Initialize the switch
	int idx = sw_init();
	all_sw[idx].type = SW_SPRING;

	// Register commands
	if (cmd_desc == NULL) {
		cmd_desc = "";
	}

	if (XPLMFindCommand(cmd_name) == NULL) {
		all_sw[idx].cmd_toggle = XPLMCreateCommand(cmd_name, cmd_desc);
	}
	else {
		all_sw[idx].cmd_toggle = XPLMFindCommand(cmd_name);
	}
	XPLMRegisterCommandHandler(all_sw[idx].cmd_toggle, sw_spring_cb, true, (void *)idx);

	// Register datarefs
	if (dr_name != NULL) {
		XPLMRegisterDataAccessor(
			dr_name,
			xplmType_Int,
			true,
			sw_spring_get_state,
			sw_spring_write_state,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			(void *)idx,
			(void *)idx
		);
	}

	if (dr_anim_name != NULL) {
		XPLMRegisterDataAccessor(
			dr_anim_name,
			xplmType_Double,
			false,
			NULL,
			NULL,
			NULL,
			NULL,
			sw_spring_get_anim,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			(void *)idx,
			NULL
		);
	}

	return idx;
}

switch_t sw_multi_init(const char *dr_name, const char *dr_anim_name, const char *cmd_name_l, const char *cmd_desc_l,
					   const char *cmd_name_r, const char *cmd_desc_r, const int min_range, const int max_range,
					   const int default_value,
					   const bool starter) {
	// Initialize the switch
	int idx = sw_init();
	all_sw[idx].type = SW_MULTI;

	all_sw[idx].state = default_value;
	all_sw[idx].min = min_range;
	all_sw[idx].max = max_range;
	all_sw[idx].starter = starter;

	// Register commands
	if (XPLMFindCommand(cmd_name_l) == NULL) {
		all_sw[idx].cmd_toggle_l = XPLMCreateCommand(cmd_name_l, cmd_desc_l);
	}
	else {
		all_sw[idx].cmd_toggle_l = XPLMFindCommand(cmd_name_l);
	}

	if (XPLMFindCommand(cmd_name_r) == NULL) {
		all_sw[idx].cmd_toggle_r = XPLMCreateCommand(cmd_name_r, cmd_desc_r);
	}
	else {
		all_sw[idx].cmd_toggle_r = XPLMFindCommand(cmd_name_r);
	}

	// Register datarefs
	if (dr_name != NULL) {
		XPLMRegisterDataAccessor(
			dr_name,
			xplmType_Int,
			true,
			sw_multi_get_state,
			sw_multi_write_state,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			(void *)idx,
			(void *)idx
		);
	}

	if (dr_anim_name != NULL) {
		XPLMRegisterDataAccessor(
			dr_anim_name,
			xplmType_Double,
			false,
			NULL,
			NULL,
			NULL,
			NULL,
			sw_multi_get_anim,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			(void *)idx,
			NULL
		);
	}

	return idx;
}

void sw_destroy() {
	// Unregister all commands
	for (int i = 0; i < all_sw_size; i++) {
		switch (all_sw[i].type) {
			case SW_SPRING:
				XPLMUnregisterCommandHandler(all_sw[i].cmd_toggle, sw_spring_cb, 1, (void *)i);
				break;
			case SW_MULTI:
				XPLMUnregisterCommandHandler(all_sw[i].cmd_toggle_l, sw_multi_l_cb, 1, (void *)i);
				XPLMUnregisterCommandHandler(all_sw[i].cmd_toggle_r, sw_multi_r_cb, 1, (void *)i);
				break;
			default:
				XPLMUnregisterCommandHandler(all_sw[i].cmd_toggle, sw_basic_cb, 1, (void *)i);
		}
	}

	// Free all memory
	free(all_sw);
}
