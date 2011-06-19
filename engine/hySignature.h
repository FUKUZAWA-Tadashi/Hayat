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

            // �E�Ӓl���̑�������ł����X�^�b�N��ɐς܂�Ă��邩�ݒ�
            void setNumOnStack(hyu32 n);

            // ���̉E��Signature���w�荶��Signature�ɑ���\���ǂ������ׂ�
            // �K�v�Ȃ璆�g��W�J���Ē��ׂ� (Signature�͕ω�����)
            // ����\�Ȃ�X�^�b�N������Signature�ɉ����Đ������āAtrue��Ԃ�
            bool canSubstTo(const Sig_t* sigBytes, const HClass* defValClass, hyu16 defValOffs);

            // �X�^�b�N�ɂ���E�Ӓl��j��
            void clearRight(void);

            // ����ł̃X�^�b�N��̉E�Ӓl���̑��̌�
            int getNumOnStack(void);

            // �E�Ӓl�̈ʒu�����ꂽ�ꍇ�ɏC������
            void adjustStack(hys32 n) { m_sp += n; }

        protected:
            // canSubstTo�̖{�̃��[�`��
            bool m_canSubst(const Sig_t*& left, Sig_t*& right, hyu32& sp, const HClass* defValClass, hyu16 defValOffs);
            // �X�^�b�N������Signature�ɉ����Đ�������
            void m_adjustTo(const Sig_t* sigBytes, const HClass* defValClass, hyu16 defValOffs);
            // �X�^�b�N��Signature�ɉ����Đ�������
            void m_adjust(const Sig_t*& left, const Sig_t*& right, hyu32& sp, const HClass* defValClass, hyu16 defValOffs);
            // �l�X�g������1�ɂ܂Ƃ߂�
            void m_bundle(const Sig_t*& right, hyu32& sp);
            // m_sigs��̃|�C���^p,q�̈ʒu�ɏ������߂�悤�ɁAm_sigs��
            // �O���Q�Ə�ԂȂ�ʏ�g�p��ԂɈڍs�ڍs���Ap,q�̃A�h���X�𒲐�����
            void m_onwrite(Sig_t*& p, Sig_t*& q);
            // m_sigs�̎w��ʒu��n���̋�Ԃ�}��
            // �������ړ����N�����p�̒l�͕ω�����B������rnp���ω�������B
            void m_insert(Sig_t*& p, hyu32 n, Sig_t*& rnp);
            // �X�^�b�N��sp�ʒu�̒l��W�J
            int m_expand(hyu32 sp);
            // �l�X�e�B���O�I���܂ŃX�L�b�v
            void m_skip(Sig_t*& right, hyu32& sp);
            // �l���W�J�\���ǂ���
            bool m_isExpandable(Value& v);

#ifdef HMD_DEBUG
        public:
            void printSigs(void);
            static void printSig(const Sig_t*& p);
#endif

        protected:
            TArray<Sig_t>   m_sigs;
            Context*        m_context;
            hyu32             m_sp; // m_context.stack��̑��d����E�ӂ̐擪�ʒu

        };
    }
}

#endif /* m_HYSIGNATURE_H_ */
