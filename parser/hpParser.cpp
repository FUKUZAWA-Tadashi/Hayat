#include "hpParser.h"


extern void p__init(void); // initialize generated parsers

using namespace Hayat::Common;
using namespace Hayat::Parser;


TArray<Parser*>*    Parser::m_holder = NULL;
TArray<Parser::Error_t>*    Parser::m_errors = NULL;
int                 Parser::m_printIntermediateLevel = 0;
bool                Parser::m_bUseMemoize = true;
SyntaxTree::Childs* const Parser::m_INFIX_COMMON_FATAL_ERROR = (SyntaxTree::Childs*)1;

/*static*/ const char* Parser::m_sequenceName = "(seq)";
/*static*/ const char* Parser::m_orderedChoiceName = "(choice)";

Parser::Parser(void)
{
    HMD_ASSERT(m_holder != NULL); // Parser::initialize() needed
    m_holder->add(this);
}


SyntaxTree* Parser::createSyntaxTree(hyu32 startPos, SyntaxTree::Childs* childs)
{
    return new SyntaxTree(this, startPos, m_curPos(), childs);
}

SyntaxTree* Parser::createSyntaxTree(hyu32 startPos, hyu32 endPos, SyntaxTree::Childs* childs)
{
    return new SyntaxTree(this, startPos, endPos, childs);
}

void Parser::initialize(int initHolderCapacity)
{
    HMD_ASSERT(m_holder == NULL);
    m_holder = new TArray<Parser*>(initHolderCapacity);
    m_holder->setContentsMemID("Phol");
    m_errors = new TArray<Error_t>(10);
    m_errors->setContentsMemID("Perr");
    p__init();
}

void Parser::finalize(void)
{
    TArrayIterator<Parser*> itr(m_holder);
    while (itr.hasMore())
        delete itr.next();
    delete m_holder;
    m_holder = NULL;
    delete m_errors;
    m_errors = NULL;
}

bool Parser::cmpStr(SyntaxTree* st, const char* str)
{
    return gpInp->cmpStr(st->str, str);
}
    
void Parser::addSequenceToParent(TArray<Parser*>* parentSeq)
{
    parentSeq->add(this);
}

void Parser::addSyntaxTreeToParent(SyntaxTree::Childs* arr, SyntaxTree* st)
{
    if (st->isValidTree()) {
        arr->add(st);
    }
}

void Parser::m_reduceSyntaxTreeToParent(SyntaxTree::Childs* arr, SyntaxTree* st)
{
    if (! st->isValidTree())
        return;
    for (TArrayIterator<SyntaxTree*> itr(st->childs); itr.hasMore(); ) {
        SyntaxTree* t = itr.next();
        if (t->isValidTree()) {
            t->parser->addSyntaxTreeToParent(arr, t);
        }
    }
    if (! m_bUseMemoize) {
        // no more need
        if (st->childs != NULL)
            delete st->childs;
        delete st;
    }
}

Parser* Parser::seq(Parser* p)
{
    if (name() == m_sequenceName) {
        ((Parser_Sequence*)this)->add(p);
        return this;
    }
    return new Parser_Sequence(this, p);
}

Parser* Parser::choice(Parser* p)
{
    if (name() == m_orderedChoiceName) {
        ((Parser_OrderedChoice*)this)->add(p);
        return this;
    }
    return new Parser_OrderedChoice(this, p);
}

Parser* Parser::star(void)
{
    return new Parser_ZeroOrMore(this);
}
    
Parser* Parser::plus(void)
{
    return new Parser_OneOrMore(this);
}
    
Parser* Parser::optional(void)
{
    return new Parser_Optional(this);
}
    
Parser* Parser::andPred(void)
{
    return new Parser_AndPredicate(this);
}
    
Parser* Parser::notPred(void)
{
    return new Parser_NotPredicate(this);
}

Parser* Parser::noTree(void)
{
    return new Parser_NoTree(this);
}




Parser::m_ErrorPos Parser::m_curErrPos(void)
{
    m_ErrorPos errPos;
    errPos.parsePos = m_curPos();
    errPos.errIdx = m_errors->size();
    return errPos;
}

void Parser::m_back(Parser::m_ErrorPos& errPos)
{
    m_errors->chop(errPos.errIdx);
    gpInp->setPos(errPos.parsePos);
}

