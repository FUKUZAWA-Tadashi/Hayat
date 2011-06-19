/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCSIGNATURE_H_
#define m_HYCSIGNATURE_H_

#include "hySignatureBase.h"
#include "hyArray.h"
#include "hyCCompileError.h"

using namespace Hayat::Common;


class Test_CSignature;

namespace Hayat {
    namespace Compiler {

        class Signature : public SignatureBase {
            friend class ::Test_CSignature;

        public:

            class IllegalArgException : public CException {
            public:
                IllegalArgException(const char* mes) : CException(mes) {}
                static void throwEx(const char* mes) {
                    IllegalArgException ex(mes);
                    throw ex;
                }
            };

            
            static void* operator new(size_t size);
            static void operator delete(void* p);

            Signature(void);
            ~Signature() {}

            // 引数シグネチャ追加： エラーがあれば IllegalArgException を投げる
            void    addSig(Sig_t sig);
            // ネスティングしたサブレベルを開始し、そのサブレベルを返す
            Signature*      openSub(void);
            // 現レベルを終了して親レベルを返す
            Signature*      close(void);

            // 左辺シグネチャに右辺シグネチャを代入可能かどうか
            static bool canSubst(const Signature& left, const Signature& right);
            bool    canSubst(const Signature& right) const { return canSubst(*this, right); }
            static bool canSubst(const Sig_t*& left, const Sig_t*& right);

            hyu8*     getSigBytes(void) { return (hyu8*) m_sigs.top(); }
            hyu32     getSigBytesLen(void) { return m_sigs.size(); }


            // デフォルト値インデックスの調整
            void        adjustDefaultValIdx(int startIdx = 0) { Sig_t* p = m_sigs.top(); SignatureBase::m_adjustDefaultValIdx(p, startIdx); }
            // 引数の個数
            static hyu8   arity(const Sig_t* p);

#ifdef HMD_DEBUG
        public:
            void debugPrintSignature(void);
#endif

        protected:

            void            m_clear(void);
            int             m_getNestnum(void) { HMD_DEBUG_ASSERT(m_isNestnum(m_sigs[0])); return m_sig2nestnum(m_sigs[0]); }
            void            m_addNestnum(void) { HMD_DEBUG_ASSERT(m_isNestnum(m_sigs[0])); ++(m_sigs[0]); HMD_DEBUG_ASSERT(m_isNestnum(m_sigs[0])); }
            void            m_addSubSignature(const TArray<Sig_t>& subSig);
            static void     m_adjustDefaultValIdx(Sig_t*& p, int& idx);


        protected:
            hyu32             m_lastSigIdx;
            TArray<Sig_t>   m_sigs;
            Signature*      m_parent;

        };

    }
}

#endif /* m_HYCSIGNATURE_H_ */
