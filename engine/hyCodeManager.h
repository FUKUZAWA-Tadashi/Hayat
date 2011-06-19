/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCODEMANAGER_H_
#define m_HYCODEMANAGER_H_

#include "hySymbolID.h"
#include "hyBMap.h"
#include "hyValueArray.h"
#include "hyStringBox.h"

using namespace Hayat::Common;

namespace Hayat {
    namespace Engine {
        class Bytecode;
        class Thread;
        class Context;
        class HClass;

        class CodeManager {
            friend class GC;
        public:

            // �o�C�g�R�[�h���������Ƀ��[�h��������ɌĂԃR�[���o�b�N�̌^
            typedef void    (*LoadedCallback_t)(const char*);


            CodeManager(void);
            ~CodeManager();

            void    initialize(void);
            void    finalize(void);
            // �o�C�g�R�[�h�ǂݍ���
            Bytecode* readBytecode(const char* filename);
            // �o�C�g�R�[�h���������Ƀ��[�h��������ɌĂԃR�[���o�b�N��o�^
            void    setBytecodeLoadedCallback(LoadedCallback_t cb) { m_loadedCallback = cb; }
            // �o�C�g�R�[�h���Ǘ��ɒǉ�
            // bAutoload��true���ƁA�o�C�g�R�[�h�̌�n���͓����ł��
            void    addBytecode(Bytecode* pBytecode, bool bAutoload = false);
            Bytecode* getBytecode(SymbolID_t bytecodeSym);
            void    deleteBytecode(Bytecode* pBytecode);

            void    clearAllFlag(void);
            void    setFlag(SymbolID_t bytecodeSym);
            bool    getFlag(SymbolID_t bytecodeSym);

            // �o�C�g�R�[�h��������Γǂݍ���
            // �ǂݍ��܂ꂽ�o�C�g�R�[�h�́Afinalize()���ĂԂ܂ŊJ�����Ȃ��B
            // �o�C�g�R�[�h�̌�n���͓����ł��
            // todo: �o�C�g�R�[�h��GC�Ώۂɂ���
            Bytecode* require(SymbolID_t bytecodeSym, const hyu8* bytecodeName);

            // GC�ɑΉ��ł��� Context ���쐬�B
            // �����ō쐬����Context�́A���̃I�u�W�F�N�g����Q�Ƃ���Ă��Ȃ��Ă�
            // GC�ŉ������Ȃ��BreleaseContext()�ŁAGC�ɉ�������悤�ɂȂ�B
            // �t�ɁAreleaseContext()���Ă΂��ɕ��u����ƁAGC�ŉ�����ꂸ��
            // �����Ǝc�鎖�ɂȂ�̂Œ��ӁB
            Context* createContext(void);
            // createContext()�ō����Context���������
            void     releaseContext(Context*);
            // �����ō쐬����Context���AGC�ɑΉ��ł���悤�ɂ����œo�^����
            void     addContext(Context*);
            // addContext()�œo�^����Context���Ǘ�����O��
            void     removeContext(Context*);

            void     setWorkingBytecode(const Bytecode* pBytecode) { m_workingBytecode = pBytecode; }
            const Bytecode* getWorkingBytecode(void) { return m_workingBytecode; }


            // �����[�h�F�ȑO�̃o�C�g�R�[�h���㏑������`�Ń��[�h����
            Bytecode*   reloadBytecode(const char* filename, SymbolID_t asName = SYMBOL_ID_ERROR, bool bInit = true);
            // �ȑO�̃o�C�g�R�[�h���㏑������
            void    overrideBytecode(Bytecode* pBytecode, SymbolID_t asName = SYMBOL_ID_ERROR, bool bInit = true);
            // �����[�h���ɋ��N���X�ɂ����ĐV�N���X�ɖ����ϐ����c�����ǂ�����ݒ�
            void    setKeepObsoleteVar_whenReload(bool b) { m_ov_keepObsoleteVar = b; }
            // �����[�h���ɃN���X�ϐ��l�����N���X����V�N���X�ɃR�s�[���邩��ݒ�
            void    setCopyClassVar_whenReload(bool b) { m_ov_copyClassVar = b; }
            // �����[�h���ɒ萔�l�����N���X����V�N���X�ɃR�s�[���邩��ݒ�
            void    setCopyConstVar_whenReload(bool b) { m_ov_copyConstVar = b; }
            // �����[�h����u���ւ���ꂽ�o�C�g�R�[�h�ŁA�܂����������
            // �c���Ă�����̂̃V���{����������Array�ɓ����
            void    takeReplacedBytecodeSyms(ValueArray* arr);


            // �u��������ꂽ�o�C�g�R�[�h�̎g�p���t���O���N���A
            void    clearCodeUsingFlag(void);
            // codeAddr�̕����̃o�C�g�R�[�h���g�p���ł��鎖��o�^
            void    usingCodeAt(const hyu8* codeAddr);

            void    unmarkStringBox() { m_stringBox.unmark(); }
            // �����񂪒u��������ꂽ�o�C�g�R�[�h�̒��̃e�[�u�����w���Ă�����A
            // m_stringBox�ɃR�s�[���Ă�������w���悤�ɕύX
            // m_stringBox�̒����w���Ă����炻����}�[�N
            void    markString(const char** pStr);
            void    sweepStringBox() { m_stringBox.sweep(); }

            // �ǂ�������Q�Ƃ���Ă��Ȃ��o�C�g�R�[�h���폜
            void    deleteUnnecessaryBytecode(void);

        public:
            void     m_GC_mark(void);

            // GC��mark�t�F�[�Y�̃C���N�������^�����s
            void            startMarkIncremental(void);
            void            markIncremental(void);
            void            startMarkIncremental_2(void);
            void            markIncremental_2(void);
            void            markAllStack(void);


        protected:
            TArray<Context*>    m_contexts;

            LoadedCallback_t    m_loadedCallback;

            struct tbl_st {
                Bytecode*   bytecode;
                bool        flag;
                bool        autoloadFlag;
            };

            // �L���ȃo�C�g�R�[�h
            BMap<SymbolID_t, tbl_st>    m_tbl;

            tbl_st*         m_searchTbl(SymbolID_t bytecodeSym) { return m_tbl.find(bytecodeSym); }


            struct repl_st {
                Bytecode*   bytecode;
                bool        inUse; // �o�C�g�R�[�h���s���t���O
            };

            // �u��������ꂽ�o�C�g�R�[�h
            TArray<repl_st>             m_replaced;

            // �u��������ꂽ�o�C�g�R�[�h���ɂ����āA�܂��g���Ă���
            // ��������A�����Ɉړ�����
            StringBox       m_stringBox;

            // �����[�h���ɋ��N���X�ɂ����ĐV�N���X�ɖ����ϐ����c�����ǂ���
            bool            m_ov_keepObsoleteVar;
            // �����[�h���ɃN���X�ϐ��l�����N���X����V�N���X�ɃR�s�[���邩
            bool            m_ov_copyClassVar;
            // �����[�h���ɒ萔�l�����N���X����V�N���X�ɃR�s�[���邩
            bool            m_ov_copyConstVar;


            const Bytecode*     m_workingBytecode;
        };


        extern CodeManager      gCodeManager;

    }
}
#endif /* m_HYCODEMANAGER_H_ */
