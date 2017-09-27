# Fake Atari Learning Environment Makefile

.PHONY=clean

ifeq ($(SDL),)
	SDL := /usr/include/SDL
endif

OBJDIR := obj
OBJS := $(addprefix $(OBJDIR)/,ale_interface.o Settings.o agcd_interface.o ColourPalette.o phosphor_blend.o display_screen.o)
CXXFLAGS := -O3 -march=native -pipe -fPIC -pie -I$(SDL) $(CXXFLAGS) -std=c++11
LDFLAGS := -lz -lpng -lm $(LDFLAGS) -lSDL

ifeq ($(CLION_EXE_DIR),)
	CLION_EXE_DIR := .
endif

LIBALE := $(CLION_EXE_DIR)/libale.so

$(OBJDIR)/%.o : src/%.cpp
	$(CXX) $(CXXFLAGS) $< -c -o $@

all: $(OBJS) $(LIBALE)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

$(LIBALE): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -shared -o $(LIBALE)

clean:
	rm -fr $(OBJDIR) $(LIBALE)

tools:
	make -C tools

ale: $(LIBALE)
