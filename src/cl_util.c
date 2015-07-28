#include "cl_util.h"

static bool DEBUG = false;

/*
 * Set the debug value for these functions
 */
void cl_set_debug(bool b)
{
  DEBUG = b;
}

/*
 * Set up a TextLayer in one line. _add_child() left out for flexibility
 */
TextLayer* cl_text_layer_create(GRect location, GColor colour, GColor background, bool custom_font, GFont *g_font, const char *res_id, GTextAlignment alignment)
{
  TextLayer *layer = text_layer_create(location);
  text_layer_set_text_color(layer, colour);
  text_layer_set_background_color(layer, background);
  if(custom_font == true)
  {
    text_layer_set_font(layer, g_font);
  }
  else
  {
    text_layer_set_font(layer, fonts_get_system_font(res_id));
  }
  text_layer_set_text_alignment(layer, alignment);

  return layer;
}

/*
 * Convenience app_log shortcut
 */
void cl_applog(char* message)
{
  app_log(APP_LOG_LEVEL_INFO, "C", 0, message);
}

/*
 * Convert AppMessageResult to readable console output. Pebble SDK really needs this!
 */
void cl_interpret_message_result(AppMessageResult app_message_error)
{
  if(app_message_error == APP_MSG_OK)
  {
    cl_applog("APP_MSG_OK");
  }

  else if(app_message_error == APP_MSG_SEND_TIMEOUT)
  {
    cl_applog("APP_MSG_SEND_TIMEOUT");
  }

  else if(app_message_error == APP_MSG_SEND_REJECTED)
  {
    cl_applog("APP_MSG_SEND_REJECTED");
  }

  else if(app_message_error == APP_MSG_NOT_CONNECTED)
  {
    cl_applog("APP_MSG_NOT_CONNECTED");
  }

  else if(app_message_error == APP_MSG_APP_NOT_RUNNING)
  {
    cl_applog("APP_MSG_APP_NOT_RUNNING");
  }

  else if(app_message_error == APP_MSG_INVALID_ARGS)
  {
    cl_applog("APP_MSG_INVALID_ARGS");
  }

  else if(app_message_error == APP_MSG_BUSY)
  {
    cl_applog("APP_MSG_BUSY");
  }

  else if(app_message_error == APP_MSG_BUFFER_OVERFLOW)
  {
    cl_applog("APP_MSG_BUFFER_OVERFLOW");
  }

  else if(app_message_error == APP_MSG_ALREADY_RELEASED)
  {
    cl_applog("APP_MSG_ALREADY_RELEASED");
  }

  else if(app_message_error == APP_MSG_CALLBACK_ALREADY_REGISTERED)
  {
    cl_applog("APP_MSG_CALLBACK_ALREADY_REGISTERED");
  }

  else if(app_message_error == APP_MSG_CALLBACK_NOT_REGISTERED)
  {
    cl_applog("APP_MSG_CALLBACK_NOT_REGISTERED");
  }

  else if(app_message_error == APP_MSG_OUT_OF_MEMORY)
  {
    cl_applog("APP_MSG_OUT_OF_MEMORY");
  }
}
