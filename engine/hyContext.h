/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCONTEXT_H_
#define m_HYCONTEXT_H_

#include "hyCodePtr.h"
#include "hyStack.h"
#include "hyCellList.h"
#include "hyThread.h"
#include "hyObject.h"

namespace Hayat {
    namespace Engine {

        extern bool isJumpControlLabel(SymbolID_t label);

        class Fiber;
        class Exception;

        class Context {
            friend class VM;

        public:
            static void* operator new(size_t size);
            static void operator delete(void* ptr);
            static Context* create(void);
            void    destroy(void);

            Context(void);
            ~Context();

            Context* initialize(hyu32 stackSize = 0, hyu32 frameStackSize = 0);
            void    finalize(void);
            void    cleanup(void);  // �X�^�b�N���Z�b�g
            Object* getObj(void) { return Object::fromCppObj(this); }

            hyu8      getCodeHYU8(void);
            hys8      getCodeHYS8(void);
            hyu16     getCodeHYU16(void);
            hys16     getCodeHYS16(void);
            hyu32     getCodeHYU32(void);
            hys32     getCodeHYS32(void);
            hyf32     getCodeHYF32(void);
            SymbolID_t      getCodeSymbolID(void);

            inline void push(Value d) { stack.push(d); }
            inline Value pop(void) { return stack.pop(); }
            inline void pushBool(bool b) { stack.push(Value::fromBool(b)); }
            inline void pushNil(void) { stack.push(NIL_VALUE); }
            void pushInt(hys32 i) { stack.push(Value::fromInt(i)); }
            void pushFloat(hyf32 f) { stack.push(Value::fromFloat(f)); }
            void pushSymbol(SymbolID_t sym) { stack.push(Value::fromSymbol(sym)); }
            void pushString(const char* s) { stack.push(Value::fromString(s)); }
            void pushObj(Object* o) { stack.push(Value::fromObj(o)); }
            void pushClass(const HClass* pClass) {stack.push(Value::fromClass(pClass));}
            void pushList(ValueList* list) {stack.push(Value::fromList(list));}
            bool popBool(void) { return stack.pop().toBool(); }
            hys32  popInt(void) { return stack.pop().toInt(); }
            hyf32  popFloat(void) { return stack.pop().toFloat(); }
            SymbolID_t popSymbol(void) { return stack.pop().toSymbol(); }
            const char* popString(void) { return stack.pop().toString(); }
            Object* popObj(void) { return stack.pop().toObj(); }
#ifdef HMD_DEBUG
            Object* popObj(SymbolID_t sym) {
                Object* obj = popObj();
                HMD_ASSERT(obj->type()->symCheck(sym));
                return obj;
            }
#else
            Object* popObj(SymbolID_t) { return popObj(); }
#endif
            template<typename T> T* popCppObj() {
                Object* obj = popObj(); return obj->cppObj<T>(); }
            template<typename T> T* popCppObj(SymbolID_t sym) {
                Object* obj = popObj(sym); return obj->cppObj<T>(); }

            const HClass* popClass(void) { return stack.pop().toClass(); }
            ValueList* popList(void) { return stack.pop().toList(); }



            const HClass* lexicalScope(void) { return curFrame->lexicalScope; }
            const HClass* ownerClass(void) { return curFrame->ownerClass; }


