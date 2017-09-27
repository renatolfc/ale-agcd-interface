# Fake Atari Learning Environment Makefile

.PHONY=clean

OBJDIR := obj
OBJS := $(addprefix $(OBJDIR)/,agcd-to-hdf5.o)
CXXFLAGS := -O3 -march=native -pipe -fPIC -pie $(CXXFLAGS) -std=c++11
LDFLAGS := -lpng -lhdf5 $(LDFLAGS)

HDF5 := agcd-to-hdf5

$(OBJDIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) $< -c -o $@

all: $(OBJS) $(HDF5)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

$(HDF5): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $(HDF5)

clean:
	rm -fr $(OBJDIR) $(HDF5)