void Parser::m_fail(Parser::m_ErrorPos& errPos)
{
    Error_t err = {this, m_curPos()};
    m_errors->add(err);
    gpInp->setPos(errPos.parsePos);
}

SyntaxTree* Parser::m_parse_sequence(TArray<Parser*>* ps)
{
    m_ErrorPos startPos = m_curErrPos();
    SyntaxTree::Childs* childs = new SyntaxTree::Childs(ps->size());
    childs->setContentsMemID("pcsq");
    int errorCutId = 0;
    hyu32 errorCutPos = 0;

    for (TArrayIterator<Parser*> itr(ps); itr.hasMore(); ) {
        Parser* p = itr.next();
        SyntaxTree* st = p->parse();
        if (st->isErrorCut()) {
            errorCutId = st->errorCutId;
            errorCutPos = st->str.startPos;
            delete st;
        } else if (! st->isFail()) {
            p->addSyntaxTreeToParent(childs, st);
        } else {
            if (st->isFailNotError() && errorCutId > 0) {
                SyntaxTree* e = new SyntaxTree(errorCutPos, errorCutId, this);
                if (! m_bUseMemoize)
                    delete st;
                st = e;
            }
            m_fail(startPos);
            if (! m_bUseMemoize) {
                SyntaxTree::Childs::deleteRecursively(childs);
            } else {
                delete childs;
            }
            return st;
        }
    }
    return createSyntaxTree(startPos.parsePos, childs);
}
    
SyntaxTree* Parser::m_parse_orderedChoice(TArray<Parser*>* ps)
{
    m_ErrorPos startPos = m_curErrPos();

    int i = 0;
    for (TArrayIterator<Parser*> itr(ps); itr.hasMore(); ++i) {
        Parser* p = itr.next();
        SyntaxTree* st = p->parse();
        if (st->isError()) {
            m_fail(startPos);
            return st;
        }
        if (! st->isFail()) {
            SyntaxTree::Childs* childs = new SyntaxTree::Childs(1);
            childs->setContentsMemID("pcoc");
            p->addSyntaxTreeToParent(childs, st);
            SyntaxTree* ordst = createSyntaxTree(startPos.parsePos, childs);
            ordst->chooseNum = i;
            return ordst;
        }
        if (itr.hasMore())
            m_back(startPos);
    }
    m_fail(startPos);
    return m_PARSE_FAILED;
}
    
SyntaxTree* Parser::m_parse_m2n(Parser* p, int m, int n)
{
    int count = 0;
    m_ErrorPos startPos = m_curErrPos();
    SyntaxTree::Childs* childs = new SyntaxTree::Childs(m > 0 ? m : 1);
    childs->setContentsMemID("pcmn");
    for (;;) {
        SyntaxTree* st = p->parse();
        if (st->isError()) {
            m_fail(startPos);
            return st;
        }
        if (! st->isFail()) {
            p->addSyntaxTreeToParent(childs, st);
            if (++count >= MAX_MANY) count = MAX_MANY - 1;
            if (count >= n)
                break;      // all ok
        } else {
            break;
        }
    }
    if (count < m) {
        m_fail(startPos);
        if (! m_bUseMemoize) {
            SyntaxTree::Childs::deleteRecursively(childs);
        } else {
            delete childs;
        }
        return m_PARSE_FAILED;
    }
    m_errors->clear();
    return createSyntaxTree(startPos.parsePos, childs);
}

SyntaxTree* Parser::m_parse_andPredicate(Parser* p)
{
    m_ErrorPos startPos = m_curErrPos();
    SyntaxTree* st = p->parse();
    if (st->isFail()) {
        m_fail(startPos);
        return m_PARSE_FAILED;
    }
    m_back(startPos);
    SyntaxTree::deleteRecursively(st);
    return m_NO_SYNTAX_TREE;
}
    
SyntaxTree* Parser::m_parse_notPredicate(Parser* p)
{
    m_ErrorPos startPos = m_curErrPos();
    SyntaxTree* st = p->parse();
    if (st->isFatalError()) {
        m_fail(startPos);
        return st;
    }
    m_back(startPos);
    if (! st->isFail()) {
        if (! m_bUseMemoize) {
            SyntaxTree::deleteRecursively(st);
        }
        return m_PARSE_FAILED;
    }
    return m_NO_SYNTAX_TREE;
}

