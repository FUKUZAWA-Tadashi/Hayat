
#include "hpOperator.h"
#include "hpParser.h"


using namespace Hayat::Common;
using namespace Hayat::Parser;


Operator::Operator(void) : m_expIdents(1), m_precs(1)
{
}

Operator::~Operator()
{
    TArrayIterator<TArray<hyu32>* > it(&m_precs);
    while (it.hasMore())
        delete it.next();
    m_precs.clear();
}


bool Operator::entry(Substr& sIdent, hyu32 prec)
{
    if (prec <= 0) {
        fprintf(stderr, "ERROR: operator precedence must > 0\n");
        return false;
    }
    TArray<hyu32>* precArr;
    int i = m_getIdx(sIdent);
    if (i < 0) {
        m_expIdents.add(sIdent);
        precArr = new TArray<hyu32>(4);
        m_precs.add(precArr);
    } else {
        precArr = m_precs[i];
    }

    int precsSize = precArr->size();
    for (i = 0; i < precsSize; ++i) {
        if (precArr->nth(i) == prec){
            char n[256];
            gpInp->copyStr(n, 256, sIdent);
            fprintf(stderr, "ERROR: duplicate operator definition\n");
            fprintf(stderr, "   rule = %s, prec = %d\n",n, prec);
            return false;
        }
    }
    precArr->add(prec);
    return true;
}

void Operator::sort(void)
{
    int opSize = m_expIdents.size();
    for (int i = 0; i < opSize; ++i) {
        TArray<hyu32>* precArr = m_precs[i];
        int precsSize = precArr->size();
        for (int j = 0; j < precsSize - 1; ++j) {
            hyu32 p = precArr->nth(j);
            hyu32 pm = p;
            int x = j;
            for (int k = j+1; k < precsSize; ++k) {
                hyu32 p2 = precArr->nth(k);
                if (p2 < pm) {
                    x = k;
                    pm = p2;
                }
            }
            if (x > j) {
                precArr->nth(j) = pm;
                precArr->nth(x) = p;
            }
        }
    }
}

hyu32 Operator::getHigher(Substr& sIdent, hyu32 prec)
{
    TArray<hyu32>* precArr = getPrecs(sIdent);
    HMD_ASSERT(precArr != NULL);
    int precsSize = precArr->size();
    for (int i = 0; i < precsSize; ++i) {
        if (precArr->nth(i) == prec){
            if (i == precsSize-1)
                return 0;
            return precArr->nth(i+1);
        }
    }
    HMD_ASSERT(false);
    return 0;
}

bool Operator::isOperand(Substr& sIdent)
{
    return (m_getIdx(sIdent) >= 0);
}

TArray<hyu32>* Operator::getPrecs(Substr& sIdent)
{
    int idx = m_getIdx(sIdent);
    if (idx >= 0)
        return m_precs[idx];
    return NULL;
}

int Operator::m_getIdx(Substr& sIdent)
{
    int opSize = m_expIdents.size();
    for (int i = 0; i < opSize; ++i) {
        Substr s = m_expIdents[i];
        if (gpInp->cmpStr(sIdent, s))
            return i;
    }
    return -1;
}
