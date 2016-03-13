#include <pebble.h>

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

AppSync app;

// utility function to strip a leading space or zero from a string.
char* strip(char* input) {
  return input + (((strlen(input) > 1) && (input[0] == '0')) ? 1 : 0);
}

// callback function for rendering the background layer
void background_update_callback(Layer *me, GContext* ctx) {
  graphics_context_set_fill_color(ctx, COLOR_FOREGROUND);
  graphics_fill_rect(ctx, GRect(2,8,68,68), 4, GCornersAll);
  graphics_fill_rect(ctx, GRect(74,8,68,68), 4, GCornersAll);
  graphics_fill_rect(ctx, GRect(2,92,140,32), 4, GCornersAll);

  graphics_fill_rect(ctx, GRect(2,128,32,32), 4, GCornersAll);
  graphics_fill_rect(ctx, GRect(38,128,32,32), 4, GCornersAll);
  graphics_fill_rect(ctx, GRect(74,128,68,32), 4, GCornersAll);

  graphics_context_set_stroke_color(ctx, COLOR_BACKGROUND);
  graphics_draw_line(ctx, GPoint(2,41), GPoint(142,41));
  graphics_draw_line(ctx, GPoint(2,108), GPoint(142,108));
  graphics_draw_line(ctx, GPoint(2,144), GPoint(142,144));
}

// set update the time and date text layers
void display_time(struct tm *tick_time) {
  static char hour[]   = "12";
  static char minute[] = "21";
  static char day[]    = "            ";
  static char date[]   = "  ";
  static char month[]  = "  ";
  static char year[]   = "    ";

  strftime(hour, sizeof(hour), "%H", tick_time);
  strftime(minute, sizeof(minute), "%M", tick_time);
  strftime(day, sizeof(day), "%A", tick_time);
  strftime(date, sizeof(date), "%e", tick_time);
  strftime(month, sizeof(month), "%m", tick_time);
  strftime(year, sizeof(year), "%Y", tick_time);

  text_layer_set_text(hour_text, strip(hour));
  text_layer_set_text(minute_text, minute);
  text_layer_set_text(day_text, day);
  text_layer_set_text(date_text, date);
  text_layer_set_text(month_text, strip(month));
  text_layer_set_text(year_text, year);
}

// callback function for minute tick events that update the time and date display
void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  display_time(tick_time);
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
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, COLOR_BACKGROUND);

  background = layer_create(layer_get_frame(window_get_root_layer(window)));
  layer_set_update_proc(background, &background_update_callback);
  layer_add_child(window_get_root_layer(window), background);

  hour_text = text_layer_create(GRect(2, 12, 68, 64));
  init_text(hour_text, RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_46);
  minute_text = text_layer_create(GRect(74, 12, 68, 64));
  init_text(minute_text, RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_46);
  day_text = text_layer_create(GRect(2, 94, 140, 30));
  init_text(day_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22);

  date_text = text_layer_create(GRect(2, 130, 32, 30));
  init_text(date_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22);
  month_text = text_layer_create(GRect(38, 130, 32, 30));
  init_text(month_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22);
  year_text = text_layer_create(GRect(74, 130, 68, 30));
  init_text(year_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(hour_text));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(minute_text));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(day_text));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_text));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(month_text));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(year_text));

	time_t now = time(NULL);
  display_time(localtime(&now));
  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
}

void handle_deinit() {
  tick_timer_service_unsubscribe();
  text_layer_destroy(hour_text);
  text_layer_destroy(minute_text);
  text_layer_destroy(day_text);
  text_layer_destroy(date_text);
  text_layer_destroy(month_text);
  text_layer_destroy(year_text);
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
