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

// set update the time and date text layers
void display_time(struct tm *tick_time) {
  static char hour[]   = "  ";
  static char minute[] = "  ";
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
void init_text(TextLayer* textlayer, ResourceId font, GTextAlignment alignment) {
  text_layer_set_text_alignment(textlayer, alignment);
  text_layer_set_text_color(textlayer, COLOR_BACKGROUND);
  text_layer_set_background_color(textlayer, GColorClear);
  text_layer_set_font(textlayer, fonts_load_custom_font(resource_get_handle(font)));
}

// callback function for rendering the background layer
void background_update_callback(Layer *me, GContext* ctx) {
  graphics_context_set_fill_color(ctx, COLOR_FOREGROUND);
  graphics_fill_rect(ctx, GRect(2,  12,  64, 60), 4, GCornersAll); // hours
  graphics_fill_rect(ctx, GRect(70,  34,  4,  4), 0, GCornerNone);  // colon
  graphics_fill_rect(ctx, GRect(70,  46,  4,  4), 0, GCornerNone);  //   "
  graphics_fill_rect(ctx, GRect(78, 12,  64, 60), 4, GCornersAll); // mins

  graphics_fill_rect(ctx, GRect(2,  90, 140, 32), 4, GCornersAll); // weekday

  graphics_fill_rect(ctx, GRect(2,  128, 32, 32), 4, GCornersAll); // day
  graphics_fill_rect(ctx, GRect(35, 158,  4,  4), 1, GCornersAll); // .
  graphics_fill_rect(ctx, GRect(40, 128, 32, 32), 4, GCornersAll); // month
  graphics_fill_rect(ctx, GRect(73, 158,  4,  4), 1, GCornersAll); // .
  graphics_fill_rect(ctx, GRect(78, 128, 64, 32), 4, GCornersAll); // year

  graphics_context_set_stroke_color(ctx, COLOR_BACKGROUND);
  graphics_draw_line(ctx, GPoint( 2,  41), GPoint(142,  41)); // time
  graphics_draw_line(ctx, GPoint( 2, 106), GPoint(142, 106)); // weekday
  graphics_draw_line(ctx, GPoint( 2, 144), GPoint(142, 144)); // date
}

// app initialization
void init() {
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, COLOR_BACKGROUND);

  background = layer_create(layer_get_frame(window_get_root_layer(window)));
  layer_set_update_proc(background, &background_update_callback);
  layer_add_child(window_get_root_layer(window), background);

  hour_text = text_layer_create(GRect(6, 12, 54, 64));
  init_text(hour_text, RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_46, GTextAlignmentRight);
  minute_text = text_layer_create(GRect(82, 12, 54, 64));
  init_text(minute_text, RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_46, GTextAlignmentRight);

  day_text = text_layer_create(GRect(2, 92, 140, 30));
  init_text(day_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22, GTextAlignmentCenter);

  date_text = text_layer_create(GRect(5, 130, 26, 30));
  init_text(date_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22, GTextAlignmentRight);
  month_text = text_layer_create(GRect(43, 130, 26, 30));
  init_text(month_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22, GTextAlignmentRight);
  year_text = text_layer_create(GRect(82, 130, 60, 30));
  init_text(year_text, RESOURCE_ID_FONT_ROBOTO_BOLD_22, GTextAlignmentCenter);

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

void deinit() {
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
  init();
  app_event_loop();
  deinit();
}