SyntaxTree::Childs* Parser::m_parse_infix_common(Parser* exp, Parser* op, bool rep)
{
    // Exp (Op Exp)* のパース
    // rep が false なら Exp (Op Exp)? のパース
    // 結果はSyntaxTreeの配列で [exp op exp op exp ....]
    m_ErrorPos startPos = m_curErrPos();
    SyntaxTree* st = exp->parse();
    if (st->isError()) {
        m_fail(startPos);
        return m_INFIX_COMMON_FATAL_ERROR;
    }
    if (st->isFail()) {
        m_fail(startPos);
        return NULL;
    }
    SyntaxTree::Childs* childs = new SyntaxTree::Childs(3);
    childs->setContentsMemID("pcif");
    childs->add(st);
    //exp->addSyntaxTreeToParent(childs, st);
    for (;;) {
        m_ErrorPos midPos = m_curErrPos();
        SyntaxTree* sto = op->parse();
        if (sto->isError()) {
            m_fail(startPos);
            if (! m_bUseMemoize) {
                SyntaxTree::Childs::deleteRecursively(childs);
            } else {
                delete childs;
            }
            return m_INFIX_COMMON_FATAL_ERROR;
        }
        if (sto->isFail()) {
            m_back(midPos);
            break;
        }
        SyntaxTree* ste = exp->parse();
        if (ste->isError()) {
            m_fail(startPos);
            if (! m_bUseMemoize) {
                SyntaxTree::Childs::deleteRecursively(childs);
            } else {
                delete childs;
            }
            return m_INFIX_COMMON_FATAL_ERROR;
        }
        if (ste->isFail()) {
            if (! m_bUseMemoize)
                SyntaxTree::deleteRecursively(sto);
            m_back(midPos);
            break;
        }
        op->addSyntaxTreeToParent(childs, sto);
        exp->addSyntaxTreeToParent(childs, ste);
        if (!rep)
            break;
    }
    return childs;
}

SyntaxTree* Parser::m_parse_op_infixl(Parser* my, Parser* exp, Parser* op)
{
    SyntaxTree::Childs* childs = m_parse_infix_common(exp, op, true);
    if (childs == m_INFIX_COMMON_FATAL_ERROR)
        return m_FATAL_PARSER_ERROR;
    if (childs == NULL)
        return m_PARSE_FAILED;
    int size = childs->size();
    if (size == 1) {
        SyntaxTree* tmp = childs->replace(0, NULL);
        delete childs;
        return tmp;
    }
    SyntaxTree* st = childs->replace(0, NULL);
    for (int i = 1; i < size; i += 2) {
        SyntaxTree* sto = childs->replace(i, NULL);
        SyntaxTree* ste = childs->replace(i+1, NULL);
        SyntaxTree::Childs* node = new SyntaxTree::Childs(3);
        node->setContentsMemID("pcil");
        node->add(st); node->add(sto); node->add(ste);
        st = my->createSyntaxTree(st->str.startPos, ste->str.endPos, node);
    }
    delete childs;
    return st;
}
    
SyntaxTree* Parser::m_parse_op_infixr(Parser* my, Parser* exp, Parser* op)
{
    SyntaxTree::Childs* childs = m_parse_infix_common(exp, op, true);
    if (childs == m_INFIX_COMMON_FATAL_ERROR)
        return m_FATAL_PARSER_ERROR;
    if (childs == NULL)
        return m_PARSE_FAILED;
    int size = childs->size();
    if (size == 1) {
        SyntaxTree* tmp = childs->replace(0, NULL);
        delete childs;
        return tmp;
    }
    SyntaxTree* st = childs->replace(size-1, NULL);
    for (int i = size-3; i >= 0 ; i -= 2) {
        SyntaxTree* ste = childs->replace(i, NULL);
        SyntaxTree* sto = childs->replace(i+1, NULL);
        SyntaxTree::Childs* node = new SyntaxTree::Childs(3);
        node->setContentsMemID("pcir");
        node->add(ste); node->add(sto); node->add(st);
        Substr ss(ste->str.startPos, st->str.endPos);
        st = my->createSyntaxTree(ste->str.startPos, st->str.endPos, node);
    }
    delete childs;
    return st;
}
    
