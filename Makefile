TARGET		:= AntiBlacklist
TITLE		:= ABLK00001
VERSION		:= 01.20
SOURCES		:= source/include/lua source/include/ftp source/include source \
			source/include/audiodec
INCLUDES	:= include

LIBS = -lvita2d -lSceKernel_stub \
	-lSceAppmgr_stub -lSceSysmodule_stub -lSceCtrl_stub \
	-lm -lSceAppUtil_stub -lScePgf_stub -ljpeg -lfreetype \
	-lc -lScePower_stub -lSceCommonDialog_stub -lpng16 -lz \
	-lSceGxm_stub -lSceDisplay_stub -lsqlite3 -lSceVshBridge_stub

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++
CFLAGS  = -Wl,-q -O3 -Wall
CXXFLAGS  = $(CFLAGS) -fno-exceptions -std=gnu++11
ASFLAGS = $(CFLAGS)

all: $(TARGET).vpk

$(TARGET).vpk: $(TARGET).velf
	vita-make-fself $< .\build\eboot.bin
	vita-mksfoex -s APP_VER=$(VERSION) -s TITLE_ID=$(TITLE) "$(TARGET)" param.sfo
	cp -f param.sfo ./build/sce_sys/param.sfo
	vita-pack-vpk -s param.sfo -b eboot.bin \
		--add build/sce_sys/icon0.png=sce_sys/icon0.png \
		--add build/sce_sys/livearea/contents/bg.png=sce_sys/livearea/contents/bg.png \
		--add build/sce_sys/livearea/contents/startup.png=sce_sys/livearea/contents/startup.png \
		--add build/sce_sys/livearea/contents/template.xml=sce_sys/livearea/contents/template.xml \
	$(TARGET).vpk
	#------------ Comment this if you don't have 7zip ------------------
	#7z a -tzip ./$(TARGET).vpk -r .\build\sce_sys\* .\build\eboot.bin 
	#-------------------------------------------------------------------

%.velf: %.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@
	vita-make-fself $@ eboot.bin

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS)
