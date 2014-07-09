#pragma once

TextLayer *bt_layer;
TextLayer *bt_border_layer;

void layer_set_hidden (Layer *layer, bool hidden);
void handle_bluetooth(bool);
void bluetooth_init(bool);
void bluetooth_deinit(void);
char *translate_error(AppMessageResult result);

