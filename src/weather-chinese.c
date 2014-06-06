#include "pebble.h"

Window *window;

BitmapLayer *bluetooth_layer;
TextLayer *text_updateTime_layer;
BitmapLayer *battery_layer;

TextLayer *text_date_layer;
TextLayer *text_pm25_layer;
TextLayer *text_daily_weather_layer;

TextLayer *text_time_layer;

BitmapLayer *day1_weather_icon_layer;
TextLayer *text_day1_weather_layer;
BitmapLayer *day2_weather_icon_layer;
TextLayer *text_day2_weather_layer;

enum WeatherKey {
  TEMP = 0x0,
  TEMP_MIN_DAY1 = 0x1,
  TEMP_MAX_DAY1 = 0x2,
  ICON_DAY1 = 0x3,
  DESC_DAY1 = 0x4,
  TEMP_MIN_DAY2 = 0x5,
  TEMP_MAX_DAY2 = 0x6,
  ICON_DAY2 = 0x7,
  DESC_DAY2 = 0x8,
  PM25 = 0x9,
  TS = 0xa
};

char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

void get_current_time(char *time_text, struct tm *tick_time) {
  static char time_str[] = "00:00";
  char *time_format;

  if (!tick_time) {
    time_t now = time(NULL);
    tick_time = localtime(&now);
  }

  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_str, 6, time_format, tick_time);

  if (!clock_is_24h_style() && (time_str[0] == '0')) {
    memmove(time_str, &time_str[1], sizeof(time_str) - 1);
  }
  memmove(time_text, time_str, sizeof(time_str));
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  APP_LOG(APP_LOG_LEVEL_INFO, "start handle_minute_tick");
  // Need to be static because they're used by the system later.
  static char time_text[20];
  get_current_time(time_text, tick_time);

  text_layer_set_text(text_time_layer, time_text);

  static char date_text[30];
  time_t now = time(NULL);
  tick_time = localtime(&now);
  snprintf(date_text, sizeof(date_text), "%d月%d日", tick_time->tm_mon + 1,tick_time->tm_mday);
  text_layer_set_text(text_date_layer, date_text);

  APP_LOG(APP_LOG_LEVEL_INFO, "end handle_minute_tick");
}

int getResourceId(char* icon) {

  if (strcmp(icon, "01d") == 0) {
    return RESOURCE_ID_IMAGE_01D_BLACK;
  } else if (strcmp(icon, "01n") == 0) {
    return RESOURCE_ID_IMAGE_01N_BLACK;
  } else if (strcmp(icon, "02d") == 0) {
    return RESOURCE_ID_IMAGE_02D_BLACK;
  } else if (strcmp(icon, "02n") == 0) {
    return RESOURCE_ID_IMAGE_02N_BLACK;
  } else if (strcmp(icon, "03d") == 0) {
    return RESOURCE_ID_IMAGE_03D_BLACK;
  } else if (strcmp(icon, "03n") == 0) {
    return RESOURCE_ID_IMAGE_03N_BLACK;
  } else if (strcmp(icon, "04d") == 0) {
    return RESOURCE_ID_IMAGE_04D_BLACK;
  } else if (strcmp(icon, "04n") == 0) {
    return RESOURCE_ID_IMAGE_04N_BLACK;
  } else if (strcmp(icon, "09d") == 0) {
    return RESOURCE_ID_IMAGE_09D_BLACK;
  } else if (strcmp(icon, "09n") == 0) {
    return RESOURCE_ID_IMAGE_09N_BLACK;
  } else if (strcmp(icon, "10d") == 0) {
    return RESOURCE_ID_IMAGE_10D_BLACK;
  } else if (strcmp(icon, "10n") == 0) {
    return RESOURCE_ID_IMAGE_10N_BLACK;
  } else if (strcmp(icon, "11d") == 0) {
    return RESOURCE_ID_IMAGE_11D_BLACK;
  } else if (strcmp(icon, "11n") == 0) {
    return RESOURCE_ID_IMAGE_11N_BLACK;
  } else if (strcmp(icon, "13d") == 0) {
    return RESOURCE_ID_IMAGE_13D_BLACK;
  } else if (strcmp(icon, "13n") == 0) {
    return RESOURCE_ID_IMAGE_13N_BLACK;
  } else if (strcmp(icon, "50d") == 0) {
    return RESOURCE_ID_IMAGE_50D_BLACK;
  } else if (strcmp(icon, "50n") == 0) {
    return RESOURCE_ID_IMAGE_50N_BLACK;
  }
  return 0;
}

