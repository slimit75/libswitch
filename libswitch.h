#ifndef LIBSWITCH_H
#define LIBSWITCH_H

#include <XPLMDataAccess.h>

struct {
	int pos;
	float anim_pos;
	XPLMDataRef pos_dr;
	XPLMDataRef anim_pos_dr;
	float gain;
} typedef switch_t;


/**
 *
 * @param sw Switch to modify
 */
void sw_calc_gain(switch_t *sw);

/**
 *
 * @param sw Switch to modify
 */
void sw_button_cb(switch_t *sw);

#endif //LIBSWITCH_H