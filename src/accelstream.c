#include <pebble.h>
#include "cl_util.h"

#DEFINE KEY_START 0
#DEFINE SAMPLE_BUFFER_CAPACITY 50
#DEFINE NUM_SAMPLES 10
#DEFINE MAX_SAMPLES_TO_SEND (app_message_outbox_size_maximum() / sizeof(AccelData)) - 1

static Window *window;
static TextLayer *x_layer;

static AccelData sample_buffer[SAMPLE_BUFFER_CAPACITY];
static int queue_start = 0, queue_size = 0;

static int subscribed = 0;

static void debug_log(char *message)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, message); 
}

// Window management
static void window_load(Window *window)
{
  x_layer = cl_text_layer_create(GRect(0, 0, 144, 24), GColorBlack, GColorClear, false, 0, FONT_KEY_GOTHIC_18, GTextAlignmentLeft);
  text_layer_set_text(x_layer, "Waiting for Android...");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(x_layer));
}

static void window_unload(Window *window)
{
  text_layer_destroy(x_layer);
}

// Queue management
static inline int queue_full() {
  return queue_size >= SAMPLE_BUFFER_CAPACITY - 1;
}

static inline int queue_empty() {
  return queue_size == 0;
}

static int queue_push(AccelData *data) {
  if (!queue_full()) {
    int queue_spot = (queue_start + queue_size) % SAMPLE_BUFFER_CAPACITY;
    memcpy(sample_buffer + queue_spot, data, sizeof(AccelData));
    queue_size++;
    return 1;
  } else {
    debug_log("Tried to push onto full queue.\n");
    return 0;
  }
}

static AccelData *queue_pop() {
  if (queue_empty()) {
    debug_log("Tried to pop from empty queue.\n");
    return NULL;
  } else {
    AccelData *retval = sample_buffer + queue_start;
    queue_size--;
    queue_start = (queue_start + 1) % SAMPLE_BUFFER_CAPACITY;
    return retval;
  }
}

// Accelerometer data
static void accel_new_data(AccelData *data, uint32_t num_samples)
{
  for(unsigned int i = 0; i < num_samples && !queue_full(); i++) {
    queue_push(data + i);
  }

  if(queue_full()) {
    debug_log("Queue filled.");
  }
}

static void send_next_data()
{
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  for(int i = 0; i < MAX_SAMPLES_TO_SEND && !queue_empty(); i++)
  {
    dict_write_data(iter, i, (const uint8_t *)queue_pop(), sizeof(AccelData));
  }

  app_message_outbox_send();
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

// Message service handlers
static void in_received_handler(DictionaryIterator *iter, void *context)
{
  Tuple *t = dict_read_first(iter);

  while(t != NULL)
  {
    process_tuple(t);
    t = dict_read_next(iter);
  }
}

static void out_sent_handler(DictionaryIterator *iter, void *context)
{
  send_next_data();
}

static void in_dropped_handler(AppMessageResult reason, void *context)
{
  cl_interpret_message_result(reason);
}

static void out_failed_handler(DictionaryIterator *iter, AppMessageResult result, void *context)
{
  cl_interpret_message_result(result);
}

static void init(void)
{
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  subscribed = 0;

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