int getBatteryIcon(uint32_t percentage) {
  if (percentage > 80 && percentage <= 100) {
    return RESOURCE_ID_IMAGE_BATTERY_100_BLACK;
  } else if (percentage > 60 && percentage <= 80) {
    return RESOURCE_ID_IMAGE_BATTERY_80_BLACK;
  } else if (percentage > 40 && percentage <= 60) {
    return RESOURCE_ID_IMAGE_BATTERY_60_BLACK;
  } else if (percentage > 20 && percentage <= 40) {
    return RESOURCE_ID_IMAGE_BATTERY_40_BLACK;
  } else if (percentage > 10 && percentage <= 20) {
    return RESOURCE_ID_IMAGE_BATTERY_20_BLACK;
  } else {
    return RESOURCE_ID_IMAGE_BATTERY_EMPTY_BLACK;
  }
}

GBitmap* getIcon(GBitmap* weather_icon_bitmap, Tuple* icon) {
  if (weather_icon_bitmap) {
    gbitmap_destroy(weather_icon_bitmap);
  }
  weather_icon_bitmap = gbitmap_create_with_resource(getResourceId(icon->value->cstring));
  return weather_icon_bitmap;
}


void in_received_handler(DictionaryIterator *received, void *content) {

  APP_LOG(APP_LOG_LEVEL_INFO, "start in_received_handler");

  Tuple *temp = dict_find(received, TEMP);
  Tuple *temp_min_day1 = dict_find(received, TEMP_MIN_DAY1);
  Tuple *temp_max_day1 = dict_find(received, TEMP_MAX_DAY1);
  Tuple *desc_day1 = dict_find(received, DESC_DAY1);
  Tuple *icon_day1 = dict_find(received, ICON_DAY1);
  Tuple *temp_min_day2 = dict_find(received, TEMP_MIN_DAY2);
  Tuple *temp_max_day2 = dict_find(received, TEMP_MAX_DAY2);
  Tuple *desc_day2 = dict_find(received, DESC_DAY2);
  Tuple *icon_day2 = dict_find(received, ICON_DAY2);
  Tuple *pm25 = dict_find(received, PM25);
  static char weather_text_day0[20];
  static char weather_text_day1[40];
  static char weather_text_day2[40];
  static GBitmap *day1_weather_icon_bitmap = NULL;
  static GBitmap *day2_weather_icon_bitmap = NULL;

  if (temp) {
    snprintf(weather_text_day0, sizeof(weather_text_day0), "%s℃", temp->value->cstring);
    text_layer_set_text(text_daily_weather_layer, weather_text_day0);
  }

  if (temp_min_day1 && temp_max_day1 && desc_day1) {
    snprintf(weather_text_day1, sizeof(weather_text_day1), "今:%s-%s℃", temp_min_day1->value->cstring, temp_max_day1->value->cstring);

    text_layer_set_text(text_day1_weather_layer, weather_text_day1);

    bitmap_layer_set_bitmap(day1_weather_icon_layer, getIcon(day1_weather_icon_bitmap, icon_day1));
  }

  if (temp_min_day2 && temp_max_day2 && desc_day2) {
    snprintf(weather_text_day2, sizeof(weather_text_day2), "明:%s-%s℃", temp_min_day2->value->cstring, temp_max_day2->value->cstring);

    text_layer_set_text(text_day2_weather_layer, weather_text_day2);

    bitmap_layer_set_bitmap(day2_weather_icon_layer, getIcon(day2_weather_icon_bitmap, icon_day2));
  }

  static char pm25text[20];
  if (pm25) {
    snprintf(pm25text, sizeof(pm25text), "PM2.5:%s", pm25->value->cstring);
    text_layer_set_text(text_pm25_layer, pm25text);
  }

  static char update_time[20];
  get_current_time(update_time, NULL);

  snprintf(update_time, 14, "%s更新", update_time);

  text_layer_set_text(text_updateTime_layer, update_time);

  APP_LOG(APP_LOG_LEVEL_INFO, "end in_received_handler");
}

