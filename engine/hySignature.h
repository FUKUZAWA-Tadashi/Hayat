/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYSIGNATURE_H_
#define m_HYSIGNATURE_H_

#include "hySignatureBase.h"
#include "hyValue.h"
#include "hyArray.h"


namespace Hayat {
    namespace Engine {

        class Context;

        class Signature : public SignatureBase {

        public:
            static void* operator new(size_t);
            static void operator delete(void*);

            Signature(hyu8 numOnStack, const Sig_t* sigBytes, hyu32 sigBytesLen, Context* context);
            Signature(hyu8 numArgs, Context* context);
            ~Signature();

            // 右辺値その他が現状でいくつスタック上に積まれているか設定
            void setNumOnStack(hyu32 n);

            // この右辺Signatureが指定左辺Signatureに代入可能かどうか調べる
            // 必要なら中身を展開して調べる (Signatureは変化する)
            // 代入可能ならスタックを左辺Signatureに沿って整理して、trueを返す
            bool canSubstTo(const Sig_t* sigBytes, const HClass* defValClass, hyu16 defValOffs);

            // スタックにある右辺値を破棄
            void clearRight(void);

            // 現状でのスタック上の右辺値その他の個数
            int getNumOnStack(void);

            // 右辺値の位置がずれた場合に修正する
            void adjustStack(hys32 n) { m_sp += n; }

        protected:
            // canSubstToの本体ルーチン
            bool m_canSubst(const Sig_t*& left, Sig_t*& right, hyu32& sp, const HClass* defValClass, hyu16 defValOffs);
            // スタックを左辺Signatureに沿って整理する
            void m_adjustTo(const Sig_t* sigBytes, const HClass* defValClass, hyu16 defValOffs);
            // スタックをSignatureに沿って整理する
            void m_adjust(const Sig_t*& left, const Sig_t*& right, hyu32& sp, const HClass* defValClass, hyu16 defValOffs);
            // ネスト部分を1つにまとめる
            void m_bundle(const Sig_t*& right, hyu32& sp);
            // m_sigs上のポインタp,qの位置に書き込めるように、m_sigsが
            // 外部参照状態なら通常使用状態に移行移行し、p,qのアドレスを調整する
            void m_onwrite(Sig_t*& p, Sig_t*& q);
            // m_sigsの指定位置にn個分の空間を挿入
            // メモリ移動が起きるとpの値は変化する。同時にrnpも変化させる。
            void m_insert(Sig_t*& p, hyu32 n, Sig_t*& rnp);
            // スタックのsp位置の値を展開
            int m_expand(hyu32 sp);
            // ネスティング終了までスキップ
            void m_skip(Sig_t*& right, hyu32& sp);
            // 値が展開可能かどうか
            bool m_isExpandable(Value& v);

#ifdef HMD_DEBUG
        public:
            void printSigs(void);
            static void printSig(const Sig_t*& p);
#endif

        protected:
            TArray<Sig_t>   m_sigs;
            Context*        m_context;
            hyu32             m_sp; // m_context.stack上の多重代入右辺の先頭位置

        };
    }
}

#endif /* m_HYSIGNATURE_H_ */