            // ���\�b�h�Ăяo��
            // �X�^�b�Ntop���C���X�^���X
            // �C���X�^���X�� Class �N���X�Ȃ�΁A���̃N���X���\�b�h���Ăяo��
            // ���s����Ɨ�O�𓊂���
            // [instance argn .. arg2 arg1] -> [retval]
            void    methodCall(SymbolID_t methodSymbol, Signature* pSig);
            void    methodCall(SymbolID_t methodSymbol, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                sig.setNumOnStack(numArgs + 1);
                methodCall(methodSymbol, &sig);
            }
            bool    methodCall_ifExist(SymbolID_t methodSymbol, int numArgs);
            // ���\�b�h�Ăяo��
            // �X�^�b�Ntop�͈����̍Ō�̒l
            // �C���X�^���X�� Class �N���X�Ȃ�΁A���̃N���X���\�b�h���Ăяo��
            // ���s����Ɨ�O�𓊂���
            // [argn .. arg2 arg1 instance] -> [retval]
            void    methodCallR(SymbolID_t methodSymbol, Signature* pSig);
            void    methodCallR(SymbolID_t methodSymbol, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                sig.setNumOnStack(numArgs);
                methodCallR(methodSymbol, &sig);
            }
            bool    methodCallR_ifExist(SymbolID_t methodSymbol, int numArgs);
            // ���\�b�h�Ăяo��
            // �X�^�b�Ntop�͈����̍Ō�̒l
            // �C���X�^���X�� Class �N���X�Ȃ�΁A���̃N���X���\�b�h���Ăяo��
            // ���s����Ɨ�O�𓊂���
            // [argn .. arg2 arg1] -> [retval]
            void    methodCall(const Value& instance, SymbolID_t methodSymbol, Signature* pSig);
            void    methodCall(const Value& instance, SymbolID_t methodSymbol, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                sig.setNumOnStack(numArgs);
                methodCall(instance, methodSymbol, &sig);
            }
            bool    methodCall_ifExist(const Value& instance, SymbolID_t methodSymbol, int numArgs);


            // �Ö�self�A�������̓N���X���\�b�h�A�������͑g�ݍ��݊֐��Ăяo��
            // [argn .. arg2 arg1] -> [retval]
            void    sMethodCall(SymbolID_t methodSymbol, Signature* pSig);
            void    sMethodCall(SymbolID_t methodSymbol, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                sMethodCall(methodSymbol, &sig);
            }
            // �w��N���X�̃��\�b�h�Ăяo��
            // �Ö�self�A�������̓N���X���\�b�h�A�������͑g�ݍ��݊֐����Ăяo��
            // [argn .. arg2 arg1] -> [retval]
            void    scopeMethodCall(SymbolID_t methodSymbol, const HClass* pClass, Signature* pSig);
            void    scopeMethodCall(SymbolID_t methodSymbol, const HClass* pClass, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                scopeMethodCall(methodSymbol, pClass, &sig);
            }

            void    callBytecodeTop(const Bytecode* pBytecode);
            bool    execBytecodeTop(const Bytecode* pBytecode);

            // ���\�b�h���s
            // [instance argn .. arg2 arg1] -> [retval]
            // ���s����Ɨ�O�𓊂���
            bool    execMethod(SymbolID_t methodSymbol, Signature* pSig);
            bool    execMethod(SymbolID_t methodSymbol, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                sig.setNumOnStack(numArgs + 1);
                return execMethod(methodSymbol, &sig);
            }
            // ���\�b�h���s
            // [argn .. arg2 arg1 instance] -> [retval]
            // ���s����Ɨ�O�𓊂���
            bool    execMethodR(SymbolID_t methodSymbol, Signature* pSig);
            bool    execMethodR(SymbolID_t methodSymbol, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                sig.setNumOnStack(numArgs + 1);
                return execMethodR(methodSymbol, &sig);
            }
            // ���\�b�h���s
            // [argn .. arg2 arg1] -> [retval]
            // ���s����Ɨ�O�𓊂���
            bool    execMethod(const Value & instance, SymbolID_t methodSymbol, Signature* pSig);
            bool    execMethod(const Value & instance, SymbolID_t methodSymbol, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                sig.setNumOnStack(numArgs);
                return execMethod(instance, methodSymbol, &sig);
            }

