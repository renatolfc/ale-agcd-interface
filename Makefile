# Fake Atari Learning Environment Makefile

.PHONY=clean

OBJDIR := obj
OBJS := $(addprefix $(OBJDIR)/,ale_interface.o Settings.o agcd_interface.o)
CXXFLAGS := -O3 -march=native -pipe -fPIC -pie $(CXXFLAGS) -std=c++11
LDFLAGS := -lz -lpng -lm $(LDFLAGS)

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

ale: $(LIBALE)
