TARGET		:= AntiBlacklist
TITLE		:= ABLK00001
SOURCES		:= source
INCLUDES	:= include

LIBS = -lvita2d -lSceLibKernel_stub -lSceVshBridge_stub \
	-lSceCtrl_stub -lm -lScePgf_stub -ljpeg -lc \
	-lScePower_stub -lSceCommonDialog_stub -lpng16 -lfreetype \
	-lz -lSceSysmodule_stub -lSceGxm_stub -lSceDisplay_stub -lSceAppMgr_stub

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++
CFLAGS  = -Wl,-q -O3 -Wall -DSQLITE_OS_OTHER=1 -DSQLITE_TEMP_STORE=3 -DSQLITE_THREADSAFE=0
CXXFLAGS  = $(CFLAGS) -fno-exceptions -std=gnu++11
ASFLAGS = $(CFLAGS)

all: $(TARGET).vpk

$(TARGET).vpk: $(TARGET).velf
	vita-make-fself -c $< build/eboot.bin
	vita-mksfoex -s TITLE_ID=$(TITLE) "$(TARGET)" build/sce_sys/param.sfo
	vita-pack-vpk -s build/sce_sys/param.sfo -b build/eboot.bin \
		-a build/sce_sys/icon0.png=sce_sys/icon0.png \
		-a build/sce_sys/livearea/contents/bg.png=sce_sys/livearea/contents/bg.png \
		-a build/sce_sys/livearea/contents/startup.png=sce_sys/livearea/contents/startup.png \
		-a build/sce_sys/livearea/contents/template.xml=sce_sys/livearea/contents/template.xml $@

%.velf: %.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@
	vita-make-fself $@ eboot.bin

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS)
