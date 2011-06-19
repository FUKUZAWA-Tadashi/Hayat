/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCJUMPCONTROLTREE_H_
#define m_HYCJUMPCONTROLTREE_H_

#include "machdep.h"
#include "hySymbolID.h"
#include "hyArray.h"
#include "hyBMap.h"


using namespace Hayat::Common;


namespace Hayat {
    namespace Compiler {

        class Context;

        class JumpControlTree {

        public:
            static const hyu32 INVALID_ADDR;// = (hyu32)-1;

            static void* operator new(size_t size);
            static void operator delete(void* p);

            JumpControlTree(JumpControlTree* parent);
            ~JumpControlTree();
            
            JumpControlTree*    parent(void) { return m_parent; }
            
            JumpControlTree*    newChild(void);
            // ローカルなラベルアドレスを取得
            hyu32         getLocalLabelAddr(SymbolID_t label);
            // 親(外側)も対象に含めてラベルアドレスを取得
            hyu32         getLabelAddr(SymbolID_t label);
            // ローカルなラベルのアドレスを設定
            void        addLabel(SymbolID_t label, hyu32 addr);
            // ラベルをアドレスに解決すべき場所を記憶させる
            void        addResolveAddr(SymbolID_t label, hyu32 resolveAddr);
            // 子孫の分も再帰的にラベルをアドレスに解決
            void        resolve(Context* context);
            // バイトコード列を生成
            TArray<hyu8>* genCode(void);

            // ラベルが登録されているかどうか、子孫の分も含めて検索
            bool        haveLabel(SymbolID_t label);

        protected:
            JumpControlTree*    m_parent;
            BMap<SymbolID_t, hyu32>       m_labelAddrs;    // label => addr
            BMap<SymbolID_t, TArray<hyu32> >       m_res;   // label => [resolveAddr]
            TArray<JumpControlTree*>    m_childs;
        public:
            hyu32         startAddr;

        protected:
            // ローカルなラベルをアドレスに解決
            void        m_resolveLocal(Context* context);
        };

    }
}

#endif /* m_HYCJUMPCONTROLTREE_H_ */
