/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */


#ifdef MS_DLL_EXPORT_DECL
# define DLL_EXPORT     _declspec(dllexport)
#else
# define DLL_EXPORT
#endif


#include "hyValue.h"
#include "hyObject.h"
#include "hyClass.h"
#include "hyContext.h"
#include "hyBytecode.h"
#include "hyThread.h"
#include "hyStringBuffer.h"
#include "hySymbolTable.h"
#include "HSymbol.h"


using namespace Hayat::Common;
using namespace Hayat::Engine;



// この変数は、後で stdlib ロード時に initStdlib() にて初期化しなおす
// 初期化前に使ってしまうバグがあった場合にわかりやすい値を代入しておく
#define NONMEMADDR(x) ((const HClass*)(0xfffefdfb-((x)*4)))
DLL_EXPORT const HClass* Hayat::Engine::HC_Object =        NONMEMADDR(0);
DLL_EXPORT const HClass* Hayat::Engine::HC_NilClass =      NONMEMADDR(1);
DLL_EXPORT const HClass* Hayat::Engine::HC_Int =           NONMEMADDR(2);
DLL_EXPORT const HClass* Hayat::Engine::HC_Float =         NONMEMADDR(3);
DLL_EXPORT const HClass* Hayat::Engine::HC_Class =         NONMEMADDR(4);
DLL_EXPORT const HClass* Hayat::Engine::HC_Bool =          NONMEMADDR(5);
DLL_EXPORT const HClass* Hayat::Engine::HC_Symbol =        NONMEMADDR(6);
DLL_EXPORT const HClass* Hayat::Engine::HC_String =        NONMEMADDR(7);
DLL_EXPORT const HClass* Hayat::Engine::HC_Thread =        NONMEMADDR(8);
DLL_EXPORT const HClass* Hayat::Engine::HC_Exception =     NONMEMADDR(9);
DLL_EXPORT const HClass* Hayat::Engine::HC_Array =         NONMEMADDR(10);
DLL_EXPORT const HClass* Hayat::Engine::HC_Stack =         NONMEMADDR(11);
DLL_EXPORT const HClass* Hayat::Engine::HC_Hash =          NONMEMADDR(12);
DLL_EXPORT const HClass* Hayat::Engine::HC_StringBuffer =  NONMEMADDR(13);
DLL_EXPORT const HClass* Hayat::Engine::HC_List =          NONMEMADDR(14);
DLL_EXPORT const HClass* Hayat::Engine::HC_Closure =       NONMEMADDR(15);
DLL_EXPORT const HClass* Hayat::Engine::HC_Context =       NONMEMADDR(16);
DLL_EXPORT const HClass* Hayat::Engine::HC_Fiber =         NONMEMADDR(17);
DLL_EXPORT const HClass* Hayat::Engine::HC_Method =        NONMEMADDR(18);
DLL_EXPORT const HClass* Hayat::Engine::HC_Math =          NONMEMADDR(19);

// 疑似クラス
static HClass m_HC_body_REF(HSymR_REF);
static HClass m_HC_body_INDIRECT_REF(HSymR_INDIRECT_REF);
static HClass m_HC_body_INDIRECT_ENT(HSymR_INDIRECT_ENT);
static HClass m_HC_body_RELOCATED_OBJ(HSymR_RELOCATED_OBJ);
DLL_EXPORT const HClass* Hayat::Engine::HC_REF = &m_HC_body_REF;
DLL_EXPORT const HClass* Hayat::Engine::HC_INDIRECT_REF = &m_HC_body_INDIRECT_REF;
DLL_EXPORT const HClass* Hayat::Engine::HC_INDIRECT_ENT = &m_HC_body_INDIRECT_ENT;
#ifdef HY_ENABLE_RELOCATE_OBJECT
const HClass* Hayat::Engine::HC_RELOCATED_OBJ = &m_HC_body_RELOCATED_OBJ;
#endif


