//v1.3.
#include <pebble.h>

#define KEY_STATION1 1
#define KEY_STATION2 2
#define KEY_DEPART_TIME 3
#define KEY_DELAY 4
#define KEY_ARRIVE_TIME 5

char station1[32] = "Wissahickon";
char station2[32] = "Suburban Station";

static char *p_departStation;
static char *p_arriveStation;
  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *title_layer;

static Window *s_trainInfo_window;
static StatusBarLayer *s_statusbar_layer;
static BitmapLayer *s_trainbar_layer;
static TextLayer *s_train_departTime_layer;
static TextLayer *s_train_arriveTime_layer;
static TextLayer *s_train_station1_layer;
static TextLayer *s_train_station2_layer;
static TextLayer *s_train_countdown_layer;

ActionBarLayer *action_bar;

static GFont s_time_font;
static GFont s_weather_font;

int getMinutesLeft(char *pTrainDeparts, int delay){
	// Get departure time hours and min
	static int pDepartHr = 0;
	static int pDepartMn = 0;
	char buffer[3];

	if (strlen(pTrainDeparts) == 6){
		pDepartHr = atoi(pTrainDeparts);
		pDepartMn = atoi(strncpy(buffer, pTrainDeparts+2, 2));
	}
	else if (strlen(pTrainDeparts) == 7) {
		pDepartHr = atoi(strncpy(buffer, pTrainDeparts, 2));
		pDepartMn = atoi(strncpy(buffer, pTrainDeparts+3, 2));
	}
	else {
		// Handles other departure times i.e "Cancelled"
		pDepartHr = 0;
		pDepartMn = 0;
	}
  
	// Look for 'PM'
	if (strstr(pTrainDeparts, "PM")){
		if (pDepartHr < 12) {
			pDepartHr += 12;
		}
	}
	// Future: handle times past midnight
  
	time_t timeNow;
	struct tm *departTime;
	//char bufferDepart[16];
  
	time(&timeNow);
	departTime = localtime(&timeNow);
	departTime->tm_hour = pDepartHr;
	departTime->tm_min = pDepartMn;
	int minutesUntilDeparture = ((mktime(departTime) - timeNow)/60) + delay;
	//strftime(bufferDepart, 16, "%H:%M", departTime);
	//printf("Depart time: %s\n", bufferDepart);
 
	return minutesUntilDeparture;
}

static void getData(char *p_departStation, char *p_arriveStation){
	// Begin dictionary
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	// Add a key-value pair
	dict_write_cstring(iter, KEY_STATION1, p_departStation);
	dict_write_cstring(iter, KEY_STATION2, p_arriveStation);

	// Send the message!
	app_message_outbox_send();
}
  
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	// Store incoming information
	static char depart_buffer[16];
	static char delay_buffer[16];
	static char arrive_buffer[16];
	static char depart_layer_buffer[32];

	// Read tuples for data
	Tuple *depart_tuple = dict_find(iterator, KEY_DEPART_TIME);
	Tuple *delay_tuple = dict_find(iterator, KEY_DELAY);
	Tuple *arrive_tuple = dict_find(iterator, KEY_ARRIVE_TIME);

	// If all data is available, use it
	if (depart_tuple && delay_tuple) {
		snprintf(depart_buffer, sizeof(depart_buffer), "%s", depart_tuple->value->cstring);
		snprintf(delay_buffer, sizeof(delay_buffer), "%s", delay_tuple->value->cstring);
		snprintf(arrive_buffer, sizeof(arrive_buffer), "%s", arrive_tuple->value->cstring); 
    
		// Get minutes until departure
		static char buffer[4];
		snprintf(buffer, sizeof(buffer), "%d", getMinutesLeft(depart_buffer, atoi(delay_buffer)));
		text_layer_set_text(s_train_countdown_layer, buffer);
    
		// Assemble the depart layer string
		snprintf(depart_layer_buffer, sizeof(depart_layer_buffer), "%s +%sm", depart_buffer, delay_buffer);
  
		//Update the text
		text_layer_set_text(s_train_departTime_layer, depart_layer_buffer);
		text_layer_set_text(s_train_station1_layer, p_departStation);
		text_layer_set_text(s_train_station2_layer, p_arriveStation);
		text_layer_set_text(s_train_arriveTime_layer, arrive_buffer);
	}
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