SyntaxTree* Parser::m_parse_op_infixn(Parser* my, Parser* exp, Parser* op)
{
    SyntaxTree::Childs* childs = m_parse_infix_common(exp, op, false);
    if (childs == m_INFIX_COMMON_FATAL_ERROR)
        return m_FATAL_PARSER_ERROR;
    if (childs == NULL)
        return m_PARSE_FAILED;
    int size = childs->size();
    if (size == 1) {
        SyntaxTree* tmp = childs->replace(0, NULL);
        delete childs;
        return tmp;
    }
    SyntaxTree* st = childs->replace(0, NULL);
    if (size == 3) {
        SyntaxTree* sto = childs->replace(1, NULL);
        SyntaxTree* ste = childs->replace(2, NULL);
        SyntaxTree::Childs* node = new SyntaxTree::Childs(3);
        node->setContentsMemID("pcin");
        node->add(st); node->add(sto); node->add(ste);
        Substr ss(st->str.startPos, ste->str.endPos);
        st = my->createSyntaxTree(st->str.startPos, ste->str.endPos, node);
    }
    delete childs;
    return st;
}
    
SyntaxTree* Parser::m_parse_op_prefix(Parser* my, Parser* exp, Parser* op, bool allowRepeat)
{
    m_ErrorPos startPos = m_curErrPos();
    SyntaxTree::Childs* firstChilds = new SyntaxTree::Childs(2);
    firstChilds->setContentsMemID("pcpr");
    SyntaxTree::Childs* lastChilds = firstChilds;
    for (;;) {
        m_ErrorPos midPos = m_curErrPos();
        SyntaxTree* sto = op->parse();
        if (sto->isError()) {
            m_fail(startPos);
            SyntaxTree::Childs::deleteRecursively(firstChilds);
            return sto;
        }
        if (sto->isFail()) {
            m_back(midPos);
            break;
        } else {
            if (lastChilds->size() > 0) {
                SyntaxTree::Childs* newChilds = new SyntaxTree::Childs(2);
                newChilds->setContentsMemID("pcpr");
                lastChilds->add(my->createSyntaxTree(m_curPos(), newChilds));
                lastChilds = newChilds;
            }
            op->addSyntaxTreeToParent(lastChilds, sto);
            if (! allowRepeat)
                break;
        }
    }
    SyntaxTree* ste = exp->parse();
    if (ste->isFail()) {
        m_fail(startPos);
        SyntaxTree::Childs::deleteRecursively(firstChilds);
        return ste;
    }

    if (lastChilds->size() == 0) {
        // no op
        SyntaxTree::Childs::deleteRecursively(firstChilds);
        return ste;
    }

    lastChilds->add(ste);
    return my->createSyntaxTree(startPos.parsePos, firstChilds);
}

    
SyntaxTree* Parser::m_parse_op_postfix(Parser* my, Parser* exp, Parser* op, bool allowRepeat)
{
    m_ErrorPos startPos = m_curErrPos();
    SyntaxTree* ste = exp->parse();
    if (ste->isFail()) {
        m_fail(startPos);
        return ste;
    }

    for (;;) {
        m_ErrorPos midPos = m_curErrPos();
        SyntaxTree* sto = op->parse();
        if (sto->isError()) {
            m_fail(startPos);
            SyntaxTree::deleteRecursively(ste);
            return sto;
        }
        if (sto->isFail()) {
            m_back(midPos);
            return ste;
        }

        SyntaxTree::Childs* childs = new SyntaxTree::Childs(2);
        childs->setContentsMemID("pcpo");
        exp->addSyntaxTreeToParent(childs, ste);
        op->addSyntaxTreeToParent(childs, sto);
        ste = my->createSyntaxTree(startPos.parsePos, childs);

        if (! allowRepeat)
            return ste;
    }
}

