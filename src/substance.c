/* the Klebeian Group
 * uses code from Simplicity by Pebble

 //natve background, black
 //inverted background, white
 //inverter hidden true, white
 //inverter hidden false, black
 */
#include <pebble.h>
#include <bluetooth.h>
#include <substance.h>

#define SCREEN_PKEY 0
#define SCREEN_DEFAULT 0
#define DATE_SETTING_PKEY 1
#define DATE_SETTING_DEFAULT 1
#define DAY_SETTING_PKEY 2
#define DAY_SETTING_DEFAULT 0
#define SETTING_DAY_POSITION_PKEY 3
#define DAY_POSITION_DEFAULT 1
#define VIBRATE_PKEY 4
#define VIBRATE_DEFAULT 0
#define SECONDS_PKEY 5
#define SECONDS_DEFAULT 0
//  constants

enum Settings {
    SETTING_SCREEN_KEY = 1,
    SETTING_DATE_KEY,
    SETTING_DAY_KEY,
    SETTING_SECONDS_KEY,
    SETTING_DAY_POSITION_KEY,
    SETTING_VIBRATE_KEY,
};

//const char* kDateFormat[] = { "%b %d :%S", "%e %B :%S" };
const char* kDateFormat[] = { "%b %d", "%d %B" };

//  variables
static Window* window;
static TextLayer* text_date_layer;
static TextLayer* text_time_layer;
static TextLayer* text_day_layer;
static TextLayer* read_day_layer;
uint32_t sunInt;
int debug_flag = 0;
int countDown = 0;
int fontCount = 0;
int switchFlag = 0;

//static Bitmap
Layer* line_layer;
Layer* power_bar_layer;
Layer *top_line_layer;
Layer *bottom_line_layer;
//Layer* line_layer;
//static Bitmap
Layer* seconds_layer;
static InverterLayer* inverter_layer;

static char time_text[] = "00.00", date_text[]	= "Xxxxxxxxxxxxxxxxxxxxxxx 00", day_text[] = "Xuuuuuann";
static enum SettingScreen { screen_white		= 0, screen_black, screen_follow, screen_count } screen;
static enum SettingDate { date_month_day 		= 0, date_day_month, date_count } date;
static enum SettingDay  { day_show              = 0, day_hide, day_count } day;
static enum SettingSeconds { seconds_hide 		= 0, seconds_show, seconds_count} seconds;
static enum SettingDayPosition {position_high 	= 0, position_low, position_count } position;
static enum SettingVibrate { vibrate_none 		= 0, vibrate_bt, vibrate_count } vibrate;

static AppSync app;
static uint8_t buffer[256];

void line_layer_update_callback(Layer *layer, GContext* ctx) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

void seconds_layer_update_callback(Layer *layer, GContext* ctx) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

void white_layer_update_callback(Layer *layer, GContext* ctx) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

void black_layer_update_callback(Layer *layer, GContext* ctx) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}


static void handle_battery(BatteryChargeState charge_state) {
    int xPos = charge_state.charge_percent;
    xPos = (144 * xPos) / 100;
    layer_set_frame(power_bar_layer, GRect(xPos, 97, 1, 2));
    layer_set_update_proc(power_bar_layer, black_layer_update_callback);

    if (charge_state.is_charging) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "charge_state.is_charging, battery %i", charge_state.charge_percent);
        layer_set_hidden(top_line_layer, false);
        layer_set_hidden(bottom_line_layer, false);
    } else {

        APP_LOG(APP_LOG_LEVEL_DEBUG, "battery not charging, battery %i", charge_state.charge_percent);
        layer_set_hidden(top_line_layer, true);
        layer_set_hidden(bottom_line_layer, true);
    }
}

uint32_t getSunInt(void) {
    //struct tm* pbltime;
    time_t now;
//  if (pbltime == NULL) {
    now = time(NULL);
    //pbltime = localtime(&now);
    uint32_t nowInt = now;
    uint32_t sunInt = (nowInt % 86400) / 3600;
    return sunInt;
}