// 基本オブジェクト
DLL_EXPORT const Value  Hayat::Engine::NIL_VALUE = Value(HC_NilClass, (hyu32)0);
DLL_EXPORT const Value  Hayat::Engine::TRUE_VALUE = Value(HC_Bool, (hyu32)1);
DLL_EXPORT const Value  Hayat::Engine::FALSE_VALUE = Value(HC_Bool, (hyu32)0);
DLL_EXPORT const Value  Hayat::Engine::INT_0_VALUE = Value(HC_Int, (hyu32)0);
DLL_EXPORT const Value  Hayat::Engine::INT_1_VALUE = Value(HC_Int, (hyu32)1);
DLL_EXPORT const Value  Hayat::Engine::INT_M1_VALUE = Value(HC_Int, (hyu32)-1);
DLL_EXPORT const Value  Hayat::Engine::EMPTY_LIST_VALUE = Value(HC_List, (void*)NULL);



void Value::initStdlib(Bytecode& bytecode)
{
    const HClass* pMain = bytecode.mainClass();

    HC_Object = pMain->getInnerClass(HSym_Object);
    HC_NilClass = pMain->getInnerClass(HSym_NilClass);
    HC_Bool = pMain->getInnerClass(HSym_Bool);
    HC_Int = pMain->getInnerClass(HSym_Int);;
    HC_Float = pMain->getInnerClass(HSym_Float);
    HC_Math = pMain->getInnerClass(HSym_Math);
    HC_Class = pMain->getInnerClass(HSym_Class);
    HC_Symbol = pMain->getInnerClass(HSym_Symbol);
    HC_String = pMain->getInnerClass(HSym_String);
    HC_Thread = pMain->getInnerClass(HSym_Thread);
    HC_Exception = pMain->getInnerClass(HSym_Exception);
    HC_Array = pMain->getInnerClass(HSym_Array);
    HC_Stack = pMain->getInnerClass(HSym_Stack);
    HC_Hash = pMain->getInnerClass(HSym_Hash);
    HC_StringBuffer = pMain->getInnerClass(HSym_StringBuffer);
    HC_List = pMain->getInnerClass(HSym_List);
    HC_Closure = pMain->getInnerClass(HSym_Closure);
    HC_Context = pMain->getInnerClass(HSym_Context);
    HC_Fiber = pMain->getInnerClass(HSym_Fiber);
    HC_Method = pMain->getInnerClass(HSym_Method);
    m_setConstValues();
}

void Value::destroyStdlib(void)
{
    HC_Object =        NONMEMADDR(0);
    HC_NilClass =      NONMEMADDR(1);
    HC_Int =           NONMEMADDR(2);
    HC_Float =         NONMEMADDR(3);
    HC_Class =         NONMEMADDR(4);
    HC_Bool =          NONMEMADDR(5);
    HC_Symbol =        NONMEMADDR(6);
    HC_String =        NONMEMADDR(7);
    HC_Thread =        NONMEMADDR(8);
    HC_Exception =     NONMEMADDR(9);
    HC_Array =         NONMEMADDR(10);
    HC_Stack =         NONMEMADDR(11);
    HC_Hash =          NONMEMADDR(12);
    HC_StringBuffer =  NONMEMADDR(13);
    HC_List =          NONMEMADDR(14);
    HC_Closure =       NONMEMADDR(15);
    HC_Context =       NONMEMADDR(16);
    HC_Fiber =         NONMEMADDR(17);
    HC_Method =        NONMEMADDR(18);

    m_setConstValues();
}

void Value::m_setConstValues(void)
{
    ((Value*)&NIL_VALUE)->type = HC_NilClass;
    ((Value*)&TRUE_VALUE)->type = HC_Bool;
    ((Value*)&FALSE_VALUE)->type = HC_Bool;
    ((Value*)&INT_0_VALUE)->type = HC_Int;
    ((Value*)&INT_1_VALUE)->type = HC_Int;
    ((Value*)&INT_M1_VALUE)->type = HC_Int;
    ((Value*)&EMPTY_LIST_VALUE)->type = HC_List;
}