SyntaxTree* Parser::m_parse_op_ternary(Parser* my, Parser* exp, Parser* op1, Parser* op2)
{
    m_ErrorPos startPos = m_curErrPos();
    SyntaxTree* ste = exp->parse();
    if (ste->isFail()) {
        m_fail(startPos);
        return m_PARSE_FAILED;
    }

    SyntaxTree::Childs* childs = new SyntaxTree::Childs(5);
    childs->setContentsMemID("pcte");
    m_ErrorPos midPos = m_curErrPos();
    exp->addSyntaxTreeToParent(childs, ste);
    SyntaxTree* st = op1->parse();
    if (! st->isFail()) {
        op1->addSyntaxTreeToParent(childs, st);
        st = my->parse();
        if (! st->isFail()) {
            my->addSyntaxTreeToParent(childs, st);
            st = op2->parse();
            if (! st->isFail()) {
                op2->addSyntaxTreeToParent(childs, st);
                st = my->parse();
                if (! st->isFail()) {
                    // success
                    my->addSyntaxTreeToParent(childs, st);
                    return my->createSyntaxTree(startPos.parsePos, childs);
                }
            }
        }
    }

    m_back(midPos);
    delete childs;
    return ste;
}

SyntaxTree* Parser::m_parse_noTree(Parser* p)
{
    SyntaxTree* st = p->parse();
    if (st->isFail())
        return st;
    if (! m_bUseMemoize) {
        SyntaxTree::deleteRecursively(st);
    }
    return m_NO_SYNTAX_TREE;
}

SyntaxTree* Parser::m_parse_string(const wchar_t* wstr)
{
    m_ErrorPos startPos = m_curErrPos();
    wchar_t c;
    while (*wstr != (wchar_t)0) {
        try {
            c = gpInp->getChar();
        } catch (hyu32 pos) {
            (void)pos;
            m_fail(startPos);
            return m_PARSE_FAILED;
        }
        if (c != *wstr) {
            m_fail(startPos);
            return m_PARSE_FAILED;
        }
        ++wstr;
    }
    return createSyntaxTree(startPos.parsePos);
}


SyntaxTree* Parser::m_parse_char(wchar_t chr)
{
    m_ErrorPos startPos = m_curErrPos();
    wchar_t c;
    try {
        c = gpInp->getChar();
    } catch (hyu32 pos) {
        (void)pos;
        m_fail(startPos);
        return m_PARSE_FAILED;
    }
    if (c == chr) {
        return createSyntaxTree(startPos.parsePos);
    }
    m_fail(startPos);
    return m_PARSE_FAILED;
}

SyntaxTree* Parser::m_parse_anyChar(void)
{
    m_ErrorPos startPos = m_curErrPos();
    if (gpInp->isEOF()) {
        m_fail(startPos);
        return m_PARSE_FAILED;
    }
    try {
        gpInp->getChar();
    } catch (hyu32 pos) {
        (void)pos;
        m_fail(startPos);
        return m_PARSE_FAILED;
    }
    return createSyntaxTree(startPos.parsePos);
}

SyntaxTree* Parser::m_parse_rangeChar(wchar_t c1, wchar_t c2)
{
    m_ErrorPos startPos = m_curErrPos();
    wchar_t c;
    try {
        c = gpInp->getChar();
    } catch (hyu32 pos) {
        (void)pos;
        m_fail(startPos);
        return m_PARSE_FAILED;
    }
    if ((c >= c1) && (c <= c2)) {
        return createSyntaxTree(startPos.parsePos);
    }
    m_fail(startPos);
    return m_PARSE_FAILED;
}

SyntaxTree* Parser::m_parse_EOF(void)
{
    m_ErrorPos startPos = m_curErrPos();
    if (gpInp->isEOF()) {
        return createSyntaxTree(startPos.parsePos);
    }
    m_fail(startPos);
    return m_PARSE_FAILED;
}


SyntaxTree* Parser_ErrorCut::parse(void)
{
    hyu32 cp = m_curPos();
    Substr ss(cp, cp);
    return new SyntaxTree(this, ss, m_errorCutId);
}

Parser_Sequence::Parser_Sequence(Parser* ps[], int n)
    : Parser()
{
    m_ps = new TArray<Parser*>(n);
    for (int i = 0; i < n; i++) {
        ps[i]->addSequenceToParent(m_ps);
    }
}

Parser_Sequence::Parser_Sequence(Parser* p1, Parser* p2)
    : Parser()
{
    m_ps = new TArray<Parser*>(2);
    m_ps->setContentsMemID("pseq");
    p1->addSequenceToParent(m_ps);
    p2->addSequenceToParent(m_ps);
}

void Parser_Sequence::addSequenceToParent(TArray<Parser*>* parentSeq)
{
    for (TArrayIterator<Parser*> itr(m_ps); itr.hasMore(); )
        parentSeq->add(itr.next());
}

