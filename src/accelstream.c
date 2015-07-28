#include <pebble.h>
#include "cl_util.h"

#define KEY_START 0

#define SAMPLE_BUFFER_CAPACITY 50
#define NUM_SAMPLES 10

static Window *window;
static TextLayer *x_layer, *y_layer, *z_layer;
static AccelData latest_data[NUM_SAMPLES];
static int latest_samples;
static int subscribed = 0;

static void debug_log(char *message)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, message); 
}

static void window_load(Window *window)
{
  x_layer = cl_text_layer_create(GRect(0, 0, 144, 24), GColorBlack, GColorClear, false, 0, FONT_KEY_GOTHIC_18, GTextAlignmentLeft);
  text_layer_set_text(x_layer, "Waiting for Android...");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(x_layer));

  y_layer = cl_text_layer_create(GRect(0, 24, 144, 24), GColorBlack, GColorClear, false, 0, FONT_KEY_GOTHIC_18, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(y_layer));

  z_layer = cl_text_layer_create(GRect(0, 48, 144, 24), GColorBlack, GColorClear, false, 0, FONT_KEY_GOTHIC_18, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(z_layer));
}

static void window_unload(Window *window)
{
  text_layer_destroy(x_layer);
  text_layer_destroy(y_layer);
  text_layer_destroy(z_layer);
}

static void accel_new_data(AccelData *data, uint32_t num_samples)
{
  if(latest_samples) {
    debug_log("Overwriting samples.");
  }
  memcpy(latest_data, data, num_samples * sizeof(AccelData));
  latest_samples = num_samples;
}

static void in_dropped_handler(AppMessageResult reason, void *context)
{
  //cl_interpret_message_result(reason);
}

static void send_next_data()
{
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  for(int i = 0; i < latest_samples; i++)
  {
    dict_write_data(iter, i, (const uint8_t *)(&(latest_data[i])), sizeof(AccelData));
  }
  
  latest_samples = 0;

  app_message_outbox_send();
}

static void out_sent_handler(DictionaryIterator *iter, void *context)
{
  //CAUTION - INFINITE LOOP
  send_next_data();
}

static void process_tuple(Tuple *t)
{
  switch(t->key)
  {
  case KEY_START:
    text_layer_set_text(x_layer, "Connection established.");
    send_next_data();
    if(!subscribed) {
      accel_data_service_subscribe(NUM_SAMPLES, (AccelDataHandler)accel_new_data);
      accel_service_set_sampling_rate(ACCEL_SAMPLING_50HZ);
      subscribed = 1;
    }
  break;
  }
}

static void in_received_handler(DictionaryIterator *iter, void *context)
{
  Tuple *t = dict_read_first(iter);

  while(t != NULL)
  {
    process_tuple(t);
    t = dict_read_next(iter);
  }
}

static void out_failed_handler(DictionaryIterator *iter, AppMessageResult result, void *context)
{
  //cl_interpret_message_result(result);
}

static void init(void)
{
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  subscribed = 0;
  latest_samples = 0;

  app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);

  int in_size = app_message_inbox_size_maximum();
  int out_size = app_message_outbox_size_maximum();
  app_message_open(in_size, out_size);

  window_stack_push(window, true);
}

static void deinit(void)
{
  accel_data_service_unsubscribe();
  window_destroy(window);
}

int main(void)
{
  init();
  app_event_loop();
  deinit();
}
