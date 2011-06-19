
#include "pgPegParser.h"


//============================================================
//@ Number		<% %[0-9]+
SyntaxTree* P_Number::uParse(void)
{
    m_ErrorPos startPos = m_curErrPos();
    if (gpInp->skip_decimalNum()) {
        return createSyntaxTree(startPos.parsePos);
    }
    m_fail(startPos);
    return m_PARSE_FAILED;
}

//============================================================
//@ Ident		<% %Ident1 %Ident2*
//@ #Ident1		<% %[a-z] / %[A-Z] / %"_"
//@ #Ident2		<% %Ident1 / %[0-9]
SyntaxTree* P_Ident::uParse(void)
{
    m_ErrorPos startPos = m_curErrPos();
    if (gpInp->skip_ident()) {
        return createSyntaxTree(startPos.parsePos);
    }
    m_fail(startPos);
    return m_PARSE_FAILED;
}

//============================================================
//@ NotTokenPred	<% !Ident2
SyntaxTree* P_NotTokenPred::uParse(void)
{
    wchar_t c = gpInp->getChar();
    gpInp->ungetChar();
    if (c >= L'a' && c <= L'z')
        return m_PARSE_FAILED;
    if (c >= L'0' && c <= L'9')
        return m_PARSE_FAILED;
    if (c >= L'A' && c <= L'Z')
        return m_PARSE_FAILED;
    if (c == L'_')
        return m_PARSE_FAILED;
    return m_NO_SYNTAX_TREE;
}

//============================================================
//@ Comment		<% %"#" %XCharA* %("\n" / "\r")+
SyntaxTree* P_Comment::uParse(void)
{
    m_ErrorPos startPos = m_curErrPos();
    wchar_t c = gpInp->getChar();
    if (c == L'#') {
        gpInp->skip_beforeEol();
        gpInp->skip_eols();
        return createSyntaxTree(startPos.parsePos);
    }
    m_fail(startPos);
    return m_PARSE_FAILED;
}
