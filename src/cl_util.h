/* Utility functions for my own development */

//V 2.1.0 - 09/07/14

#include <pebble.h>

#ifndef CL_UTIL_H
#define CL_UTIL_H

void cl_set_debug(bool b);
TextLayer* cl_text_layer_create(GRect location, GColor colour, GColor background, bool custom_font, GFont *g_font, const char *res_id, GTextAlignment alignment);
void cl_applog(char* message);
void cl_interpret_message_result(AppMessageResult app_message_error);

#endif