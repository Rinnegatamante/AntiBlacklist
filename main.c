#include <psp2/apputil.h>
#include <psp2/ctrl.h>
#include <psp2/io/fcntl.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/sysmodule.h>
#include <psp2/power.h>
#include <sqlite3.h> 
#include <vita2d.h> 
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BOOL char
#define FALSE 0
#define TRUE 1

#define APP_NAME "Whitelist"
#define APP_VERSION "1.21"
#define MAIN_MENU 0
#define INSTALL_V1 1
#define INSTALL_V2 2
#define EXIT 4
#define REBOOT 5
#define CARD_SWAP 6
#define APP_DB "ur0:/shell/db/app.db"
#define LIST_VITA_UR0 "ur0:/game/launch/list_launch_vita.dat"
#define LIST_EMU_UR0 "ur0:/game/launch/list_launch_emu.dat"
#define LIST_VITA_VS0 "vs0:/data/internal/launch/list_launch_vita.dat"
#define LIST_EMU_VS0 "vs0:/data/internal/launch/list_launch_emu.dat"
#define WHITELIST_SIZE 41
#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544
#define PADDING 10
#define FONT_SIZE 1.0f
#define LINE_HEIGHT 25



int debug_y = 400;

//char *list_vita_ur0 = "ur0:/game/launch/list_launch_vita.dat";
//char *list_emu_ur0 = "ur0:/game/launch/list_launch_emu.dat";
//char *list_vita_vs0 = "vs0:/data/internal/launch/list_launch_vita.dat";
//char *list_emu_vs0 = "vs0:/data/internal/launch/list_launch_emu.dat";

const char * sql_create_trigger_gpc = "CREATE TRIGGER CHANGE_CATEGORY_GPC AFTER INSERT ON tbl_appinfo WHEN new.val LIKE 'gpc' BEGIN UPDATE tbl_appinfo SET val='gdb' WHERE key='566916785' and titleid=new.titleid; END;";
const char * sql_create_trigger_gp = "CREATE TRIGGER CHANGE_CATEGORY_GP AFTER INSERT ON tbl_appinfo WHEN new.val LIKE 'gp' BEGIN UPDATE tbl_appinfo SET val='gdb' WHERE key='566916785' and titleid=new.titleid; END;";
const char * sql_create_trigger_gdc = "CREATE TRIGGER CHANGE_CATEGORY_GDC AFTER INSERT ON tbl_appinfo WHEN new.val LIKE 'gdc' BEGIN UPDATE tbl_appinfo SET val='gdb' WHERE key='566916785' and titleid=new.titleid; END;";
const char * sql_create_trigger_gd = "CREATE TRIGGER CHANGE_CATEGORY_GD AFTER INSERT ON tbl_appinfo WHEN new.val LIKE 'gd' BEGIN UPDATE tbl_appinfo SET val='gdb' WHERE key='566916785' and titleid=new.titleid; END;";
const char * sql_get_trigger_count = "SELECT COUNT(name) FROM sqlite_master WHERE type='trigger' AND tbl_name='tbl_appinfo' AND name IN ('CHANGE_CATEGORY_GPC', 'CHANGE_CATEGORY_GP', 'CHANGE_CATEGORY_GDC', 'CHANGE_CATEGORY_GD'); END;";
const char * sql_drop_trigger_gpc = "DROP TRIGGER CHANGE_CATEGORY_GPC;";
const char * sql_drop_trigger_gp = "DROP TRIGGER CHANGE_CATEGORY_GP;";
const char * sql_drop_trigger_gdc = "DROP TRIGGER CHANGE_CATEGORY_GDC;";
const char * sql_drop_trigger_gd = "DROP TRIGGER CHANGE_CATEGORY_GD;";

extern char* sqlite3_temp_directory;

extern unsigned char _binary_triangle_png_start;
extern unsigned char _binary_circle_png_start;
extern unsigned char _binary_cross_png_start;
extern unsigned char _binary_square_png_start;

uint32_t white = RGBA8(0xFF, 0xFF, 0xFF, 0xFF);
uint32_t white_smoke = RGBA8(0xF5, 0xF5, 0xF5, 0xFF);
uint32_t gainsboro = RGBA8(0xDC, 0xDC, 0xDC, 0xFF);
uint32_t light_gray = RGBA8(0xD3, 0xD3, 0xD3, 0xFF);
uint32_t silver = RGBA8(0xC0, 0xC0, 0xC0, 0xFF);
uint32_t dark_gray = RGBA8(0xA9, 0xA9, 0xA9, 0xFF);
uint32_t gray = RGBA8(0x80, 0x80, 0x80, 0xFF);
uint32_t dim_gray = RGBA8(0x69, 0x69, 0x69, 0xFF);
uint32_t black = RGBA8(0x00, 0x00, 0x00, 0xFF);