void handle_battery(BatteryChargeState charge_state) {

  APP_LOG(APP_LOG_LEVEL_INFO, "start handle_battery");

  static GBitmap *battery_bitmap = NULL;

  if (charge_state.is_charging) {
    battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGE_BLACK);
  } else {
    if (charge_state.charge_percent <= 20) {
      vibes_long_pulse();
    }
    battery_bitmap = gbitmap_create_with_resource(getBatteryIcon(charge_state.charge_percent));
    APP_LOG(APP_LOG_LEVEL_INFO, "battery_text %d%%", charge_state.charge_percent);
  }


  bitmap_layer_set_bitmap(battery_layer, battery_bitmap);

  APP_LOG(APP_LOG_LEVEL_INFO, "end handle_battery");
}

void handle_bluetooth(bool connected) {

  APP_LOG(APP_LOG_LEVEL_INFO, "connected");
  static GBitmap *bluetooth_bitmap = NULL;

  if (connected) {
    if (!bluetooth_bitmap) {
      bluetooth_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_BLACK);
    }
  } else {
    gbitmap_destroy(bluetooth_bitmap);
    bluetooth_bitmap = NULL;
    vibes_short_pulse();
  }
  bitmap_layer_set_bitmap(bluetooth_layer, bluetooth_bitmap);
}

void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "App Message Dropped!Reason: %i - %s", reason, translate_error(reason));
}

Layer* prepareTextLayer(TextLayer* layer) {
  text_layer_set_text_color(layer, GColorWhite);
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_font(layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IPAG_12)));
  return text_layer_get_layer(layer);
}

void app_message_init(void) {
  app_message_open(256,16);
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
}

void init(void) {
  window = window_create();
  app_message_init();

  window_stack_push(window, true);
  window_set_background_color(window, GColorBlack);

  Layer *window_layer = window_get_root_layer(window);

  //system bar layer----bluetooth
  bluetooth_layer = bitmap_layer_create(GRect(3, 1, 40, 16));
  layer_add_child(window_layer, bitmap_layer_get_layer(bluetooth_layer));
 
  //system bar layer----update time
  text_updateTime_layer = text_layer_create(GRect(43, 1, 60, 16));
  layer_add_child(window_layer, prepareTextLayer(text_updateTime_layer));
 
  //system bar layer----battery
  battery_layer = bitmap_layer_create(GRect(103, 1, 30, 16));
  bitmap_layer_set_alignment(battery_layer, GAlignRight);
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_layer));
 
  //date text layer
  text_date_layer = text_layer_create(GRect(3, 17, 42, 16));
  layer_add_child(window_layer, prepareTextLayer(text_date_layer));

  //PM2.5 text layer
  text_pm25_layer = text_layer_create(GRect(45, 17, 60, 16));
  prepareTextLayer(text_pm25_layer);
  //text_layer_set_font(text_pm25_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, prepareTextLayer(text_pm25_layer));

  //daily weather layer
  text_daily_weather_layer = text_layer_create(GRect(105, 17, 36, 16));
  layer_add_child(window_layer, prepareTextLayer(text_daily_weather_layer));

  //time layer
  text_time_layer = text_layer_create(GRect(3, 33, 138, 54));
  prepareTextLayer(text_time_layer);
  text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

  //day 1 icon layer
  day1_weather_icon_layer = bitmap_layer_create(GRect(3, 87, 69, 64));
  bitmap_layer_set_alignment(day1_weather_icon_layer, GAlignCenter);
  layer_add_child(window_layer, bitmap_layer_get_layer(day1_weather_icon_layer));

  //day 1 weather text layer
  text_day1_weather_layer = text_layer_create(GRect(3, 151, 69, 16));
  layer_add_child(window_layer, prepareTextLayer(text_day1_weather_layer));

  //day 2 icon layer
  day2_weather_icon_layer = bitmap_layer_create(GRect(72, 87, 69, 64));
  bitmap_layer_set_alignment(day2_weather_icon_layer, GAlignCenter);
  layer_add_child(window_layer, bitmap_layer_get_layer(day2_weather_icon_layer));

  //day 2 weather text layer
  text_day2_weather_layer = text_layer_create(GRect(72, 151, 69, 16));
  layer_add_child(window_layer, prepareTextLayer(text_day2_weather_layer));

  handle_battery(battery_state_service_peek());
  handle_bluetooth(bluetooth_connection_service_peek());

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  battery_state_service_subscribe(&handle_battery);
  bluetooth_connection_service_subscribe(&handle_bluetooth);

  handle_minute_tick(NULL, MINUTE_UNIT);
}

void deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  app_message_deregister_callbacks();
}


int main(void) {
  init();

  app_event_loop();

  deinit();
}
