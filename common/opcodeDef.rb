#!/usr/bin/ruby

# copyright 2010 FUKUZAWA Tadashi

#
# Hayat opcode definition
#

# if executed with option -C, output files:
#	hyC_Opcode.h
#	hyC_Opcode.cpp
#
# if executed with option -E, output files:
#	opcode_def.h
#	opcode_func_defs.inc
#	opcode_func_table.inc
#	opcode_nameTable.cpp


require 'tempfile'
require 'fileutils'

def diffWrite(file)
  begin
    tmpFile = Tempfile.open(File.basename(file), File.dirname(file))
    yield tmpFile
    tmpFile.flush
    FileUtils.install(tmpFile.path, file)
  ensure
    tmpFile.close(true)
  end
end


class Opcode
    
    @@opcodes = []

    INT8 = "INT8"
    UINT8 = "UINT8"
    UINT16 = "UINT16"
    INT = "INT"
    FLOAT = "FLOAT"
    STRCONST = "STRCONST"
    SYMBOL = "SYMBOL"
    CLASS = "CLASS"
    METHOD = "METHOD"
    LOCALVAR = "LOCALVAR"
    MAYLOCALVAR = "MAYLOCALVAR"
    CLASSVAR = "CLASSVAR"
    MEMBVAR = "MEMBVAR"
    GLOBALVAR = "GLOBALVAR"
    CONSTVAR = "CONSTVAR"
    RELATIVE = "RELATIVE"
    JUMPSYMBOL = "JUMPSYMBOL"
    SCOPE = "SCOPE"
    SIGNATURE = "SIGNATURE"

    def self.defOpcode (*v)
      @@opcodes.push(v)
    end

    # [top,2nd,3rd,..]

    # nop
    defOpcode("nop")
    # top���R�s�[ [x] -> [x,x]
    defOpcode("copy_top")
    # 2nd���R�s�[ [x,y] -> [y,x,y]
    defOpcode("copy_2nd")
    # 3rd���R�s�[ [x,y,z] -> [z,x,y,z]
    defOpcode("copy_3rd")
    # nil ���v�b�V�� []->[nil]
    defOpcode("push_nil")
    # self ���v�b�V�� []->[self]
    defOpcode("push_self")
    # ���� 0 ���v�b�V�� []->[0]
    defOpcode("push_i0")
    # ���� 1 ���v�b�V�� []->[1]
    defOpcode("push_i1")
    # ���� -1 ���v�b�V�� []->[-1]
    defOpcode("push_im1")
    # �����t��8bit�������v�b�V�� []->[i]
    defOpcode("push_i8", INT8)
    # �������v�b�V�� []->[i]
    defOpcode("push_i", INT)
    # ���� 0.0 ���v�b�V�� []->[0.0]
    defOpcode("push_f0")
    # ���� 1.0 ���v�b�V�� []->[1.0]
    defOpcode("push_f1")
    # ���� -1.0 ���v�b�V�� []->[-1.0]
    defOpcode("push_fm1")
    # �������v�b�V�� []->[r]
    defOpcode("push_f", FLOAT)
    # ������萔���v�b�V�� []->[s]
    defOpcode("push_strconst", STRCONST)
    # �V���{�����v�b�V�� []->[s]
    defOpcode("push_symbol", SYMBOL)
    # true���v�b�V�� [] -> [true]
    defOpcode("push_true")
    # false���v�b�V�� [] -> [false]
    defOpcode("push_false")
    # Array���e�����ɂ����鏉���l��� [xn, .. ,x2,x1,array]->[array]
    defOpcode("substArray", UINT8)     # UINT8 = �ǉ���������
    # Hash���e�����ɂ����鏉���l��� [vn,kn, .. ,v2,k2,v1,k1,hash]->[hash]
    defOpcode("substHash", UINT8)      # UINT8 = �ǉ��������y�A��
    # �󃊃X�g���v�b�V�� [] -> ['()]
    defOpcode("pushEmptyList")
    # cons�Z������� [x,list] -> [list]
    defOpcode("cons")
    # �|�b�v [x]->[]
    defOpcode("pop")
    # 2nd���̂Ă� [x,y,z] -> [x,z]
    defOpcode("drop_2nd")
    # ����0�̃C���X�^���X���\�b�h�R�[�� [self]->[retval]
    defOpcode("insMethod_0", METHOD)
    # ����1�̃C���X�^���X���\�b�h�R�[�� [self,x]->[retval]
    defOpcode("insMethod_1", METHOD)
    # ����2�̃C���X�^���X���\�b�h�R�[�� [self,x2,x1]->[retval]
    defOpcode("insMethod_2", METHOD)
    # ����n�̃C���X�^���X���\�b�h�R�[�� [self,xn,..,x1]->[retval]
    defOpcode("insMethod", SIGNATURE, METHOD)
    # ����1�̃C���X�^���X���\�b�h�R�[��2 [x,self]->[retval]
    defOpcode("insMethodR_1", METHOD)
    # ����2�̃C���X�^���X���\�b�h�R�[��2 [x2,x1,self]->[retval]
    defOpcode("insMethodR_2", METHOD)
    # ����n�̃C���X�^���X���\�b�h�R�[��2 [xn,..,x1,self]->[retval]
    defOpcode("insMethodR", SIGNATURE, METHOD)
    # ����0�̃��\�b�h�R�[�� []->[retval]
    defOpcode("method_0", METHOD)
    # ����1�̃��\�b�h�R�[�� [x]->[retval]
    defOpcode("method_1", METHOD)
    # ����2�̃��\�b�h�R�[�� [x2,x1]->[retval]
    defOpcode("method_2", METHOD)
    # ����n�̃��\�b�h�R�[�� [xn,...,x1]->[retval]
    defOpcode("method", SIGNATURE, METHOD)
    # ����n�̎w��N���X�̃��\�b�h�R�[�� [class,xn,...,x1]->[retval]
    defOpcode("scopeMethod", SIGNATURE, METHOD)
    # ����n��new���Z�q [xn,..,x1]->[newObj]
    defOpcode("new", CLASS, SIGNATURE)
    # ���[�J���ϐ����v�b�V�� []->[x]
    defOpcode("getLocal", LOCALVAR)
    # ���[�J���ϐ����Z�b�g [x]->[x]
    defOpcode("setLocal", LOCALVAR)
    # �N���X�ϐ����v�b�V�� []->[x]
    defOpcode("getClassVar", CLASSVAR)
    # �N���X�ϐ����Z�b�g [x]->[x]
    defOpcode("setClassVar", CLASSVAR)
    # �����o�ϐ����v�b�V�� []->[x]
    defOpcode("getMembVar", MEMBVAR)
    # �����o�ϐ����Z�b�g [x]->[x]
    defOpcode("setMembVar", MEMBVAR)
    # �O���[�o���ϐ����v�b�V�� []->[x]
    defOpcode("getGlobal", GLOBALVAR)
    # �O���[�o���ϐ����Z�b�g [x]->[x]
    defOpcode("setGlobal", GLOBALVAR)
    # �N���X���v�b�V�� []->[x]
    defOpcode("getClass", CLASS)
    # �萔���v�b�V�� []->[x]
    defOpcode("getConstVar", CONSTVAR)
    # �萔���Z�b�g [x]->[x]
    defOpcode("setConstVar", CONSTVAR)
    # getter�A�N�Z�X [obj]->[x]
    defOpcode("getMember", MEMBVAR)
    # setter�A�N�Z�X [obj,x]->[x]
    defOpcode("setMember", MEMBVAR, METHOD)
    # �f�t�H���g�l���Z�b�g [x]->[]
    defOpcode("setDefaultVal", UINT16) # UINT16 = �f�t�H���g�l�C���f�b�N�X
    # �Ō�̒l��߂�l�Ƃ��āA���^�[��
    defOpcode("return")
    # callAway�ŌĂ΂ꂽclosure��������A�Ō�̒l��߂�l�Ƃ��ăN���[�W���𐶐��������[�`�����烊�^�[��
    # ���̑��̏ꍇ��return�Ɠ���
    defOpcode("returnAway")
    # �������W�����v
    defOpcode("jump", RELATIVE)
    # false�̎��W�����v [b]->[]
    defOpcode("jump_ifFalse", RELATIVE)
    # true�̎��W�����v [b]->[]
    defOpcode("jump_ifTrue", RELATIVE)
    # �O�����x���ւ̃W�����v
    defOpcode("jumpControl", JUMPSYMBOL)
    # �X�^�b�N�g�b�v�ɐς܂ꂽ���x���ւ̃W�����v [symbol] -> []
    defOpcode("goto")
    # �N���X�C�j�V�����C�U���Ă�(���Ɏ��s����Ă���Ύ��s���Ȃ�)
    defOpcode("classInit", CLASS)
    # 1 tick �� wait []->[1]
    defOpcode("waitTick_1")
    # �w��tick������wait [n] -> [n] (�X�^�b�N�͕ω����Ȃ�)
    defOpcode("waitTicks")
    # �X�R�[�v�ɑΉ�����N���X���擾 []->[class]
    defOpcode("getScopeClass", SCOPE)
    # �w��N���X�̒萔������	[class]->[x]
    defOpcode("getClassConst", CONSTVAR)
    # �w��N���X�̒萔�܂��̓C���i�[�N���X������	[class]->[x]
    defOpcode("getScopedVal", SYMBOL)
    # jumpControl�X�^�[�g
    defOpcode("jumpControlStart", MAYLOCALVAR, MAYLOCALVAR, UINT8) # UINT8 = �W�����v�e�[�u���ԍ�
    # jumpControl�G���h
    defOpcode("jumpControlEnd")
    # ��O�𓊂��� [obj,sym]->[]
    defOpcode("throw")
    # finally�߂ɃW�����v����	[x]->[]    x��try�u���b�N�̒l�ƂȂ�
    defOpcode("jump_finally")
    # finally�߂̏I���
    defOpcode("end_finally")

    # �N���[�W���𐶐�����
    defOpcode("closure", UINT16, UINT8) # UINT16 = class���ł�closure�C���f�b�N�X�AUINT8 = �O�����[�J���ϐ��̌�
    # �O���̃X�R�[�v�̃��[�J���ϐ������L���� [closure]->[closure]
    defOpcode("shareLocal", UINT16, UINT16) # UINT16=�N���[�W���̃��[�J���ϐ�idx�AUINT16=�O�����[�J���ϐ�index
    # yield
    defOpcode("yield")
    # ���݂�context��fiber�ɂ���
    defOpcode("beFiber")
    # top��2nd���R�s�[ [x,y] -> [x,y,x,y]
    defOpcode("copy_top_2nd")
    # top��2nd������ [x,y] -> [y,x]
    defOpcode("swap")
    # top��3rd�̉��Ɉړ� [x,y,z] -> [y,z,x]
    defOpcode("rotate_r")
    # ���ꂩ��X�^�b�N�ɐςޑ��d����E�ӂ̃V�O�l�`�����w�� []->[]
    defOpcode("ms_right", UINT16) # UINT16 = �V�O�l�`���ԍ�
    # ���d������ӂɉE�ӂ�����\�����ׂāA���s�Ȃ�W�����v
    defOpcode("ms_testLeft", UINT16, RELATIVE) # UINT16 = �V�O�l�`���ԍ�
    # �X�^�b�N�ɐς܂�Ă��鑽�d����E�ӂ�����΃N���A [xn,...,x1]->[]
    defOpcode("ms_end")
    # �R���p�C�����ɂ͉������Ȃ������V���{���ɑΉ�����l�𓮓I�Ɏ擾
    defOpcode("getSomething", SYMBOL)
    # sprintf(fmt,x)�����s  [x] -> [stringBuffer]
    defOpcode("sprintf", STRCONST, UINT8) # UINT8 = x�̌^:0->int,1->float,2->string

    @@debugSymbol = {
      INT8 => "i",
      UINT8 => "1",
      UINT16 => "2",
      INT => "I",
      FLOAT => "F",
      STRCONST => "S",
      SYMBOL => "Y",
      CLASS => "C",
      METHOD => "M",
      LOCALVAR => "l",
      MAYLOCALVAR => "m",
      CLASSVAR => "c",
      MEMBVAR => "f",
      GLOBALVAR => "g",
      CONSTVAR => "A",
      RELATIVE => "R",
      JUMPSYMBOL => "y",
      SCOPE => "s",
      SIGNATURE => "p"
    }

    def self.debTable (v)
      d = v[1..-1].map{|i| @@debugSymbol[i]}.join('')
      "  {\"#{v[0]}\", \"#{d}\"},"
    end


    def self.outHeader (out)
      out.print <<'__EOD__'
