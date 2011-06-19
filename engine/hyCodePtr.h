/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyValue.h"
#include "hyClass.h"


#ifndef m_HYCODEPTR_H_
#define m_HYCODEPTR_H_


namespace Hayat {
    namespace Engine {
        class Bytecode;
        //class Object;
        class StringBuffer;


        class CodePtr {
        public:
            CodePtr(const hyu8* startAddr = NULL) { initialize(startAddr); }
            void            initialize(const hyu8* startAddr);
            void            finalize(void);
            const hyu8*       addr(void) const { return m_ptr; }
            void            advance(int n) { m_ptr += n; }
            void            jump(const hyu8* addr) { m_ptr = addr; }

            bool            haveCode(void) { return m_ptr != NULL; }

            hyu8              operator*() { return *m_ptr; }
            void            operator+=(int n) { m_ptr += n; }

        protected:
            const hyu8*       m_ptr;

        };




        // インスタンス情報を含んだバイトコード情報
        // FFIにも対応
        class MethodPtr : public CodePtr {
        public:
            MethodPtr(void) { finalize(); }
            MethodPtr(const Value& self) : m_self(self) {
                setMethodAddr(self.getScope(), NULL); }
            MethodPtr(const Value& self, const hyu8* methodAddr, hyu16 methodID = (hyu16)-1) : m_self(self) {
                setMethodAddr(self.getScope(), methodAddr, methodID); }

            // インスタンスを設定
            void setSelf(const Value& self) { m_self = self; }
            // バイトコードとして設定
            void setMethodAddr(const HClass* lexicalScope, const hyu8* methodAddr, hyu16 methodID = (hyu16)-1);
            // FFIとして設定
            void setFfi(const HClass* lexicalScope, Ffi_t ffi);

            // クラスメソッドなら true, インスタンスメソッドなら false を設定
            // FFIの場合のみ有効
            void setClassMethod(bool bClassMethod) { m_bClassMethod = bClassMethod; }
            // メソッドIDを取得 ; FFIではない場合のみ有効
            hyu16 getMethodID(void) const { HMD_ASSERT(!m_bFfi); return m_methodID; }

            const HClass* lexicalScope(void) const { return m_lexicalScope; }

            void finalize(void);

            const Value&    getSelf(void) const { return m_self; }
            Ffi_t       getFfi(void) const { return (Ffi_t)m_ptr; }

            bool    isFfi(void) const { return m_bFfi; }

            // クラスメソッドかどうか ： FFIの場合のみ有効
            bool    isClassMethod(void) const  { return m_bClassMethod; }

        protected:
            Value       m_self;
            // スーパークラスのメソッドを呼ぶ場合、m_lexicalScopeは
            // スーパークラスを指す。でなければ m_self.getScope()と同じ。
            const HClass*     m_lexicalScope;
            bool        m_bFfi;  // FFIかどうか
            union {
                bool    m_bClassMethod; // ffi時有効
                hyu16     m_methodID; // !ffi時有効
            };

        public:
            void m_GC_mark(void);
        };

    }
}


#endif /* m_HYCODEPTR_H_ */