static void update_time() {
	// Get a tm structure
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
  
	// Write the current hours and minutes into a buffer
	static char s_buffer[8];
	strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

	// Display this time on the TextLayer
	text_layer_set_text(s_time_layer, s_buffer);
}


void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	printf("%s \n", "DOWN button clicked");

	p_departStation = station1;
	p_arriveStation = station2;

	getData(p_departStation, p_arriveStation);

	window_stack_push(s_trainInfo_window, true);
	//Window *window = (Window *)context;
}
void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	printf("%s \n", "UP button clicked");

	p_departStation = station2;
	p_arriveStation = station1;

	getData(p_departStation, p_arriveStation);

	window_stack_push(s_trainInfo_window, true);
	//Window *window = (Window *)context;
}
void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	//... called on single click, and every 1000ms of being held ...
	printf("%s \n", "SELECT button clicked");
	//Window *window = (Window *)context;
}  

void config_provider(Window *window) {
	// single click / repeat-on-hold config:
	window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_SELECT, 1000, select_single_click_handler);
}
/*
void train_config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
}
*/  


static void main_window_load(Window *window) {
	// Get information about the Window
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	// Create GFont
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));  

	// Create the TextLayer with specific bounds
	s_time_layer = text_layer_create(
	GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorBlack);
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	text_layer_set_text(s_time_layer, "00:00");
	//text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_font(s_time_layer, s_time_font);

	title_layer = text_layer_create(GRect(0, 0, 144, 20));
	text_layer_set_text(title_layer, "SEPTA R6");

	// Add it as a child layer to the Window's root layer
	layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
	layer_add_child(window_layer, text_layer_get_layer(title_layer));
}

static void main_window_unload(Window *window) {
	text_layer_destroy(s_time_layer);
	text_layer_destroy(title_layer);
}

