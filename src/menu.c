#include "menu.h"

static List_Elements listElements = {0};
static unsigned int current_item_active = 0;
static unsigned int caption_index =0;

void Menu_Init() {
	listElements.count = 0;
	//listElements.items =
}

void Menu_Free(){
	for(int i=0; i<listElements.count; i++){
		free_c(listElements.items[i]);
	}
	listElements.count = 0;
}

void Menu_Add_Item( char* caption,  functiontype fnc ){
	Menu_Add_Item_Param(caption, fnc, "");
}
void Menu_Add_Item_Param( char* caption,  functiontype fnc, char* param1 ){
	MENU_List_Item *item = malloc_c(sizeof(MENU_List_Item));
	strcpy(item->caption, caption);
	strcpy(item->param1, param1);
	item->is_selected = listElements.count > 0 ? 0 : 1;
	item->function = fnc;

	listElements.items[listElements.count] = item;
	listElements.count++;
}

int get_active_item_index(){
	for (int i=0; i<listElements.count;i++){
		if ( listElements.items[i]->is_selected == 0 ){
			continue;
		}

		return i;
	}

	return 0;
}

void list_select_item_at_index(int index){
	for (int i=0; i<listElements.count;i++){
		if ( i == index ){
			listElements.items[i]->is_selected = 1;
		} else {
			listElements.items[i]->is_selected = 0;
		}
	}
	current_item_active = 0;
	caption_index = 0;
}


MENU_List_Item* Menu_Get_Active_Item(){
	return listElements.items[get_active_item_index()];
}

void render_menu() {
	current_item_active++;
	SSD1306_Fill(SSD1306_COLOR_BLACK);

	int from,to,displayed =0;
	int active = get_active_item_index();
	from = active-1;
	to = active+2;

	if( from < 0 ){
		from = 0;
		to = 3;
	}

	if (to > listElements.count ){
		to = listElements.count;
	}

	if( current_item_active > 10 ){
		caption_index++;
		current_item_active = 0;
	}
	current_item_active++;

	for(int i=from; i<to;i++) {
		MENU_List_Item* item = listElements.items[i];
		SSD1306_GotoXY(0, displayed*18+2);

		if(caption_index >= strlen(item->caption)){
			caption_index = 0;
		}

		if (item->is_selected == 1) {
			SSD1306_DrawFilledRectangle(0, displayed*18, 128, 18+2,SSD1306_COLOR_WHITE);
			SSD1306_Puts(&item->caption[strlen(item->caption) > 11 ? caption_index : 0], &Font_11x18, SSD1306_COLOR_BLACK);
		} else {
			SSD1306_Puts(item->caption, &Font_11x18, SSD1306_COLOR_WHITE);
		}

		displayed++;
	}

	SSD1306_UpdateScreen();
}


void controller_menu(int btn) {
	if (btn & BTN2){
		int i = get_active_item_index();
		int prev_item_i = i-1;
		if ( prev_item_i < 0 ) {
			prev_item_i = listElements.count-1;
		}
		list_select_item_at_index(prev_item_i);
	}

	if (btn & BTN3){
		int i = get_active_item_index();
		int next_item_i = i+1;
		if ( next_item_i >= listElements.count ) {
			next_item_i = 0;
		}
		list_select_item_at_index(next_item_i);
	}

	if (btn & BTN4) {
		int i = get_active_item_index();
		listElements.items[i]->function();
	}

	if (btn & BTN1) {
		Display_Set_Active_Controller(controller_homepage);
		Display_Set_Active_Render(render_homepage);
	}
}

