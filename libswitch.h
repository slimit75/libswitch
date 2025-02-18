#ifndef LIBSWITCH_H
#define LIBSWITCH_H

#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>

struct {
	int state;
	float anim_pos;
	XPLMDataRef dr_state;
	XPLMDataRef dr_anim;
	XPLMCommandRef cmd_toggle;
	float act_gain;
} typedef switch_t;

/**
 * @brief Initialize a basic, 2 position switch
 *
 * @param dr_name Name of the switches state dataref. This is for systems, and should not be used for animations.
 * @param dr_anim_name Name of the switch's animation dataref. This is for animations only, and should not be used for animations.
 * @param cmd_name Name of the switch's toggle command. This should be used in clickspots.
 *
 * @return A pointer to the switch object.
 */
switch_t *sw_basic_init(const char *dr_name, const char *dr_anim_name, const char *cmd_name);

/**
 * @brief Refreshes all registered switches.
 */
void sw_ref();

/**
 * @brief Destroys all registered switches.
 */
void sw_destroy_all();

#endif //LIBSWITCH_H