TARGET = whitelist
TITLE = LIST00001
VERSION = 01.21

OBJS = main.o \
	vita_sqlite.o \
	sqlite-3.6.23.1/sqlite3.o \
	triangle.o circle.o cross.o square.o 

LIBS = -lvita2d -lSceKernel_stub \
	-lSceAppmgr_stub -lSceSysmodule_stub -lSceCtrl_stub \
	-lm -lSceAppUtil_stub -lScePgf_stub -ljpeg -lfreetype \
	-lc -lScePower_stub -lSceCommonDialog_stub -lpng -lpng16 -lz \
	-lSceGxm_stub -lSceDisplay_stub -lsqlite3 -lSceVshBridge_stub

DEFINES = -DSQLITE_OS_OTHER=1 -DSQLITE_TEMP_STORE=3 -DSQLITE_THREADSAFE=0

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CFLAGS  = -Wl,-q -O3 -std=c99 $(DEFINES)
CXXFLAGS  = $(CFLAGS) -fno-exceptions -std=gnu++11
ASFLAGS = $(CFLAGS)

all: $(TARGET).vpk

$(TARGET).vpk: $(TARGET).velf
	vita-make-fself $< .\pkg\eboot.bin
	vita-mksfoex -s APP_VER=$(VERSION) -s TITLE_ID=$(TITLE) "$(TARGET)" param.sfo
	cp -f param.sfo ./pkg/sce_sys/param.sfo
	vita-pack-vpk -s param.sfo -b eboot.bin \
		--add pkg/sce_sys/icon0.png=sce_sys/icon0.png \
		--add pkg/sce_sys/livearea/contents/startup.png=sce_sys/livearea/contents/startup.png \
		--add pkg/sce_sys/livearea/contents/bg.png=sce_sys/livearea/contents/bg.png \
		--add pkg/sce_sys/livearea/contents/template.xml=sce_sys/livearea/contents/template.xml \
	$(TARGET).vpk

eboot.bin: $(TARGET).velf
	vita-make-fself -s $< $@

%.velf: %.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@
	vita-make-fself $@ eboot.bin

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

%.o: %.png
	$(PREFIX)-ld -r -b binary -o $@ $^

clean:
	@rm -rf $(TARGET).vpk $(TARGET).velf $(TARGET).elf $(OBJS) \
		eboot.bin param.sfo
