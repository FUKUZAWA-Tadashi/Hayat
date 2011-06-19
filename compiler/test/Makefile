# -*- makefile -*-
#

#
# CppUnit でコンパイラをユニットテストする
#


HAYAT_ROOT	=	../..
SCRIPT_DIR	=	.
COMMON_DIR	=	$(HAYAT_ROOT)/common
COMPILER_DIR	=	$(HAYAT_ROOT)/compiler
COMPILER_LIBDIR	=	$(COMPILER_DIR)/lib
OBJSDIR         = 	obj
PARSER_DIR	=	$(HAYAT_ROOT)/parser
HOUKEN_DIR	=	$(HAYAT_ROOT)/houken

ifneq ($(MAKECMDGOALS),clean)
include $(COMPILER_DIR)/MACHDEP
endif
T_MACHDEP_DIR 	= 	$(COMPILER_DIR)/$(MACHDEP_DIR)

ifeq ($(shell uname -o),Cygwin)
ICONV_LIB	=	-liconv
ADD_LDFLAGS	=
else
ICONV_LIB	=
ADD_LDFLAGS	=
endif


CXXFLAGS	+=	-g -O0 -Wall -I$(COMPILER_DIR) -I$(PARSER_DIR) -I$(T_MACHDEP_DIR) -DTEST__CPPUNIT -DHY_WORK_WITHOUT_VM -I$(COMMON_DIR)
LDFLAGS		+=	$(ADD_LDFLAGS) -lcppunit -L$(COMPILER_LIBDIR) $(ICONV_LIB)


# common source files
include $(COMMON_DIR)/COMMON_SRCS

PARSER_SRCS =	$(PARSER_DIR)/hpInputBuffer.cpp \
		$(PARSER_DIR)/hpSourceInfo.cpp

SRCS	=	$(COMMON_SRCS) \
		$(T_MACHDEP_DIR)/machdep.cpp \
		$(T_MACHDEP_DIR)/mbcsconv.cpp \
		$(PARSER_SRCS) \
		$(COMPILER_DIR)/hyCCompileError.cpp \
		tester.cpp \
		$(COMPILER_DIR)/hyCSymbolTable.cpp \
		Test_hyCSymbolTable.cpp \
		$(COMPILER_DIR)/hyCFileOut.cpp \
		Test_hyCFileOut.cpp \
		$(COMPILER_DIR)/hyCBytecode.cpp \
		Test_hyCBytecode.cpp \
		$(COMPILER_DIR)/hyCOperand.cpp \
		Test_hyCOperand.cpp \
		$(COMPILER_DIR)/hyC_opcode.cpp \
		Test_hyCOpcode.cpp \
		$(COMPILER_DIR)/hyCStrTable.cpp \
		Test_hyCStrTable.cpp \
		$(COMPILER_DIR)/hyCContext.cpp \
		Test_hyCContext.cpp \
		$(COMPILER_DIR)/hyCClassInfo.cpp \
		$(COMPILER_DIR)/hyCIdent.cpp \
		Test_hyCClassInfo.cpp \
		$(COMPILER_DIR)/hyCFfiType.cpp \
		$(COMPILER_DIR)/hyCPackage.cpp \
		Test_hyCPackage.cpp \
		Test_hyCJumpControlTree.cpp \
		$(COMPILER_DIR)/hyCJumpControlTree.cpp \
		Test_hyCSignature.cpp \
		$(COMPILER_DIR)/hyCSignature.cpp \
		Test_filelock.cpp \


#

OBJS    = $(addprefix $(OBJSDIR)/,$(subst .c,.o,$(subst .cpp,.o,$(notdir $(SRCS)))))

ifeq ($(shell uname -o),Cygwin)
EXE_EXT	= .exe
else
EXE_EXT	=
endif

TARGET_BASENAME = tester
TARGET	=	$(TARGET_BASENAME)$(EXE_EXT)

FILES_USED_FOR_TEST = _diff_write_test  _test_sym_file  _test_strtable_file  _test_lock_file



.PHONY:	all prep run clean

all:	prep run

prep:
	-@mkdir -p $(OBJSDIR)

clean:
	rm -rf $(OBJSDIR)
	rm -f $(TARGET_BASENAME) $(TARGET_BASENAME).exe
	rm -f $(TARGET).stackdump
	rm -f $(FILES_USED_FOR_TEST)

run: $(TARGET)
	@echo '-=-=-=-=-=-=-=-=-=-=-{ compiler/test'
	-./$(TARGET)
	@echo '-=-=-=-=-=-=-=-=-=-=-}'
	rm -f $(FILES_USED_FOR_TEST)




$(COMPILER_DIR)/hyC_opcode.cpp : $(COMMON_DIR)/opcodeDef.rb
	(cd $(COMPILER_DIR) && ruby ../common/opcodeDef.rb -C)

$(OBJSDIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJSDIR)/%.o : $(COMMON_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJSDIR)/%.o : $(COMPILER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJSDIR)/%.o : $(PARSER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJSDIR)/%.o : $(T_MACHDEP_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<


$(TARGET):	$(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