            // ���\�b�h���s
            // [argn .. arg2 arg1] -> [retval]
            // ���\�b�h��������� 0, �����Ď��s������ 1, ���s���s�� -1 ��Ԃ�
            int     execMethod_ifExist(const Value& instance, SymbolID_t methodSymbol, Signature* pSig);
            int     execMethod_ifExist(const Value& instance, SymbolID_t methodSymbol, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                return execMethod_ifExist(instance, methodSymbol, &sig);
            }

            Value   getLocalVar(int varIdx);
            void    setLocalVar(int varIdx, const Value& val);
            Object* shareLocalVar(int outVarIdx);
            Value   getMembVar(SymbolID_t varSym);
            void    setMembVar(SymbolID_t varSym, const Value&  val);
            Value   getClassVar(SymbolID_t varSym);
            void    setClassVar(SymbolID_t varSym, const Value&  val);
            Value   getGlobalVar(SymbolID_t varSym);
            void    setGlobalVar(SymbolID_t varSym, const Value&  val);
            Value   getConstVar(SymbolID_t varSym);
            void    setConstVar(SymbolID_t varSym, const Value&  val);
            void    setDefaultVal(hyu16 defValIdx, const Value& val) { const_cast<HClass*>(m_getSelfClass())->setDefaultVal(defValIdx, val); }

            // ��Othrow
            void    throwException(SymbolID_t type, Value exVal);
            void    throwException(Exception* e);
            void    throwJumpCtlException(SymbolID_t label, Value val); // next,break�p
            void    throwGotoException(SymbolID_t label, Value val); // goto�p
            // printf�`���Ń��b�Z�[�W�����Athrow����
            void    throwException(SymbolID_t type, const char* msg, ...);
            // try��catch�ł��Ȃ���O��throw
            void    throwError(Value exVal);
            // printf�`���Ń��b�Z�[�W�����A�G���[�Ƃ���throw����
            void    throwError(const char* msg, ...);


            // numArgs�̈������Amode�ɏ]����concat���A���ꂼ��̊Ԃ� , ������
            // �\�����āA�������X�^�b�N����폜
            void    debugPrintValues(int numArgs, int mode, const char* m);


            void    jumpControlStart(void);
            void    jumpControlEnd(void);
        

            // ���s���t���[�����A��Context�Ɉړ�����
            Context* fork(int stackAdjust);

        protected:
            // [self argn .. arg1]
            // self��Class�N���X�Ȃ�΁A�N���X���\�b�h�Ăяo���A
            // �����łȂ���΃C���X�^���X���\�b�h�Ăяo��
            // �Ăяo���ɐ�������� true ��Ԃ�
            bool    m_methodCall(SymbolID_t methodSymbol, Signature* pSig);
            // [argn .. arg1 self]
            // self��Class�N���X�Ȃ�΁A�N���X���\�b�h�Ăяo���A
            // �����łȂ���΃C���X�^���X���\�b�h�Ăяo��
            // �Ăяo���ɐ�������� true ��Ԃ�
            bool    m_methodCallR(SymbolID_t methodSymbol, Signature* pSig);
            // [argn .. arg1]
            // self��Class�N���X�Ȃ�΁A�N���X���\�b�h�Ăяo���A
            // �����łȂ���΃C���X�^���X���\�b�h�Ăяo��
            // �Ăяo���ɐ�������� true ��Ԃ�
            bool    m_methodCall(const Value& self, SymbolID_t methodSymbol, Signature* pSig);
            // ���\�b�h���������Č��ʂ�methodPtr�Ɋi�[����
            bool    m_buildMethodPtr(MethodPtr& methodPtr, SymbolID_t methodSymbol, Signature* pSig);

            // ���\�b�h�������������ʂ�methodPtr�ɓ����Ă����ԂŃ��\�b�h���Ăяo��
            void    m_doMethodCall(MethodPtr& methodPtr, SymbolID_t methodSymbol, Signature* pSig);

