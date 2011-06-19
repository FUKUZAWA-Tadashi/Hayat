
#ifndef m_OPERATOR_H_
#define m_OPERATOR_H_

#include "hyArray.h"
#include "hpSubstr.h"


using namespace Hayat::Common;

namespace Hayat {
    namespace Parser {

        class Operator {

        public:
            Operator(void);
            ~Operator();

            // register operator precedence 'prec' of rule 'sIdent' 
            bool entry(Substr& sIdent, hyu32 prec);

            // sort all precedence 
            void sort(void);

            // return 1 level higher than prec
            // if none, return 0
            hyu32 getHigher(Substr& sIdent, hyu32 prec);

            // return true if sIdent is target rule name
            bool isOperand(Substr& sIdent);

            TArray<Substr>* expIdents(void) { return &m_expIdents; }

            // get precedence array of rule 'sIdent'
            TArray<hyu32>* getPrecs(Substr& sIdent);

        protected:
            TArray<Substr>          m_expIdents; // target rule names 
            TArray<TArray<hyu32>* >   m_precs; // precedence arrays per rule 

            // index of sIdent; if none return -1
            int m_getIdx(Substr& sIdent);

        };
    }
}

#endif /* m_OPERATOR_H_ */
