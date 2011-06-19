/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCCONTEXT_H_
#define m_HYCCONTEXT_H_

#include "hyStack.h"
#include "hyArray.h"
#include "hyCSymbolTable.h"
#include "hyCBytecode.h"
#include "hyCPackage.h"
#include "hyCJumpControlTree.h"
#include "hyCSignature.h"
#include "hyC_opcode.h"
#include <stdio.h>

class Test_hyCContext;

namespace Hayat {
    namespace Compiler {

        class JumpControlTree;

        class Context {
            friend class ::Test_hyCContext;

        public:
            static void* operator new(size_t size);
            static void operator delete(void* p);

            // �S�̂̏�����
            static void initializeAll(void);
            // �S�̂̌�n��
            static void finalizeAll(void);

            // �J�����g�R���e�L�X�g��Ԃ�
            static Context* current(void) { return m_current; }

            // �p�b�P�[�W�N���X�R���e�L�X�g������čŏ��̃R���e�L�X�g�Ƃ���
            static void newPackageClass(ClassInfo*);

            // �R���e�L�X�g���X�^�b�N�ɐς݁A�J�����g�Ƃ���
            static void push(Context* child);
            // �X�^�b�N����J�����g���~�낵��1�O�̂��J�����g�Ƃ���
            static void pop(void);
            // �J�����g�̎q�R���e�L�X�g������ăX�^�b�N�ɐς݁A�V���ȃJ�����g�Ƃ���
            static Context* createChild(void);
            // �J�����g�̃C���i�[�N���X�R���e�L�X�g������ăX�^�b�N�ɐς݁A�J�����g�Ƃ���
            static Context* createInnerClass(SymbolID_t classSym);
            // ���p�b�P�[�W���擾
            static Package* getPackage(void) { return m_package; }

            // sayCommandStart()�ɓn���V���A���i���o�[
            static hys32 sayCommandIndexCount(void) { return m_sayCommandIndex++; }
            // sayCommandStart()�ɓn���V���A���i���o�[�F�J�E���g�A�b�v���Ȃ�
            static hys32 sayCommandIndex(void) { return m_sayCommandIndex; }

            // goto �̔�ѐ惉�x�����SContext���ɂ��邩�ǂ����`�F�b�N
            static void jumpLabelCheck(void);

        protected:
            static Stack<Context*> m_contextStack;
            static Context* m_current; // �J�����g�R���e�L�X�g
            static TArray<Context*> m_pool; // �SContext���L��
            static Package* m_package;
            static hys32 m_sayCommandIndex;

        public:
            Context(Context* outerContext = NULL);
            ~Context();

            ClassInfo*  classInfo(void) { return m_classInfo; }
            Context*    outerContext(void) { return m_outerContext; }

            // class�J�n
            void newClass(SymbolID_t classSym);
            // �X�[�p�[�N���X�ǉ�
            void addSuper(SymbolID_t superClassSym, Context* searchContext);

            // �ŏI����
            void postCompileProcess(hyu16 defaultValOffs, hyu16 signatureID);

            // �X�R�[�v�ɑΉ�����ClassInfo������
            ClassInfo* getScopeClassInfo(Scope_t scope, bool ignoreUsing = false);

            typedef struct {
                hys16     idx;            // �C���f�b�N�X
                hys16     outerIdx;       // �O��context�Ƌ��L�̏ꍇ
                bool    substFlag;      // ����t���O
            } LocalVar_t;
            static const hys16 NO_LOCAL_VAR = (hys16)0x8000; // ���[�J���ϐ������݂��Ȃ��Ƃ����Ӗ���idx

            // ====== ���ʎq�̌��� ======
            // ���̃X�R�[�v�ƊO�̃X�R�[�v�Ƀ��[�J���ϐ������邩
            LocalVar_t* getMyLocalVar(const char* p, hyu32 len, LocalVarSymID_t* pSym = NULL);
            // ���̃X�R�[�v�Ɛe�N���X�Ɏ��ʎq�����邩
            Var_t       myIdent(const char* p, hyu32 len);
            Var_t       myIdent(InputBuffer* inp, Substr_st& ss) {
                return myIdent(inp->addr(ss.startPos),ss.len()); }