uint32_t green_triangle = RGBA8(0x00, 0xFF, 0xCC, 0xFF);
uint32_t red_circle = RGBA8(0xFF, 0x99, 0x99, 0xFF);
uint32_t blue_cross = RGBA8(0x99, 0xCC, 0xFF, 0xFF);
uint32_t pink_square = RGBA8(0xFF, 0x99, 0xFF, 0xFF);

uint32_t lime_green = RGBA8(0x32, 0xCD, 0x32, 0xFF); // success
uint32_t tomato = RGBA8(0xFF, 0x63, 0x47, 0xFF); // error
uint32_t gold = RGBA8(0xFF, 0xCC, 0x00, 0xFF); // warn
uint32_t cornflower_blue = RGBA8(0x64, 0x95, 0xED, 0xFF); // info


//uint32_t dark_gray = RGBA8(0x33, 0x33, 0x33, 0xFF);
//uint32_t light_gray = RGBA8(0x66, 0x66, 0x66, 0xFF);

// list_launch_emu.dat (firmware 3.60 LVER00111)
// SHA-1: f7604e6ef02a85f597fe9144213cd753b17d742f
uint8_t whitelist_emu[WHITELIST_SIZE] = {
	0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x5f, 0x76, 0x65, 0x72, 
	0x73, 0x69, 0x6f, 0x6e, 0x3a, 0x30, 0x30, 0x31, 0x0a, 0x6c, 
	0x69, 0x73, 0x74, 0x5f, 0x74, 0x79, 0x70, 0x65, 0x3a, 0x62, 
	0x0a, 0x4c, 0x56, 0x45, 0x52, 0x30, 0x30, 0x31, 0x31, 0x31, 
	0x0a
};

// list_launch_vita.dat (firmware 3.60 LVER00125)
// SHA-1: eda61d69ae80eb16404b2f9c55682230539a47d4
uint8_t whitelist_vita[WHITELIST_SIZE] = {
	0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x5f, 0x76, 0x65, 0x72, 
	0x73, 0x69, 0x6f, 0x6e, 0x3a, 0x30, 0x30, 0x31, 0x0a, 0x6c, 
	0x69, 0x73, 0x74, 0x5f, 0x74, 0x79, 0x70, 0x65, 0x3a, 0x62, 
	0x0a, 0x4c, 0x56, 0x45, 0x52, 0x30, 0x30, 0x31, 0x32, 0x35, 
	0x0a
};

//list_launch_teleport.dat (firmware 3.60)
//SHA-1: f403f0d2e1a122afc948a30f9da2cab880ef5230

//version_launch.dat (firmware 3.60)
//SHA-1: fcf63eea43d4c5cf8adc1e64ac2cea57617a59a9

static int callback(void *data, int argc, char **argv, char **azColName){
	int i;
	for ( i = 0; i < argc; i++ ) printf ( "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL" );
    return 0;
}

vita2d_pgf* debug_font;

void drawText(int x, int y, uint32_t color, char* text){
	int i;
	for (i=0;i<3;i++){
		vita2d_start_drawing();
		int height = vita2d_pgf_text_height(debug_font, FONT_SIZE, text);
		vita2d_pgf_draw_text(debug_font, x, y+height, color, FONT_SIZE, text);
		vita2d_end_drawing();
		vita2d_wait_rendering_done();
		vita2d_swap_buffers();
	}
}

void drawTextLine(int x, int * y, uint32_t color, char* text){
	int yy = *y;
	int i;
	for (i=0;i<3;i++){
		vita2d_start_drawing();
		int height = vita2d_pgf_text_height(debug_font, FONT_SIZE, text);
		vita2d_pgf_draw_text(debug_font, x, yy+height, color, FONT_SIZE, text);
		vita2d_end_drawing();
		vita2d_wait_rendering_done();
		vita2d_swap_buffers();
	}
	(*y) += LINE_HEIGHT; // advance a line
}

void drawLoopText(int x, int y, uint32_t color, char* text){
	int height = vita2d_pgf_text_height(debug_font, FONT_SIZE, text);
	vita2d_pgf_draw_text(debug_font, x, y+height, color, FONT_SIZE, text);
}

void drawLoopTextLine(int x, int * y, uint32_t color, char* text){
	int yy = *y;
	int height = vita2d_pgf_text_height(debug_font, FONT_SIZE, text);
	vita2d_pgf_draw_text(debug_font, x, yy+height, color, FONT_SIZE, text);
	(*y) += LINE_HEIGHT; // advance a line
}

void drawTexture(int x, int y, vita2d_texture *texture){
	int i;
	for (i=0;i<3;i++){
		vita2d_start_drawing();
		vita2d_draw_texture(texture, x, y); 
		vita2d_end_drawing();
		vita2d_wait_rendering_done();
		vita2d_swap_buffers();
	}
}

void drawLoopTexture(int x, int y, vita2d_texture *texture){
	vita2d_draw_texture(texture, x, y); 
}

