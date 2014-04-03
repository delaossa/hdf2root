include $(HDF2ROOT)/config/Makefile.common

OBJS  = hdf2root_methods

EXES = hdf2root

CXXFLAGS     += $(HDF5CFLAGS) -I$(INCDIR) -I. 
LIBS         := $(ROOTLIBS) $(HDF5LIBS)

SHLIBFILE    := lib$(LIB).$(DllSuf)

all: lib bin

obj: $(addsuffix .o,$(OBJS))

lib: $(LIBDIR)/$(SHLIBFILE)

bin: $(addsuffix .e,$(EXES))

%.o: %.cc
	@echo "Compiling $<"
	$(CXX) -c $(CXXFLAGS) $<

%.e: %.o
	@mkdir -p $(BINDIR)
	@echo "Linking $<"
	$(LD) $(LDFLAGS) -o $@ $< $(LIBS) $(H2RLIBS)
	ln -sf $(SRCDIR)/$@ $(BINDIR)/$(subst .e,,$@)


$(LIBDIR)/$(SHLIBFILE): $(addsuffix .o,$(OBJS))
	@echo "Building shared library $@ ... "
	@mkdir -p $(LIBDIR)
ifeq ($(PLATFORM),macosx)
	$(LD) $(SOFLAGS) $@ $< -o $@ $(LIBS)
else
	$(LD) $(SOFLAGS) $< -o $(LIBDIR)/$(SHLIBFILE) $(LIBS)
endif
#	ln -fs $(SHLIBFILE) lib/lib$(LIB).1.so
	@echo "Done"


clean:
	@rm -f *.o
	@rm -f *.e
	@rm -f *~
	@rm -f $(LIBDIR)/$(SHLIBFILE)
	@for file in $(EXES); do (rm -f $$file.e); done
	@for file in $(EXES); do (rm -f $(EXESDIR)/$$file); done

# Rule to set up a symbolic links to the created shared library
linklib:
	@if [[ ( ( `root-config --platform` == "macosx" ) && \
			( ! -e lib/lib$(LIB).1.dylib ) ) || \
			( ! -e lib/lib$(LIB).1.so ) ]]; then \
		printf "Setting up soft links to the HDF2ROOT library ... "; \
		ln -sf $(SHLIBFILE) $(LIBDIR)/lib$(LIB).1.so; \
		if [[ `root-config --platform` == "macosx" ]]; then \
			ln -sf $(SHLIBFILE) $(LIBDIR)/lib$(LIB).1.dylib; \
		fi; \
		echo "Done"; \
	fi

info:	
	@echo "Library: $(LIBDIR)/$(SHLIBFILE)"
	@echo "SOFLAGS = $(SOFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "CURDIR  = $(CURDIR)"


.PHONY : obj lib bin clean info linklib
