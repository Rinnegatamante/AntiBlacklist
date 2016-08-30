#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <stdio.h>
#include <sqlite3.h> 
#include <vita2d.h> 
#include <stdlib.h>
#include <psp2/sysmodule.h>
#include <psp2/apputil.h>

#define WHITELIST_SIZE 41
#define MAIN_MENU 0
#define INSTALL_V1 1
#define INSTALL_V2 2
#define UNINSTALL_V2 3
#define EXIT 4

extern char* sqlite3_temp_directory;

static int callback(void *data, int argc, char **argv, char **azColName){
	int i;
	for ( i = 0; i < argc; i++ ) printf ( "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL" );
    return 0;
}

vita2d_pgf* debug_font;
void drawText(uint32_t y, char* text, uint32_t color){
	int i;
	for (i=0;i<3;i++){
		vita2d_start_drawing();
		vita2d_pgf_draw_text(debug_font, 2, y, color, 1.0, text	);
		vita2d_end_drawing();
		vita2d_wait_rendering_done();
		vita2d_swap_buffers();
	}
}

void drawLoopText(uint32_t y, char* text, uint32_t color){
	vita2d_pgf_draw_text(debug_font, 2, y, color, 1.0, text	);
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

int main(){
	
	// Initialization
	char *zErrMsg = 0;
	int exit_code = 0;
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0xFF));
	debug_font = vita2d_load_default_pgf();
	uint32_t white = RGBA8(0xFF, 0xFF, 0xFF, 0xFF);
	uint32_t green = RGBA8(0x00, 0xFF, 0x00, 0xFF);
	uint32_t red = RGBA8(0xFF, 0x00, 0x00, 0xFF);
	int state = MAIN_MENU;
	SceCtrlData pad;
	sqlite3 *db;
	int fd;
	
	// v1 patch file (list_launch_emu.dat fw3.60 LVER00111)
	uint8_t whitelist_emu[WHITELIST_SIZE] = {
		0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x5f, 0x76, 0x65, 0x72, 
		0x73, 0x69, 0x6f, 0x6e, 0x3a, 0x30, 0x30, 0x31, 0x0a, 0x6c, 
		0x69, 0x73, 0x74, 0x5f, 0x74, 0x79, 0x70, 0x65, 0x3a, 0x62, 
		0x0a, 0x4c, 0x56, 0x45, 0x52, 0x30, 0x30, 0x31, 0x31, 0x31, 
		0x0a
	};
	
	// v1 patch file (list_launch_vita.dat fw3.60 LVER00125)
	uint8_t whitelist_vita[WHITELIST_SIZE] = {
		0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x5f, 0x76, 0x65, 0x72, 
		0x73, 0x69, 0x6f, 0x6e, 0x3a, 0x30, 0x30, 0x31, 0x0a, 0x6c, 
		0x69, 0x73, 0x74, 0x5f, 0x74, 0x79, 0x70, 0x65, 0x3a, 0x62, 
		0x0a, 0x4c, 0x56, 0x45, 0x52, 0x30, 0x30, 0x31, 0x32, 0x35, 
		0x0a
	};
	
	// Main loop
	for (;;){
		switch (state){
			case MAIN_MENU:
				vita2d_start_drawing();
				vita2d_clear_screen();
				drawLoopText(20,"AntiBlacklist v.1.1 by Rinnegatamante",white);
				drawLoopText(60,"Press Cross to install v1 patch.",white);
				drawLoopText(80,"Press Circle to install v2 patch.",white);
				drawLoopText(100,"Press Square to uninstall v2 patch.",white);
				drawLoopText(120,"Press Start to exit.",white);
				sceCtrlPeekBufferPositive(0, &pad, 1);
				vita2d_end_drawing();
				vita2d_wait_rendering_done();
				vita2d_swap_buffers();
				if (pad.buttons & SCE_CTRL_CROSS) state = INSTALL_V1;
				else if (pad.buttons & SCE_CTRL_CIRCLE) state = INSTALL_V2;
				else if (pad.buttons & SCE_CTRL_SQUARE) state = UNINSTALL_V2;
				else if (pad.buttons & SCE_CTRL_START) state = EXIT;
				break;
			case INSTALL_V1:				
				clearScreen();
				
				// Applying v1 patch
				drawText(20,"*** v1 Patch ***",white);
				drawText(40,"Opening list_launch files...",white);
				FILE* f1 = fopen("ur0:/game/launch/list_launch_vita.dat", "wb");
				FILE* f2 = fopen("ur0:/game/launch/list_launch_emu.dat", "wb");
				if (f1 == NULL || f2 == NULL){
					drawText(60,"ERROR: Cannot open list_launch files on ur0:/.", red);
					if (f1 != NULL) fclose(f1);
					if (f2 != NULL) fclose(f2);
				}else{
					drawText(60,"Patching list_launch files on ur0:/...",white);
					fwrite(&whitelist_vita, 1, WHITELIST_SIZE, f1);
					fwrite(&whitelist_emu, 1, WHITELIST_SIZE, f2);
					fclose(f1);
					fclose(f2);
				}
				f1 = fopen("ux0:/game/launch/list_launch_vita.dat", "wb");
				f2 = fopen("ux0:/game/launch/list_launch_emu.dat", "wb");
				if (f1 == NULL || f2 == NULL){
					drawText(80,"ERROR: Cannot open list_launch files on ux0:/.", red);
					if (f1 != NULL) fclose(f1);
					if (f2 != NULL) fclose(f2);
				}else{
					drawText(80,"Patching list_launch files on ux0:/...",white);
					fwrite(&whitelist_vita, 1, WHITELIST_SIZE, f1);
					fwrite(&whitelist_emu, 1, WHITELIST_SIZE, f2);
					fclose(f1);
					fclose(f2);
				}
				f1 = fopen("vs0:/data/internal/launch/list_launch_vita.dat", "wb");
				f2 = fopen("vs0:/data/internal/launch/list_launch_emu.dat", "wb");
				if (f1 == NULL || f2 == NULL){
					drawText(100,"ERROR: Cannot open list_launch files on vs0:/.", red);
					if (f1 != NULL) fclose(f1);
					if (f2 != NULL) fclose(f2);
				}else{
					drawText(100,"Patching list_launch files on vs0:/...",white);
					fwrite(&whitelist_vita, 1, WHITELIST_SIZE, f1);
					fwrite(&whitelist_emu, 1, WHITELIST_SIZE, f2);
					fclose(f1);
					fclose(f2);
					drawText(120,"Done!",green);
				}
				
				sceKernelDelayThread(2000000);
				state = MAIN_MENU;
				break;
			case INSTALL_V2:
				clearScreen();
				
				// Opening app.db for v2 patch
				drawText(20,"*** v2 Patch ***",white);
				drawText(40,"Opening app database",white);
				fd = sqlite3_open("ur0:/shell/db/app.db", &db);
				if(fd != SQLITE_OK){
					char error[512];
					sprintf(error, "ERROR: Can't open app database: %s", sqlite3_errmsg(db));
					drawText(60,error,red);
					drawText(80,"v2 Patch aborted...",white);
					sceKernelDelayThread(2000000);
				}else{
		
					// Applying v2 patch
					char query[1024];
					sprintf(query,"%s","CREATE TRIGGER CHANGE_CATEGORY_GPC AFTER INSERT ON tbl_appinfo WHEN new.val LIKE 'gpc' BEGIN UPDATE tbl_appinfo SET val='gdb' WHERE key='566916785' and titleid=new.titleid; END;");	
					drawText(60,"Executing 1st query",white);
					fd = sqlite3_exec(db, query, callback, 0, &zErrMsg);
					if( fd != SQLITE_OK ){
						char error[512];
						sprintf(error, "ERROR: SQL error: %s", zErrMsg);
						sqlite3_free(zErrMsg);
						drawText(400,error,red);
					}
		
					sprintf(query,"%s","CREATE TRIGGER CHANGE_CATEGORY_GP AFTER INSERT ON tbl_appinfo WHEN new.val LIKE 'gp' BEGIN UPDATE tbl_appinfo SET val='gdb' WHERE key='566916785' and titleid=new.titleid; END;");					
					drawText(80,"Executing 2nd query",white);
					fd = sqlite3_exec(db, query, callback, 0, &zErrMsg);
					if( fd != SQLITE_OK ){
						char error[512];
						sprintf(error, "ERROR: SQL error: %s", zErrMsg);
						sqlite3_free(zErrMsg);
						drawText(420,error,red);
					}
					sprintf(query,"%s","CREATE TRIGGER CHANGE_CATEGORY_GDC AFTER INSERT ON tbl_appinfo WHEN new.val LIKE 'gdc' BEGIN UPDATE tbl_appinfo SET val='gdb' WHERE key='566916785' and titleid=new.titleid; END;");
					drawText(100,"Executing 3rd query",white);
					fd = sqlite3_exec(db, query, callback, 0, &zErrMsg);
					if( fd != SQLITE_OK ){
						char error[512];
						sprintf(error, "ERROR: SQL error: %s", zErrMsg);
						sqlite3_free(zErrMsg);
						drawText(440,error,red);
					}
					sprintf(query,"%s","CREATE TRIGGER CHANGE_CATEGORY_GD AFTER INSERT ON tbl_appinfo WHEN new.val LIKE 'gd' BEGIN UPDATE tbl_appinfo SET val='gdb' WHERE key='566916785' and titleid=new.titleid; END;");
					drawText(120,"Executing 4th query",white);
					fd = sqlite3_exec(db, query, callback, 0, &zErrMsg);
					if( fd != SQLITE_OK ){
						char error[512];
						sprintf(error, "ERROR: SQL error: %s", zErrMsg);
						sqlite3_free(zErrMsg);
						drawText(460,error,red);
					}
		
					// Closing app.db
					drawText(140,"Closing app database",white);
					sqlite3_close(db);
					drawText(160,"Done!",green);
					
					// Drawing instructions
					drawText(200,"To make changes effective you must:",green);
					drawText(220,"- Power off your PSVITA TV system.",green);	
					drawText(240,"- Eject memory card.",green);
					drawText(260,"- Power on console and wait database refreshing.",green);
					drawText(280,"- Power off your PSVITA TV system.",green);
					drawText(300,"- Reinsert memory card.",green);
					drawText(320,"- Power on your console.",green);		
					drawText(360,"Press Triangle to return main menu",green);		
					do{sceCtrlPeekBufferPositive(0, &pad, 1);}while(!(pad.buttons & SCE_CTRL_TRIANGLE));
					
				}
				state = MAIN_MENU;
				break;
			case UNINSTALL_V2:
				clearScreen();
				
				// Opening app.db for v2 patch
				drawText(20,"*** v2 Patch ***",white);
				drawText(40,"Opening app database",white);
				fd = sqlite3_open("ur0:/shell/db/app.db", &db);
				if(fd != SQLITE_OK){
					char error[512];
					sprintf(error, "ERROR: Can't open app database: %s", sqlite3_errmsg(db));
					drawText(60,error,red);
					drawText(80,"v2 Patch aborted...",white);
					sceKernelDelayThread(2000000);
				}else{
		
					// Removing v2 patch
					char query[1024];
					sprintf(query,"%s","DROP TRIGGER CHANGE_CATEGORY_GPC;");	
					drawText(60,"Executing 1st query",white);
					fd = sqlite3_exec(db, query, callback, 0, &zErrMsg);
					if( fd != SQLITE_OK ){
						char error[512];
						sprintf(error, "ERROR: SQL error: %s", zErrMsg);
						sqlite3_free(zErrMsg);
						drawText(400,error,red);
					}
		
					sprintf(query,"%s","DROP TRIGGER CHANGE_CATEGORY_GP;");					
					drawText(80,"Executing 2nd query",white);
					fd = sqlite3_exec(db, query, callback, 0, &zErrMsg);
					if( fd != SQLITE_OK ){
						char error[512];
						sprintf(error, "ERROR: SQL error: %s", zErrMsg);
						sqlite3_free(zErrMsg);
						drawText(420,error,red);
					}
					sprintf(query,"%s","DROP TRIGGER CHANGE_CATEGORY_GDC;");
					drawText(100,"Executing 3rd query",white);
					fd = sqlite3_exec(db, query, callback, 0, &zErrMsg);
					if( fd != SQLITE_OK ){
						char error[512];
						sprintf(error, "ERROR: SQL error: %s", zErrMsg);
						sqlite3_free(zErrMsg);
						drawText(440,error,red);
					}
					sprintf(query,"%s","DROP TRIGGER CHANGE_CATEGORY_GD;");
					drawText(120,"Executing 4th query",white);
					fd = sqlite3_exec(db, query, callback, 0, &zErrMsg);
					if( fd != SQLITE_OK ){
						char error[512];
						sprintf(error, "ERROR: SQL error: %s", zErrMsg);
						sqlite3_free(zErrMsg);
						drawText(460,error,red);
					}
		
					// Closing app.db
					drawText(140,"Closing app database",white);
					sqlite3_close(db);
					drawText(160,"Done!",green);
					
					// Drawing instructions
					drawText(200,"To make changes effective you must:",green);
					drawText(220,"- Power off your PSVITA TV system.",green);	
					drawText(240,"- Eject memory card.",green);
					drawText(260,"- Power on console and wait database refreshing.",green);
					drawText(280,"- Power off your PSVITA TV system.",green);
					drawText(300,"- Reinsert memory card.",green);
					drawText(320,"- Power on your console.",green);		
					drawText(360,"Press Triangle to return main menu",green);		
					do{sceCtrlPeekBufferPositive(0, &pad, 1);}while(!(pad.buttons & SCE_CTRL_TRIANGLE));
					
					}
				state = MAIN_MENU;
				break;
			case EXIT:
				exit_code = 1;
				break;
		}
		if (exit_code)	break;
	}
		
	sceKernelExitProcess(0);
	return 0;
	
}