static void trainInfo_window_load(Window *trainInfo_window) {
	// Get information about the Window
	Layer *window_layer = window_get_root_layer(trainInfo_window);
	GRect bounds = layer_get_bounds(window_layer);
	window_set_background_color(trainInfo_window, GColorBlack);

	// Create status bar layer
	s_statusbar_layer = status_bar_layer_create();
	//int16_t width = layer_get_bounds(window_layer).size.w - ACTION_BAR_WIDTH;
	int16_t width = layer_get_bounds(window_layer).size.w;
	GRect frame = GRect(0, 0, width, STATUS_BAR_LAYER_HEIGHT);
	layer_set_frame(status_bar_layer_get_layer(s_statusbar_layer), frame);
	layer_add_child(window_layer, status_bar_layer_get_layer(s_statusbar_layer));

	// Create train bar layer
	s_trainbar_layer = bitmap_layer_create(
	  GRect(PBL_IF_ROUND_ELSE(30, 0), PBL_IF_ROUND_ELSE(43, 38), 16, 114));
	bitmap_layer_set_bitmap(s_trainbar_layer, gbitmap_create_with_resource(RESOURCE_ID_TRAIN_BAR));
	bitmap_layer_set_background_color(s_trainbar_layer, GColorClear);
	layer_add_child(window_get_root_layer(trainInfo_window), bitmap_layer_get_layer(s_trainbar_layer));

	// Create station1 layer
	s_train_station1_layer = text_layer_create(
	  GRect(PBL_IF_ROUND_ELSE(30, 0), PBL_IF_ROUND_ELSE(21, 16), (bounds.size.w), 18));
	text_layer_set_background_color(s_train_station1_layer, GColorClear);
	text_layer_set_font(s_train_station1_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_color(s_train_station1_layer, GColorWhite);
	text_layer_set_text_alignment(s_train_station1_layer, GTextAlignmentLeft);
	text_layer_set_overflow_mode(s_train_station1_layer, GTextOverflowModeWordWrap);
	//text_layer_set_text(s_train_station1_layer, station1);
	layer_add_child(window_get_root_layer(trainInfo_window), text_layer_get_layer(s_train_station1_layer));
  
	// Create depart time layer
	s_train_departTime_layer = text_layer_create(
	  GRect(PBL_IF_ROUND_ELSE(49, 19), 29, (bounds.size.w - 19), 24));
	text_layer_set_background_color(s_train_departTime_layer, GColorClear);
	text_layer_set_font(s_train_departTime_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_color(s_train_departTime_layer, GColorWhite);
	text_layer_set_text_alignment(s_train_departTime_layer, GTextAlignmentLeft);
	text_layer_set_overflow_mode(s_train_departTime_layer, GTextOverflowModeWordWrap);
	text_layer_set_text(s_train_departTime_layer, "Loading...");
	layer_add_child(window_get_root_layer(trainInfo_window), text_layer_get_layer(s_train_departTime_layer));

	// Create GFont
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));  
	// Create countdown layer
	s_train_countdown_layer = text_layer_create(
	GRect(PBL_IF_ROUND_ELSE(49, 19), 76, (bounds.size.w - 19), 48));
	text_layer_set_background_color(s_train_countdown_layer, GColorClear);
	text_layer_set_text_color(s_train_countdown_layer, GColorWhite);
	text_layer_set_text_alignment(s_train_countdown_layer, GTextAlignmentLeft);
	text_layer_set_font(s_train_countdown_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS));
	//text_layer_set_font(s_train_countdown_layer, s_time_font);
	text_layer_set_text(s_train_countdown_layer, "00");
	layer_add_child(window_get_root_layer(trainInfo_window), text_layer_get_layer(s_train_countdown_layer));

	// Create arrive time Layer
	s_train_arriveTime_layer = text_layer_create(
	  GRect(PBL_IF_ROUND_ELSE(49, 19), 127, (bounds.size.w - 19), 24));
	text_layer_set_background_color(s_train_arriveTime_layer, GColorClear);
	text_layer_set_font(s_train_arriveTime_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	text_layer_set_text_color(s_train_arriveTime_layer, GColorWhite);
	text_layer_set_text_alignment(s_train_arriveTime_layer, GTextAlignmentLeft);
	text_layer_set_overflow_mode(s_train_arriveTime_layer, GTextOverflowModeWordWrap);
	layer_add_child(window_get_root_layer(trainInfo_window), text_layer_get_layer(s_train_arriveTime_layer));
  
	// Create station2 layer
	s_train_station2_layer = text_layer_create(
	  GRect(PBL_IF_ROUND_ELSE(30, 0), 149, (bounds.size.w), 18));
	text_layer_set_background_color(s_train_station2_layer, GColorClear);
	text_layer_set_font(s_train_station2_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_color(s_train_station2_layer, GColorWhite);
	text_layer_set_text_alignment(s_train_station2_layer, GTextAlignmentLeft);
	text_layer_set_overflow_mode(s_train_station2_layer, GTextOverflowModeWordWrap);
	//text_layer_set_text(s_train_station1_layer, station2);
	layer_add_child(window_get_root_layer(trainInfo_window), text_layer_get_layer(s_train_station2_layer));

	// Display the window
	window_stack_push(s_trainInfo_window, true);
}

static void trainInfo_window_unload(Window *trainInfo_window) {
	// Destroy train window elements
	status_bar_layer_destroy(s_statusbar_layer);
	bitmap_layer_destroy(s_trainbar_layer);
	text_layer_destroy(s_train_station1_layer);
	text_layer_destroy(s_train_station2_layer);
	text_layer_destroy(s_train_departTime_layer);
	fonts_unload_custom_font(s_weather_font);
	text_layer_destroy(s_train_arriveTime_layer);
	text_layer_destroy(s_train_countdown_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	// Update if the main window is on top
	if (window_stack_get_top_window() == s_main_window){
		update_time();
	}

	// Update if the train info window is on top
	if (window_stack_get_top_window() == s_trainInfo_window){
		getData(p_departStation, p_arriveStation);
	}  
}

static void init() { 
	// Create main Window element and assign to pointer
	s_main_window = window_create();

	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});

	// Show the Window on the watch, with animated=true
	window_stack_push(s_main_window, true);

	// Register with TickTimerService
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	// Update the time
	update_time();

	// Set click provider 
	window_set_click_config_provider(s_main_window, (ClickConfigProvider) config_provider);

	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);

	// Open AppMessage
	//app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	app_message_open(128, 128);
	
	// Create the train info window 
	s_trainInfo_window = window_create();
	window_set_window_handlers(s_trainInfo_window, (WindowHandlers) {
		.load = trainInfo_window_load,
		.unload = trainInfo_window_unload
	});
}

static void deinit() {
	// Destroy Window
	window_destroy(s_main_window);
	window_destroy(s_trainInfo_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}