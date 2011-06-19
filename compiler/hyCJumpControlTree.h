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
            // ���[�J���ȃ��x���A�h���X���擾
            hyu32         getLocalLabelAddr(SymbolID_t label);
            // �e(�O��)���ΏۂɊ܂߂ă��x���A�h���X���擾
            hyu32         getLabelAddr(SymbolID_t label);
            // ���[�J���ȃ��x���̃A�h���X��ݒ�
            void        addLabel(SymbolID_t label, hyu32 addr);
            // ���x�����A�h���X�ɉ������ׂ��ꏊ���L��������
            void        addResolveAddr(SymbolID_t label, hyu32 resolveAddr);
            // �q���̕����ċA�I�Ƀ��x�����A�h���X�ɉ���
            void        resolve(Context* context);
            // �o�C�g�R�[�h��𐶐�
            TArray<hyu8>* genCode(void);

            // ���x�����o�^����Ă��邩�ǂ����A�q���̕����܂߂Č���
            bool        haveLabel(SymbolID_t label);

        protected:
            JumpControlTree*    m_parent;
            BMap<SymbolID_t, hyu32>       m_labelAddrs;    // label => addr
            BMap<SymbolID_t, TArray<hyu32> >       m_res;   // label => [resolveAddr]
            TArray<JumpControlTree*>    m_childs;
        public:
            hyu32         startAddr;

        protected:
            // ���[�J���ȃ��x�����A�h���X�ɉ���
            void        m_resolveLocal(Context* context);
        };

    }
}

#endif /* m_HYCJUMPCONTROLTREE_H_ */
