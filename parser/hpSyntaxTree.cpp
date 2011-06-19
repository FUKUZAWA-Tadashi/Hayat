
#include "hpSyntaxTree.h"
#include "hpParser.h"
#include "hpInputBuffer.h"



using namespace Hayat::Parser;


static SyntaxTree st__PARSE_FAILED(SyntaxTree::V_FAIL);
static SyntaxTree st__NO_SYNTAX_TREE(SyntaxTree::V_INVALID);
static SyntaxTree st__FATAL_PARSER_ERROR(SyntaxTree::V_FATAL_ERROR);
static SyntaxTree st__NOT_PARSED_YET(SyntaxTree::V_INVALID);
static SyntaxTree st__PARSING(SyntaxTree::V_INVALID);

SyntaxTree* const Hayat::Parser::m_PARSE_FAILED = &st__PARSE_FAILED;
SyntaxTree* const Hayat::Parser::m_NO_SYNTAX_TREE = &st__NO_SYNTAX_TREE;
SyntaxTree* const Hayat::Parser::m_FATAL_PARSER_ERROR = &st__FATAL_PARSER_ERROR;
SyntaxTree* const Hayat::Parser::m_NOT_PARSED_YET = &st__NOT_PARSED_YET;
SyntaxTree* const Hayat::Parser::m_PARSING = &st__PARSING;


bool SyntaxTree::m_usePool;
TArray<TArray<SyntaxTree>* > SyntaxTree::m_syntaxTreeMemPool(0);
TArray<TArray<SyntaxTree::Childs>* > SyntaxTree::m_childsMemPool(0);


template<typename P> P* poolAlloc(TArray< TArray<P>* >& pool)
{
    TArray<P>* p;
    if (pool.size() < 1) {
        p = new TArray<P>(100);
        p->setContentsMemID("pool");
        HMD_DEBUG_ASSERT(p != NULL);
        pool.add(p);
    } else {
        p = pool[pool.size() - 1];
        if (p->remain() < 1) {
            p = new TArray<P>(p->size() + 100);
            p->setContentsMemID("pool");
            HMD_DEBUG_ASSERT(p != NULL);
            pool.add(p);
        }
    }
    return p->addSpaces(1);
}



SyntaxTree::Childs::Childs(hyu32 capacity)
    : TArray<SyntaxTree*>(capacity)
{
}

SyntaxTree::Childs::~Childs()
{
}

void* SyntaxTree::Childs::operator new(size_t size)
{
    HMD_ASSERT(size == sizeof(Childs));
    if (m_usePool) {
        return (void*) poolAlloc<Childs>(m_childsMemPool);
    } else {
        return (void*) gMemPool->alloc(sizeof(Childs), "stch");
    }
}

void SyntaxTree::Childs::operator delete(void* p)
{
    if (! m_usePool) {
        gMemPool->free(p);
    }
    // else   delete all by finalizePool()
}

// delete all SyntaxTree recursively, and then delete itself
void SyntaxTree::Childs::deleteRecursively(SyntaxTree::Childs* p)
{
    if (m_usePool)
        return;
    if (p == NULL)
        return;
    // HMD_PRINTF("delete childs %x\n",p);
    TArrayIterator<SyntaxTree*> itr(p);
    while (itr.hasMore()) {
        SyntaxTree* st = itr.next();
        if (st != NULL)
            SyntaxTree::deleteRecursively(st);
    }
    delete p;
}



SyntaxTree::SyntaxTree(Valid_e v)
    : str(0,0), parser(NULL), errorCutId(0),
      chooseNum(-1), m_valid(v)
{
    //HMD_PRINTF("new(%x) null\n",this);
}

SyntaxTree::SyntaxTree(Parser* p, hyu32 startPos, hyu32 endPos, SyntaxTree::Childs* ch)
    : str(startPos, endPos), parser(p), childs(ch),
      chooseNum(-1), m_valid(V_VALID)
{
    //HMD_PRINTF("new(%x) st=%d\n",this,startPos);
}