void setScreenInversion(int screen) {
    APP_LOG(APP_LOG_LEVEL_INFO, "setScreenInversion called");
    if (screen == screen_black) {
        layer_set_hidden(inverter_layer_get_layer(inverter_layer), screen);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "screen == screen_black, %d", screen);
    } else if (screen == screen_white) {
        layer_set_hidden(inverter_layer_get_layer(inverter_layer), screen);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "screen == screen_white, %d", screen);
    } else if (screen == screen_follow) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "screen == screen_follow %d", screen);
        sunInt = getSunInt();
        if (sunInt <= 6) {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "inverter hidden true, black, sunInt %lu <= 6", sunInt);
            layer_set_hidden(inverter_layer_get_layer(inverter_layer), true);
        } else if (sunInt >= 19) {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "inverter hidden true, black: sunInt %lu >= 19", sunInt);
            layer_set_hidden(inverter_layer_get_layer(inverter_layer), true);
        } else {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "inverter hidden false, white, 7 <= sunInt %lu >= 19", sunInt);
            layer_set_hidden(inverter_layer_get_layer(inverter_layer), false);
        }
    }
}

void fontSwitch(void) {
    int time_x = 0;
    int time_y = 48;
    int date_x = 1;
    int date_y = 32;
    int day_x = -1;
    int day_y = 12;
    if (switchFlag == 0) {
        APP_LOG(APP_LOG_LEVEL_INFO, "switchFlag = %d, change fonts to EASY", switchFlag); /*
		text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ALT_38)));
		text_layer_set_font(text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ALT_20)));
		text_layer_set_font(text_day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ALT_12)));
		*/
        text_layer_set_font(text_day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ORBITRON_LIGHT_13)));
        text_layer_set_font(text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ORBITRON_BOLD_14)));
        text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ORBITRON_BOLD_42)));

        GRect time_frame = layer_get_frame(text_layer_get_layer(text_time_layer));
        GRect date_frame = layer_get_frame(text_layer_get_layer(text_date_layer));
        GRect day_frame = layer_get_frame(text_layer_get_layer(text_day_layer));
        time_frame.origin.x = time_frame.origin.x + time_x;
        time_frame.origin.y = time_frame.origin.y + time_y;
        date_frame.origin.x = date_frame.origin.x + date_x;
        date_frame.origin.y = date_frame.origin.y + date_y;
        day_frame.origin.x = day_frame.origin.x + day_x;
        day_frame.origin.y = day_frame.origin.y + day_y;
        layer_set_frame(text_layer_get_layer(text_time_layer), time_frame);
        layer_set_frame(text_layer_get_layer(text_date_layer), date_frame);
        layer_set_frame(text_layer_get_layer(text_day_layer), day_frame);
        switchFlag = 1;
    } else if (switchFlag == 1) {
        APP_LOG(APP_LOG_LEVEL_INFO, "switchFlag = %d, change fonts to DEFAULT", switchFlag);
        text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_87)));
        text_layer_set_font(text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_42)));
        text_layer_set_font(text_day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_26)));
        GRect time_frame = layer_get_frame(text_layer_get_layer(text_time_layer));
        GRect date_frame = layer_get_frame(text_layer_get_layer(text_date_layer));
        GRect day_frame = layer_get_frame(text_layer_get_layer(text_day_layer));
        time_frame.origin.x = time_frame.origin.x - time_x;
        time_frame.origin.y = time_frame.origin.y - time_y;
        date_frame.origin.x = date_frame.origin.x - date_x;
        date_frame.origin.y = date_frame.origin.y - date_y;
        day_frame.origin.x = day_frame.origin.x - day_x;
        day_frame.origin.y = day_frame.origin.y - day_y;
        layer_set_frame(text_layer_get_layer(text_time_layer), time_frame);
        layer_set_frame(text_layer_get_layer(text_date_layer), date_frame);
        layer_set_frame(text_layer_get_layer(text_day_layer), day_frame); 
        switchFlag = 0;
    }
}

