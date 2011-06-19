/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYOBJECT_H_
#define m_HYOBJECT_H_

#include "hyClass.h"
#include "hyStaticAssert.h"


/*
  オブジェクト(インスタンス)のデータ構造

  +----- (A)
  | void* m_type         bit0 = GCマークフラグ (0 = mark, 1 = unmark)
  |                     上位ビット = HClass*
  +----- field(0)のアドレス
  |
  | メンバ変数領域
  | 
  |
  +----- (B)
  | hyu32 メンバ変数領域のバイト数
  +----- (C)
  |
  | C++ インスタンス領域
  |
  |
  +-----

(A) -> (C)
  T* cppObj<T>()

(C) -> (A)
  Object* fromCppObj(const void*)

*/


using namespace Hayat::Common;


class Test_hyGC;                // for unittest

namespace Hayat {
    namespace Common {
        class MemPool;
    }

    namespace Engine {
        class Thread;
        class ThreadManager;

        /// 基本オブジェクト
        class Object {
            friend class GC;
            friend class ::Test_hyGC;           // for unittest
            friend class ThreadManager;     // for Thread GC

        protected:

            union m_TypeMask_t {
                void*           voidp;  // ポインタ
                unsigned int    intv;   // ポインタをビット演算するため
            } m_type;
            // m_type.voidp と m_type.intv は同じサイズでないといけない
            STATIC_ASSERT(sizeof(void*) == sizeof(unsigned int), m_TypeMask_t_size);

            static const unsigned int        m_MASK_TYPE = ~3;
            static const unsigned int        m_MASK_GC_MARK = 1;

            void    m_mark(void) { m_type.intv &= ~m_MASK_GC_MARK; }
            void    m_unmark(void) { m_type.intv |= m_MASK_GC_MARK; }

        public:
            // 自分がマークされていなければ、
            // 自分にマークして、さらにtypeに応じて保持しているObject*をマークする
            void    m_GC_mark(void);

#ifdef HY_ENABLE_BYTECODE_RELOAD
            // クラスを変更する。新旧クラスのメンバ変数の調整を行なう。
            // メンバ変数の個数が異なる場合、移動が行なわれる。
            // 新オブジェクトの場所を返す。
            Object* classGeneration(const HClass* newClass);
#if 0
            void s_dump(void);
#endif
#endif

        protected:
            // デストラクタ
            void    m_finalize(Context* finalizeContext);

        public:
            static Object* create(const HClass* type, size_t size);
            static Object* create(SymbolID_t bytecodeSym, SymbolID_t classSym, size_t size);

            void            destroy(void);
 
            size_t          cellSize(void);  // 対応するMemCellのバイト数
            size_t          size(void);     // 型情報を除いたフィールドメモリサイズ

            // 内容のうち、メンバ変数が全部等しいかどうか
            bool            equals_memb(Context* context, Object* o);

            // 内容が等しいかどうか
            bool            equals(Context* context, Object* o);

            /// オブジェクトの型
            const HClass*   type(void) { return (const HClass*)(m_type.intv & m_MASK_TYPE); }

            bool    isMarked(void) { return (m_type.intv & m_MASK_GC_MARK) == 0; }

            /// フィールドのアドレス + offs(bytes)
            void*           field(int offs = 0) { return (void*)(((char*)this) + sizeof(Object) + offs); }

            /// n番目のスーパークラスオブジェクトを取得
            Object*         superObj(int n) { return *(Object**)field(sizeof(Object*) * n); }

            /// C++インスタンスのアドレス
            template <typename T> T* cppObj(void) {
			  return (T*) field(type()->fieldSize() + sizeof(hyu32)); }
#ifdef HMD_DEBUG
            template <typename T> T* cppObj(SymbolID_t sym) {
                HMD_ASSERT(type()->symCheck(sym));
                return (T*) field(type()->fieldSize() + sizeof(hyu32)); }
#else
            template <typename T> T* cppObj(SymbolID_t) {
                return (T*) field(type()->fieldSize() + sizeof(hyu32)); }
#endif

            /// C++インスタンスのアドレスから、Objectのアドレスを計算
            static Object* fromCppObj(const void* cppObj);


            /// GCのためにmarkする。但し中身は関知しない。
            void            markSelfOnly(void) { m_mark(); }

            // GCの対象ではなくする (デフォルトはGC対象になっている)
            void    unsetGCObject(void);
            // GCの対象にする
            void    setGCObject(void);
            // GCの対象かどうか
            bool    isGCObject(void);

        };

    }
}

#endif /* m_HYOBJECT_H_ */
