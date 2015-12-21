#include <pebble.h>

#define DATEFORMAT_KEY 0

#define COLOR_FOREGROUND GColorWhite
#define COLOR_BACKGROUND GColorBlack

Window* window;

Layer* background;
TextLayer* hour_text;
TextLayer* minute_text;
TextLayer* day_text;
TextLayer* date_text;
TextLayer* month_text;
TextLayer* year_text;
TextLayer* ampm_text;

enum Settings { setting_dateformat = 0, settings_count};
enum DateFormat { ddmmyyyy = 0, mmddyyyy, yyyymmdd, dateformat_count } date_format;

AppSync app;
uint8_t buffer[256];

// utility function to strip a leading space or zero from a string.
char* strip(char* input) {
  if (strlen(input) > 1 && (input[0] == ' ' || input[0] == '0')) {
    return input + 1;
  }
  return input;
}

// callback function for rendering the background layer
void background_update_callback(Layer *me, GContext* ctx) {
  graphics_context_set_fill_color(ctx, COLOR_FOREGROUND);
  graphics_fill_rect(ctx, GRect(2,8,68,68), 4, GCornersAll);
  graphics_fill_rect(ctx, GRect(74,8,68,68), 4, GCornersAll);
  graphics_fill_rect(ctx, GRect(2,92,140,32), 4, GCornersAll);

  if (date_format == yyyymmdd) {
    graphics_fill_rect(ctx, GRect(2,128,68,32), 4, GCornersAll);
    graphics_fill_rect(ctx, GRect(74,128,32,32), 4, GCornersAll);
    graphics_fill_rect(ctx, GRect(110,128,32,32), 4, GCornersAll);
  }  else {
    graphics_fill_rect(ctx, GRect(2,128,32,32), 4, GCornersAll);
    graphics_fill_rect(ctx, GRect(38,128,32,32), 4, GCornersAll);
    graphics_fill_rect(ctx, GRect(74,128,68,32), 4, GCornersAll);
  }

  graphics_context_set_stroke_color(ctx, COLOR_BACKGROUND);
  graphics_draw_line(ctx, GPoint(2,41), GPoint(142,41));
  graphics_draw_line(ctx, GPoint(2,108), GPoint(142,108));
  graphics_draw_line(ctx, GPoint(2,144), GPoint(142,144));
}

// set update the time and date text layers
void display_time(struct tm *tick_time) {
  if (tick_time == NULL) {
		time_t now = time(NULL);
    tick_time = localtime(&now);
  }

  static char hour[]   = "12";
  static char minute[] = "21";
  static char day[]    = "            ";
  static char date[]   = "  ";
  static char month[]  = "  ";
  static char year[]   = "    ";
  static char ampm[]   = "  ";

  if (clock_is_24h_style())  {
    strftime(hour, sizeof(hour), "%H", tick_time);
  }  else {
    strftime(hour, sizeof(hour), "%l", tick_time);
  }

  strftime(minute, sizeof(minute), "%M", tick_time);
  strftime(day, sizeof(day), "%A", tick_time);
  strftime(date, sizeof(date), "%e", tick_time);
  strftime(month, sizeof(month), "%m", tick_time);
  strftime(year, sizeof(year), "%Y", tick_time);

  if (!clock_is_24h_style()) {
    strftime(ampm, sizeof(ampm), "%p", tick_time);
  }

  text_layer_set_text(hour_text, strip(hour));
  text_layer_set_text(minute_text, minute);
  text_layer_set_text(day_text, day);
  text_layer_set_text(date_text, strip(date));
  text_layer_set_text(month_text, strip(month));
  text_layer_set_text(year_text, year);
  text_layer_set_text(ampm_text, ampm);
}

// callback function for minute tick events that update the time and date display
void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  display_time(tick_time);
}

void update_date_format() {
  int date = 0;
  int month = 0;
  int year = 0;

  switch (date_format) {
    case ddmmyyyy:
      date = 2;
      month = 38;
      year = 74;
      break;
    case mmddyyyy:
      date = 38;
      month = 2;
      year = 74;
      break;
    case yyyymmdd:
      date = 110;
      month = 74;
      year = 2;
      break;
    default:
      break;
  }

  GRect rect = layer_get_frame(text_layer_get_layer(date_text));
  rect.origin.x = date;
  layer_set_frame(text_layer_get_layer(date_text), rect);
  rect = layer_get_frame(text_layer_get_layer(month_text));
  rect.origin.x = month;
  layer_set_frame(text_layer_get_layer(month_text), rect);
  rect = layer_get_frame(text_layer_get_layer(year_text));
  rect.origin.x = year;
  layer_set_frame(text_layer_get_layer(year_text), rect);
}