void accel_tap_handler(AccelAxisType axis, int32_t direction) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "accl press received");
    fontSwitch();

    /*	countDown = 8;
    	countdown_routines();
    	accel_tap_service_unsubscribe();
    	APP_LOG(APP_LOG_LEVEL_INFO, "change fonts to BITHAM");
    //	text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OCREXT_40)));
    	text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_EASY_86)));
    //	text_layer_set_font(text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OCREXT_28)));
    	text_layer_set_font(text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_EASY_44)));
    //	text_layer_set_font(text_day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OCREXT_18)));
    	text_layer_set_font(text_day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_EASY_26)));  */


    /*
    	GRect time_frame = layer_get_frame(text_layer_get_layer(text_time_layer));
    	GRect date_frame = layer_get_frame(text_layer_get_layer(text_date_layer));
    	GRect day_frame = layer_get_frame(text_layer_get_layer(text_day_layer));
    	time_frame.origin.x = time_frame.origin.x - 3;
    	time_frame.origin.y = time_frame.origin.y + 46;
    	date_frame.origin.x = date_frame.origin.x - 2;
    	date_frame.origin.y = date_frame.origin.y + 32;
    	day_frame.origin.x = day_frame.origin.x - 1;
    	day_frame.origin.y = day_frame.origin.y + 12;
    	layer_set_frame(text_layer_get_layer(text_time_layer), time_frame);
    	layer_set_frame(text_layer_get_layer(text_date_layer), date_frame);
    	layer_set_frame(text_layer_get_layer(text_day_layer), day_frame);   */
}

void countdown_routines(void) {

    if (countDown > 1) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "countDown = %d", countDown);
//	layer_get_frame(text_layer_get_layer(text_time_layer));
        countDown = countDown - 1;

    } else if (countDown == 1) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "countDown = %d", countDown);
        APP_LOG(APP_LOG_LEVEL_INFO, "setting fonts back to NEW_ALPHA");
        text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_87)));
        text_layer_set_font(text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_42)));
        text_layer_set_font(text_day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_26)));
        countDown = 0;
        /*
        GRect time_frame = layer_get_frame(text_layer_get_layer(text_time_layer));
        GRect date_frame = layer_get_frame(text_layer_get_layer(text_date_layer));
        GRect day_frame = layer_get_frame(text_layer_get_layer(text_day_layer));
        time_frame.origin.x = time_frame.origin.x + 1;
        time_frame.origin.y = time_frame.origin.y - 46;
        date_frame.origin.x = date_frame.origin.x + 2;
        date_frame.origin.y = date_frame.origin.y - 32;
        day_frame.origin.x = day_frame.origin.x + 3;
        day_frame.origin.y = day_frame.origin.y - 12;
        layer_set_frame(text_layer_get_layer(text_time_layer), time_frame);
        layer_set_frame(text_layer_get_layer(text_date_layer), date_frame);
        layer_set_frame(text_layer_get_layer(text_day_layer), day_frame);
        */
        accel_tap_service_subscribe(accel_tap_handler);
    }
}

void display_time(struct tm* pbltime) {
    time_t now;
//  if (pbltime == NULL) {
    now = time(NULL);
    pbltime = localtime(&now);
    uint32_t nowInt = now;
    uint32_t sunInt = (nowInt % 86400) / 3600;
    if (nowInt % 3600 == 0) {
        setScreenInversion(screen);
    }


    /*	if (nowInt % 6 == 0) {
    		accel_tap_handler(4, 4);
    	}  */

    strftime(date_text, sizeof(date_text), kDateFormat[date], pbltime);
    text_layer_set_text(text_date_layer, date_text);
    strftime(day_text, sizeof(day_text), "%A", pbltime);
    text_layer_set_text(text_day_layer, day_text);
    text_layer_set_text(read_day_layer, day_text);

    strftime(time_text, sizeof(time_text), clock_is_24h_style() ? "%H.%M" : "%I.%M", pbltime);
    //  Kludge to handle lack of non-padded hour format string for twelve hour clock.
    if (!clock_is_24h_style() && (time_text[0] == '0')) memmove(time_text, &time_text[1], sizeof(time_text) - 1);
    text_layer_set_text(text_time_layer, time_text);

    layer_set_hidden(text_layer_get_layer(text_day_layer), day);
    layer_set_hidden(text_layer_get_layer(read_day_layer), true);
    layer_set_hidden(seconds_layer, seconds);

    int xPos = now % 60;
    xPos = (144 * xPos) / 60;
    layer_set_frame(seconds_layer, GRect(xPos, 97, 3, 2));
    layer_set_update_proc(seconds_layer, seconds_layer_update_callback);

    countdown_routines();
}

