include $(HDF2ROOT)/config/Makefile.common

MODULES  = hdf2root_methods

EXES = hdf2root

CXX       = g++
CXXFLAGS     += $(ROOTCFLAGS) $(HDF5CFLAGS) -I$(INCDIR) -I. 
LIBS          = $(ROOTLIBS) $(HDF5LIBS)


all: modules lib bin

modules: $(addsuffix .o,$(MODULES))

lib: $(LIBDIR)/lib$(LIB).so

bin: $(addsuffix .e,$(EXES))

$(LIBDIR)/lib$(LIB).so: $(addsuffix .o,$(MODULES)) 
	@mkdir -p $(LIBDIR)
	@echo "Linking shared library $@"
	$(CXX) $(SOFLAGS) $(LDFLAGS) $(LIBS) $^ -o $@

%.o: %.cc
	@echo "Compiling $<"
	$(CXX) -c $(CXXFLAGS) $<

%.e: %.o
	@mkdir -p $(BINDIR)
	@echo "Linking $@"
	$(CXX) $(LDFLAGS) $< -o $@  -L. $(LIBS) $(H2RLIBS)
	@ln -sf $(SRCDIR)/$@ $(BINDIR)/$(subst .e,,$@)

clean:
	@rm -f *.o
	@rm -f *.e
	@rm -f *~
	@rm -f $(LIBDIR)/lib$(LIB).so
	@for file in $(EXES); do (rm -f $$file.e); done
	@for file in $(EXES); do (rm -f $(EXESDIR)/$$file); done