            // �g�b�v���x�����\�b�h(�o�C�g�R�[�h���\�b�h�AObject�N���X���\�b�h)���Ăяo��
            bool    m_topMethodCall(SymbolID_t methodSymbol, Signature* pSig);
            // �w��N���X�̃��\�b�h�Ăяo��
            bool    m_scopeMethodCall(SymbolID_t methodSymbol, const HClass* pClass, Signature* pSig);
            // �t���[��������Ɏ��s�A���݂̃t���[���܂Ŗ߂������ŏI��
            // ��O���������Č��݂̃t���[����˂������ČĂяo�����܂Ŗ߂���
            // ���܂����ꍇ�� false ��Ԃ�
            bool    m_doExec(void);

            // self���C���X�^���X�Ȃ炻�̃C���X�^���X�̃N���X�A
            // �N���X�Ȃ炻�̃N���X��Ԃ�
            const HClass* m_getSelfClass(void) { return curFrame->self.getScope(); }
            // ���L���l���������[�J���ϐ��A�h���X���擾
            Value*  m_getLocalVarAddr(hyu32 pos);
        
            // method_not_found��O�𓊂���
            void    m_throwMethodNotFoundException(const Value& self, SymbolID_t methodSym);


        public:
            // FFI�R�[������
            void    FfiCall(const MethodPtr& methodPtr, hyu8 numArgs, SymbolID_t methodSymbol);
            // FFI�I����ɌĂяo���֐�
            typedef void (*Aff_t)(Context* context);
            // FFI���̗�O����������֐�
            // ���������Ȃ�true,���������X���[�Ȃ�false��Ԃ�
            typedef bool (*FfiExceptionCatcher_t)(Context*, Exception*);
            // FFI�I����ɌĂяo���֐��̓o�^
            void        setAfterFfiFunc(Aff_t func);
            void        setAfterFfiParam(void* param);
            void*       getAfterFfiParam(void);
            // FFI�ďo����Ԃ��H
            bool        isInFfi(void);
            // FFI�I����̊֐��ďo��
            void        afterFfi(void);
            // FFI�I����̕W������
            void        ffiFinish(void);
            // FFI�I���̋�������
            void        ffiKill(void);
            // FFI���̗�O����������֐��̓o�^
            void        setFfiExceptionCatcher(FfiExceptionCatcher_t);


            // �T�u���[�`���R�[������
            void    routineCall(const MethodPtr& methodPtr, hyu8 numArgs);
            // �T�u���[�`�����^�[������
            void    routineReturn(hyu16 callAwayMethodID = (hyu16)-1);
            // �T�u���[�`���𔲂����O��fiber�ɓ�����悤�ɐݒ肷��
            // routineCall�̒���ɌĂ�
            void    exceptionToFiber(Fiber* exfiber);
            // ���΃W�����v����
            void    jumpRelative(hys32 relative);

            // finally���try�߂̒l���Z�b�g
            void    valueAfterFinally(Value& val);
            // finally�I���A���̏����ɕ��A
            void    endFinally(void);



            hyu16     getMethodID(void) { HMD_DEBUG_ASSERT(! curFrame->bFfi); return curFrame->r.methodID; }
            hys32     callStackLevel(void) { return frameStack.size() + 1; }

        public:
            void    m_GC_mark(void);
            void    m_GC_mark_stack(void);


        protected:
            enum {
                FINALLY_EXIT_TYPE_NONE,
                FINALLY_EXIT_TYPE_NORMAL,
                FINALLY_EXIT_TYPE_EXCEPTION,
                FINALLY_EXIT_TYPE_RETURN
            };
            struct JumpControlInfo_st {
                hyu16       tableNo;
                hys16       catchVar;      //catch�̃��[�J���ϐ�
                hys16       finallyValVar; //finally�I����Ɏg���l���X�g�A���Ă������[�J���ϐ�
                hyu16       finallyCallAwayMethodID; //return->finally���Ɏg�p
                hyu8        finallyExitType;
                hyu32       stackLv;
                union {
                    const hyu8*   startAddr;
                    Fiber*  fiber; // tableNo == EXCEPTION_TO_FIBER�̎��L��
                };
            };
            static const hys16 NO_CATCH_VAR = -0x8000;
            static const hyu16 EXCEPTION_TO_FIBER = 0xffff;