static void tuple_changed_callback(const uint32_t key, const Tuple* tuple_new, const Tuple* tuple_old, void* context) {
    int debug_flag = 0;
    int value = tuple_new->value->uint8;
    switch (key) {
    case SETTING_SCREEN_KEY:
        if ((value >= 0) && (value < screen_count) && (screen != value)) {
            screen = value;
            if (debug_flag == 1) {
                APP_LOG(APP_LOG_LEVEL_DEBUG, "case SETTING_SCREEN_KEY, = %d, 0 = white (inverter showing), 1 = black (inverter hidden), 2 = follow sun (set in display_time)", screen);
            }
            persist_write_int(SCREEN_PKEY, screen);
            setScreenInversion(screen);
        }
        break;

    case SETTING_DATE_KEY:
        if ((value >= 0) && (value < date_count) && (date != value)) {
            if (debug_flag == 1) {
                APP_LOG(APP_LOG_LEVEL_DEBUG, "case SETTING_DATE_KEY, = %d", value);
            }
            persist_write_int(DATE_SETTING_PKEY, value);
            date = value;
            display_time(NULL);
        }
        break;

    case SETTING_DAY_KEY:
        if ((value >= 0) && (value < day_count) && (day != value)) {
            persist_write_int(DAY_SETTING_PKEY, value);
            day = value;
            if (debug_flag > -1) {
                APP_LOG(APP_LOG_LEVEL_DEBUG, "case SETTING_DAY_KEY, = %d, ZERO is NO (day hidden) ONE is YES (day show)", day);
                APP_LOG(APP_LOG_LEVEL_DEBUG, "checking SETTING_DAY_PKEY %lu", persist_read_int(DAY_SETTING_PKEY));

            }
            display_time(NULL);
        }
        break;
    /*
    if (debug_flag > -1) {APP_LOG(APP_LOG_LEVEL_DEBUG, "value %i, day_cound %i, day %i", value, day_count, day);}

    if (debug_flag > -1) {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "wrote DAY POSITION KEY %d", position);
            int intBuffer = persist_read_int(DAY_SETTING_PKEY);
            APP_LOG(APP_LOG_LEVEL_DEBUG, "int intBuffer = %i ", intBuffer);
        }

        if (position == position_high) {
            if (debug_flag > -1) {
                APP_LOG(APP_LOG_LEVEL_DEBUG, "position set to position_high");
            }
            //				layer_set_bounds(text_layer_get_layer(text_day_layer), GRect(9, 44, 204-7, 168));
            layer_set_frame(text_layer_get_layer(text_day_layer), GRect(9, 44, 204-7, 168));
            layer_set_frame(text_layer_get_layer(read_day_layer), GRect(9, 58, 204-7, 168));
        } else if (position == position_low) {
            if (debug_flag > -1) {
                APP_LOG(APP_LOG_LEVEL_DEBUG, "position set to position_low");
            }
            //				layer_set_bounds(text_layer_get_layer(text_day_layer), GRect(9, 148, 204-7, 168));
            layer_set_frame(text_layer_get_layer(text_day_layer), GRect(7, 136, 204-7, 168));
            layer_set_frame(text_layer_get_layer(read_day_layer), GRect(7, 150, 204-7, 168));
        }
        display_time(NULL);
    }
    break;
    */

    case SETTING_DAY_POSITION_KEY:
        if (debug_flag > -1) {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "value %i, position_count %i, position %i", value, position_count, position);
        }
        if ((value >= 0) && (value < position_count) && (position != value)) {
            position = value;
            persist_write_int(SETTING_DAY_POSITION_PKEY, position);
            if (debug_flag > -1) {
                APP_LOG(APP_LOG_LEVEL_DEBUG, "wrote SETTING DAY POSITION KEY %d", position);
                APP_LOG(APP_LOG_LEVEL_DEBUG, "checking SETTING_DAY_POSITION_PKEY %lu", persist_read_int(SETTING_DAY_POSITION_PKEY));
                int intBuffer = persist_read_int(SETTING_DAY_POSITION_PKEY);
                APP_LOG(APP_LOG_LEVEL_DEBUG, "int intBuffer = %i ", intBuffer);
            }

            if (position == position_high) {
                if (debug_flag > -1) {
                    APP_LOG(APP_LOG_LEVEL_DEBUG, "position set to position_high");
                }
//				layer_set_bounds(text_layer_get_layer(text_day_layer), GRect(9, 44, 204-7, 168));
                layer_set_frame(text_layer_get_layer(text_day_layer), GRect(9, 44, 204-7, 168));
                layer_set_frame(text_layer_get_layer(read_day_layer), GRect(9, 58, 204-7, 168));
            } else if (position == position_low) {
                if (debug_flag > -1) {
                    APP_LOG(APP_LOG_LEVEL_DEBUG, "position set to position_low");
                }
//				layer_set_bounds(text_layer_get_layer(text_day_layer), GRect(9, 148, 204-7, 168));
                layer_set_frame(text_layer_get_layer(text_day_layer), GRect(7, 136, 204-7, 168));
                layer_set_frame(text_layer_get_layer(read_day_layer), GRect(7, 150, 204-7, 168));
            }
            display_time(NULL);
        }
        break;

    case SETTING_VIBRATE_KEY:
        if ((value >= 0) && (value < vibrate_count) && (vibrate != value)) {
            vibrate = value;
            persist_write_int(VIBRATE_PKEY, value);
            if (debug_flag == 1) {
                APP_LOG(APP_LOG_LEVEL_DEBUG, "writing VIBRATE_PKEY %d", vibrate);
            }
            if (vibrate == vibrate_bt) {
                bluetooth_connection_service_subscribe(&handle_bluetooth);
                bluetooth_init(bluetooth_connection_service_peek());
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "bt subscribe");
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate_bt %d", vibrate_bt);
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate_none %d", vibrate_none);
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate %d", vibrate);

            } else {
                bluetooth_connection_service_unsubscribe();
                bluetooth_deinit();
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "bt unsubscribe");
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate_bt %d", vibrate_bt);
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate_none %d", vibrate_none);
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate %d", vibrate);
            }
        }
        break;

    case SETTING_SECONDS_KEY:
        if ((value >= 0) && (value < seconds_count) && (seconds != value)) {
            persist_write_int(SECONDS_PKEY, value);
            if (debug_flag == 1) {
                APP_LOG(APP_LOG_LEVEL_DEBUG, "case SETTING_SECONDS_KEY, = %d, ZERO is NO (seconds hidden) ONE is YES (seconds show)", seconds);
            }
            seconds = value;
            display_time(NULL);
        }
        break;

    } //end switchKey
} //end function