// ハッシュコードを計算
hyu32 Value::hashCode(Context* context)
{
    bool b = context->execMethod(*this, HSym_hashCode, 0);
    HMD_DEBUG_ASSERT(b);
    return (hyu32) context->pop().toInt();
}

// 等しいかどうか
bool Value::equals(Context* context, const Value& o) const
{
    if (*this == o)
        return true;        // 完全に同じ
    if ((data == o.data) && (getType() == o.getType()))
        return true;    // 参照先が完全に同じ

    // '==' 演算する
    context->push(o);
    bool b = context->execMethod(*this, HSymx_3d3d, 1);
    HMD_DEBUG_ASSERT(b);
    return context->pop().toBool();
}
    

const hyu8* Value::getTypeName(void) const
{
    return getType()->name();
}

SymbolID_t Value::getTypeSymbol(void) const
{
    return getType()->getSymbol();
}

#if defined(HMD_DEBUG) && !defined(TEST__CPPUNIT)
void Value::checkType(SymbolID_t classSym) const
{
    if (type->getSymbol() == classSym)
        return;

    if (type == HC_REF) {
        HMD_FATAL_ERROR("type %s expected but was *ref:%s\n  (data = 0x%x)\n",
                        gSymbolTable.id2str(classSym),
                        objPtr->type()->name(),
                        data);
    } else {
        HMD_FATAL_ERROR("type %s expected but was %s\n  (data = 0x%x)\n",
                        gSymbolTable.id2str(classSym),
                        type->name(),
                        data);
    }
}

void Value::checkType_unref(SymbolID_t classSym) const
{
    if (type->getSymbol() == classSym)
        return;
    if (type == HC_REF) {
        if (objPtr->type()->getSymbol() == classSym)
            return;
        HMD_FATAL_ERROR("type %s expected but was *ref:%s\n  (data = 0x%x)\n",
                        gSymbolTable.id2str(classSym),
                        objPtr->type()->name(),
                        data);
    } else {
        HMD_FATAL_ERROR("type %s expected but was %s\n  (data = 0x%x)\n",
                        gSymbolTable.id2str(classSym),
                        type->name(),
                        data);
    }
}
#endif

const HClass* Value::getType(void) const
{
    if (type != HC_REF)
        return type;
    return objPtr->type();
}

// typeがクラスならそのクラスを返す
// インスタンスならそのインスタンスのクラスを返す
const HClass* Value::getScope(void) const
{
    if (type->symCheck(HSym_Class))
        return (const HClass*)ptrData;
    return getType();
}

hyf32 Value::toFloat(void) const
{
    if (type == HC_Float)
        return floatData;
    if (type == HC_Int)
        return (hyf32) intData;
    HMD_FATAL_ERROR("type float expected but was %s\n  (data = 0x%x)\n",
                    type->name(),
                    data);
    return 0.0f; // never reach here; avoid compiler warning
}

const char* Value::toString(void) const
{
    if (type->symCheck(HSym_String))
        return (const char*)ptrData;
    CHECKTYPE_UNREF(HSym_StringBuffer);
    StringBuffer* sb = objPtr->cppObj<StringBuffer>();
    return sb->top();
}


#ifdef HMD_DEBUG
Object* Value::toObj(SymbolID_t sym) const
{
    HMD_DEBUG_ASSERT(type == HC_REF);
    HMD_ASSERT(objPtr->type()->symCheck(sym));
    return objPtr;
}
#endif


void* Value::m_toCppObj(void)
{
    return toObj()->cppObj<void>();
}

template<> void* Stack<Value>::operator new(size_t size)
{
#ifndef HMD_DEBUG
    (void)size;
#endif
    HMD_DEBUG_ASSERT(size == sizeof(Stack<Value>));
    Object* obj = Object::create(HC_Stack, sizeof(Stack<Value>) + sizeof(hyu32));
    return (void*) obj->cppObj< Stack<Value> >();
}

template<> void Stack<Value>::operator delete(void* p)
{
    Object::fromCppObj(p)->destroy();
}

template<> hyu32 Stack<Value>::getExpandCapacity(void)
{
    return 16;
}
