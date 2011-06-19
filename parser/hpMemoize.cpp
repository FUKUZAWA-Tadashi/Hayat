
#include "hpMemoize.h"
#include "hpSyntaxTree.h"


using namespace Hayat::Parser;


Memoize::Memoize(void)
    : m_memoArr()
{}

Memoize::~Memoize()
{
    m_memoArr.clear();
}

#define USE_BINSEARCH 0

#if USE_BINSEARCH
int Memoize::m_search(hyu32 pos)
{
    int i = 0;
    int j = m_memoArr.size();
    while (i < j) {
        int k = (i + j) / 2;
        hyu32 kPos = m_memoArr[k].pos;
        if (kPos == pos)
            return k;
        if (kPos < pos)
            i = k + 1;
        else
            j = k;
    }    
    return j;
}

#else
int Memoize::m_search(hyu32 pos)
{
    int msize = m_memoArr.size();
    for (int i = msize - 1; i >= 0; --i) {
        hyu32 kPos = m_memoArr[i].pos;
        if (kPos == pos)
            return i;
        if (kPos < pos) {
            return i+1;
        }
    }
    return msize;
}
#endif

SyntaxTree* Memoize::getAt(hyu32 pos)
{
    int idx = m_search(pos);
    if ((hyu32)idx >= m_memoArr.size())
        return m_NOT_PARSED_YET;
    Memo m = m_memoArr[idx];
    if (m.pos == pos)
        return m.st;
    return m_NOT_PARSED_YET;
}

void Memoize::setAt(hyu32 pos, SyntaxTree* st)
{
    int idx = m_search(pos);
    if ((hyu32)idx >= m_memoArr.size()) {
        Memo m = {pos, st};
        m_memoArr.add(m);
    } else {
        Memo m = m_memoArr[idx];
        m.st = st;
        if (m.pos == pos) {
            m_memoArr[idx] = m;
        } else {
            m.pos = pos;
            m_memoArr.insert(idx) = m;
        }
    }
}

void Memoize::remove(SyntaxTree* st)
{
    for (int i = 0; (hyu32)i < m_memoArr.size(); ) {
        if (m_memoArr[i].st == st) {
            m_memoArr.remove(i);
            continue;
        }
        ++i;
    }
}