/*  this file was generated by common/opcodeDef.rb . DO NOT EDIT!  */

#ifndef _HYCOPCODE_H_
#define _HYCOPCODE_H_

#include "hyCOperand.h"

namespace Hayat {
    namespace Compiler {

        enum OpcodeLabel_e {
__EOD__
      @@opcodes.each do |v|
        out.puts "            OPL_#{v[0]},"
      end

      out.print <<'__EOD__'
        };

        // �I�y�����h�����̃I�y�R�[�h
#define DEF_OPCODE0(op)                         \
        class OP_##op {                         \
        public:                                 \
            static void addCodeTo(Bytecode& b);   \
        };

        // �I�y�����h1�̃I�y�R�[�h
#define DEF_OPCODE1(op,opr1)                            \
        class OP_##op {                                 \
        public:                                         \
            static void addCodeTo(Bytecode& b, OPR_##opr1 v1);  \
        };

        // �I�y�����h2�̃I�y�R�[�h
#define DEF_OPCODE2(op,opr1,opr2)                               \
        class OP_##op {                                         \
        public:                                                 \
            static void addCodeTo(Bytecode& b, OPR_##opr1 v1, OPR_##opr2 v2); \
        };


        // �I�y�����h3�̃I�y�R�[�h
#define DEF_OPCODE3(op,opr1,opr2,opr3)                          \
        class OP_##op {                                         \
        public:                                                 \
            static void addCodeTo(Bytecode& b, OPR_##opr1 v1, OPR_##opr2 v2, OPR_##opr3 v3); \
        };

__EOD__
      @@opcodes.each do |v|
        out.puts "        DEF_OPCODE#{v.size - 1}(#{v.join(', ')});"
      end      
      