void clearScreen(){
	int i;
	for (i=0;i<3;i++){
		vita2d_start_drawing();
		vita2d_clear_screen();
		vita2d_end_drawing();
		vita2d_wait_rendering_done();
		vita2d_swap_buffers();
	}
}

int getTextWidth(char* text){
	return vita2d_pgf_text_width(debug_font, FONT_SIZE, text);
}

int getTextHeight(char* text){
	return vita2d_pgf_text_height(debug_font, FONT_SIZE, text);
}

int rightAlignText(char *text){
	return SCREEN_WIDTH - PADDING - getTextWidth(text);
}

int readIntoBuffer(char *path, void **buffer) {
	SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
	if (fd < 0)
		return fd;
	
	int size = sceIoLseek32(fd, 0, SCE_SEEK_END);
	sceIoLseek32(fd, 0, SCE_SEEK_SET);

	*buffer = malloc(size);
	if (!*buffer) {
		sceIoClose(fd);
		return -1;
	}

	int read = sceIoRead(fd, *buffer, size);
	sceIoClose(fd);
	return read;
}

int compareContents(char *path, uint8_t *contents){
	int is_match = 0;
	void *buffer = NULL;
	int size = readIntoBuffer(path, &buffer);
	if (size < 0)
		return size;

	int result = memcmp(buffer, &contents, size);
	if (result == 0)
		is_match = 1;

	free(buffer);
	return is_match;
}

void newLine(int * vertical_offset){ // next line
	(*vertical_offset) += LINE_HEIGHT;
}

void revertX(int * horizontal_offset){
	(*horizontal_offset) = PADDING;
}

void revertY(int * vertical_offset){
	(*vertical_offset) = PADDING;
}

void revertXY(int * horizontal_offset, int * vertical_offset){
	(*horizontal_offset) = PADDING;
	(*vertical_offset) = PADDING;
}

void revertMargin(int * horizontal_offset, int margin){
	(*horizontal_offset) = margin;
}

void drawStatusPane(int vita_ur0, int emu_ur0, int vita_vs0, int emu_vs0, int has_triggers){
	
	int is_patched_ur0 = 0;
	int is_patched_vs0 = 0;
	int is_patched_v1 = 0;
	int is_patched_v2 = 0;
	
	if ((vita_ur0 == 0) && (emu_ur0 == 0))
		is_patched_ur0 = 1;
	if ((vita_vs0 == 0) && (emu_vs0 == 0))
		is_patched_vs0 = 1;
	if ((is_patched_ur0 == 1) && (is_patched_vs0 == 1))
		is_patched_v1 = 1;
	if (has_triggers == 1)
		is_patched_v2 = 1;
	
	int pane_x = SCREEN_WIDTH/2+((SCREEN_WIDTH/2)/2)-(PADDING*2); // start of status pane
	int margin_left = pane_x+(PADDING*4);
	int margin_right = SCREEN_WIDTH-PADDING;
	int offset_x = pane_x;
	int offset_y = PADDING;
	int text_width = 0;
	int text_height = 0;
	
	//drawLoopText(uint32_t x, uint32_t y, uint32_t color, char* text)
	vita2d_draw_rectangle(offset_x, 0, ((SCREEN_WIDTH/2)/2)+(PADDING*2), SCREEN_HEIGHT, white);
	vita2d_draw_rectangle(offset_x, 0, PADDING*3, SCREEN_HEIGHT, white_smoke);
	vita2d_draw_rectangle(offset_x, 0, PADDING, SCREEN_HEIGHT, dim_gray);
	
	// panel title
	drawLoopTextLine(rightAlignText("Patch Status"), &offset_y, black, "Patch Status");
	
	newLine(&offset_y);
	
	char status0[256];
	char status1[256];
	char status2[256];
	char status3[256];
	
	snprintf(status0, sizeof(status0), "args: %d, %d, %d, %d, %d", vita_ur0, emu_ur0, vita_vs0, emu_vs0, has_triggers);
	snprintf(status1, sizeof(status1), "list_launch (ur0) [%s]", (is_patched_ur0 == 1) ? "x" : "_");
	snprintf(status2, sizeof(status2), "list_launch (vs0) [%s]", (is_patched_vs0 == 1) ? "x" : "_");
	snprintf(status3, sizeof(status3), "app.db triggers [%s]", (is_patched_v2 == 1) ? "x" : "_");
	
	//drawLoopText(rightAlignText(status0), offset_y, dim_gray, status0);
	revertMargin(&offset_x, margin_left);
	drawLoopText(offset_x, offset_y, dim_gray, "v1 patch");
	offset_x += getTextWidth("v1 patch ");
	if (is_patched_v1 == 1){
		drawLoopText(offset_x, offset_y, lime_green, "installed");
	}
	else{
		if (((is_patched_ur0 == 0) && (is_patched_vs0 == 1)) || ((is_patched_ur0 == 1) && (is_patched_vs0 == 0))){
			drawLoopText(offset_x, offset_y, gold, "partial");
		}
		else{
			drawLoopText(offset_x, offset_y, tomato, "absent");
		}
	}
	newLine(&offset_y);

	drawLoopTextLine(rightAlignText(status1), &offset_y, dim_gray, status1);
	drawLoopTextLine(rightAlignText(status2), &offset_y, dim_gray, status2);
	
	newLine(&offset_y);

	revertMargin(&offset_x, margin_left);
	drawLoopText(offset_x, offset_y, dim_gray, "v2 patch");
	offset_x += getTextWidth("v2 patch ");
	if (is_patched_v2 == 1){
		drawLoopText(offset_x, offset_y, lime_green, "installed");
	}
	else{
		drawLoopText(offset_x, offset_y, tomato, "absent");
	}
	newLine(&offset_y);

	drawLoopTextLine(rightAlignText(status3), &offset_y, dim_gray, status3);
	
	newLine(&offset_y);
	
	char debug1[256];
	char debug2[256];
	char debug3[256];
	char debug4[256];
	
	snprintf(debug1, sizeof(debug1), "LIST_VITA_UR0 [%s]", (vita_ur0 == 0) ? "x": "_");
	snprintf(debug2, sizeof(debug2), "LIST_EMU_UR0 [%s]", (emu_ur0 == 0) ? "x": "_");
	snprintf(debug3, sizeof(debug3), "LIST_VITA_VS0 [%s]", (vita_vs0 == 0) ? "x": "_");
	snprintf(debug4, sizeof(debug4), "LIST_EMU_VS0 [%s]",  (emu_vs0 == 0) ? "x": "_");

	//drawLoopTextLine(rightAlignText(debug1), &offset_y, cornflower_blue, debug1);
	//drawLoopTextLine(rightAlignText(debug2), &offset_y, cornflower_blue, debug2);
	//drawLoopTextLine(rightAlignText(debug3), &offset_y, cornflower_blue, debug3);
	//drawLoopTextLine(rightAlignText(debug4), &offset_y, cornflower_blue, debug4);

	// credits
	char *credits1 = "based on AntiBlacklist";
	char *credits2 = "by Rinnegatamante";
	drawLoopText(rightAlignText(credits2), SCREEN_HEIGHT-PADDING-getTextHeight(credits2), silver, credits2);
	drawLoopText(rightAlignText(credits1), SCREEN_HEIGHT-PADDING-getTextHeight(credits1)-25, silver, credits1);

	//vita2d_draw_line(float x0, float y0, float x1, float y1, unsigned int color)
}