SyntaxTree::SyntaxTree(Parser* p, Substr& ss, SyntaxTree::Childs* ch)
    : str(ss), parser(p), childs(ch),
      chooseNum(-1), m_valid(V_VALID)
{
    //HMD_PRINTF("new(%x) st=%d\n",this,ss.startPos);
}

SyntaxTree::SyntaxTree(Parser* p, Substr& ss, int errId)
    : str(ss), parser(p), errorCutId(errId),
      chooseNum(-1), m_valid(V_ERRORCUT)
{
    //HMD_PRINTF("new(%x) errId=%d\n",this,errId);
}

SyntaxTree::SyntaxTree(hyu32 errPos, int errId, Parser* parser)
    : str(errPos, errPos), parser(parser), errorCutId(errId),
      chooseNum(-1), m_valid(V_ERROR)
{
    //HMD_PRINTF("new(%x) errId=%d\n",this,errId);
}

SyntaxTree::~SyntaxTree()
{
    // メモ化オン時には、SyntaxTree::finalizePool() が一括削除する。
    // 個別ではdeleteしない。
    // SyntaxTree::finalizePool() は ~SyntaxTree() を呼ばない。

    // メモ化オフ時用デストラクタ
    //if (m_valid == V_VALID)
    //    if (childs != NULL)
    //        delete childs;
}

void* SyntaxTree::operator new(size_t size)
{
    HMD_ASSERT(size == sizeof(SyntaxTree));
    if (m_usePool) {
        return (void*) poolAlloc<SyntaxTree>(m_syntaxTreeMemPool);
    } else {
        return (void*) gMemPool->alloc(sizeof(SyntaxTree), "st  ");
    }
}

void SyntaxTree::operator delete(void* p)
{
    if (! m_usePool) {
        gMemPool->free(p);
    }
    // else   delete all by finalizePool()
}

void SyntaxTree::initializePool(bool usePool)
{
    m_usePool = usePool;
    if (usePool) {
        m_syntaxTreeMemPool.initialize(0);
        m_childsMemPool.initialize(0);
    }
}

void SyntaxTree::finalizePool(void)
{
    if (! m_usePool)
        return;

    // ~SyntaxTree() を呼ばない。
    TArrayIterator<TArray<SyntaxTree>* > itr(&m_syntaxTreeMemPool);
    while (itr.hasMore()) {
        delete itr.next();
    }
    m_syntaxTreeMemPool.finalize();

    TArrayIterator<TArray<Childs>* > itr2(&m_childsMemPool);
    while (itr2.hasMore()) {
        TArray<Childs>* carr = itr2.next();
        hyu32 n = carr->size();
        for (hyu32 i = 0; i < n; ++i) {
            Childs& cr = carr->nth(i);
            cr.finalize();
        }
        delete carr;
    }
    m_childsMemPool.finalize();
}

SyntaxTree* SyntaxTree::erase(int idx)
{
    HMD_ASSERT(isValidTree());
    HMD_ASSERT(childs != NULL);
    HMD_ASSERT((hyu32)idx < childs->size());
    SyntaxTree*& stref = childs->nth(idx);
    SyntaxTree* st = stref;
    stref = NULL;
    if (st->isValidTree()) {
        delete st;
    }
    return this;
}

SyntaxTree* SyntaxTree::shrink(void)
{
    HMD_ASSERT(isValidTree());
    if (childs != NULL)
        childs->deleteVal(NULL);
    return this;
}

void SyntaxTree::deleteAllChild(void)
{
    HMD_ASSERT(isValidTree());
    if (childs == NULL)
        return;

    TArrayIterator<SyntaxTree*> itr(childs);
    while (itr.hasMore()) {
        SyntaxTree* st = itr.next();
        if (st != NULL)
            delete st;
    }
    delete childs;
    childs = NULL;
}

void SyntaxTree::deleteRecursively(SyntaxTree* p)
{
    if (m_usePool)
        return;
    if (p == NULL)
        return;
    if (! p->isValidTree())
        return;
    // HMD_PRINTF("delete st %x\n",p);
    Childs::deleteRecursively(p->childs);
    delete p;
}
