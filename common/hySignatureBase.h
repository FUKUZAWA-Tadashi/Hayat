/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYSIGNATUREBASE_H_
#define m_HYSIGNATUREBASE_H_


#include "machdep.h"

namespace Hayat {
    namespace Common {

        // シグネチャ処理の共用部分
        class SignatureBase {
        public:
            // 区切り: 0xff: m_isSep()がtrue
            // ネスティング: 0x80～0xbf: m_isNestnum()がtrue
            // マルチ: 0xf0,0xf1: m_isMulti()がtrue
            // デフォルト値付きマルチ: 0xc0～0xef: m_isMulti()とm_isMultiDefaultVal()がtrue
            // デフォルト値: 0x40～0x6f: m_isDefaultVal()がtrue
            // ノーマル: 0～0x3f: m_isNormal()がtrue
            typedef hyu8  Sig_t;

            static const Sig_t  MULTI = 0xf0;   // *
            static const Sig_t  MULTI2 = 0xf1;  // ffiの可変長引数
            static const Sig_t  DEFVAL = 0x40;   // デフォルト値
            static const Sig_t  DEFVAL_MULTI = 0xc0; // デフォルト値付き *
            static const int    MAXNUM_DEFAULT_VAL = 0x2f;
            static const Sig_t  NESTNUM_1 = 0x81; // == m_nestnum2sig(1)
            static const Sig_t  NESTNUM_2 = 0x82; // == m_nestnum2sig(2)
            static const Sig_t  NESTNUM_3 = 0x83; // == m_nestnum2sig(3)
            static const Sig_t  SEP = 0xff;

            // 事前登録シグネチャID
            static const hyu16 ID_0 = 0;          // (0)
            static const hyu16 ID_1 = 1;          // (1)
            static const hyu16 ID_2 = 2;          // (2)
            static const hyu16 ID_3 = 3;          // (3)
            static const hyu16 ID_4 = 4;          // (4)
            static const hyu16 ID_5 = 5;          // (5)
            static const hyu16 ID_6 = 6;          // (6)
            static const hyu16 ID_7 = 7;          // (7)
            static const hyu16 ID_8 = 8;          // (8)
            static const hyu16 ID_MULTI = 9;      // (*)
            

            static const size_t NUM_PREDEFINED_PARAMS = 19;
            static const Sig_t PREDEFINED_PARAMS[NUM_PREDEFINED_PARAMS][3];


        protected:
            static bool     m_isNormal(Sig_t sig) { return sig <= 0x3f; }
            static bool     m_isSep(Sig_t sig) { return sig == SEP; }
            static bool     m_isNestnum(Sig_t sig) { return (sig >= 0x80) && (sig <= 0xbf); }
            static bool     m_isMulti(Sig_t sig) { return (sig >= 0xc0) && (sig <= 0xf1); }
            static bool     m_isDefaultVal(Sig_t sig) { return (sig >= 0x40) && (sig <= 0x6f); }
            static bool     m_isMultiDefaultVal(Sig_t sig) { return (sig >= 0xc0) && (sig <= 0xef); }
            static int      m_sig2nestnum(Sig_t sig) { return sig - 0x80; }
            static Sig_t    m_nestnum2sig(int nestnum) {
                HMD_DEBUG_ASSERT((nestnum >= 0) && (nestnum <= 0x3f));
                return nestnum + 0x80;
            }
            static int      m_sig2DefaultValIdx(Sig_t sig) {
                if (m_isDefaultVal(sig)) return sig - 0x40;
                if (m_isMultiDefaultVal(sig)) return sig - 0xc0;
                HMD_FATAL_ERROR("invalid signature");
                return -1;
            }

            // ネスティングの中身を丸ごとスキップ
            static void     m_skip(const Sig_t*& p);
            // 引数の個数
            static int      m_arity(const Sig_t*& p);
            // デフォルト値インデックスの調整
            static void     m_adjustDefaultValIdx(Sig_t*& p, int& idx);


        public:
            static Sig_t    defaultValIdx2sig(int idx) { HMD_DEBUG_ASSERT((idx>=0)&&(idx<=0x2f)); return (Sig_t)(idx + 0x40); }
            static Sig_t    defaultValIdx2multisig(int idx) { HMD_DEBUG_ASSERT((idx>=0)&&(idx<=0x2f)); return (Sig_t)(idx + 0xc0); }
            static void     snAddPrintSig(char* buf, size_t bufSize, const Sig_t*& p);
            static void     debugPrintSignature(const Sig_t* p);
            static void     printPredefinedSignatures();

        };

    }
}

#endif /* m_HYSIGNATUREBASE_H_ */