int main(){
	
	// Initialization
	char *zErrMsg = 0;
	int exit_code = 0;
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0xFF));
	debug_font = vita2d_load_default_pgf();

	// allocate button texture memory
	vita2d_texture *button_triangle;
	vita2d_texture *button_circle;
	vita2d_texture *button_cross;
	vita2d_texture *button_square;
	
	// load the statically compiled images
	button_triangle = vita2d_load_PNG_buffer(&_binary_triangle_png_start);
	button_circle = vita2d_load_PNG_buffer(&_binary_circle_png_start);
	button_cross = vita2d_load_PNG_buffer(&_binary_cross_png_start);
	button_square = vita2d_load_PNG_buffer(&_binary_square_png_start);
	
	int state = MAIN_MENU;
	SceCtrlData pad;
	sqlite3 *db;
	int fd;

	int offset_x = 0; // character offset
	int offset_y = 0; // line offset
	int offset_image = 20; // 20x20 button image width
	int offset_correction = 1; // vertical (image) adjustment if necessary
	
	int reboot_required = 0;
	int has_triggers = 0; // are the triggers in app.db?
	int is_patched_vs0 = 0; // are the list files patched on vs0?
	int is_patched_ur0 = 0; // are the list files already on ur0?
	int is_v1_patched = 0; // shortcut
	int errors_v1_ur0 = 0;
	int errors_v1_vs0 = 0;
	int errors_v2 = 0;
	int result_ur0_vita; // file compare results; 0 = identical
	int result_ur0_emu;
	int result_vs0_vita;
	int result_vs0_emu;
	
	// Main loop
	for (;;){
		
		// determine patch installed status
		switch (state){
			
			case MAIN_MENU:
			case INSTALL_V1:
			case INSTALL_V2:
			
				// reset status values
				has_triggers = 0;
				is_patched_vs0 = 0;
				is_patched_ur0 = 0;
				is_v1_patched = 0;
				result_ur0_vita = -1;
				result_ur0_emu = -1;
				result_vs0_vita = -1;
				result_vs0_emu = -1;
				
				// check the v1 patch files have been applied
				if ((state == MAIN_MENU) || (state == INSTALL_V1)){
					
					void *buffer1 = NULL;
					void *buffer2 = NULL;
					void *buffer3 = NULL; 
					void *buffer4 = NULL; 
					
					int sz1 = readIntoBuffer(LIST_VITA_UR0, &buffer1);
					int sz2 = readIntoBuffer(LIST_EMU_UR0, &buffer2);
					int sz3 = readIntoBuffer(LIST_VITA_VS0, &buffer3);
					int sz4 = readIntoBuffer(LIST_EMU_VS0, &buffer4);
					//if (sz1 < 0) return size;
					
					result_ur0_vita = memcmp(buffer1, whitelist_vita, sz1); 
					result_ur0_emu = memcmp(buffer2, whitelist_emu, sz2); 
					result_vs0_vita = memcmp(buffer3, whitelist_vita, sz3); 
					result_vs0_emu = memcmp(buffer4, whitelist_emu, sz4); 
					
					free(buffer1);
					free(buffer2);
					free(buffer3);
					free(buffer4);
					
					if ((result_ur0_vita == 0) && (result_ur0_emu == 0))
						is_patched_ur0 = 1;
					if ((result_vs0_vita == 0) && (result_vs0_emu == 0))
						is_patched_vs0 = 1;
					if ((is_patched_ur0 == 1) && (is_patched_vs0 == 1))
						is_v1_patched = 1;
				}

				// check if the v2 triggers are present in app.db
				if ((state == MAIN_MENU) || (state == INSTALL_V2)){
					int ret = 0;
					ret = sqlite3_open(APP_DB, &db);
					if (ret == SQLITE_OK){
						sqlite3_stmt *stmt;
						ret = sqlite3_prepare_v2(db, sql_get_trigger_count, -1, &stmt, 0);
						if (ret == SQLITE_OK) {
							if (sqlite3_step(stmt) == SQLITE_ROW){
								int trigger_total = sqlite3_column_int(stmt, 0); //+1
								if (trigger_total == 4){
									has_triggers = 1;
								}
							}
						}
						sqlite3_finalize(stmt); // delete the compiled statement
					}
					else{
						char msg[512];
						sprintf(msg, "ERROR: cannot open app.db - %s", zErrMsg);
						sqlite3_free(zErrMsg);
						offset_y = PADDING+400;
						drawText(PADDING, offset_y, tomato, msg);
					}
					sqlite3_close(db);
				}
				break;
		}
		
		switch (state){
			
			case MAIN_MENU:
			
				vita2d_start_drawing();
				vita2d_clear_screen();

				// draw the status pane
				drawStatusPane(result_ur0_vita, result_ur0_emu, result_vs0_vita, result_vs0_emu, has_triggers);
	
				// reset to top-left
				revertXY(&offset_x, &offset_y);
				
				// homebrew title & breadcrumb menu
				char app_full_name[256] = {0};
				snprintf(app_full_name, sizeof(app_full_name), "%s v%s", APP_NAME, APP_VERSION); 
				drawLoopTextLine(offset_x, &offset_y, white, app_full_name); 
	
				newLine(&offset_y);
				
				// v1 instructions
				drawLoopText(offset_x, offset_y, white, "Press");
				offset_x += getTextWidth("Press ");
				drawLoopTexture(offset_x, offset_y+offset_correction, button_cross);
				offset_x += offset_image + getTextWidth(" ");
				if ((is_patched_ur0 == 1) && (is_patched_vs0 == 1)){
					drawLoopText(offset_x, offset_y, white, "to (re)install the");
					offset_x += getTextWidth("to (re)install the ");
				}
				else{
					drawLoopText(offset_x, offset_y, white, "to install the");
					offset_x += getTextWidth("to install the ");
				}
				drawLoopText(offset_x, offset_y, blue_cross, "v1 patch");
				offset_x += getTextWidth("v1 patch ");
				drawLoopText(offset_x, offset_y, white, "(list_launch files)");
				offset_x += getTextWidth("(list_launch files) ");
				// allow (re)install (of ur0) without mounting vs0 partition if already patched
				if (is_patched_vs0 == 0){
					drawLoopText(offset_x, offset_y, white, "and reboot");
				}
				newLine(&offset_y);
				revertX(&offset_x);
				
				// v2 instructions
				drawLoopText(offset_x, offset_y, white, "Press");
				offset_x += getTextWidth("Press ");
				drawLoopTexture(offset_x, offset_y+offset_correction, button_circle);
				offset_x += offset_image + getTextWidth(" ");
				drawLoopText(offset_x, offset_y, white, "to");
				offset_x += getTextWidth("to ");
				if (has_triggers == 0){
					drawLoopText(offset_x, offset_y, white, "install");
					offset_x += getTextWidth("install ");
				}
				else{
					drawLoopText(offset_x, offset_y, white, "uninstall");
					offset_x += getTextWidth("uninstall ");
				}
				drawLoopText(offset_x, offset_y, white, "the");
				offset_x += getTextWidth("the ");
				drawLoopText(offset_x, offset_y, red_circle, "v2 patch");
				offset_x += getTextWidth("v2 patch ");
				drawLoopText(offset_x, offset_y, white, "(app.db mod)");
				newLine(&offset_y);
				revertX(&offset_x);
				
				// exit instructions
				drawLoopText(offset_x, offset_y, dim_gray, "[Start]");
				offset_x += getTextWidth("[Start] ");
				drawLoopText(offset_x, offset_y, white, "or");
				offset_x += getTextWidth("or ");
				drawLoopText(offset_x, offset_y, dim_gray, "[Options]");
				offset_x += getTextWidth("[Options] ");
				drawLoopText(offset_x, offset_y, white, "will exit this program");
				newLine(&offset_y);
				revertX(&offset_x);
				
				newLine(&offset_y);
				
				//swap card instuctions (bonus function)
				drawLoopTextLine(offset_x, &offset_y, white, "(bonus function)");
				drawLoopText(offset_x, offset_y, white, "Pressing");
				offset_x += getTextWidth("Pressing ");
				drawLoopTexture(offset_x, offset_y+offset_correction, button_square);
				offset_x += offset_image + getTextWidth(" ");
				drawLoopText(offset_x, offset_y, white, "will prep the memory card for");
				offset_x += getTextWidth("will prep the memory card for ");
				drawLoopText(offset_x, offset_y, pink_square, "swap");
				newLine(&offset_y);
				revertX(&offset_x);
				
				sceCtrlPeekBufferPositive(0, &pad, 1);
				vita2d_end_drawing();
				vita2d_wait_rendering_done();
				vita2d_swap_buffers();
				
				if (pad.buttons & SCE_CTRL_CROSS) state = INSTALL_V1;
				else if (pad.buttons & SCE_CTRL_CIRCLE) state = INSTALL_V2;
				else if (pad.buttons & SCE_CTRL_TRIANGLE) state = MAIN_MENU; // todo: replace with v3
				else if (pad.buttons & SCE_CTRL_SQUARE) state = CARD_SWAP;
				else if (pad.buttons & SCE_CTRL_START) state = EXIT;
				break;
				
			case INSTALL_V1:
			
				// reset error flags
				errors_v1_ur0 = 0;
				errors_v1_vs0 = 0;
				
				clearScreen();

				// reset to top-left
				revertXY(&offset_x, &offset_y);
				
				// menu
				drawText(offset_x, offset_y, white, APP_NAME);
				offset_x += getTextWidth(APP_NAME);
				drawText(offset_x, offset_y, white, " > ");
				offset_x += getTextWidth(" > ");
				drawText(offset_x, offset_y, blue_cross, "v1 Patch");
				newLine(&offset_y);
				revertX(&offset_x);
				
				newLine(&offset_y);
				
				// Applying v1 patch
				drawTextLine(offset_x, &offset_y, white, "opening list_launch files ..");

				//v1 (ur0:/)
				FILE* f1 = fopen(LIST_VITA_UR0, "wb");
				FILE* f2 = fopen(LIST_EMU_UR0, "wb");
				
				if (f1 == NULL || f2 == NULL){
					drawTextLine(offset_x, &offset_y, tomato, "ERROR: cannot open list_launch files on ur0:/ ..");
					if (f1 != NULL) fclose(f1);
					if (f2 != NULL) fclose(f2);
					errors_v1_ur0 = 1;
				}else{
					drawTextLine(offset_x, &offset_y, white, "patching list_launch files on ur0:/ ..");
					fwrite(&whitelist_vita, 1, WHITELIST_SIZE, f1);
					fwrite(&whitelist_emu, 1, WHITELIST_SIZE, f2);
					fclose(f1);
					fclose(f2);
				}
				
				//v1 (vs0:/)
				if (is_patched_vs0 == 0){
					// mount vs0 as RW  (0x300=vs0) ((3 * 0x100))
					// https://github.com/tomtomdu80/VitaRW
					drawTextLine(offset_x, &offset_y, white, "mounting vs0:/ as read-write to change files ..");
					void *buf = malloc(0x100);
					vshIoUmount(3 * 0x100, 0, 0, 0); // id, unk1, unk2, unk3 (flags ?)
					_vshIoMount(3 * 0x100, 0, 2, buf); // id, unk, permission, work_buffer
					sceKernelDelayThread(3 * 1000 * 1000);
					
					f1 = fopen(LIST_VITA_VS0, "wb");
					f2 = fopen(LIST_EMU_VS0, "wb");
					
					if (f1 == NULL || f2 == NULL){
						// should never reach here.. but if we do explain what's happening
						drawTextLine(offset_x, &offset_y, tomato, "ERROR: cannot open list_launch files on vs0:/..");
						drawTextLine(offset_x, &offset_y, gold, "(you must first launch VitaRW to make this change)");
						if (f1 != NULL) fclose(f1);
						if (f2 != NULL) fclose(f2);
						errors_v1_vs0 = 1;
					}else{
						drawTextLine(offset_x, &offset_y, white, "patching list_launch files on vs0:/ ..");
						fwrite(&whitelist_vita, 1, WHITELIST_SIZE, f1);
						fwrite(&whitelist_emu, 1, WHITELIST_SIZE, f2);
						fclose(f1);
						fclose(f2);
						reboot_required = 1; // require a reboot to reset partition permissions
					}
				}
				
				if ((errors_v1_ur0 == 0) && (errors_v1_vs0 == 0)){
					drawTextLine(offset_x, &offset_y, lime_green, "done.");
				}
				else{
					drawTextLine(offset_x, &offset_y, tomato, "done, with errors.");
				}
				
				// reboot (to reset partitions) if the permissions were changed
				// todo: re-mount vs0:/ as read-only if/when there is a (verified) safe way to do it
				if (reboot_required == 1){
					drawTextLine(offset_x, &offset_y, white, "automatically rebooting in 3 seconds ..");
					sceKernelDelayThread(3 * 1000 * 1000);
					state = REBOOT;
				}
				else{
					newLine(&offset_y);
					
					// main menu button
					drawText(offset_x, offset_y, white, "Press");
					offset_x += getTextWidth("Press ");
					drawTexture(offset_x, offset_y+offset_correction, button_triangle); 
					offset_x += offset_image + getTextWidth(" ");
					drawText(offset_x, offset_y, white, "to return to the main menu");
					newLine(&offset_y);
					revertX(&offset_x);
					
					do{ sceCtrlPeekBufferPositive(0, &pad, 1); }
					while(!(pad.buttons & SCE_CTRL_TRIANGLE));
					
					state = MAIN_MENU;
				}
				break;
				
			case INSTALL_V2:
			
				// reset error flag
				errors_v2 = 0;
				
				clearScreen();

				// reset to top-left
				revertXY(&offset_x, &offset_y);
				
				// menu
				drawText(offset_x, offset_y, white, APP_NAME);
				offset_x += getTextWidth(APP_NAME);
				drawText(offset_x, offset_y, white, " > ");
				offset_x += getTextWidth(" > ");
				drawText(offset_x, offset_y, red_circle, "v2 Patch");
				offset_x += getTextWidth("v2 Patch ");
				if (has_triggers == 1){
					drawText(offset_x, offset_y, white, "(uninstall)");
				}
				else{
					drawText(offset_x, offset_y, white, "(install)");
				}
				newLine(&offset_y);
				revertX(&offset_x);

				newLine(&offset_y);
				
				// Opening app.db for v2 patch
				drawTextLine(offset_x, &offset_y, white, "opening app database ..");
				
				fd = sqlite3_open(APP_DB, &db);
				
				if(fd != SQLITE_OK){
					char msg[512];
					sprintf(msg, "ERROR: can't open app database: %s", sqlite3_errmsg(db));
					drawTextLine(offset_x, &offset_y, tomato, msg);
					drawTextLine(offset_x, &offset_y, white, "database modification aborted ..");
					sceKernelDelayThread(2 * 1000 * 1000);
					errors_v2 += 1;
				}else{

					// Applying v2 patch
					// trigger 1/4
					if (has_triggers == 1){
						drawTextLine(offset_x, &offset_y, white, "removing triggers ..");
						fd = sqlite3_exec(db, sql_drop_trigger_gpc, callback, 0, &zErrMsg);
					}
					else{
						drawTextLine(offset_x, &offset_y, white, "adding triggers ..");
						fd = sqlite3_exec(db, sql_create_trigger_gpc, callback, 0, &zErrMsg);
					}

					if( fd != SQLITE_OK ){
						char msg[512];
						sprintf(msg, "ERROR (gpc): %s", zErrMsg);
						sqlite3_free(zErrMsg);
						drawTextLine(offset_x, &offset_y, tomato, msg);
						errors_v2 += 1;
					}
					
					// trigger 2/4
					if (has_triggers == 1)
						fd = sqlite3_exec(db, sql_drop_trigger_gp, callback, 0, &zErrMsg);
					else
						fd = sqlite3_exec(db, sql_create_trigger_gp, callback, 0, &zErrMsg);

					if( fd != SQLITE_OK ){
						char msg[512];
						sprintf(msg, "ERROR (gp): %s", zErrMsg);
						sqlite3_free(zErrMsg);
						drawTextLine(offset_x, &offset_y, tomato, msg);
						errors_v2 += 1;
					}

					// trigger 3/4
					if (has_triggers == 1)
						fd = sqlite3_exec(db, sql_drop_trigger_gdc, callback, 0, &zErrMsg);
					else
						fd = sqlite3_exec(db, sql_create_trigger_gdc, callback, 0, &zErrMsg);

					if( fd != SQLITE_OK ){
						char msg[512];
						sprintf(msg, "ERROR (gdc): %s", zErrMsg);
						sqlite3_free(zErrMsg);
						drawTextLine(offset_x, &offset_y, tomato, msg);
						errors_v2 += 1;
					}
					
					// trigger 4/4
					if (has_triggers == 1)
						fd = sqlite3_exec(db, sql_drop_trigger_gd, callback, 0, &zErrMsg);
					else
						fd = sqlite3_exec(db, sql_create_trigger_gd, callback, 0, &zErrMsg);	

					if( fd != SQLITE_OK ){
						char msg[512];
						sprintf(msg, "ERROR (gd): %s", zErrMsg);
						sqlite3_free(zErrMsg);
						drawTextLine(offset_x, &offset_y, tomato, msg);
						errors_v2 += 1;
					}
		
					// Closing app.db
					drawTextLine(offset_x, &offset_y, white, "closing app database ..");
					sqlite3_close(db);
					
					if (errors_v2 == 0){
						drawTextLine(offset_x, &offset_y, lime_green, "done.");
						
						newLine(&offset_y);
						
						// Drawing instructions
						drawTextLine(offset_x, &offset_y, gold, "(additional steps required)"); 
						drawTextLine(offset_x, &offset_y, gold, "- power off your device"); 
						drawTextLine(offset_x, &offset_y, gold, "- remove the memory card"); 
						drawTextLine(offset_x, &offset_y, gold, "- power on the device; let the database refresh"); 
						drawTextLine(offset_x, &offset_y, gold, "- power off your device (again)"); 
						drawTextLine(offset_x, &offset_y, gold, "- insert the memory card"); 
						drawTextLine(offset_x, &offset_y, gold, "- power on the device one more time"); 
					}
					else{
						drawTextLine(offset_x, &offset_y, tomato, "done, with errors.");
					}

					newLine(&offset_y);
					
					// main menu button
					drawText(offset_x, offset_y, white, "Press");
					offset_x += getTextWidth("Press ");
					drawTexture(offset_x, offset_y+offset_correction, button_triangle); 
					offset_x += offset_image + getTextWidth(" ");
					drawText(offset_x, offset_y, white, "to return to the main menu");
					newLine(&offset_y);
					revertX(&offset_x);
					
					do{ sceCtrlPeekBufferPositive(0, &pad, 1); }
					while(!(pad.buttons & SCE_CTRL_TRIANGLE));
				}
				state = MAIN_MENU;
				break;
				
			case CARD_SWAP:
				clearScreen();
				
				// reset to top-left
				revertXY(&offset_x, &offset_y);

				// memory card swap
				drawText(offset_x, offset_y, white, APP_NAME);
				offset_x += getTextWidth(APP_NAME);
				drawText(offset_x, offset_y, white, " > ");
				offset_x += getTextWidth(" > ");
				drawText(offset_x, offset_y, pink_square, "memory card swap");
				newLine(&offset_y);
				revertX(&offset_x);
				
				newLine(&offset_y);
				
				drawTextLine(offset_x, &offset_y, white, "removing existing id.dat ..");
				sceIoRemove("ux0:/id.dat");
				
				drawTextLine(offset_x, &offset_y, white, "writing empty file to ux0:/id.dat ..");
				SceUID uid;
				uid = sceIoOpen("ux0:/id.dat", SCE_O_WRONLY|SCE_O_CREAT, 0777);
				sceIoWrite(uid, NULL, 0x0);

				drawTextLine(offset_x, &offset_y, lime_green, "done.");
				drawTextLine(offset_x, &offset_y, lime_green, "the card may now be transferred to another device");
				 
				newLine(&offset_y);
				 
				// main menu button
				drawText(offset_x, offset_y, white, "Press");
				offset_x += getTextWidth("Press ");
				drawTexture(offset_x, offset_y+offset_correction, button_triangle); 
				offset_x += offset_image + getTextWidth(" ");
				drawText(offset_x, offset_y, white, "to return to the main menu");
				newLine(&offset_y);				
				revertX(&offset_x);
				
				do{ sceCtrlPeekBufferPositive(0, &pad, 1); }
				while(!(pad.buttons & SCE_CTRL_TRIANGLE));

				state = MAIN_MENU;
				break;
			
			case REBOOT:
			
				exit_code = 2;
				break;
				
			case EXIT:
			
				exit_code = 1;
				break;
		}
		if (exit_code)	break;
	}
	
	// cleanup vita2d
	vita2d_fini();
	vita2d_free_texture(button_triangle);
	vita2d_free_texture(button_circle);
	vita2d_free_texture(button_cross);
	vita2d_free_texture(button_square);
	vita2d_free_pgf(debug_font);
	
	if (exit_code == 2){
		// work towards enabling safe vpk compatibility (Henkaku r6+)
		// SceVshBridge > ScePower
		// scePowerRequestColdReboot() is whitelisted
		return scePowerRequestColdReset(); // was vshPowerRequestColdReset()
	}
	else{
		sceKernelExitProcess(0);
		return 0;
	}
}