static void tuple_changed_callback(const uint32_t key, const Tuple* tuple_new, const Tuple* tuple_old, void* context) {
  //  we know these values are uint8 format
  int value = tuple_new->value->uint8;
  switch (key) {
    case setting_dateformat:
      if ((value >= 0) && (value < dateformat_count) && (date_format != value)) {
        //  update value
        date_format = value;
        update_date_format();
        display_time(NULL);
      }
      break;
  }
}

void app_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void* context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "app error %d", app_message_error);
}

// utility function for initializing a text layer
void init_text(TextLayer* textlayer, ResourceId font) {
  text_layer_set_text_alignment(textlayer, GTextAlignmentCenter);
  text_layer_set_text_color(textlayer, COLOR_BACKGROUND);
  text_layer_set_background_color(textlayer, GColorClear);
  text_layer_set_font(textlayer, fonts_load_custom_font(resource_get_handle(font)));
}

// callback function for the app initialization
void handle_init() {
  date_format = 0;
  if (persist_exists(DATEFORMAT_KEY)) {
    date_format = persist_read_int(DATEFORMAT_KEY);
  }

  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, COLOR_BACKGROUND);

  background = layer_create(layer_get_frame(window_get_root_layer(window)));
  layer_set_update_proc(background, &background_update_callback);
  layer_add_child(window_get_root_layer(window), background);

  hour_text = text_layer_create(GRect(2, 8 + 4, 68, 68 - 4));
  init_text(hour_text, RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_46);
  minute_text = text_layer_create(GRect(74, 8 + 4, 68, 68 - 4));
  init_text(minute_text, RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_46);
  day_text = text_layer_create(GRect(2, 92 + 2, 140, 32 - 2));
  init_text(day_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22);
  ampm_text = text_layer_create(GRect(4, 48, 16, 12));
  init_text(ampm_text, RESOURCE_ID_FONT_ROBOTO_CONDENSED_SUBSET_10);

  if (date_format == ddmmyyyy) {
    date_text = text_layer_create(GRect(2, 128 + 2, 32, 32 - 2));
    init_text(date_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22);
    month_text = text_layer_create(GRect(38, 128 + 2, 32, 32 - 2));
    init_text(month_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22);
    year_text = text_layer_create(GRect(74, 128 + 2, 68, 32 - 2));
    init_text(year_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22);
  } else if (date_format == mmddyyyy)  {
    month_text = text_layer_create(GRect(2, 128 + 2, 32, 32 - 2));
    init_text(month_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22);
    date_text = text_layer_create(GRect(38, 128 + 2, 32, 32 - 2));
    init_text(date_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22);
    year_text = text_layer_create(GRect(74, 128 + 2, 68, 32 - 2));
    init_text(year_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22);
  } else if (date_format == yyyymmdd)  {
    year_text = text_layer_create(GRect(2, 128 + 2, 68, 32 - 2));
    init_text(year_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22);
    month_text = text_layer_create(GRect(74, 128 + 2, 32, 32 - 2));
    init_text(month_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22);
    date_text = text_layer_create(GRect(110, 128 + 2, 32, 32 - 2));
    init_text(date_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22);
  }

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(hour_text));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(minute_text));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(day_text));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_text));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(month_text));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(year_text));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(ampm_text));

  Tuplet tuples[] = {
    TupletInteger(setting_dateformat, date_format)
  };

  app_message_open(160,160);
  app_sync_init(&app, buffer, sizeof(buffer), tuples, ARRAY_LENGTH(tuples), tuple_changed_callback, app_error_callback, NULL);
  display_time(NULL);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
}

void handle_deinit() {
  tick_timer_service_unsubscribe();
  persist_write_int(DATEFORMAT_KEY, date_format);
  text_layer_destroy(hour_text);
  text_layer_destroy(minute_text);
  text_layer_destroy(day_text);
  text_layer_destroy(ampm_text);
  text_layer_destroy(date_text);
  text_layer_destroy(month_text);
  text_layer_destroy(year_text);

  app_sync_deinit(&app);

  // TODO: unload custom fonts

  layer_destroy(background);
  window_destroy(window);
}

// main entry point of this Pebble watchface
int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
