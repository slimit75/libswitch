#include "libswitch.h"

#include <stdlib.h>
#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>

#define SWITCH_GAIN 0.2f

struct {
	// Data for basic switches
	int state;
	float act_gain;
	float anim_pos;
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

//sw_t *all_basic_sw;
//sw_t *all_spring_sw;
//sw_t *all_multi_sw;
sw_t *all_sw;

int sw_basic_cb(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon) {
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
	return 1;
}

int sw_multi_l_cb(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon) {
	int i = (int)inRefcon;

	if ((inPhase == 0) && (all_sw[i].state > all_sw[i].min)) {
		all_sw[i].act_gain = -SWITCH_GAIN;
		all_sw[i].state -= 1;
	}

	return 1;
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

int sw_basic_get_state(void *inRefcon) {
	return all_sw[(int)inRefcon].state;
}

void sw_basic_write_state(void *inRefcon, int inValue) {
	all_sw[(int)inRefcon].state = inValue;
}

float sw_basic_get_anim(void *inRefcon) {
	return all_sw[(int)inRefcon].anim_pos;
}

int sw_spring_get_state(void *inRefcon) {
	return all_sw[(int)inRefcon].state;
}

void sw_spring_write_state(void *inRefcon, int inValue) {
	all_sw[(int)inRefcon].state = inValue;
}

float sw_spring_get_anim(void *inRefcon) {
	return all_sw[(int)inRefcon].anim_pos;
}

int sw_multi_get_state(void *inRefcon) {
	return all_sw[(int)inRefcon].state;
}

void sw_multi_write_state(void *inRefcon, int inValue) {
	all_sw[(int)inRefcon].state = inValue;
}

float sw_multi_get_anim(void *inRefcon) {
	return all_sw[(int)inRefcon].anim_pos;
}

switch_t sw_basic_init(const char *dr_name, const char *dr_anim_name, const char *cmd_name, const char *cmd_desc) {
	int idx;
	if (all_sw == nullptr) {
		all_sw = malloc(sizeof(all_sw[0]));
		idx = 0;
	}
	else {
		all_sw = realloc(all_sw, sizeof(&all_sw) + sizeof(all_sw[0]));
		idx = (sizeof(&all_sw) / sizeof(all_sw[0]));
	}

	all_sw[idx].state = 0;
	all_sw[idx].anim_pos = 0;
	all_sw[idx].act_gain = 0;

	all_sw[idx].cmd_toggle = XPLMCreateCommand(cmd_name, cmd_desc);
	XPLMRegisterCommandHandler(all_sw[idx].cmd_toggle, sw_basic_cb, 1, (void *)idx);

	if (dr_name != nullptr) {
		XPLMRegisterDataAccessor(
			dr_name,
			xplmType_Int,
			true,
			sw_basic_get_state,
			sw_basic_write_state,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			(void *)idx,
			(void *)idx
		);
	}

	if (dr_anim_name != nullptr) {
		XPLMRegisterDataAccessor(
			dr_anim_name,
			xplmType_Float,
			false,
			nullptr,
			nullptr,
			sw_basic_get_anim,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			(void *)idx,
			nullptr
		);
	}

	return idx;
}

switch_t sw_spring_init(const char *dr_name, const char *dr_anim_name, const char *cmd_name, const char *cmd_desc) {
	int idx;
	if (all_sw == nullptr) {
		all_sw = malloc(sizeof(all_sw[0]));
		idx = 0;
	}
	else {
		all_sw = realloc(all_sw, sizeof(&all_sw) + sizeof(all_sw[0]));
		idx = (sizeof(&all_sw) / sizeof(all_sw[0]));
	}

	all_sw[idx].state = 0;
	all_sw[idx].anim_pos = 0;
	all_sw[idx].act_gain = 0;

	all_sw[idx].cmd_toggle = XPLMCreateCommand(cmd_name, cmd_desc);
	XPLMRegisterCommandHandler(all_sw[idx].cmd_toggle, sw_spring_cb, true, (void *)idx);

	if (dr_name != nullptr) {
		XPLMRegisterDataAccessor(
			dr_name,
			xplmType_Int,
			true,
			sw_spring_get_state,
			sw_spring_write_state,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			(void *)idx,
			(void *)idx
		);
	}

	if (dr_anim_name != nullptr) {
		XPLMRegisterDataAccessor(
			dr_anim_name,
			xplmType_Float,
			false,
			nullptr,
			nullptr,
			sw_spring_get_anim,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			(void *)idx,
			nullptr
		);
	}

	return idx;
}

switch_t sw_multi_init(const char *dr_name, const char *dr_anim_name, const char *cmd_name_l, const char *cmd_desc_l,
					   const char *cmd_name_r, const char *cmd_desc_r, int min_range, int max_range, int default_value,
					   bool starter) {
	int idx;
	if (all_sw == nullptr) {
		all_sw = malloc(sizeof(all_sw[0]));
		idx = 0;
	}
	else {
		all_sw = realloc(all_sw, sizeof(&all_sw) + sizeof(all_sw[0]));
		idx = (sizeof(&all_sw) / sizeof(all_sw[0]));
	}

	all_sw[idx].state = default_value;
	all_sw[idx].anim_pos = 0;
	all_sw[idx].act_gain = 0;
	all_sw[idx].min = min_range;
	all_sw[idx].max = max_range;
	all_sw[idx].starter = starter;

	all_sw[idx].cmd_toggle_l = XPLMCreateCommand(cmd_name_l, cmd_desc_l);
	all_sw[idx].cmd_toggle_r = XPLMCreateCommand(cmd_name_r, cmd_desc_r);
	XPLMRegisterCommandHandler(all_sw[idx].cmd_toggle_l, sw_multi_l_cb, true, (void *)idx);
	XPLMRegisterCommandHandler(all_sw[idx].cmd_toggle_r, sw_multi_r_cb, true, (void *)idx);

	if (dr_name != nullptr) {
		XPLMRegisterDataAccessor(
			dr_name,
			xplmType_Int,
			true,
			sw_multi_get_state,
			sw_multi_write_state,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			(void *)idx,
			(void *)idx
		);
	}

	if (dr_anim_name != nullptr) {
		XPLMRegisterDataAccessor(
			dr_anim_name,
			xplmType_Float,
			false,
			nullptr,
			nullptr,
			sw_multi_get_anim,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			(void *)idx,
			nullptr
		);
	}

	return idx;
}

void sw_destroy() {
	for (int i = 0; i < (sizeof(&all_sw) / sizeof(all_sw[0])); i++) {
		XPLMUnregisterCommandHandler(all_sw[i].cmd_toggle, sw_basic_cb, 1, (void *)i);
	}

	for (int i = 0; i < (sizeof(&all_sw) / sizeof(all_sw[0])); i++) {
		XPLMUnregisterCommandHandler(all_sw[i].cmd_toggle, sw_spring_cb, 1, (void *)i);
	}

	for (int i = 0; i < (sizeof(&all_sw) / sizeof(all_sw[0])); i++) {
		XPLMUnregisterCommandHandler(all_sw[i].cmd_toggle_l, sw_multi_l_cb, 1, (void *)i);
		XPLMUnregisterCommandHandler(all_sw[i].cmd_toggle_r, sw_multi_r_cb, 1, (void *)i);
	}

	free(all_sw);
}