static void app_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void* context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "app error %d", app_message_error);
}

static void handle_tick(struct tm* tick_time, TimeUnits units_changed) {
//	if (countDown > 0) {
//		countDown = countDown - 1;
//		fontCount = 1;
//	} else {
    display_time(tick_time);
//		if (fontCount == 1) {
//			text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_86)));
//			text_layer_set_font(text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_44)));
//			text_layer_set_font(text_day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_26)));
//			fontCount = 0;
//		}
//	}
    //seconds_ticker_update();
//	if ((vibrate == vibrate_hourly) && (units_changed & HOUR_UNIT))
//    vibes_short_pulse();
}

void read_persist(void) {
    if (debug_flag > -1) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "read_persistent called, reading initial values");
    }
    screen	 	= (persist_exists(SCREEN_PKEY)) 		? 	persist_read_int(SCREEN_PKEY)			: 	SCREEN_DEFAULT;
    date	 		= (persist_exists(DATE_SETTING_PKEY)) 	? 	persist_read_int(DATE_SETTING_PKEY)	: 	DATE_SETTING_DEFAULT;
    day			= (persist_exists(DAY_SETTING_PKEY))	? 	persist_read_int(DAY_SETTING_PKEY)		: 	DAY_SETTING_DEFAULT;
    if (debug_flag > -1) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "read DAY_SETTING_PKEY %d", day);
    }
    seconds		= (persist_exists(SECONDS_PKEY))		?	persist_read_int(SECONDS_PKEY)		:	SECONDS_DEFAULT;
    vibrate	 	= (persist_exists(VIBRATE_PKEY)) 		? 	persist_read_int(VIBRATE_PKEY)  		: 	VIBRATE_DEFAULT;
    if (debug_flag > -1) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate key was read, is %d", vibrate);
    }
    if (debug_flag > -1) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "read VIBRATE_PKEY %d", vibrate);
    }
    position	= (persist_exists(SETTING_DAY_POSITION_PKEY)) 		? 	persist_read_int(SETTING_DAY_POSITION_PKEY)	: 	DAY_POSITION_DEFAULT;
    if (debug_flag > -1) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "day_position key was read, is %d", position);
    }
    if (debug_flag > -1) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "read SETTING_DAY_POSITION_PKEY %d", position);
    }


    if (debug_flag > -1) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "reading screen %d from SCREEN_PKEY", screen);
    }
    //oldSunriseInt	= sunriseInt;
}

