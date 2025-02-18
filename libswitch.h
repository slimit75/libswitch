#ifndef LIBSWITCH_H
#define LIBSWITCH_H

typedef int switch_t;

/**
 * @brief Initialize a basic, 2 position switch
 *
 * @param dr_name Name of the switches state dataref. This is for systems, and should not be used for animations.
 * @param dr_anim_name Name of the switch's animation dataref. This is for animations only, and should not be used for animations.
 * @param cmd_name Name of the switch's toggle command. This should be used in clickspots.
 * @param cmd_desc Description to show up in the X-Plane UI.
 *
 * @return An ID for the switch.
 */
switch_t sw_basic_init(const char *dr_name, const char *dr_anim_name, const char *cmd_name, const char *cmd_desc);

/**
 * @brief Initialize a basic, spring-loaded, 2 position switch
 *
 * @param dr_name Name of the switches state dataref. This is for systems, and should not be used for animations.
 * @param dr_anim_name Name of the switch's animation dataref. This is for animations only, and should not be used for animations.
 * @param cmd_name Name of the switch's toggle command. This should be used in clickspots.
 * @param cmd_desc Description to show up in the X-Plane UI.
 *
 * @return An ID for the switch.
 */
switch_t sw_spring_init(const char *dr_name, const char *dr_anim_name, const char *cmd_name, const char *cmd_desc);

/**
 * @brief Initialize a switch with multiple positions.
 *
 * @param dr_name Name of the switches state dataref. This is for systems, and should not be used for animations.
 * @param dr_anim_name Name of the switch's animation dataref. This is for animations only, and should not be used for animations.
 * @param cmd_name_l Name of the switch's toggle command. This should be used in clickspots.
 * @param cmd_desc_l Description to show up in the X-Plane UI.
 * @param cmd_name_r Name of the switch's toggle command. This should be used in clickspots.
 * @param cmd_desc_r Description to show up in the X-Plane UI.
 * @param min_range Minimum value the switch can be.
 * @param max_range Maximum value the switch can be.
 * @param default_value Default value of the switch.
 * @param starter If true, the last position will be spring-loaded.
 *
 * @return An ID for the switch.
 */
switch_t sw_multi_init(const char *dr_name, const char *dr_anim_name, const char *cmd_name_l, const char *cmd_desc_l, const char *cmd_name_r, const char *cmd_desc_r, int min_range, int max_range, int default_value, bool starter);

int sw_basic_get_state(void *inRefcon);
void sw_basic_write_state(void *inRefcon, int inValue);

int sw_spring_get_state(void *inRefcon);
void sw_spring_write_state(void *inRefcon, int inValue);

int sw_multi_get_state(void *inRefcon);
void sw_multi_write_state(void *inRefcon, int inValue);

/**
 * @brief Refreshes all registered switches.
 */
void sw_ref();

/**
 * @brief Destroys all registered switches.
 */
void sw_destroy();

#endif //LIBSWITCH_H