            Var_t       searchVar(const char* p, hyu32 len);
            Var_t       searchVar(InputBuffer* inp, Substr_st& ss) {
                return searchVar(inp->addr(ss.startPos),ss.len()); }

            // ���[�J���ϐ������݂���΂����Ԃ�(�O���R���e�L�X�g�͖���)
            LocalVar_t* getLocalVar(LocalVarSymID_t varSym) { return m_localVars.find(varSym); }
            // �O���R���e�L�X�g�̉����[�J���ϐ���Ԃ�
            LocalVar_t* getOuterLocalVar(SymbolID_t varSym);
            // �����[�J���ϐ���������΍쐬
            LocalVar_t& getLocalVarCreate(LocalVarSymID_t varSym);

            // ���\�b�h���ŏ��Ɋm�ۂ��ׂ����[�J���ϐ��̐�
            int numLocalVarAlloc(void) { return m_localVars.size() - m_numParamVar; }
            // ���\�b�h�������쐬
            void createParamVar(LocalVarSymID_t varSym);

            // �V�O�l�`���o�C�g���ID�ɕϊ�
            hyu16 getSignatureID(const hyu8* sigBytes, hyu32 len) {
                return m_package->getSignatureID(sigBytes, len); }

            // ����̖������[�J���ϐ����`�F�b�N
            void checkNoSubstLocalVar(void);

            // �ێ����Ă���o�C�g�R�[�h
            Bytecode& bytecode(void) { return m_bytecode; }

            // ���p�b�P�[�W�����N
            void require(SymbolID_t sym) { m_package->require(sym); }

            template <typename OPCODE> void addCode(void) {
                OPCODE::addCodeTo(m_bytecode);
            }
            template <typename OPCODE, typename OPERAND1> void addCode(OPERAND1 opr1) {
                OPCODE::addCodeTo(m_bytecode, opr1);
            }
            template <typename OPCODE, typename OPERAND1, typename OPERAND2> void addCode(OPERAND1 opr1, OPERAND2 opr2) {
                OPCODE::addCodeTo(m_bytecode, opr1, opr2);
            }
            template <typename OPCODE, typename OPERAND1, typename OPERAND2, typename OPERAND3> void addCode(OPERAND1 opr1, OPERAND2 opr2, OPERAND3 opr3) {
                OPCODE::addCodeTo(m_bytecode, opr1, opr2, opr3);
            }
            void addCodePushInt(hys32 v);
            void addCodePushFloat(hyf32 v);
            bool removeLastPop(void) { return m_bytecode.removeLastCode(OPL_pop); }

            void setClosureContext(void) { m_bClosureContext = true; m_canSeeOuterLocal = true; }
            bool isClosureContext(void) { return m_bClosureContext; }

            // ���\�b�h�ǉ�
            void addMethod(SymbolID_t methodSym, Context* methodContext);
            // �C���i�[�N���X�o�^
            void addInnerClass(Context* classContext);
            // �N���[�W���o�^
            void addClosure(Context* callContext, Context* closureContext);

            // ���݂̃o�C�g�R�[�h�o�^�ʒu
            hyu32 codeAddr(void) { return m_bytecode.getSize(); }


            // ���x���o�^
            void addLabel(SymbolID_t labelSym) {
                m_jumpControlInfo->addLabel(labelSym, codeAddr());
            }
            // jump control�ɂ�����W�����v�R�[�h�ǉ�
            template <typename OP_JMP> hyu32 addGotoCode(SymbolID_t labelSym = SYMBOL_ID_ERROR) {
                hyu32 pos = codeAddr() + 1;
                if (labelSym == SYMBOL_ID_ERROR) {
                    addCode<OP_JMP>(-2);
                } else {
                    hyu32 adr = m_jumpControlInfo->getLocalLabelAddr(labelSym);
                    if (adr == JumpControlTree::INVALID_ADDR) {
                        // ���������x��
                        addCode<OP_JMP>(-3);
                        m_jumpControlInfo->addResolveAddr(labelSym, pos);
                    } else {
                        addCode<OP_JMP>(adr - pos - OPR_RELATIVE::SIZE);
                    }
                }
                return pos;
            }
            // jump control �J�n
            void jumpControlStart(SymbolID_t catchVar = SYMBOL_ID_ERROR, SymbolID_t finallyValVar = SYMBOL_ID_ERROR);
            // jump control �I��
            void jumpControlEnd(void);
            // jump control�̃A�h���X����
            void resolveJumpControl(void);
            // �W�����v�A�h���X���� jumpAddr���ȗ�����ƌ��݈ʒu
            void resolveJumpAddr(hyu32 resolveAddr, hyu32 jumpAddr = JumpControlTree::INVALID_ADDR);
            // jump���߂�jumpControl���߂ɒu��������
            void replaceJumpControl(hyu32 resolveAddr, SymbolID_t label);
            // finally�Ŏg�p����ꎞ�I�ȕϐ���p��
            LocalVarSymID_t     useFinallyValVar(void);
            // finally�ł̈ꎞ�ϐ��g�p�I��
            void        endUseFinallyValVar(void);