Parser_OrderedChoice::Parser_OrderedChoice(Parser* ps[], int n)
    : Parser()
{
    m_ps = new TArray<Parser*>(n);
    m_ps->setContentsMemID("pord");
    for (int i = 0; i < n; i++)
        m_ps->add(ps[i]);
}

Parser_OrderedChoice::Parser_OrderedChoice(Parser* p1, Parser* p2)
    : Parser()
{
    m_ps = new TArray<Parser*>(2);
    m_ps->setContentsMemID("pord");
    m_ps->add(p1);
    m_ps->add(p2);
}

SyntaxTree* UserParser::parse(void)
{
    m_ErrorPos startPos = m_curErrPos();
    SyntaxTree* st = m_parse1(startPos);
    if (st != NULL)
        return st;

    st = uParse();

    return m_parse2(startPos, st);
}

SyntaxTree* UserParser::m_parse1(m_ErrorPos startPos)
{
    const char* n = name();
    SyntaxTree* st;
        
    if (m_bUseMemoize) {
        st = m_memo.getAt(startPos.parsePos);
        if (st == m_PARSING) {
            // left recursion
            char pbuf[128];
            gpInp->sprintSourceInfo(pbuf, 128, startPos.parsePos);
            HMD_PRINTF("LEFT RECURSION DETECTED: %s %s\n", n, pbuf);
            return m_FATAL_PARSER_ERROR;
        } else if (st != m_NOT_PARSED_YET) {
            if (st->isValidTree())
                gpInp->setPos(st->str.endPos);
            if (m_printIntermediateLevel > 1 ||
                (m_printIntermediateLevel > 0 && !st->isFail())) {
                char pbuf[128];
                gpInp->sprintSourceInfo(pbuf, 128, startPos.parsePos);
                HMD_PRINTF("%s %s -> memoized ", n, pbuf);
                if (st->isValidTree()) {
                    char b[44];
                    gpInp->copySummary(b, 40, st->str);
                    HMD_PRINTF("pos=%d-%d '%s'\n", st->str.startPos, st->str.endPos, b);
                } else {
                    if (st == m_NO_SYNTAX_TREE)
                        HMD_PRINTF("no syntax tree\n");
                    else if (st == m_PARSE_FAILED)
                        HMD_PRINTF("parse failed\n");
                    else if (st->isErrorCut())
                        HMD_PRINTF("ErrorCut(%d)\n",st->errorCutId);
                    else
                        HMD_PRINTF("(UNKNOWN BUG?)\n");
                }
            }
            return st;
        }

        // not parsed yet
        m_memo.setAt(startPos.parsePos, m_PARSING);
    }
        
    if (m_printIntermediateLevel > 1) {
        char pbuf[128];
        gpInp->sprintSourceInfo(pbuf, 128, startPos.parsePos);
        HMD_PRINTF("try %s %s pos=%d:\n", n, pbuf, startPos.parsePos);
    }

    return NULL;
}
    
SyntaxTree* UserParser::m_parse2(m_ErrorPos startPos, SyntaxTree* st)
{
    const char* n = name();

    if (st->isError() || st->isErrorCut()) {
        m_fail(startPos);
        return m_FATAL_PARSER_ERROR;
    }
    if (st->isFail()) {
        if (m_printIntermediateLevel > 1) {
            char pbuf[128];
            gpInp->sprintSourceInfo(pbuf, 128, startPos.parsePos);
            HMD_PRINTF("%s %s -> fail\n", n, pbuf);
        }
        m_fail(startPos);
        if (m_bUseMemoize)
            m_memo.setAt(startPos.parsePos, m_PARSE_FAILED);
        return m_PARSE_FAILED;
    }

    if (m_printIntermediateLevel > 0) {
        char pbuf[128];
        gpInp->sprintSourceInfo(pbuf, 128, startPos.parsePos);
        if (st->isValidTree()) {
            char b[44];
            gpInp->copySummary(b, 40, st->str);
            HMD_PRINTF("%s %s -> pos=%d-%d '%s'\n", n, pbuf, st->str.startPos, st->str.endPos, b);
        } else {
            HMD_PRINTF("%s %s -> (notree)\n", n, pbuf);
        }
    }

    if (m_bUseMemoize)
        m_memo.setAt(startPos.parsePos, st);

    return st;
}