void handle_init(void) {
    APP_LOG(APP_LOG_LEVEL_INFO, "substance v 1.3.1 init sequence");
    //  initialize window
    window = window_create();
    window_set_background_color(window, GColorBlack);
    window_stack_push(window, true);
    //  default settings
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "before read persist screen %d", screen);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "after read persist screen %d", screen);
    //screen = screen_black;
    //date = date_month_day;
    //vibrate = vibrate_none;os

    //layer syntax = xpos, ypos, width, height (thanks for violating one of the oldest customs in the world, mang);
    Layer* root_layer = window_get_root_layer(window);

    bt_layer = text_layer_create(GRect(6, -10, 144, 60));
    bt_border_layer = text_layer_create(GRect(2,2,31,20));
    text_layer_set_background_color(bt_border_layer, GColorWhite);
    text_layer_set_background_color(bt_layer, GColorClear);
    text_layer_set_text_color(bt_layer, GColorBlack);
    //text_layer_set_font(bt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    text_layer_set_font(bt_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_28)));
    text_layer_set_overflow_mode(bt_layer, GTextOverflowModeFill);
    text_layer_set_text_alignment(bt_layer, GTextAlignmentLeft);
    text_layer_set_text(bt_layer, "bt");

    text_date_layer = text_layer_create(GRect(8, 47-2, 204-8, 168-68));
    text_layer_set_text_color(text_date_layer, GColorWhite);
    text_layer_set_background_color(text_date_layer, GColorClear);
    text_layer_set_font(text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_42)));
    //text_layer_set_font(text_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    layer_add_child(root_layer, text_layer_get_layer(text_date_layer));

//    text_time_layer = text_layer_create(GRect(8, 45, 204-7, 168));
    