            // �W�����v���x�����o�^����Ă��邩
            bool haveLabel(SymbolID_t label);
            // resolve��ɉ�������Ȃ������W�����v���x��
            const TArray<SymbolID_t>& getUnresolvedJumpControlLabels(void) { return m_unresolvedJumpControlLabels; }



            // �\�[�X�R�[�h�ʒu�����Z�b�g
            void setSourceInfo(hyu32 parsePos);


            // �o�C�g�R�[�h����Array�ɏo��
            void writeByteCodes(TArray<hyu8>* out);
            // �o�C�g�R�[�h�����t�@�C���o��
            void fwriteByteCodes(FILE* fp);
            // �p�b�P�[�W�����N�����t�@�C���o��
            void fwriteLinks(FILE* fp);
            // �f�o�b�O�����t�@�C���o��
            void fwriteDebugInfo(FILE* fp, hyu32 offs);
            // FFI�錾����C++�\�[�X���쐬
            void writeFfi(void);
            // FFI�ďo���p�֐��e�[�u����C++�\�[�X���o��
            void writeFfiTbl(const char* ffiTblFileName);


        protected:
            // �����̃��x���ȉ��̃f�o�b�O�����t�@�C���o��
            void m_fwriteDebugInfo(FILE* fp, TArray<const char*>& paths, hyu32 offset);

        protected:
            Context*    m_outerContext;
            ClassInfo*  m_classInfo;
            BMap<LocalVarSymID_t, LocalVar_t>        m_localVars;
            hyu8        m_numParamVar;
            hyu8        m_finallyNestLevel;
            bool        m_canSeeOuterLocal;
            bool        m_bClosureContext;
            Bytecode    m_bytecode;
            TArray<JumpControlTree*>    m_jumpControlTbl;
            JumpControlTree*            m_jumpControlInfo;
            // resolve���jumpControl���߂ƂȂ������x��
            TArray<SymbolID_t>          m_unresolvedJumpControlLabels;
            BMap<SymbolID_t, Context*>  m_innerClasses;


        public:
            // SyntaxTree::compile()�̌��ʁA�X�^�b�N�ɒl��push����R�[�h��ǉ�������
            bool        bPushCode;
            // bPushCode��false�Ȃ�Apush_nil����
            void        needLastVal(void) { if (!bPushCode) { addCode<OP_push_nil>(); bPushCode = true; } }
            // bPushCode��true�Ȃ�Apop����
            void        popLastVal(void) { if (bPushCode) { addCode<OP_pop>(); bPushCode = false; } }

            // leftValue���R���p�C�����鎞�Afalse�Ȃ�l���o���Atrue�Ȃ���
            bool        bCompileSetVal;

            // SyntaxTree::compile()�������̒l���ꎞ�I�ɋL�������邽�߂̕ϐ�
            volatile union {
                hys32             tmp_int;
                hyu32             tmp_hyu32;
                hyf32             tmp_float;
                SymbolID_t      tmp_symbol;
                void*           tmp_ptr;
                ArgD_t          tmp_argD;
                FfiSigDecl_t tmp_ffiSigDecl;
                Signature*      tmp_signature;
            };
        };

    }
}

#endif /* m_HYCCONTEXT_H_ */
