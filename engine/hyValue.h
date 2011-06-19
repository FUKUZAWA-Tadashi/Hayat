/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYVALUE_H_
#define m_HYVALUE_H_

#include "HSymbol.h"
#include "hyTuning.h"
#include "hyStack.h"
#include "hyCellList.h"


#ifdef TEST__CPPUNIT
#include <ostream>
#endif


namespace Hayat {
    namespace Engine {
        class Value;
        class Bytecode;
        class Context;
        class Object;
        class HClass;
        typedef Stack<Value> ValueStack;
        typedef CellList<Value> ValueList;

        HMD_EXTERN_DECL const HClass* HC_Object;          // 全クラスのスーパークラス 
        HMD_EXTERN_DECL const HClass* HC_NilClass;
        HMD_EXTERN_DECL const HClass* HC_Int;
        HMD_EXTERN_DECL const HClass* HC_Float;
        HMD_EXTERN_DECL const HClass* HC_Math;
        HMD_EXTERN_DECL const HClass* HC_Class;
        HMD_EXTERN_DECL const HClass* HC_Bool;
        HMD_EXTERN_DECL const HClass* HC_Symbol;
        HMD_EXTERN_DECL const HClass* HC_String;
        HMD_EXTERN_DECL const HClass* HC_Thread;
        HMD_EXTERN_DECL const HClass* HC_Exception;
        HMD_EXTERN_DECL const HClass* HC_Array;
        HMD_EXTERN_DECL const HClass* HC_Stack;
        HMD_EXTERN_DECL const HClass* HC_Hash;
        HMD_EXTERN_DECL const HClass* HC_StringBuffer;
        HMD_EXTERN_DECL const HClass* HC_List;
        HMD_EXTERN_DECL const HClass* HC_Closure;
        HMD_EXTERN_DECL const HClass* HC_Context;
        HMD_EXTERN_DECL const HClass* HC_Fiber;
        HMD_EXTERN_DECL const HClass* HC_Method;

        // Object参照
        HMD_EXTERN_DECL const HClass* HC_REF;
        // 間接ローカル変数参照
        HMD_EXTERN_DECL const HClass* HC_INDIRECT_REF;
        // 間接ローカル変数実体
        HMD_EXTERN_DECL const HClass* HC_INDIRECT_ENT;
#ifdef HY_ENABLE_RELOCATE_OBJECT
        // Object移動情報
        extern const HClass* HC_RELOCATED_OBJ;
#endif

        class Value {

        public:
            const HClass*   type;
            union {
                hyu32         data;
                hyf32         floatData;
                hys32         intData;
                void*       ptrData;
                Object*     objPtr;
            };

        public:
#if defined(HMD_DEBUG) && !defined(TEST__CPPUNIT)
#define CHECKTYPE(t) checkType(t)
#define CHECKTYPE_UNREF(t) checkType_unref(t)
            void checkType(SymbolID_t classSym) const;
            void checkType_unref(SymbolID_t classSym) const;
#else
#define CHECKTYPE(t) ((void)t)
#define CHECKTYPE_UNREF(t) ((void)t)
            void checkType(SymbolID_t) const {};
            void checkType_unref(SymbolID_t) const {};
#endif

            Value(void);
            Value(const Value &o) { *this = o; }
            Value(const HClass* klass, hyu32 val) { type = klass; data = val; }
            Value(const HClass* klass, hyf32 val) { type = klass; floatData = val; }
            Value(const HClass* klass, hys32 val) { type = klass; intData = val; }
            Value(const HClass* klass, void* val) { type = klass; ptrData = val; }

            static Value fromBool(bool b);
            static Value fromInt(hys32 i) {return Value(HC_Int, i);}
            static Value fromFloat(hyf32 f) {return Value(HC_Float, f);}
            static Value fromObj(Object* o);
            static Value fromSymbol(SymbolID_t sym) {return Value(HC_Symbol, (hyu32)sym);}
            static Value fromClass(const HClass* pClass) {return Value(HC_Class, (void*)pClass);}
            static Value fromString(const char* str) {return Value(HC_String, (void*)str);}
            static Value fromList(ValueList* list) {return Value(HC_List, (void*)list);}

