/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCJumpControlTree.h"
#include "hyCSymbolTable.h"
#include "hyCContext.h"
#include "hyMemPool.h"
#include "hyEndian.h"

namespace Hayat {
    namespace Compiler {
        extern SymbolTable gSymTable;
    }
}
using namespace Hayat::Common;
using namespace Hayat::Compiler;;


extern void compileError(const char* fmt, ...);

const hyu32 JumpControlTree::INVALID_ADDR = (hyu32)-1;

void* JumpControlTree::operator new(size_t size)
{
    HMD_DEBUG_ASSERT_EQUAL(sizeof(JumpControlTree), size);
    MemCell* p = gMemPool->alloc(sizeof(JumpControlTree));
    p->setMemID("JCT ");
    return (void*)p;
}

void JumpControlTree::operator delete(void* p)
{
    gMemPool->free(p);
}

JumpControlTree::JumpControlTree(JumpControlTree* parent)
    : m_parent(parent), m_labelAddrs(0), m_res(0), m_childs(0),
      startAddr(0)
{
}

JumpControlTree::~JumpControlTree()
{
    m_labelAddrs.finalize();
    TArray<TArray<hyu32> >& resolveAddrsArr = m_res.values();
    TArrayIterator<TArray<hyu32> > itr(&resolveAddrsArr);
    while (itr.hasMore())
        itr.next().finalize();
    m_res.finalize();
    m_childs.finalize();
}

JumpControlTree* JumpControlTree::newChild(void)
{
    JumpControlTree* p = new JumpControlTree(this);
    m_childs.add(p);
    return p;
}

hyu32 JumpControlTree::getLocalLabelAddr(SymbolID_t label)
{
    hyu32* p = m_labelAddrs.find(label);
    if (p != NULL)
        return *p;
    return INVALID_ADDR;
}

hyu32 JumpControlTree::getLabelAddr(SymbolID_t label)
{
    hyu32* p = m_labelAddrs.find(label);
    if (p != NULL)
        return *p;
    if (m_parent != NULL)
        return m_parent->getLabelAddr(label);
    return INVALID_ADDR;
}

void JumpControlTree::addLabel(SymbolID_t label, hyu32 addr)
{
    if (m_labelAddrs.find(label) != NULL)
        compileError(M_M("same label '%s' in same context"), gSymTable.id2str(label));
    m_labelAddrs[label] = addr;
}

void JumpControlTree::addResolveAddr(SymbolID_t label, hyu32 resolveAddr)
{
    TArray<hyu32>* p = m_res.find(label);
    if (p != NULL) {
        p->add(resolveAddr);
    } else {
        TArray<hyu32>& arr = m_res[label];
        arr.initialize(1);
        arr.setContentsMemID("JCTr");
        arr.add(resolveAddr);
    }
}

void JumpControlTree::resolve(Context* context)
{
    m_resolveLocal(context);
    hyu32 n = m_childs.size();
    for (hyu32 i = 0; i < n; i++) {
        m_childs[i]->resolve(context);
    }
}

void JumpControlTree::m_resolveLocal(Context* context)
{
    TArray<SymbolID_t>& labelArr = m_res.keys();
    TArray<TArray<hyu32> >& resolveAddrsArr = m_res.values();
    hyu32 n = labelArr.size();
    for (hyu32 i = 0; i < n; i++) {
        SymbolID_t label = labelArr[i];
        TArray<hyu32>& resolveAddrs = resolveAddrsArr[i];
        hyu32 adr = getLocalLabelAddr(label);
        if (adr != INVALID_ADDR) {
            hyu32 m = resolveAddrs.size();
            for (hyu32 j = 0; j < m; j++) {
                context->resolveJumpAddr(resolveAddrs[j], adr);
            }
        } else {
            hyu32 m = resolveAddrs.size();
            for (hyu32 j = 0; j < m; j++) {
                context->replaceJumpControl(resolveAddrs[j], label);
            }
        }
    }
}

TArray<hyu8>* JumpControlTree::genCode(void)
{
    TArray<hyu8>* bin = new TArray<hyu8>(8);
    TArray<hyu8> lbl;
    TArray<hyu8> adr;
    TArray<SymbolID_t>& syms = m_labelAddrs.keys();
    hyu32 n = syms.size();
    HMD_ASSERT(n < 256);
    for (hyu32 i = 0; i < n; i++) {
        SymbolID_t sym = syms[i];
        packOut<SymbolID_t>(lbl, sym);
        packOut<hys32>(adr, m_labelAddrs[sym] - startAddr);
    }
    bin->add(n); bin->add(2); bin->add(1); bin->add(0);
    bin->add(lbl);
    bin->align(4, 0xf9);
    bin->add(adr);

    return bin;
}

bool JumpControlTree::haveLabel(SymbolID_t label)
{
    if (m_labelAddrs.keys().isInclude(label))
        return true;
    hyu32 n = m_childs.size();
    for (hyu32 i = 0; i < n; ++i)
        if (m_childs[i]->haveLabel(label))
            return true;
    return false;
}
