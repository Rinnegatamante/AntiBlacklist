#include <vitasdk.h>
#include <stdio.h>
#include "sqlite3.h" 
#include <vita2d.h> 
#include <stdlib.h>
#include <string.h>

#define WHITELIST_SIZE 41

#define MAIN_MENU    0
#define INSTALL_V1   1
#define INSTALL_V2   2
#define UNINSTALL_V2 3
#define EXIT         4

extern char* sqlite3_temp_directory;

// Copy-pasted from VitaShell by TheFlow
int vshIoUmount(int id, int a2, int a3, int a4);
int _vshIoMount(int id, const char *path, int permission, void *buf);
int vshIoMount(int id, const char *path, int permission, int a4, int a5, int a6) {
	uint32_t buf[6];

	buf[0] = a4;
	buf[1] = a5;
	buf[2] = a6;
	buf[3] = 0;
	buf[4] = 0;
	buf[5] = 0;

	return _vshIoMount(id, path, permission, buf);
}

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
				drawLoopText(20,"AntiBlacklist v.1.2 by Rinnegatamante",white);
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
				
				// ur0 files overwriting
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
				
				// ux0 files overwriting
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
				
				// vs0 files overwriting
				drawText(100,"Mounting vs0:/ partition with RW permissions...", white);
				vshIoUmount(0x300, 0, 0, 0);
				vshIoMount(0x300, NULL, 2, 0, 0, 0);
				f1 = fopen("vs0:/data/internal/launch/list_launch_vita.dat", "wb");
				f2 = fopen("vs0:/data/internal/launch/list_launch_emu.dat", "wb");
				if (f1 == NULL || f2 == NULL){
					drawText(120,"ERROR: Cannot open list_launch files on vs0:/.", red);
					if (f1 != NULL) fclose(f1);
					if (f2 != NULL) fclose(f2);
					drawText(140,"Patch partially applied, run VitaRW to fully install it.",green);
				}else{
					drawText(120,"Patching list_launch files on vs0:/...",white);
					fwrite(&whitelist_vita, 1, WHITELIST_SIZE, f1);
					fwrite(&whitelist_emu, 1, WHITELIST_SIZE, f2);
					fclose(f1);
					fclose(f2);
					drawText(140,"Mounting vs0:/ partition with R permissions...", white);
					vshIoUmount(0x300, 0, 0, 0);
					vshIoUmount(0x300, 1, 0, 0);
					vshIoMount(0x300, NULL, 0, 0, 0, 0);
					f1 = fopen("vs0:/data/internal/launch/list_launch_vita.dat", "rb+");
					if (f1 != NULL){
						drawText(160,"ERROR: Failed! vs0:/ is still writeable!!!", red);
						fclose(f1);
					}
					drawText(160 + ((f1 != NULL) ? 20 : 0),"Done!",green);
				}
				
				drawText(180 + ((f1 != NULL) ? 20 : 0),"Press Triangle to return back.",green);
				do{sceCtrlPeekBufferPositive(0, &pad, 1);}while(!(pad.buttons & SCE_CTRL_TRIANGLE));
				
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
					sprintf(query,"%s","CREATE TRIGGER CHANGE_FEATURE_PSTV AFTER INSERT ON tbl_appinfo WHEN 1=1 BEGIN UPDATE tbl_appinfo SET val = val & ~8 WHERE key='2412347057' and titleid=new.titleid; END;");
					drawText(140,"Executing 5th query",white);
					fd = sqlite3_exec(db, query, callback, 0, &zErrMsg);
					if( fd != SQLITE_OK ){
						char error[512];
						sprintf(error, "ERROR: SQL error: %s", zErrMsg);
						sqlite3_free(zErrMsg);
						drawText(460,error,red);
					}		
					// Closing app.db
					drawText(160,"Closing app database",white);
					sqlite3_close(db);
					
					//Triggering a database restore
					drawText(180,"Nulling MID value in id.dat",white);
					FILE* fd = fopen("ux0:/id.dat", "r+");
					fseek(fd, 0, SEEK_END);
					int id_size = ftell(fd);
					char* id_buf = (char*)malloc(id_size);
					fseek(fd, 0, SEEK_SET);
					fread(id_buf, id_size, 1, fd);
					char* mid_offs = strstr(id_buf, "MID=");
					if (mid_offs == NULL){
						fseek(fd, 0, SEEK_END); // Just to be sure
						fwrite("\nMID=\n", 6, 1, fd);
						fclose(fd);
					}else{
						fclose(fd);
						fd = fopen("ux0:/id.dat", "w+");
						char* mid_end = strstr(mid_offs, "\n");
						if (mid_end == NULL) fwrite(id_buf, mid_offs - id_buf + 4, 1, fd);
						else{
							memcpy(&mid_offs[4], mid_end, id_size - (mid_end - id_buf));
							fwrite(id_buf, id_size - (mid_end - (mid_offs + 4)), 1, fd);
						}
						fclose(fd);
					}
					
					drawText(200,"Done!",green);
					drawText(240,"To make changes effective, a reboot is required",green);
					drawText(260,"Press Triangle to perform a console reboot",green);
					
					do{sceCtrlPeekBufferPositive(0, &pad, 1);}while(!(pad.buttons & SCE_CTRL_TRIANGLE));
					scePowerRequestColdReset();
					
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
					sprintf(query,"%s","DROP TRIGGER CHANGE_FEATURE_PSTV;");
					drawText(140,"Executing 5th query",white);
					fd = sqlite3_exec(db, query, callback, 0, &zErrMsg);
					if( fd != SQLITE_OK ){
						char error[512];
						sprintf(error, "ERROR: SQL error: %s", zErrMsg);
						sqlite3_free(zErrMsg);
						drawText(460,error,red);
					}
		
					// Closing app.db
					drawText(160,"Closing app database",white);
					sqlite3_close(db);
					
					//Triggering a database restore
					drawText(180,"Nulling MID value in id.dat",white);
					FILE* fd = fopen("ux0:/id.dat", "r+");
					fseek(fd, 0, SEEK_END);
					int id_size = ftell(fd);
					char* id_buf = (char*)malloc(id_size);
					fseek(fd, 0, SEEK_SET);
					fread(id_buf, id_size, 1, fd);
					char* mid_offs = strstr(id_buf, "MID=");
					if (mid_offs == NULL){
						fseek(fd, 0, SEEK_END); // Just to be sure
						fwrite("\nMID=\n", 6, 1, fd);
						fclose(fd);
					}else{
						fclose(fd);
						fd = fopen("ux0:/id.dat", "w+");
						char* mid_end = strstr(mid_offs, "\n");
						if (mid_end == NULL) fwrite(id_buf, mid_offs - id_buf + 4, 1, fd);
						else{
							memcpy(&mid_offs[4], mid_end, id_size - (mid_end - id_buf));
							fwrite(id_buf, id_size - (mid_end - (mid_offs + 4)), 1, fd);
						}
						fclose(fd);
					}
					
					drawText(200,"Done!",green);
					drawText(220,"To make changes effective, a reboot is required",green);
					drawText(240,"Press Triangle to perform a console reboot",green);
					
					do{sceCtrlPeekBufferPositive(0, &pad, 1);}while(!(pad.buttons & SCE_CTRL_TRIANGLE));
					scePowerRequestColdReset();
					
				}
				state = MAIN_MENU;
				break;
			case EXIT:
				exit_code = 1;
				break;
		}
		if (exit_code)	break;
	}
	
	return 0;
	
}
