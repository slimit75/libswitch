#include "libswitch.h"

#include <stdlib.h>
#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>

#define SWITCH_GAIN 0.2f

struct {
	int state;
	float act_gain;
	float anim_pos;
	XPLMDataRef dr_state;
	XPLMDataRef dr_anim;
	XPLMCommandRef cmd_toggle;
} typedef sw_basic_t;

struct {
	int min;
	int max;
	int state;
	float act_gain;
	float anim_pos;
	bool starter;
	XPLMDataRef dr_state;
	XPLMDataRef dr_anim;
	XPLMCommandRef cmd_toggle_l;
	XPLMCommandRef cmd_toggle_r;
} typedef sw_multi_t;

sw_basic_t *all_basic_sw;
sw_basic_t *all_spring_sw;
sw_multi_t *all_multi_sw;

int sw_basic_cb(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon) {
	int i = (int)inRefcon;

	if (all_basic_sw[i].state == 0) {
		all_basic_sw[i].act_gain = SWITCH_GAIN;
	}
	else {
		all_basic_sw[i].act_gain = -SWITCH_GAIN;
	}

	all_basic_sw[i].state = !all_basic_sw[i].state;

	return 1;
}

int sw_spring_cb(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon) {
	int i = (int)inRefcon;

	if (inPhase == 0) {
		all_spring_sw[i].act_gain = SWITCH_GAIN;
		all_spring_sw[i].state = 1;
	}
	else if (inPhase == 2) {
		all_spring_sw[i].act_gain = -SWITCH_GAIN;
		all_spring_sw[i].state = 0;
	}
	return 1;
}

int sw_multi_l_cb(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon) {
	int i = (int)inRefcon;

	if ((inPhase == 0) && (all_multi_sw[i].state > all_multi_sw[i].min)) {
		all_multi_sw[i].act_gain = -SWITCH_GAIN;
		all_multi_sw[i].state -= 1;
	}

	return 1;
}

int sw_multi_r_cb(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon) {
	int i = (int)inRefcon;

	if ((inPhase == 0) && (all_multi_sw[i].state < all_multi_sw[i].max)) {
		all_multi_sw[i].act_gain = SWITCH_GAIN;
		all_multi_sw[i].state += 1;
	}
	else if ((all_multi_sw[i].starter == true) && (all_multi_sw[i].state == all_multi_sw[i].max) && (inPhase == 2)) {
		all_multi_sw[i].act_gain = -SWITCH_GAIN;
		all_multi_sw[i].state -= 1;
	}

	return 1;
}

// TODO: can this refresh section be better?
void sw_ref_basic(sw_basic_t *array) {
	for (int i = 0; i < (sizeof(&array) / sizeof(array[0])); i++) {
		if (array[i].act_gain != 0) {
			array[i].anim_pos += array[i].act_gain;

			// TODO: holy if statement
			if (((array[i].act_gain < 0) && (array[i].anim_pos < (float)array[i].state)) || (
					(array[i].act_gain > 0) && (array[i].anim_pos > (float)array[i].state))) {
				array[i].anim_pos = (float)array[i].state;
				array[i].act_gain = 0;
			}
		}
		else if (array[i].anim_pos != (float)array[i].state) {
			array[i].anim_pos = (float)array[i].state;
		}
	}
}

void sw_ref() {
	sw_ref_basic(all_basic_sw);
	sw_ref_basic(all_spring_sw);

	for (int i = 0; i < (sizeof(&all_multi_sw) / sizeof(all_multi_sw[0])); i++) {
		if (all_multi_sw[i].act_gain != 0) {
			all_multi_sw[i].anim_pos += all_multi_sw[i].act_gain;

			// TODO: holy if statement
			if (((all_multi_sw[i].act_gain < 0) && (all_multi_sw[i].anim_pos < (float)all_multi_sw[i].state)) || (
					(all_multi_sw[i].act_gain > 0) && (all_multi_sw[i].anim_pos > (float)all_multi_sw[i].state))) {
				all_multi_sw[i].anim_pos = (float)all_multi_sw[i].state;
				all_multi_sw[i].act_gain = 0;
			}
		}
		else if (all_multi_sw[i].anim_pos != (float)all_multi_sw[i].state) {
			all_multi_sw[i].anim_pos = (float)all_multi_sw[i].state;
		}
	}
}

int sw_basic_get_state(void *inRefcon) {
	return all_basic_sw[(int)inRefcon].state;
}

void sw_basic_write_state(void *inRefcon, int inValue) {
	all_basic_sw[(int)inRefcon].state = inValue;
}

float sw_basic_get_anim(void *inRefcon) {
	return all_basic_sw[(int)inRefcon].anim_pos;
}

int sw_spring_get_state(void *inRefcon) {
	return all_spring_sw[(int)inRefcon].state;
}

void sw_spring_write_state(void *inRefcon, int inValue) {
	all_spring_sw[(int)inRefcon].state = inValue;
}

float sw_spring_get_anim(void *inRefcon) {
	return all_spring_sw[(int)inRefcon].anim_pos;
}

int sw_multi_get_state(void *inRefcon) {
	return all_multi_sw[(int)inRefcon].state;
}

void sw_multi_write_state(void *inRefcon, int inValue) {
	all_multi_sw[(int)inRefcon].state = inValue;
}

