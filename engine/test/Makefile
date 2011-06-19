# -*- makefile -*-
#

#
# CppUnit でエンジンをユニットテストする
#


HAYAT_ROOT	=	../..
SCRIPT_DIR	=	.
COMMON_DIR	=	$(HAYAT_ROOT)/common
ENGINE_DIR	=	$(HAYAT_ROOT)/engine
BUILDDIR	=	/test
ENGINE_LIBDIR	=	$(ENGINE_DIR)/lib$(BUILDDIR)
OBJSDIR         = 	obj
FFI_DIR	=	$(SCRIPT_DIR)/ffi
SYMBOL_DIR	=	$(SCRIPT_DIR)/ffiout

ENGINE_LIB	=	$(ENGINE_LIBDIR)/libhayat.a


# MACHDEP file includes definition of $(MACHDEP_DIR)
ifneq ($(MAKECMDGOALS),clean)
include $(ENGINE_DIR)/MACHDEP
endif
T_MACHDEP_DIR	=	$(ENGINE_DIR)/$(MACHDEP_DIR)

ifeq ($(shell uname -o),Cygwin)
EXE_EXT	= .exe
ADD_LDFLAGS	=
else
EXE_EXT	=
ADD_LDFLAGS	=
endif


CXXFLAGS	+=	-g -O0 -Wall -I$(ENGINE_DIR) -I$(SYMBOL_DIR) -I$(T_MACHDEP_DIR) -DTEST__CPPUNIT -I$(COMMON_DIR)
LDFLAGS		+=	$(ADD_LDFLAGS) -lcppunit -L$(ENGINE_LIBDIR) -lhayat



SRCS	=	tester.cpp			\
		$(wildcard Test_*.cpp)

#

OBJS    = $(addprefix $(OBJSDIR)/,$(subst .cpp,.o,$(notdir $(SRCS))))

TARGET_BASENAME = tester
TARGET	=	$(TARGET_BASENAME)$(EXE_EXT)


.PHONY:	all prep script engine run clean

all:	prep run


$(OBJSDIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJSDIR)/%.o : $(FFI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJSDIR)/%.o : $(T_MACHDEP_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<


prep:
	-@mkdir -p $(OBJSDIR)

script:
	(cd $(SCRIPT_DIR) && rake )

engine:
	$(MAKE) -C $(ENGINE_DIR) all CXXFLAGS='$(CXXFLAGS)' BUILDDIR=$(BUILDDIR)

clean:
	rm -rf $(OBJSDIR)
	rm -f $(TARGET_BASENAME) $(TARGET_BASENAME).exe
	rm -f $(TARGET).stackdump
	(cd $(SCRIPT_DIR) && rake clean)
	$(MAKE) -C $(ENGINE_DIR) clean BUILDDIR=$(BUILDDIR)

$(TARGET):	script engine $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)


run: $(TARGET)
	@echo '-=-=-=-=-=-=-=-=-=-=-{ engine/test'
	-./$(TARGET)
	@echo '-=-=-=-=-=-=-=-=-=-=-}'