void UserParser::removeMemo(SyntaxTree* st)
{
    if (m_memo.size() < 1)
        return;
    m_memo.remove(st);
}

void UserParser::errorMessage(int errorCutId, hyu32 pos)
{
    SourceInfo si;
    gpInp->buildSourceInfo(&si, pos);
    HMD_PRINTF("%s:%d:%d: error %d in parser %s\n",
               si.fname, si.line, si.col,
               errorCutId, name());
}

SyntaxTree* GenParser::uParse(void)
{
    HMD_ASSERT(m_parser);
    return m_parser->parse();
}

SyntaxTree* GenParser::m_parse2(m_ErrorPos startPos, SyntaxTree* st)
{
    const char* n = name();

    if (st->isError() || st->isErrorCut()) {
        m_fail(startPos);
        if (! st->isFatalError()) {
            errorMessage(st->errorCutId, st->str.startPos);
            //char pbuf[128];
            //gpInp->sprintSourceInfo(pbuf, 128, startPos.parsePos);
            //HMD_PRINTF("%s %s -> ERROR %d\n", n, pbuf, st->errorCutId);
        }
        return m_FATAL_PARSER_ERROR;
    }
    if (st->isFail()) {
        if (m_printIntermediateLevel > 1) {
            char pbuf[128];
            gpInp->sprintSourceInfo(pbuf, 128, startPos.parsePos);
            HMD_PRINTF("%s %s -> fail\n", n, pbuf);
        }
        m_fail(startPos);
        if (m_bUseMemoize)
            m_memo.setAt(startPos.parsePos, m_PARSE_FAILED);
        return m_PARSE_FAILED;
    }

    if (m_printIntermediateLevel > 0) {
        char pbuf[128];
        gpInp->sprintSourceInfo(pbuf, 128, startPos.parsePos);
        if (st->isValidTree()) {
            char b[44];
            gpInp->copySummary(b, 40, st->str);
            HMD_PRINTF("%s %s -> pos=%d-%d '%s'\n", n, pbuf, st->str.startPos, st->str.endPos, b);
        } else {
            HMD_PRINTF("%s %s -> (notree)\n", n, pbuf);
        }
    }
    SyntaxTree::Childs* childs = new SyntaxTree::Childs(1);
    childs->setContentsMemID("pgen");
    m_parser->addSyntaxTreeToParent(childs, st);
    SyntaxTree* myTree = createSyntaxTree(startPos.parsePos, childs);
    if (m_bUseMemoize)
        m_memo.setAt(startPos.parsePos, myTree);
    actionAtParse(myTree);
    return myTree;
}


SyntaxTree* OperatorParser::parse(void)
{
    HMD_ASSERT(m_parser);
    m_ErrorPos startPos = m_curErrPos();
    SyntaxTree* st = m_parser->parse();
    const char* n = name();
    if (st->isError()) {
        m_fail(startPos);
        if (! st->isFatalError()) {
            char pbuf[128];
            gpInp->sprintSourceInfo(pbuf, 128, startPos.parsePos);
            HMD_PRINTF("%s %s -> ERROR %d\n", n, pbuf, st->errorCutId);
        }
        return m_FATAL_PARSER_ERROR;
    }
    if (st->isFail()) {
        if (m_printIntermediateLevel > 2) {
            char pbuf[128];
            gpInp->sprintSourceInfo(pbuf, 128, startPos.parsePos);
            HMD_PRINTF("%s %s -> fail\n", n, pbuf);
        }
        m_fail(startPos);
        return m_PARSE_FAILED;
    }

    if (m_printIntermediateLevel > 1) {
        char pbuf[128];
        gpInp->sprintSourceInfo(pbuf, 128, startPos.parsePos);
        if (st->isValidTree()) {
            char b[44];
            gpInp->copySummary(b, 40, st->str);
            HMD_PRINTF("%s %s -> '%s'\n", n, pbuf, b);
        } else {
            HMD_PRINTF("%s %s -> (notree)\n", n, pbuf);
        }
    }
        
#if 0
    if (st->isValidTree()) {
        if (st->numChild() == 1) {
            // no operator, so that this node is not needed
            SyntaxTree* tmp = st->replace(0, NULL);
            if (! m_bUseMemoize)
                delete st;
            return tmp;
        }
    }
#endif
    return st;
}
