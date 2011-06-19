/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyClosure.h"
#include "hyContext.h"
#include "hyDebug.h"

using namespace Hayat::Common;
using namespace Hayat::Engine;


size_t Closure::needSize(hys32 numSharedLocal)
{
    return sizeof(Closure) + sizeof(SharedLocal_st) * (numSharedLocal - 1);
}

Closure* Closure::create(hyu8 numSharedLocal)
{
    Object* obj = Object::create(HC_Closure, needSize(numSharedLocal) + sizeof(hyu32));
    return obj->cppObj<Closure>();
}

void Closure::initialize(const MethodPtr& methodPtr, hyu8 numSharedlocal, hyu16 creatorMethodID)
{
    HMD_ASSERT(needSize(numSharedlocal) <= getObj()->size());
    m_methodPtr = methodPtr;
    m_numSharedLocal = numSharedlocal;
    m_numSharedSet = 0;
    m_creatorMethodID = creatorMethodID;
    for (hyu32 i = 0; i < numSharedlocal; i++) {
        m_shareInfo[i].localIdx = (hyu8)-1;
        m_shareInfo[i].sharedLocalVar = NULL;
    }
}

void Closure::setSharedLocal(hyu8 localIdx, Object* sharedLocalVar)
{
    HMD_ASSERT(m_numSharedSet < m_numSharedLocal);
    HMD_ASSERT(m_shareInfo[m_numSharedSet].sharedLocalVar == NULL);
    //HMD__PRINTF_FK("Closure::setSharedLocal(localIdx=%d, sharedLocalVar=%x)\n",localIdx,sharedLocalVar);
    m_shareInfo[m_numSharedSet].localIdx = localIdx;
    m_shareInfo[m_numSharedSet].sharedLocalVar = sharedLocalVar;
    ++ m_numSharedSet;
}

void Closure::call(Context* context, hyu8 numArgs, bool bAway)
{
    HMD_DEBUG_ASSERT(m_numSharedSet == m_numSharedLocal);

    Signature sig(numArgs, context);
    const Signature::Sig_t* sigBytes;
    hyu16 defValOffs;

    const HClass* klass = m_methodPtr.lexicalScope();
    klass->getMethodSignatureBytes(m_methodPtr.getMethodID()-1, &sigBytes, &defValOffs);
    if (! sig.canSubstTo(sigBytes, klass, defValOffs)) {
        context->throwException(HSym_signature_mismatch, M_M("closure call signature mismatch"));
#if 0
        HMD_PRINTF("closure call signature mismatch\n");
        HMD_PRINTF("numArgs = %d\nsignature = ", numArgs);
        Signature::printSig(sigBytes);
        HMD_PRINTF("\nmethodID = ", m_methodPtr.getMethodID());
#endif
        return;
    }

    context->routineCall(m_methodPtr, sig.getNumOnStack());
    if (bAway)
        context->curFrame->r.creatorMethodID = m_creatorMethodID;
    hyu32 fp = context->curFrame->framePos;
    for (int i = 0; i < m_numSharedLocal; i++) {
        //HMD__PRINTF_FK("Closure::call : i=%d,localIdx=%d,sharedLocalVar=%x\n",i,m_shareInfo[i].localIdx,m_shareInfo[i].sharedLocalVar);
        Value* pLocalVar = context->stack.addrAt(fp + m_shareInfo[i].localIdx);
        pLocalVar->type = HC_INDIRECT_REF;
        pLocalVar->ptrData = m_shareInfo[i].sharedLocalVar;
    }
}

void Closure::m_GC_mark(void)
{
    Debug::incMarkTreeLevel();
    for (int i = 0; i < m_numSharedLocal; i++) {
        //HMD_PRINTF("mark shared obj %x\n",m_shareInfo[i].sharedLocalVar);
        GC::markObjP(&(m_shareInfo[i].sharedLocalVar));
    }
    m_methodPtr.m_GC_mark();
    Debug::decMarkTreeLevel();
}