//    int xx = 30;
//    text_time_layer = text_layer_create(GRect(8, 45, 136, 168));
        text_time_layer = text_layer_create(GRect(0, 45, 144, 168));
    text_layer_set_text_color(text_time_layer, GColorWhite);
    text_layer_set_background_color(text_time_layer, GColorClear);
    text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_87)));
    text_layer_set_text_alignment(text_time_layer, GTextAlignmentRight);
    text_layer_set_overflow_mode(text_time_layer, GTextOverflowModeFill);
    layer_add_child(root_layer, text_layer_get_layer(text_time_layer));

    read_persist();
    if (debug_flag > -1) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "position = %i", position);
    }

    if (position == position_high) {
        text_day_layer = text_layer_create(GRect(9, 44, 204-7, 168));
        read_day_layer = text_layer_create(GRect(9, 44, 204-7, 168));
    } else if (position == position_low) {
        text_day_layer = text_layer_create(GRect(7, 136, 204-7, 168));
        read_day_layer = text_layer_create(GRect(7, 136, 204-7, 168));
    }
    text_layer_set_text_color(text_day_layer, GColorWhite);
    text_layer_set_background_color(text_day_layer, GColorClear);
    text_layer_set_text_color(read_day_layer, GColorWhite);
    text_layer_set_background_color(read_day_layer, GColorClear);
    text_layer_set_font(text_day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_26)));
    text_layer_set_font(read_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    layer_add_child(root_layer, text_layer_get_layer(text_day_layer));
    //layer_add_child(root_layer, text_layer_get_layer(read_day_layer));


    GRect line_frame = GRect(8, 97, 139, 2);
    line_layer = layer_create(line_frame);
    layer_set_update_proc(line_layer, line_layer_update_callback);
    layer_add_child(root_layer, line_layer);

    GRect seconds_frame = GRect(0, 47, 139, 2);
    seconds_layer = layer_create(seconds_frame);
    layer_set_update_proc(seconds_layer, seconds_layer_update_callback);


    layer_add_child(root_layer, text_layer_get_layer(bt_border_layer));
    layer_add_child(root_layer, text_layer_get_layer(bt_layer));

    int x, y, w, h;
    x=0;
    y=97;
    w=144;
    h=2;

    top_line_layer = layer_create(GRect(x+8, y-1, w, 1));
    layer_set_update_proc(top_line_layer, white_layer_update_callback);

    bottom_line_layer = layer_create(GRect(x+8, y+2, w, 1));
    layer_set_update_proc(bottom_line_layer, white_layer_update_callback);

    power_bar_layer = layer_create(GRect(x-1, y, 1, h));
    layer_set_update_proc(power_bar_layer, black_layer_update_callback);

    layer_add_child(root_layer, top_line_layer);
    layer_add_child(root_layer, bottom_line_layer);
    layer_add_child(root_layer, power_bar_layer);
    layer_add_child(root_layer, seconds_layer);

    layer_set_hidden(top_line_layer, false);
    layer_set_hidden(bottom_line_layer, false);


    //base color is BLACK BACKGROUND inverter layer turns background WHITE
    //inverter_layer = inverter_layer_create(GRect((screen == screen_black) ? 144 : 0, 0, 144, 168));
    inverter_layer = inverter_layer_create(GRect(0, 0, 144, 168));
    layer_set_hidden(inverter_layer_get_layer(inverter_layer), 1);
    setScreenInversion(screen);
    layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(inverter_layer));

    //
    Tuplet tuples[] = {
        TupletInteger(SETTING_SCREEN_KEY, screen),
        TupletInteger(SETTING_DATE_KEY, date),
        TupletInteger(SETTING_DAY_KEY, day),
        TupletInteger(SETTING_DAY_POSITION_KEY, position),
        TupletInteger(SETTING_VIBRATE_KEY, vibrate),
        TupletInteger(SETTING_SECONDS_KEY, seconds),
    };

    layer_set_hidden(text_layer_get_layer(bt_layer), true);
    layer_set_hidden(text_layer_get_layer(bt_border_layer), true);
    app_message_open(160, 160);

    if (vibrate == vibrate_bt) {
        bluetooth_init(bluetooth_connection_service_peek());
        APP_LOG(APP_LOG_LEVEL_DEBUG, "bt service peek!");

        APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate_bt %d", vibrate_bt);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate_none %d", vibrate_none);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate %d", vibrate);
        //vibes_short_pulse();

        bluetooth_connection_service_subscribe(&handle_bluetooth);

        //APP_LOG(APP_LOG_LEVEL_DEBUG, "bt subscribe");
    } else {
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate_bt %d", vibrate_bt);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate_none %d", vibrate_none);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate %d", vibrate);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "bt not subscribed");
    }


    app_sync_init(&app, buffer, sizeof(buffer), tuples, ARRAY_LENGTH(tuples), tuple_changed_callback, app_error_callback, NULL);

    display_time(NULL);
    tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
    accel_tap_service_subscribe(accel_tap_handler);
    battery_state_service_subscribe(handle_battery);

    handle_battery(battery_state_service_peek());

    //read_persist();
}

void handle_deinit(void) {
    APP_LOG(APP_LOG_LEVEL_INFO, "deinit sequence: substance");
    fonts_unload_custom_font(fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_28)));
    fonts_unload_custom_font(fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_42)));
    fonts_unload_custom_font(fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_87)));
    fonts_unload_custom_font(fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEW_ALPHABET_26)));
    tick_timer_service_unsubscribe();
    inverter_layer_destroy(inverter_layer);
    accel_data_service_unsubscribe();
    app_sync_deinit(&app);
    bluetooth_connection_service_unsubscribe();
    battery_state_service_unsubscribe();
    text_layer_destroy(text_date_layer);
    text_layer_destroy(text_time_layer);
    text_layer_destroy(text_day_layer);
    text_layer_destroy(bt_layer);
    text_layer_destroy(bt_border_layer);
    layer_destroy(line_layer);
    layer_destroy(top_line_layer);
    layer_destroy(bottom_line_layer);
    layer_destroy(power_bar_layer);
    layer_destroy(seconds_layer);
    window_destroy(window);
}

int main(void) {
    handle_init();
    app_event_loop();
    handle_deinit();
}
