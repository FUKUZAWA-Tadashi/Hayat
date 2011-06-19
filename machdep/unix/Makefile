# -*- makefile -*-
#
# Makefile for UNIX or CYGWIN
#
#----------------------------------------------------------------------------

.PHONY:	all prep prep-ffi clean target make_target depend make_depend

all:	prep make_depend make_target

#----------------------------------------------------------------------------

HAYAT_TOP	= ..

# ex. to use BUILDDIR
# make BUILDDIR=/release
# make BUILDDIR=/debug
BUILDDIR	= 

ENGINEDIR	= $(HAYAT_TOP)/engine
OBJSDIR         = $(ENGINEDIR)/obj$(BUILDDIR)
LIBDIR          = $(ENGINEDIR)/lib$(BUILDDIR)
COMMON_DIR	= $(HAYAT_TOP)/common
TOOLS_DIR	= $(HAYAT_TOP)/tools
STDLIB_DIR	= $(HAYAT_TOP)/stdlib
STDLIB_FFI_DIR	= $(STDLIB_DIR)/ffi
STDLIB_FFIOUT_DIR	= $(STDLIB_DIR)/ffiout
SYMBOL_DIR	= $(STDLIB_FFIOUT_DIR)

FFI_SRC_FILE	= $(STDLIB_FFIOUT_DIR)/FFI_SRC_FILES


J_CXXFLAGS	= -I$(COMMON_DIR) -I$(SYMBOL_DIR) -I$(MACHDEP_DIR) -I$(STDLIB_FFI_DIR) -I$(STDLIB_FFIOUT_DIR) -I$(TOOLS_DIR) -I./ $(CXXFLAGS)

CXX_MM = $(CXX) $(J_CXXFLAGS) -MM


include Makefile.common

#----------------------------------------------------------------------------



OBJS    = $(addprefix $(OBJSDIR)/,$(subst .cpp,.o,$(notdir $(SRCS))))
TOOLS_OBJS = $(addprefix $(OBJSDIR)/,$(subst .cpp,.o,$(notdir $(TOOLS_SRCS))))
FFI_OBJS	= $(addprefix $(OBJSDIR)/,$(subst .cpp,.o,$(notdir $(FFI_SRCS))))

DEPS = $(OBJS:.o=.d) $(TOOLS_OBJS:.o=.d) $(FFI_OBJS:.o=.d)



TARGET	= $(LIBDIR)/libhayat.a



$(OBJSDIR)/%.o : %.cpp
	$(CXX) $(J_CXXFLAGS) -c -o $@ $<
$(OBJSDIR)/%.o : $(COMMON_DIR)/%.cpp
	$(CXX) $(J_CXXFLAGS) -c -o $@ $<
$(OBJSDIR)/%.o : $(MACHDEP_DIR)/%.cpp
	$(CXX) $(J_CXXFLAGS) -c -o $@ $<
$(OBJSDIR)/%.o : $(STDLIB_FFI_DIR)/%.cpp
	$(CXX) $(J_CXXFLAGS) -c -o $@ $<
$(OBJSDIR)/%.o : $(STDLIB_FFIOUT_DIR)/%.cpp
	$(CXX) $(J_CXXFLAGS) -c -o $@ $<
$(OBJSDIR)/%.o : $(TOOLS_DIR)/%.cpp
	$(CXX) $(J_CXXFLAGS) -c -o $@ $<

$(OBJSDIR)/%.d : %.cpp
	@echo making $@
	@echo -n $(OBJSDIR)/ > $@
	$(CXX_MM) $< >> $@  || (rm $@; false)
$(OBJSDIR)/%.d : $(COMMON_DIR)/%.cpp
	@echo making $@
	@echo -n $(OBJSDIR)/ > $@
	$(CXX_MM) $< >> $@  || (rm $@; false)
$(OBJSDIR)/%.d : $(MACHDEP_DIR)/%.cpp
	@echo making $@
	@echo -n $(OBJSDIR)/ > $@
	$(CXX_MM) $< >> $@  || (rm $@; false)
$(OBJSDIR)/%.d : $(STDLIB_FFI_DIR)/%.cpp
	@echo making $@
	@echo -n $(OBJSDIR)/ > $@
	$(CXX_MM) $< >> $@  || (rm $@; false)
$(OBJSDIR)/%.d : $(STDLIB_FFIOUT_DIR)/%.cpp
	@echo making $@
	@echo -n $(OBJSDIR)/ > $@
	$(CXX_MM) $< >> $@  || (rm $@; false)
$(OBJSDIR)/%.d : $(TOOLS_DIR)/%.cpp
	@echo making $@
	@echo -n $(OBJSDIR)/ > $@
	$(CXX_MM) $< >> $@  || (rm $@; false)


#----------------------------------------------------------------------------

prep: prep-common prep-ffi
	-@mkdir -p $(OBJSDIR)
	-@mkdir -p $(LIBDIR)

prep-ffi:
	ruby ./collectFfi.rb -C $(STDLIB_DIR) -o $(FFI_SRC_FILE) ffi ffiout


# $(FFI_SRC_FILE) が prep-ffi で更新されるので
# その後makeを立ち上げ直してOBJSを更新しなければならない
make_target:
	$(MAKE) -f Makefile.withDep target

target: $(TARGET)

$(TARGET) : $(OBJS) $(TOOLS_OBJS) $(FFI_OBJS)
	ar rsc $(TARGET) $(OBJS) $(TOOLS_OBJS) $(FFI_OBJS)

make_depend:
	$(MAKE) depend

depend: $(DEPS)

printdeps:
	echo $(DEPS)

clean:	clean-common
	rm -rf $(LIBDIR) $(OBJSDIR) $(FFI_SRC_FILE)


#===== End of Makefile =====
