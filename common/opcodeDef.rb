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
    # topをコピー [x] -> [x,x]
    defOpcode("copy_top")
    # 2ndをコピー [x,y] -> [y,x,y]
    defOpcode("copy_2nd")
    # 3rdをコピー [x,y,z] -> [z,x,y,z]
    defOpcode("copy_3rd")
    # nil をプッシュ []->[nil]
    defOpcode("push_nil")
    # self をプッシュ []->[self]
    defOpcode("push_self")
    # 整数 0 をプッシュ []->[0]
    defOpcode("push_i0")
    # 整数 1 をプッシュ []->[1]
    defOpcode("push_i1")
    # 整数 -1 をプッシュ []->[-1]
    defOpcode("push_im1")
    # 符号付き8bit整数をプッシュ []->[i]
    defOpcode("push_i8", INT8)
    # 整数をプッシュ []->[i]
    defOpcode("push_i", INT)
    # 実数 0.0 をプッシュ []->[0.0]
    defOpcode("push_f0")
    # 実数 1.0 をプッシュ []->[1.0]
    defOpcode("push_f1")
    # 実数 -1.0 をプッシュ []->[-1.0]
    defOpcode("push_fm1")
    # 実数をプッシュ []->[r]
    defOpcode("push_f", FLOAT)
    # 文字列定数をプッシュ []->[s]
    defOpcode("push_strconst", STRCONST)
    # シンボルをプッシュ []->[s]
    defOpcode("push_symbol", SYMBOL)
    # trueをプッシュ [] -> [true]
    defOpcode("push_true")
    # falseをプッシュ [] -> [false]
    defOpcode("push_false")
    # Arrayリテラルにおける初期値代入 [xn, .. ,x2,x1,array]->[array]
    defOpcode("substArray", UINT8)     # UINT8 = 追加代入する個数
    # Hashリテラルにおける初期値代入 [vn,kn, .. ,v2,k2,v1,k1,hash]->[hash]
    defOpcode("substHash", UINT8)      # UINT8 = 追加代入するペア数
    # 空リストをプッシュ [] -> ['()]
    defOpcode("pushEmptyList")
    # consセルを作る [x,list] -> [list]
    defOpcode("cons")
    # ポップ [x]->[]
    defOpcode("pop")
    # 2ndを捨てる [x,y,z] -> [x,z]
    defOpcode("drop_2nd")
    # 引数0個のインスタンスメソッドコール [self]->[retval]
    defOpcode("insMethod_0", METHOD)
    # 引数1個のインスタンスメソッドコール [self,x]->[retval]
    defOpcode("insMethod_1", METHOD)
    # 引数2個のインスタンスメソッドコール [self,x2,x1]->[retval]
    defOpcode("insMethod_2", METHOD)
    # 引数n個のインスタンスメソッドコール [self,xn,..,x1]->[retval]
    defOpcode("insMethod", SIGNATURE, METHOD)
    # 引数1個のインスタンスメソッドコール2 [x,self]->[retval]
    defOpcode("insMethodR_1", METHOD)
    # 引数2個のインスタンスメソッドコール2 [x2,x1,self]->[retval]
    defOpcode("insMethodR_2", METHOD)
    # 引数n個のインスタンスメソッドコール2 [xn,..,x1,self]->[retval]
    defOpcode("insMethodR", SIGNATURE, METHOD)
    # 引数0個のメソッドコール []->[retval]
    defOpcode("method_0", METHOD)
    # 引数1個のメソッドコール [x]->[retval]
    defOpcode("method_1", METHOD)
    # 引数2個のメソッドコール [x2,x1]->[retval]
    defOpcode("method_2", METHOD)
    # 引数n個のメソッドコール [xn,...,x1]->[retval]
    defOpcode("method", SIGNATURE, METHOD)
    # 引数n個の指定クラスのメソッドコール [class,xn,...,x1]->[retval]
    defOpcode("scopeMethod", SIGNATURE, METHOD)
    # 引数n個のnew演算子 [xn,..,x1]->[newObj]
    defOpcode("new", CLASS, SIGNATURE)
    # ローカル変数をプッシュ []->[x]
    defOpcode("getLocal", LOCALVAR)
    # ローカル変数をセット [x]->[x]
    defOpcode("setLocal", LOCALVAR)
    # クラス変数をプッシュ []->[x]
    defOpcode("getClassVar", CLASSVAR)
    # クラス変数をセット [x]->[x]
    defOpcode("setClassVar", CLASSVAR)
    # メンバ変数をプッシュ []->[x]
    defOpcode("getMembVar", MEMBVAR)
    # メンバ変数をセット [x]->[x]
    defOpcode("setMembVar", MEMBVAR)
    # グローバル変数をプッシュ []->[x]
    defOpcode("getGlobal", GLOBALVAR)
    # グローバル変数をセット [x]->[x]
    defOpcode("setGlobal", GLOBALVAR)
    # クラスをプッシュ []->[x]
    defOpcode("getClass", CLASS)
    # 定数をプッシュ []->[x]
    defOpcode("getConstVar", CONSTVAR)
    # 定数をセット [x]->[x]
    defOpcode("setConstVar", CONSTVAR)
    # getterアクセス [obj]->[x]
    defOpcode("getMember", MEMBVAR)
    # setterアクセス [obj,x]->[x]
    defOpcode("setMember", MEMBVAR, METHOD)
    # デフォルト値をセット [x]->[]
    defOpcode("setDefaultVal", UINT16) # UINT16 = デフォルト値インデックス
    # 最後の値を戻り値として、リターン
    defOpcode("return")
    # callAwayで呼ばれたclosureだったら、最後の値を戻り値としてクロージャを生成したルーチンからリターン
    # その他の場合はreturnと同じ
    defOpcode("returnAway")
    # 無条件ジャンプ
    defOpcode("jump", RELATIVE)
    # falseの時ジャンプ [b]->[]
    defOpcode("jump_ifFalse", RELATIVE)
    # trueの時ジャンプ [b]->[]
    defOpcode("jump_ifTrue", RELATIVE)
    # 外部ラベルへのジャンプ
    defOpcode("jumpControl", JUMPSYMBOL)
    # スタックトップに積まれたラベルへのジャンプ [symbol] -> []
    defOpcode("goto")
    # クラスイニシャライザを呼ぶ(既に実行されていれば実行しない)
    defOpcode("classInit", CLASS)
    # 1 tick の wait []->[1]
    defOpcode("waitTick_1")
    # 指定tick数だけwait [n] -> [n] (スタックは変化しない)
    defOpcode("waitTicks")
    # スコープに対応するクラスを取得 []->[class]
    defOpcode("getScopeClass", SCOPE)
    # 指定クラスの定数を検索	[class]->[x]
    defOpcode("getClassConst", CONSTVAR)
    # 指定クラスの定数またはインナークラスを検索	[class]->[x]
    defOpcode("getScopedVal", SYMBOL)
    # jumpControlスタート
    defOpcode("jumpControlStart", MAYLOCALVAR, MAYLOCALVAR, UINT8) # UINT8 = ジャンプテーブル番号
    # jumpControlエンド
    defOpcode("jumpControlEnd")
    # 例外を投げる [obj,sym]->[]
    defOpcode("throw")
    # finally節にジャンプする	[x]->[]    xがtryブロックの値となる
    defOpcode("jump_finally")
    # finally節の終わり
    defOpcode("end_finally")

    # クロージャを生成する
    defOpcode("closure", UINT16, UINT8) # UINT16 = class内でのclosureインデックス、UINT8 = 外側ローカル変数の個数
    # 外側のスコープのローカル変数を共有する [closure]->[closure]
    defOpcode("shareLocal", UINT16, UINT16) # UINT16=クロージャのローカル変数idx、UINT16=外側ローカル変数index
    # yield
    defOpcode("yield")
    # 現在のcontextをfiberにする
    defOpcode("beFiber")
    # topと2ndをコピー [x,y] -> [x,y,x,y]
    defOpcode("copy_top_2nd")
    # topと2ndを交換 [x,y] -> [y,x]
    defOpcode("swap")
    # topを3rdの下に移動 [x,y,z] -> [y,z,x]
    defOpcode("rotate_r")
    # これからスタックに積む多重代入右辺のシグネチャを指定 []->[]
    defOpcode("ms_right", UINT16) # UINT16 = シグネチャ番号
    # 多重代入左辺に右辺が代入可能か調べて、失敗ならジャンプ
    defOpcode("ms_testLeft", UINT16, RELATIVE) # UINT16 = シグネチャ番号
    # スタックに積まれている多重代入右辺があればクリア [xn,...,x1]->[]
    defOpcode("ms_end")
    # コンパイル時には解決しなかったシンボルに対応する値を動的に取得
    defOpcode("getSomething", SYMBOL)
    # sprintf(fmt,x)を実行  [x] -> [stringBuffer]
    defOpcode("sprintf", STRCONST, UINT8) # UINT8 = xの型:0->int,1->float,2->string

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

        // オペランド無しのオペコード
#define DEF_OPCODE0(op)                         \
        class OP_##op {                         \
        public:                                 \
            static void addCodeTo(Bytecode& b);   \
        };

        // オペランド1個のオペコード
#define DEF_OPCODE1(op,opr1)                            \
        class OP_##op {                                 \
        public:                                         \
            static void addCodeTo(Bytecode& b, OPR_##opr1 v1);  \
        };

        // オペランド2個のオペコード
#define DEF_OPCODE2(op,opr1,opr2)                               \
        class OP_##op {                                         \
        public:                                                 \
            static void addCodeTo(Bytecode& b, OPR_##opr1 v1, OPR_##opr2 v2); \
        };


        // オペランド3個のオペコード
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
