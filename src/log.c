#include "log.h"
#include "SSD1306.h"
#include "main_menu.h"

static Log_Entry log_entries[MAX_LOG_ENTRIES] = { };
static int msg_counter =0;
static int scroll=0;
static unsigned int current_item_active = 0;
static unsigned int caption_index =0;

void Log_Message(char* str){
	for( int i=0;i<MAX_LOG_ENTRIES; i++ ){
		strcpy(log_entries[i].message,log_entries[i+1].message);
		log_entries[i].count = log_entries[i+1].count;
	}
	Log_Entry* entry = &log_entries[MAX_LOG_ENTRIES-1];
	strncpy(entry->message, str, MAX_MESSAGE_LENGTH);
	entry->count = msg_counter++;
}

void Log_Get_Messages(Log_Entry* target_list, int limit) {
	for(int i=0;i<limit;i++){
		target_list[i] = log_entries[MAX_LOG_ENTRIES-i];
	}
}


void Log_Init(){

}

void Log_Unregister(){}

void render_log(){
	SSD1306_Fill(SSD1306_COLOR_BLACK);
	SSD1306_GotoXY(0,0);

	if( current_item_active > 10 ){
		caption_index++;
		current_item_active = 0;
	}
	current_item_active++;
	uint8_t is_first=0;

	for (int i=MAX_LOG_ENTRIES-1-scroll;i>=0; i--){
		char message_buffer[1024] = {0};
		strcpy(message_buffer, log_entries[i].message);
		for(int i=0; i<strlen(message_buffer); i++){
			if( message_buffer[i] == '\r' || message_buffer[i] == '\n' ){
				char tmp[1024] = {0};
				strcpy(tmp, &message_buffer[i+1]);
				char symbol = message_buffer[i];
				message_buffer[i] = '\\';
				message_buffer[i+1] =  symbol == '\r' ? 'r' : 'n';
				message_buffer[i+2] = '\0';
				strcat(message_buffer, tmp);
				i++;
			}
		}

		if( is_first++ == 0 ){
			if(caption_index >= strlen(message_buffer) || strlen(message_buffer) < 20 ){
				caption_index = 0;
			}

			SSD1306_Puts(&message_buffer[caption_index], &Font_7x10, SSD1306_COLOR_WHITE);
		} else {
			SSD1306_Puts(message_buffer, &Font_7x10, SSD1306_COLOR_WHITE);
		}

		SSD1306_GotoXY(0,SSD1306_GetY()+10);

	}
	SSD1306_UpdateScreen();

};

void controller_log(int btn){
	if( btn == -1 ){
		Log_Unregister();
		return;
	}

	if ( btn & BTN3 ){
		scroll++;
		caption_index =0;
		return;
	}

	if ( btn & BTN2 ){
		scroll--;
		if (scroll < 0){
			scroll = 0;
		}
		caption_index = 0;
		return;
	}

	if ( btn & BTN1 ){
		Display_Set_Active_Controller(controller_main_menu);
		Display_Set_Active_Render(render_main_menu);
		Main_Menu_Init();
		return;
	}

}
