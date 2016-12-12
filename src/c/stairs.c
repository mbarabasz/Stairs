#include "pebble.h"

#define REPEAT_INTERVAL_MS 50

static Window *s_main_window;

static ActionBarLayer *s_action_bar;
static TextLayer *s_header_layer, *s_body_layer, *s_label_layer;
static GBitmap *s_icon_plus, *s_icon_minus;

static int stairs = 0;

static void update_text() {
  static char s_body_text[18];
  snprintf(s_body_text, sizeof(s_body_text), "%u Stairs", stairs);
  text_layer_set_text(s_body_layer, s_body_text);
}

static char *username = "";
char buffer[32];

static void increment_click_handler(ClickRecognizerRef recognizer, void *context) {
  stairs++;
  update_text();
}

// Read settings from persistent storage
static void load_settings() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "reading from persistent storage");
  // Read settings from persistent storage, if they exist
  persist_read_string(1, buffer, sizeof(buffer));
  username = buffer;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "read: %s, %s.", buffer, username);
}

// Save the settings to persistent storage
static void save_settings() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "saving to peristent storage %s", username);
  persist_write_string(1, username);
}

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
 APP_LOG(APP_LOG_LEVEL_DEBUG, "comming back from settings");
  
  // USERNAME
  Tuple *username_t = dict_find(iter, MESSAGE_KEY_Username);
  if (username_t) {
    username = username_t->value->cstring;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "received %s", username);
  }
  
  save_settings();
}

static void send_request(void) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (!iter) {
    // Error creating outbound message
    return;
  }

  int value = stairs;
  dict_write_int(iter, 1, &value, sizeof(int), true);
  dict_write_cstring(iter, 2, username);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
   APP_LOG(APP_LOG_LEVEL_DEBUG, "sending request to JS %d", stairs);
  //TODO call js
  send_request();
  
}

static void decrement_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (stairs <= 0) {
    // Keep the counter at zero
    return;
  }

  stairs--;
  update_text();
}

static void click_config_provider(void *context) {
  window_single_repeating_click_subscribe(BUTTON_ID_UP, REPEAT_INTERVAL_MS, increment_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, REPEAT_INTERVAL_MS, decrement_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_SELECT, REPEAT_INTERVAL_MS, select_click_handler);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  s_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_action_bar, window);
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);

  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_UP, s_icon_plus);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_DOWN, s_icon_minus);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_SELECT, s_icon_minus);

  int width = layer_get_frame(window_layer).size.w - ACTION_BAR_WIDTH - 3;

  s_header_layer = text_layer_create(GRect(4, PBL_IF_RECT_ELSE(0, 30), width, 60));
  text_layer_set_font(s_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_background_color(s_header_layer, GColorClear);
  text_layer_set_text(s_header_layer, "Stairs Counter");
  text_layer_set_text_alignment(s_header_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
  layer_add_child(window_layer, text_layer_get_layer(s_header_layer));

  s_body_layer = text_layer_create(GRect(4, PBL_IF_RECT_ELSE(44, 60), width, 60));
  text_layer_set_font(s_body_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_background_color(s_body_layer, GColorClear);
  text_layer_set_text_alignment(s_body_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
  layer_add_child(window_layer, text_layer_get_layer(s_body_layer));

  s_label_layer = text_layer_create(GRect(4, PBL_IF_RECT_ELSE(44, 60) + 28, width, 60));
  text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_background_color(s_label_layer, GColorClear);
  text_layer_set_text(s_label_layer, "climbed");
  text_layer_set_text_alignment(s_label_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
  layer_add_child(window_layer, text_layer_get_layer(s_label_layer));

  update_text();
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_header_layer);
  text_layer_destroy(s_body_layer);
  text_layer_destroy(s_label_layer);

  action_bar_layer_destroy(s_action_bar);
}

static void init() {
 
  load_settings();
  
  s_icon_plus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_PLUS);
  s_icon_minus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_MINUS);

  stairs = 0;
  
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  
 
  window_stack_push(s_main_window, true);
  app_message_open(128, 128);
  // Open AppMessage connection
  app_message_register_inbox_received(prv_inbox_received_handler);
}

static void deinit() {

  window_destroy(s_main_window);

  gbitmap_destroy(s_icon_plus);
  gbitmap_destroy(s_icon_minus);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
