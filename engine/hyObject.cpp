/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyObject.h"
#include "hyValue.h"
#include "hyMemPool.h"
#include "HSymbol.h"
#include "hyCodeManager.h"
#include "hyBytecode.h"
#include "hyVM.h"
#include "hySymbolTable.h"
#include "hyDebug.h"
#include <string.h>


using namespace Hayat::Common;
using namespace Hayat::Engine;

Object* Object::create(SymbolID_t bytecodeSym, SymbolID_t classSym, size_t size)
{
    const Bytecode* pb = gCodeManager.getBytecode(bytecodeSym);
    if (pb == NULL) return NULL;
    const HClass* pc = pb->searchClass(classSym);
    if (pc == NULL) return NULL;
    return create(pc, size);
}
        
Object* Object::create(const HClass* type, size_t size)
{
#ifdef TEST__CPPUNIT
    // (HClass*)2000 以下はテスト用の仮クラス
    if ((hyu32)type > 2000)
#endif
        HMD__PRINTF_GC("Object::create(%s, %d)", type->name(), size);
#ifdef TEST__CPPUNIT
    else
        HMD__PRINTF_GC("Object::create(%x, %d)", type, size);
#endif
    MemCell* cell = gMemPool->alloc(size + sizeof(Object), "OBJ ");
    cell->setGCObject();
    ((Object*)cell)->m_type.voidp = (void*)type;
    HMD__PRINTF_GC(" --> %x\n", cell);

    // unmarkフェーズ時にcreateしたObjectは、unmarkする。
    // mark,sweepフェーズ時にcreateしたObjectは、mark状態。
    if (GC::isPhaseUnmark())
        ((Object*)cell)->m_unmark();

    return (Object*)cell;
}

void Object::destroy(void)
{
#ifdef TEST__CPPUNIT
    // (HClass*)2000 以下はテスト用の仮クラス
    if ((hyu32)type() > 2000)
#endif
        HMD__PRINTF_GC("Object::destroy() %x  class=%s\n",this,type()->name());
#ifdef TEST__CPPUNIT
    else
        HMD__PRINTF_GC("Object::destroy() %x  class=%x\n",this,type());
#endif
    gMemPool->free(this);
}


size_t Object::cellSize(void)
{
    return ((MemCell*)this)->size();
}

size_t Object::size(void)
{
    return ((MemCell*)this)->size() - sizeof(Object);
}

bool Object::equals_memb(Context* context, Object* o)
{
    const HClass* myType = type();
    if (myType != o->type())
        return false;
    Value* pv1 = (Value*)field(0);
    Value* pv2 = (Value*)o->field(0);
    int i = myType->m_membVarSymArr.size();
    while (i-- > 0) {
        if (! (pv1++)->equals(context, *pv2++))
            return false;
    }
    return true;
}

bool Object::equals(Context* context, Object* o)
{
    if (this == o)
        return true;
    const HClass* myType = type();
    size_t fsize = myType->fieldSize();
    HMD_DEBUG_ASSERT(size() >= fsize + sizeof(hyu32));
    size_t cppSize = size() - sizeof(hyu32) - fsize;
    if (cppSize != 0) {
        // cppObjの中身をメモリ比較
        void* p1 = field(fsize + sizeof(hyu32));
        void* p2 = o->field(fsize + sizeof(hyu32));
        if (memcmp(p1, p2, cppSize) != 0)
            return false;
    }

    return equals_memb(context, o);
}


Object* Object::fromCppObj(const void* cppObj)
{
    if (cppObj == NULL) return NULL;
    hyu32* pSize = ((hyu32*)cppObj) - 1;
    return (Object*)(((char*)pSize) - *pSize - sizeof(Object));
}

void Object::unsetGCObject(void)
{
    ((MemCell*)this)->unsetGCObject();
}

void Object::setGCObject(void)
{
    ((MemCell*)this)->setGCObject();
}

bool Object::isGCObject(void)
{
    return ((MemCell*)this)->isGCObject();
}



