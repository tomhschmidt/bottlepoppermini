#include <pebble.h>

Window *window;	
static TextLayer *left_layer;
static BitmapLayer *logo_layer;
static GBitmap *logo;
	
// Key values for AppMessage Dictionary
enum {
	STATUS_KEY = 0,	
	MESSAGE_KEY = 1
};

// Write message to buffer & send
void send_message(void){
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, STATUS_KEY, 0x1);
	dict_write_cstring(iter, MESSAGE_KEY, "Hi Phone, I'm a Pebble!");

	dict_write_end(iter);
  	app_message_outbox_send();
}

void drawLogo()
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  
  logo_layer = bitmap_layer_create(GRect(30,30,bounds.size.w, 100));
  logo = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOGO);
  bitmap_layer_set_bitmap(logo_layer, logo);
}

void writeMessage(char *message) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  left_layer = text_layer_create(GRect(10, 10, bounds.size.w /* width */, 150 /* height */));
  text_layer_set_text(left_layer, message);
  text_layer_set_font(left_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(left_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(left_layer)); 
}

// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *tuple;
	
	tuple = dict_find(received, STATUS_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Status: %d", (int)tuple->value->uint32); 
	}
	
	tuple = dict_find(received, MESSAGE_KEY);
  if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Message: %s", tuple->value->cstring);
    char *message = tuple->value->cstring;
    //char *message = "Hello";
    writeMessage(message);
    //drawLogo();
    
    if (message[0] == '1') {
      message++;
      writeMessage(message);
      vibes_short_pulse();
    } else if (message[0] == '2') {
      message++;
      writeMessage(message);
      vibes_long_pulse();
    }
	} 
}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {	
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
}

void init(void) {
	window = window_create();
	window_stack_push(window, true);
  
	// Register AppMessage handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
		
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
	send_message();
}

void deinit(void) {
	app_message_deregister_callbacks();
	window_destroy(window);
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}