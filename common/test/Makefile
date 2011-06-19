# -*- makefile -*-
#

#
# CppUnit ユニットテストする
#

.PHONY:	all prep run clean

all:	prep run


ifneq ($(MAKECMDGOALS),clean)
include ../../engine/MACHDEP
endif
T_MACHDEP_DIR 	= 	../../engine/$(MACHDEP_DIR)


ifeq ($(shell uname -o),Cygwin)
EXE_EXT	= .exe
ADD_LDFLAGS	= 
else
EXE_EXT	=
ADD_LDFLAGS	= 
endif

HAYAT_ROOT	=	../..
COMMON_DIR	=	..
ENGINE_DIR	=	../../engine
OBJSDIR         = 	obj

CXXFLAGS	+=	-g -Wall -I$(COMMON_DIR) -I$(ENGINE_DIR) -I$(T_MACHDEP_DIR) -DTEST__CPPUNIT -DHY_WORK_WITHOUT_VM
LDFLAGS		+=	$(ADD_LDFLAGS) -lcppunit


include $(COMMON_DIR)/COMMON_SRCS


SRCS	=	$(COMMON_SRCS)			\
		$(T_MACHDEP_DIR)/machdep.cpp	\
		tester.cpp			\
		$(wildcard Test_*.cpp)

#

OBJS    = $(addprefix $(OBJSDIR)/,$(subst .cpp,.o,$(notdir $(SRCS))))

TARGET_BASENAME = tester
TARGET	=	$(TARGET_BASENAME)$(EXE_EXT)


$(OBJSDIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
$(OBJSDIR)/%.o : ../%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
$(OBJSDIR)/machdep.o : $(T_MACHDEP_DIR)/machdep.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<


prep:
	mkdir -p $(OBJSDIR)

clean:
	rm -rf $(OBJSDIR)
	rm -f $(TARGET_BASENAME) $(TARGET_BASENAME).exe $(TARGET).stackdump

$(TARGET):	$(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)


run: $(TARGET)
	@echo '-=-=-=-=-=-=-=-=-=-=-{ common/test'
	-./$(TARGET)
	@echo '-=-=-=-=-=-=-=-=-=-=-}'