            // try-catch-finally, break,next, goto ���
            typedef CellList<JumpControlInfo_st>   JumpControlList;


        public:
            // finally���s�����H
            static inline bool isInFinally(const JumpControlInfo_st& inf) {
                return (inf.finallyExitType != FINALLY_EXIT_TYPE_NONE); }
            // �W�����v�A�h���X���擾�B������� NULL ��Ԃ�
            const hyu8* getJumpAddr(SymbolID_t label);
            const hyu8* getJumpAddr(SymbolID_t label, const JumpControlInfo_st& inf);

        public:
            /// �o�C�g�R�[�h���\�b�h�Ăяo�����^
            struct RoutineCallParam_st {
                const hyu8* jumpControlTableTop;  ///< JumpControl���e�[�u��
                hyu8          numLocals;          ///< ���[�J���ϐ��̐�
                hyu16         methodID;           ///< ���\�b�hID
                hyu16         creatorMethodID; ///< ����𐶐��������\�b�h��ID
                /// ���\�b�hID�́A�N���X�������R�[�h�Ȃ� 0
                /// ���\�b�h�Ȃ� ���\�b�h�ԍ�+1
                /// �N���[�W���Ȃ� �N���[�W���ԍ�+���\�b�h����+1
                JumpControlList*    jumpControls;     ///< JumpControl��񃊃X�g
            };
            /// FFI�Ăяo�����^
            struct FfiParam_st {
                SymbolID_t  methodSymbol;
                Aff_t       afterFfiFunc;
                void*       afterFfiParam;
                const Context* ffiContext;
                FfiExceptionCatcher_t   exceptionCatcher;
            };


            // �R�[���X�^�b�N
            struct Frame_st {
                hyu32     framePos; ///< stack��framePos����numArgs�̈���������
                Value   self;     ///< �Öق�self

                /// ���s���̃R�[�h�̑�����N���X
                /// �����[�h����ƐV�����N���X���w��
                const HClass* lexicalScope;

                /// ���s���̃R�[�h�����N���X
                /// �����[�h���Ă��A����͕ύX����Ȃ�
                const HClass* ownerClass;

                CodePtr returnCodePtr;  ///< ���^�[���A�h���X

                hyu8      numArgs;  ///< �����̐�

                bool    bFfi;   ///< true�Ȃ�f���L���Afalse�Ȃ�r���L��
                union {
                    RoutineCallParam_st r;
                    FfiParam_st         f;
                };
            };

            ValueStack          stack;
            CodePtr             codePtr;
            Stack<Frame_st>     frameStack; // �T�u���[�`���Ăяo���X�^�b�N
            Frame_st*           curFrame; // ���s���t���[��

            Thread*         thread;
            Fiber*      fiber;

            // �萔�ɑ���\�ȏ�Ԃ��ǂ���
            bool            canSubstConst;
            // exec�O��false�ɂ��āAthrowException��true�ɂȂ�
            bool        throwFlag;



#ifdef HMD_DEBUG
        public:
            void    debugStackTrace(void);
            void    debugPrintStack(void);
            void    debugPrintCurPos(void) { m_debugStackTrace1(-1, codePtr, *curFrame); }

        protected:
            void    m_debugStackTrace(void);
            void    m_debugStackTrace1(int level, const CodePtr codePtr, Frame_st& fr);
#else
        public:
            void    debugStackTrace(void) {}
            void    debugPrintStack(void) {}
            void    debugPrintCurPos(void) {}
#endif

        };

    }
}

#endif /* m_HYCONTEXT_H_ */
