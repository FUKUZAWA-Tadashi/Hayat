/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYFIBER_H_
#define m_HYFIBER_H_

#include "hyValue.h"

/*
  Fiber�N���X

  yield���Œl�𐶐�����B


  generate()
    yield�ɂ���č��ꂽ�l���擾����B�܂�����Ă��Ȃ����
    yield����܂Ŏ��s����B
    return�����s�����ƁAException(:fiber_return)�𔭐�������B


  defFiber times (n) {
    x = 0
    for (i = 0; i < n; i += 1) {
      x += yield i
    }
    return x
  }

  gen = times(3)    // n=3 �� Fiber�C���X�^���X�����������
  gen.generate()    // => 0  // yield i �܂Ŏ��s�A0 ���擾
  gen.yieldResult(10) // yield���̖߂�l�Ƃ��� 10 ��^����
  gen.generate()    // => 1  // yield i �܂Ŏ��s�A1 ���擾
  gen.yieldResult(20) // yield���̖߂�l�Ƃ��� 20 ��^����
  gen.generate()    // => 2  // yield i �܂Ŏ��s�A2 ���擾
  gen.yieldResult(30) // yield���̖߂�l�Ƃ��� 30 ��^����
  gen.generate()    // => return x �܂Ŏ��s�AException���������� val()�� 60



  go()
    yield �܂��� return ���܂ł����s����B
    yield �����s�����ꍇ�� true ���A return �����s�����ꍇ�ɂ� false ��Ԃ��B
    yield�ɂ���č��ꂽ�l�� getYieldVal() �Ŏ擾�ł���B
    return�ɂ���ĕԂ��ꂽ�l�� getReturnVal() �Ŏ擾�ł���B
*/

namespace Hayat {
    namespace Engine {

        class Exception;
        class Closure;

        class Fiber {
#ifdef HMD_DEBUG
            friend class Context;   // for Context::debugStackTrace()
#endif

        protected:
            Context*        m_fiberContext; // yield�����s���Ă���Context
            Context*        m_callerContext; // go()�܂���generate()���Ăяo����Context

            enum State_e {
                YIELDING,           // yield�����s���ꂽ����
                YIELD_JOIN,         // yield�߂�l�҂�
                RUNNING,            // �ʏ핔�������s��
                RETURNED,           // return�����s���ꂽ����
                FINISHED            // �I�����
            } m_state;


            static const hyu8 MODE_NONE = 0;
            static const hyu8 MODE_GO = 1; // go()���s
            static const hyu8 MODE_GENERATE = 2; // generate()���s
            static const hyu8 MODE_ITERATE = 3; // iterate()���s
            hyu8            m_mode;
            Closure*        m_iterating_closure; // iterate����closure

        public:

            static void* operator new(size_t);
            static void operator delete(void*);
        
            Fiber(Context* context) { initialize(context); }
            ~Fiber() {}

            // context��Ŏ��s���J�n�����΂���̃��\�b�h��Fiber�ɕω�������
            void initialize(Context* context);

            // context���closure���Ăяo���Ă����Fiber�ɕω�������
            void initialize(Context* context, Closure* closure, hyu8 numArgs);

            void yielding(void); // m_fiberContext���yield�����s
            void fiberReturn(void);         // fiber�����return�����s
            void fiberThrow(Exception* e);  // fiber�����O�Ŕ�����
            void go(Context* context);     // yield�܂���return�܂Ŏ��s
            void getYieldVal(Context* context); // yield�l���擾
            void getReturnVal(Context* context); // return�l���擾
            void yieldResult(Context* context, hyu8 numArgs); // yield�̖߂�l��^����
            void generate(Context* context); // yield�l��context�Ɏ����Ă���

            // �J��Ԃ�����
            void iterate(Context* context, Closure* closure);
            void iterate_affi(Context* context);
            // �J��Ԃ����ł̗�O�̏���
            bool iterate_exception_catcher(Context* context, Exception* e);

            // yield��(m_status==YIELD_JOIN)�Ȃ�yield�ɑ΂��ė�O�𓊂��A
            // �����łȂ���Ε��ʂɗ�O�𓊂���
            void thrown(Context* context, Exception* e);
            // ���s�R���e�L�X�g��m_fiberContext�ɕύX
            void switchToFiberContext(void);
            // closure��ŗ�O�����������炻���thrown()�ɓn���悤�ɂ�����Ԃ�
            // context���closure���Ăяo��
            void stakeCall(Context* context, Closure* closure, hyu8 numArgs);


            bool isFinished(void) { return m_state >= FINISHED; }

        public:
            void m_GC_mark(void);

        protected:
            static void m_contextSwitch(Context* prevContext, Context* nextContext);

        };

    }
}

#endif /* m_HYFIBER_H_ */
