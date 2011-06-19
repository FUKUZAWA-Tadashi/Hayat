/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCLASS_H_
#define m_HYCLASS_H_

#include "hySymbolID.h"
#include "hyGC.h"
#include "hyBMap.h"
#include "hyTuning.h"
#include "hySignature.h"


// FFI�e�[�u���Q�Ɛ錾
#define EXTERN_BYTECODE_FFI(codename)             \
  namespace Hayat { namespace Ffi { namespace HScb_##codename {    \
    extern const Hayat::Engine::HClass::ClassFfiEntry_t CLASS_FFI_TABLE; \
  } } }

// FFI�e�[�u�������N
#define LINK_BYTECODE_FFI(codename)               \
  Hayat::Engine::HClass::linkRootFfiTable(&Hayat::Ffi::HScb_##codename::CLASS_FFI_TABLE)



/*
  �N���X�̃o�C�g�R�[�h�f�[�^�\���ɂ��ẮAdoc/dataformat/bytecode.txt ���Q�ƁB
*/

class Test_hyGC;

namespace Hayat {
    namespace Common {
        class MemCell;
    }

    namespace Engine {
        class Object;
        class Context;
        class Value;
        class Bytecode;
        class MethodPtr;


        /// FFI�֐��^
        typedef void (*Ffi_t)(Context* context, int numArgs);


        class HClass {
            friend class Bytecode;
            friend class Object;
            friend class ::Test_hyGC;
        
        public:
            // ������
            // @param classSymbol   ���̃N���X�������V���{��
            // @param pBytecode     ��������o�C�g�R�[�h
            // @param code          �N���X�̃o�C�g�R�[�h�̐擪�A�h���X
            HClass(SymbolID_t classSymbol, const Bytecode* pBytecode, const hyu8* code);
            // �^���N���X�̏�����
            // �ϐ��⃁�\�b�h�Ȃǂ�1�������N���X���쐬
            HClass(SymbolID_t symbol);
            ~HClass(void);
            static void*    operator new(size_t size);
            static void     operator delete(void* p);

            const hyu8*       classInitializer(void) const {return m_codes;}

            const Bytecode* bytecode(void) const { return m_pBytecode; }
            // self��inner����N���X������
            const HClass*   getInnerClass(SymbolID_t classSymbol) const;
            // self,inner,super,super->innet����N���X������
            const HClass*   getNearClass(SymbolID_t classSymbol) const;
            // using->near����N���X������
            const HClass*   getClassInUsing(SymbolID_t classSymbol) const;
            // ���N���X���猩����N���X������
            const HClass*   getScopeClass(SymbolID_t classSymbol, bool bNotSearchUsingPath = false) const;
            size_t          fieldSize(void) const; //c++�C���X�^���X�������t�B�[���h�T�C�Y
            // �C���X�^���X�I�u�W�F�N�g����
            // [argn, .., arg1] -> []
            // context���w�肵�Ă���΁A�N���X���\�b�h *cppSize �ƃC���X�^���X
            // ���\�b�h initialize ���ĂԁB
            // initialize���ŗ�O����������� NULL ��Ԃ��B
            // *cppSize �������ꍇ�� context = NULL �ŌĂяo�������ł���B
            // context == NULL �܂��� bInit == false �̏ꍇinitialize�͌Ă΂�Ȃ��B
            Object*         createInstance(Context* context = NULL, Signature* pSig = NULL, bool bInit = true) const;
            // �C���X�^���X�I�u�W�F�N�g����
            // *cppSize �� initialize ���Ă΂Ȃ��B
            // C++�C���X�^���X�����̃T�C�Y���w�肷��B
            Object*         createInstance_noInit(size_t cppFieldSize) const;

            bool            isPrimitive(void) const { return m_bPrimitive; }

            bool            searchMethod(MethodPtr* pMethodPtr, SymbolID_t methodSymbol, Signature* pSig) const;
            void            setClosureMethodPtr(int num, MethodPtr* pmptr) const;
            bool            haveMethod(SymbolID_t methodSymbol) const;

            Value*          getClassVarAddr(SymbolID_t classVarSymbol) const;
            Value*          getConstVarAddr(SymbolID_t constVarSymbol) const;
            Value*          getMembVarAddr(Object* obj, SymbolID_t varSymbol) const;
            Value*          getDefaultValAddr(hyu16 defValIdx) const;
            void            setDefaultVal(hyu16 defValIdx, const Value& val);
            SymbolID_t      getSymbol(void) const { return m_classSymbol; }
            bool            symCheck(SymbolID_t sym) const { return m_classSymbol == sym; }
            void            changeSymbol(SymbolID_t sym);

            bool            isInitialized(void) const { return m_bInitialized; }
            void            initialized(void) { m_bInitialized = true; }

            const hyu8*       name(void) const;

            void            getMethodSignatureBytes(int methodNum, const Signature::Sig_t** pSigBytes, hyu16* pDefValOffs) const;

            bool            doesInherit(const HClass* other) const;


            // �S�N���X�𑖍�����1�x�������������s���邽�߂̃t���O����
            void            recSetNodeFlag(hyu8 val = 0);
            hyu8            getNodeFlag(void) { return m_nodeFlag; }
            void            setNodeFlag(hyu8 val) { m_nodeFlag = val; }


        protected:

            static void     m_rewriteJumpTableEndian(const hyu8* table);
            void            m_resolveTable(const HClass* outer);
            void            m_initHashes(void);
            const HClass*   m_super(int n) const { return (const HClass*)m_superTable[n]; }

            bool            m_lookupRoutine(MethodPtr* pMethodPtr, SymbolID_t methodSymbol, Signature* pSig) const;
            bool            m_lookupFfi(MethodPtr* pMethodPtr, SymbolID_t methodSymbol, Signature* pSig) const;
            bool            m_lookupMethod(MethodPtr* pMethodPtr, SymbolID_t methodSymbol, Signature* pSig) const;


            const Bytecode* m_pBytecode;
            const HClass*   m_outerClass;
            SymbolID_t      m_classSymbol;
            bool            m_bPrimitive;
            hyu16             m_numMethods;
            hyu16             m_numInnerClasses;
            hyu16             m_numSuper;
            hyu16             m_numDefaultVal;
            hyu16             m_numClosures;
            SymbolID_t*     m_methodSymTable;
            SymbolID_t*     m_classSymTable;
            TArray<SymbolID_t>  m_classVarSymArr;
            TArray<SymbolID_t>  m_membVarSymArr;
            TArray<SymbolID_t>  m_constVarSymArr;
            hyu32*            m_methodTable; // �o�C�g�R�[�h�ǂݍ��ݎ��ɂ̓o�C�g�R�[�h�I�t�Z�b�g�A�ǂݍ��񂾎��Ƀo�C�g�R�[�h�A�h���X�ɕϊ�
            hyu32*            m_innerClassTable; // �o�C�g�R�[�h�ǂݍ��ݎ��ɂ̓o�C�g�R�[�h�I�t�Z�b�g�A�ǂݍ��񂾎���HClass*�ɕϊ�
            hyu32*            m_superTable; // �o�C�g�R�[�h�ǂݍ��ݎ��ɂ̓N���X�����NID�A�ǂݍ��񂾎���HClass*�ɕϊ�
            hyu16             m_numUsingPaths;
            HClass**        m_usingPaths;
            Value*          m_varPool;       // �N���X�ϐ��A�萔�̎��̒u���ꏊ
            const hyu8*       m_codes;
            BMap<SymbolID_t, Value*>    m_classVarHash;
            BMap<SymbolID_t, Value*>    m_constVarHash;
            bool            m_bInitialized;
            hyu8            m_nodeFlag;

#ifdef HMD_DEBUG
            hys32             m_defaultValSetLimit;
#endif

            //============================================================
            // FFI (Foreign Function Interface)
            //============================================================
        public:
            /// FFI���\�b�h�e�[�u��
            struct FfiMethodEntry_t {
                SymbolID_t      methodSymbol;
                hyu16             signatureID;
                hyu16             defaultValueOffset;
                Ffi_t           func;
            };

            /// �N���X��FFI�e�[�u��
            struct ClassFfiEntry_t {
                SymbolID_t              classSymbol;
                hyu16                     numFfis;
                hyu16                     numInnerClasses;
                // methodSymbol�Ń\�[�g�ς݂�ffi�e�[�u��
                const FfiMethodEntry_t*	ffiMethodTable;
                // classSymbol�Ń\�[�g�ς݂�innerclass�pFFI�e�[�u��
                const ClassFfiEntry_t* const*	innerClassFfiTable;
                GC::MarkFunc_t		GCMarkFunc;
            };




        protected:
            const ClassFfiEntry_t* m_ffiTable;
            void bindFfiTable(const ClassFfiEntry_t* tbl);
            Ffi_t m_searchFfi(SymbolID_t methodSymbol, Signature* pSig) const;


        protected:
            static TArray<const ClassFfiEntry_t*> m_rootFfiTable;
        public:
            static void initializeRootFfiTable(void);
            static void finalizeRootFfiTable(void);
            static void linkRootFfiTable(const ClassFfiEntry_t* tbl);
            static const ClassFfiEntry_t* findFfiTable(SymbolID_t bytecodeSymbol);

            GC::MarkFunc_t getGCMarkFunc(void) const;



            //============================================================
            // �o�C�g�R�[�h�����[�h
            //============================================================
#ifdef HY_ENABLE_BYTECODE_RELOAD
        public:
            void        keepObsoleteVar(const HClass* oldClass);
            void        copyDataTo(HClass* dest, BMap<const HClass*,const HClass*>* pGenMap, bool bCopyClassVar, bool bCopyConstVar);

            // �X�[�p�[�N���X�������[�h����Ă����璲������
            void        regulateReloadedSuper(BMap<const HClass*,const HClass*>& genMap);
#endif


            //============================================================
            // GC
            //============================================================
        public:
            // obj�̃����o�ϐ����}�[�N
            void m_GC_mark_membVar(Object* obj) const;
            // �N���X�ϐ��A�萔���}�[�N
            void m_GC_mark_staticVar(void);

#ifdef HMD_DEBUG
        public:
            void debugPrintAbsName(void) const;
#endif

        };

    }
}

#endif /* m_HYCLASS_H_ */