      out.print <<'__EOD__'

#undef DEF_OPCODE0
#undef DEF_OPCODE1
#undef DEF_OPCODE2
#undef DEF_OPCODE3

    }
}

#endif /* _HYCOPCODE_H_ */
__EOD__

    end

    def self.outCpp (out)
      out.print <<'__EOD__'
/*  this file was generated by common/opcodeDef.rb . DO NOT EDIT!  */

#include "hyC_opcode.h"

using namespace Hayat::Compiler;

#define DECL_OPCODE0(op)                        \
    void OP_##op::addCodeTo(Bytecode& b)          \
    {                                           \
        b.addCode<hyu8>(OPL_##op);                  \
    }

#define DECL_OPCODE1(op, opr1)                          \
    void OP_##op::addCodeTo(Bytecode& b, OPR_##opr1 v1)   \
    {                                                   \
        b.addCode<hyu8>(OPL_##op);                          \
        v1.addCodeTo(b);                                \
    }

#define DECL_OPCODE2(op, opr1, opr2)                                    \
    void OP_##op::addCodeTo(Bytecode& b, OPR_##opr1 v1, OPR_##opr2 v2)    \
    {                                                                   \
        b.addCode<hyu8>(OPL_##op);                                          \
        v1.addCodeTo(b);                                                \
        v2.addCodeTo(b);                                                \
    }

#define DECL_OPCODE3(op, opr1, opr2, opr3)                              \
    void OP_##op::addCodeTo(Bytecode& b, OPR_##opr1 v1, OPR_##opr2 v2, OPR_##opr3 v3)    \
    {                                                                   \
        b.addCode<hyu8>(OPL_##op);                                      \
        v1.addCodeTo(b);                                                \
        v2.addCodeTo(b);                                                \
        v3.addCodeTo(b);                                                \
    }


__EOD__

      @@opcodes.each do |v|
        out.puts "DECL_OPCODE#{v.size - 1}(#{v.join(', ')});"
      end      

      out.print <<'__EOD__'


#undef DECL_OPCODE0
#undef DECL_OPCODE1
#undef DECL_OPCODE2
__EOD__
    end

    def self.outDefInc (out)
      out.puts "/*  this file was generated by common/opcodeDef.rb . DO NOT EDIT!  */"
      @@opcodes.each do |v|
        out.puts "	void	m_opcode_#{v[0].upcase}(void);"
      end      
    end

    def self.outTableInc (out)
      out.puts "/*  this file was generated by common/opcodeDef.rb . DO NOT EDIT!  */"
      @@opcodes.each do |v|
        out.puts "\t\t&VM::m_opcode_#{v[0].upcase},"
      end      
    end


    def self.outDefH (out)
      out.puts "/*  this file was generated by common/opcodeDef.rb . DO NOT EDIT!  */"
      @@opcodes.each_with_index do |v, i|
        out.puts "#define OPCODE_#{v[0].upcase}\t#{i}"
      end      
      out.puts "#define NUM_OPCODES #{@@opcodes.size}"
    end

    def self.outNameTable (out)
      out.print <<'__EOD__'
/*  this file was generated by common/opcodeDef.rb . DO NOT EDIT!  */
#include "machdep.h"
struct m_op_d_st { const char* name; const char* type;
} m_HAYAT_OPCODE_DEBUG_TABLE_[] = {
__EOD__
      @@opcodes.each do |v|
        out.puts Opcode.debTable(v)
      end      
      out.print <<'__EOD__'
};
__EOD__
    end
end



if $0 == __FILE__

  if ARGV[0] == '-C'
    diffWrite("hyC_opcode.h"){|f| Opcode.outHeader(f)}
    diffWrite("hyC_opcode.cpp"){|f| Opcode.outCpp(f)}
  elsif ARGV[0] == '-E'
    diffWrite("opcode_func_defs.inc"){|f| Opcode.outDefInc(f)}
    diffWrite("opcode_func_table.inc"){|f| Opcode.outTableInc(f)}
    diffWrite("opcode_def.h"){|f| Opcode.outDefH(f)}
    diffWrite("opcode_nameTable.cpp"){|f| Opcode.outNameTable(f)}
  else
    puts "option -C to create files for compiler,"
    puts "option -E to create files for engine."
    exit 1
  end

end