static void s_copyMembs(Value* oldMemb, Value* newMemb, const TArray<SymbolID_t>& oldMembVarSymArr, const TArray<SymbolID_t>& newMembVarSymArr)
{
    hys32 oldMembSize = oldMembVarSymArr.size();
    hys32 newMembSize = newMembVarSymArr.size();
    hys32 oi = 0;
    hys32 ni = 0;
    while (oi < oldMembSize && ni < newMembSize) {
        SymbolID_t os = oldMembVarSymArr[oi];
        SymbolID_t ns = newMembVarSymArr[ni];
        if (os == ns) {
            *newMemb++ = *oldMemb++;
            ++oi; ++ni;
        } else if (os < ns) {
            // 変数が減った
            ++oldMemb;
            ++oi;
        } else {
            // 変数が増えた
            *newMemb++ = NIL_VALUE;
            ++ni;
        }
    }
    while (ni++ < newMembSize)
        *newMemb++ = NIL_VALUE;
}
#if 0
void Object::s_dump(void)
{
    const HClass* pClass = type();
    HMD_PRINTF("obj %x class %s(%x)\n",this,pClass->name(),pClass);
    HMD_PRINTF("instance vars(%d) = {",pClass->m_membVarSymArr.size());
    for(int i=0;i<(int)pClass->m_membVarSymArr.size();++i)
        HMD_PRINTF("%s,",gSymbolTable.id2str(pClass->m_membVarSymArr[i]));
    HMD_PRINTF("}\n");
    hyu32* p = (hyu32*)this;
    hyu32* endp = p + cellSize() / sizeof(hyu32);
    while (p < endp) {
        HMD_PRINTF("%08x  ",*p++);
    }
    HMD_PRINTF("\n");
}
#endif
Object* Object::classGeneration(const HClass* newClass)
{
    const HClass* oldClass = type();
    //HMD_PRINTF("obj %x class gen %s(%x) -> %s(%x)\n",this,oldClass->name(),oldClass,newClass->name(),newClass);
    //s_dump();
    Object* newObj;
    hyu32 oldMembSize = oldClass->m_membVarSymArr.size();
    hyu32 newMembSize = newClass->m_membVarSymArr.size();
    hys32 sizeDiff = newMembSize - oldMembSize;
    if (sizeDiff != 0) {
        sizeDiff *= sizeof(Value);
        //HMD_PRINTF(" obj size %+d\n", sizeDiff);
        size_t size = ((MemCell*)this)->size() - sizeof(Object);
        newObj = create(newClass, size + sizeDiff);
        size -= oldMembSize * sizeof(Value);    // cppObj部分のize

        // cppobjのコピー
		hyu32 newFSize = newMembSize * sizeof(Value);
		void* pNewFSize = newObj->field(newFSize);
        memcpy(pNewFSize,
               field(oldMembSize * sizeof(Value)),
               size);
		*((hyu32*)pNewFSize) = newFSize;

        // メンバ変数のコピー
        s_copyMembs((Value*)field(0), (Value*)newObj->field(0),
                    oldClass->m_membVarSymArr, newClass->m_membVarSymArr);

        // 旧objは新objへのポインタとなる
        m_type.voidp = (void*)HC_RELOCATED_OBJ;
        // m_mark(); ↑これでマークした事になっている
        *(void**)field(0) = newObj;
        
    } else {
        newObj = this;
        size_t newMembMemSize = newMembSize * sizeof(Value);
        Value* tmpMem = (Value*)HMD_ALLOCA(newMembMemSize);
        s_copyMembs((Value*)field(0), tmpMem,
                    oldClass->m_membVarSymArr, newClass->m_membVarSymArr);
        memcpy(newObj->field(0), tmpMem, newMembMemSize);
        m_type.voidp = (void*)newClass;
    }
    //newObj->s_dump();

    newObj->m_unmark();
    return newObj;
}


void Object::m_GC_mark(void)
{
    if (isMarked())
        return;
    
    m_mark();
    const HClass* pClass = type();
#ifdef HY_ENABLE_RELOCATE_OBJECT
    HMD_DEBUG_ASSERT(pClass != HC_RELOCATED_OBJ);
#endif
    
    HMD__PRINTF_GC("GC mark obj %x %s\n", this, pClass->name());

    if (pClass == HC_INDIRECT_ENT) {
        GC::markValue(*(Value*)field());
        return;
    }
    
    
    GC::MarkFunc_t markFunc = pClass->getGCMarkFunc();
    if (markFunc != NULL)
        markFunc(this);
    
    // メンバ変数
    pClass->m_GC_mark_membVar(this);
}

void Object::m_finalize(Context* finalizeContext)
{
    HMD_DEBUG_ASSERT(finalizeContext != NULL);
    Context* saveContext = VM::getContext();
    finalizeContext->execMethod_ifExist(Value::fromObj(this), HSym_finalize, 0);
    finalizeContext->cleanup();
    VM::setContext(saveContext);
}