float sw_multi_get_anim(void *inRefcon) {
	return all_multi_sw[(int)inRefcon].anim_pos;
}

switch_t sw_basic_init(const char *dr_name, const char *dr_anim_name, const char *cmd_name, const char *cmd_desc) {
	int idx;
	if (all_basic_sw == NULL) {
		all_basic_sw = malloc(sizeof(sw_basic_t));
		idx = 0;
	}
	else {
		all_basic_sw = realloc(all_basic_sw, sizeof(all_basic_sw) + sizeof(sw_basic_t));
		idx = (sizeof(&all_basic_sw) / sizeof(sw_basic_t));
	}

	all_basic_sw[idx].state = 0;
	all_basic_sw[idx].anim_pos = 0;
	all_basic_sw[idx].act_gain = 0;

	all_basic_sw[idx].cmd_toggle = XPLMCreateCommand(cmd_name, cmd_desc);
	XPLMRegisterCommandHandler(all_basic_sw[idx].cmd_toggle, sw_basic_cb, 1, (void *)idx);

	if (dr_name) {
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

	if (dr_anim_name) {
		XPLMRegisterDataAccessor(
			dr_anim_name,
			xplmType_Float,
			false,
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
			NULL,
			NULL,
			(void *)idx,
			NULL
		);
	}

	return idx;
}

switch_t sw_spring_init(const char *dr_name, const char *dr_anim_name, const char *cmd_name, const char *cmd_desc) {
	int idx;
	if (all_spring_sw == NULL) {
		all_spring_sw = malloc(sizeof(sw_basic_t));
		idx = 0;
	}
	else {
		all_spring_sw = realloc(all_spring_sw, sizeof(all_spring_sw) + sizeof(sw_basic_t));
		idx = (sizeof(&all_spring_sw) / sizeof(sw_basic_t));
	}

	all_spring_sw[idx].state = 0;
	all_spring_sw[idx].anim_pos = 0;
	all_spring_sw[idx].act_gain = 0;

	all_spring_sw[idx].cmd_toggle = XPLMCreateCommand(cmd_name, cmd_desc);
	XPLMRegisterCommandHandler(all_spring_sw[idx].cmd_toggle, sw_spring_cb, 1, (void *)idx);

	if (dr_name) {
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

	if (dr_anim_name) {
		XPLMRegisterDataAccessor(
			dr_anim_name,
			xplmType_Float,
			false,
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
			NULL,
			NULL,
			(void *)idx,
			NULL
		);
	}

	return idx;
}

switch_t sw_multi_init(const char *dr_name, const char *dr_anim_name, const char *cmd_name_l, const char *cmd_desc_l,
					   const char *cmd_name_r, const char *cmd_desc_r, int min_range, int max_range, int default_value,
					   bool starter) {
	int idx;
	if (all_multi_sw == NULL) {
		all_multi_sw = malloc(sizeof(sw_multi_t));
		idx = 0;
	}
	else {
		all_multi_sw = realloc(all_multi_sw, sizeof(all_multi_sw) + sizeof(sw_multi_t));
		idx = (sizeof(&all_multi_sw) / sizeof(sw_multi_t));
	}

	all_multi_sw[idx].state = default_value;
	all_multi_sw[idx].anim_pos = 0;
	all_multi_sw[idx].act_gain = 0;
	all_multi_sw[idx].min = min_range;
	all_multi_sw[idx].max = max_range;
	all_multi_sw[idx].starter = starter;

	all_multi_sw[idx].cmd_toggle_l = XPLMCreateCommand(cmd_name_l, cmd_desc_l);
	all_multi_sw[idx].cmd_toggle_r = XPLMCreateCommand(cmd_name_r, cmd_desc_r);
	XPLMRegisterCommandHandler(all_multi_sw[idx].cmd_toggle_l, sw_multi_l_cb, 1, (void *)idx);
	XPLMRegisterCommandHandler(all_multi_sw[idx].cmd_toggle_r, sw_multi_r_cb, 1, (void *)idx);

	if (dr_name) {
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

	if (dr_anim_name) {
		XPLMRegisterDataAccessor(
			dr_anim_name,
			xplmType_Float,
			false,
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
			NULL,
			NULL,
			(void *)idx,
			NULL
		);
	}

	return idx;
}

void sw_destroy() {
	for (int i = 0; i < (sizeof(&all_basic_sw) / sizeof(all_basic_sw[0])); i++) {
		XPLMUnregisterCommandHandler(all_basic_sw[i].cmd_toggle, sw_basic_cb, 1, (void *)i);
	}

	for (int i = 0; i < (sizeof(&all_spring_sw) / sizeof(all_spring_sw[0])); i++) {
		XPLMUnregisterCommandHandler(all_spring_sw[i].cmd_toggle, sw_spring_cb, 1, (void *)i);
	}

	for (int i = 0; i < (sizeof(&all_multi_sw) / sizeof(all_multi_sw[0])); i++) {
		XPLMUnregisterCommandHandler(all_multi_sw[i].cmd_toggle_l, sw_multi_l_cb, 1, (void *)i);
		XPLMUnregisterCommandHandler(all_multi_sw[i].cmd_toggle_r, sw_multi_r_cb, 1, (void *)i);
	}

	free(all_basic_sw);
	free(all_spring_sw);
	free(all_multi_sw);
}