            const HClass* getType(void) const;
            bool toBool(void) const {CHECKTYPE(HSym_Bool); return (data != 0);}
            hys32 toInt(void) const {CHECKTYPE(HSym_Int); return intData;}
            hyf32 toFloat(void) const;
            SymbolID_t toSymbol(void) const {CHECKTYPE(HSym_Symbol); return (SymbolID_t)data;}
            Object* toObj(void) const {HMD_DEBUG_ASSERT(type == HC_REF); return objPtr;}
            template <typename T> T* toCppObj(void) { return (T*) m_toCppObj(); }
            template <typename T> T* toCppObj(SymbolID_t classSym) {CHECKTYPE_UNREF(classSym); return (T*) m_toCppObj();}


            template <typename T> T* ptrCast(void) { return (T*) ptrData; }
            template <typename T> T* ptrCast(SymbolID_t classSym) {CHECKTYPE(classSym); return (T*) ptrData;}

            // オブジェクトのtypeをテストする
#ifdef HMD_DEBUG
            Object* toObj(SymbolID_t sym) const;
#else
            Object* toObj(SymbolID_t) const { return objPtr;}
#endif
            const HClass* toClass(void) const {CHECKTYPE(HSym_Class); return (const HClass*)ptrData;}
            const char* toString(void) const;

            ValueList* toList(void) const {CHECKTYPE(HSym_List); return (ValueList*)ptrData;}

            // ハッシュコードを計算
            hyu32     hashCode(Context* context);
            // 等しいかどうか
            bool    equals(Context* context, const Value& o) const;


            static void initStdlib(Bytecode& bytecode);
            static void destroyStdlib(void);    // for unittest

            const hyu8*       getTypeName(void) const;
            SymbolID_t      getTypeSymbol(void) const;

            // クラスならそのクラスを返す
            // そうでなければインスタンスのクラスを返す
            const HClass* getScope(void) const;

            friend bool operator==(const Value& d1, const Value& d2) {
                return (d1.type == d2.type) && (d1.data == d2.data);
            }

#ifdef TEST__CPPUNIT
            friend std::ostream& operator<<(std::ostream& out, const Value& d) {
                out << (d.data ? "true" : "false") << ":<Bool>";
                return out;
            }
#endif


        protected:
            void* m_toCppObj(void);
            static void  m_setConstValues(void);

        };



        HMD_EXTERN_DECL const Value NIL_VALUE;       // nil
        HMD_EXTERN_DECL const Value TRUE_VALUE;      // true
        HMD_EXTERN_DECL const Value FALSE_VALUE;     // false
        HMD_EXTERN_DECL const Value INT_0_VALUE;     // 0 :<Int>
        HMD_EXTERN_DECL const Value INT_1_VALUE;     // 1 :<Int>
        HMD_EXTERN_DECL const Value INT_M1_VALUE;    // -1 :<Int>
        HMD_EXTERN_DECL const Value EMPTY_LIST_VALUE; // '()


        inline Value::Value(void) {
            *this = NIL_VALUE;
        }
        inline Value Value::fromObj(Object* o) {
            if (o == NULL) return NIL_VALUE;
            return Value(HC_REF, (void*)o);
        }
        inline Value Value::fromBool(bool b) {
            return b ? TRUE_VALUE : FALSE_VALUE;
        }

    }
}

namespace Hayat {
    namespace Common {
        template<> void* Stack<Hayat::Engine::Value>::operator new(size_t);
        template<> void Stack<Hayat::Engine::Value>::operator delete(void*);
        template<> hyu32 Stack<Hayat::Engine::Value>::getExpandCapacity(void);
    }
}


#endif /* m_HYVALUE_H_ */
