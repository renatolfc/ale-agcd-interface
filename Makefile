# Fake Atari Learning Environment Makefile

.PHONY=clean

OBJDIR := obj
OBJS := $(addprefix $(OBJDIR)/,ale_interface.o Settings.o agcd_interface.o)
CXXFLAGS := -O2 -march=native -pipe -fPIC -pie $(CXXFLAGS) 
LDFLAGS := -lz -lpng -lm $(LDFLAGS)
CXX := g++ -std=c++11
LIBALE := libale.so

$(OBJDIR)/%.o : src/%.cpp
	$(CXX) $(CXXFLAGS) $< -c -o $@

all: $(OBJS) $(LIBALE)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

$(LIBALE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -shared -o $(LIBALE)

clean:
	rm -fr $(OBJDIR) $(LIBALE)
