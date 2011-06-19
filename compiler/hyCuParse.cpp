/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "machdep.h"
#include "grammar.tab.h"
#include "hyCCompileError.h"

using namespace Hayat::Compiler;


static const wchar_t ZEN_SPACE = (wchar_t)0x3000; // L'　'
static const wchar_t ZEN_BRACKET_OPEN = (wchar_t)0x300c; // L'「'
static const wchar_t ZEN_BRACKET_CLOSE = (wchar_t)0x300d; // L'」'



static bool m_unclosed_comment;

static bool m_skip_comment1(void)
{
    if (! gpInp->cmpStr("//"))
        return false;
    gpInp->advance(2);
    gpInp->skip_beforeEol();
    return true;
}

static bool m_skip_comment2(void)
{
    m_unclosed_comment = false;
    if (! gpInp->cmpStr("/*"))
        return false;
    gpInp->advance(2);
    while (! gpInp->cmpStr("*/")) {
        if (! m_skip_comment2()) {
            gpInp->advance(1);
            if (gpInp->isEOF()) {
                m_unclosed_comment = true;
                return false;
            }
        }
    }
    gpInp->advance(2);
    return true;
}

bool hyC_isSpace(wchar_t c)
{
    return (c == L' '
            || c == L'\t'
            || c == L'\r'
            || c == L'　'	// 全角スペース 
            );
}


static bool m_skip_spcChar(void)
{
    wchar_t c;
    hyu32 spos = gpInp->getPos();
    hyu32 mpos;
    m_unclosed_comment = false;
    do {
        mpos = gpInp->getPos();
        do {
            c = gpInp->getChar();
        } while (hyC_isSpace(c));
        gpInp->ungetChar();
        if (gpInp->cmpStr("\\\r\n"))
            gpInp->advance(3);
        else if (gpInp->cmpStr("\\\r"))
            gpInp->advance(2);
        else if (gpInp->cmpStr("\\\n"))
            gpInp->advance(2);
        m_skip_comment1();
        m_skip_comment2();
        if (m_unclosed_comment)
            return false;
    } while (mpos != gpInp->getPos());

    return (spos != gpInp->getPos());
}

static bool isHex1(wchar_t c)
{
    return ((c >= L'0' && c <= L'9') || (c >= L'A' && c <= L'F') || (c >= L'a' && c <= L'f'));
}

//@ #XChar <- %XCharH / %XCharE / %XCharA
//@ #SChar <- %XCharH / %XCharE / %XCharS
//@ #XCharH <- %"\\" %("x" / "X") Hex1 Hex1
//@ #XCharE <- %"\\" .
//@ #XCharA <- !("\n" / "\r") %.
//@ #XCharS <- !"{" %.
// (!closeStr XChar)*  /  (!closeStr SChar)*
static bool m_skip_string(const wchar_t* closeStr, bool bSay)
{
    wchar_t c;
    hyu32 xpos;
    for (;;) {
        if (gpInp->isEOF()) return false;
        xpos = gpInp->getPos();
        for(const wchar_t* p = closeStr;  ; ++p) {
            if (*p == (wchar_t)0) {
                gpInp->setPos(xpos);
                return true;
            }
            if (*p != gpInp->getChar())
                break;
            if (gpInp->isEOF()) return false; 
        }
        gpInp->setPos(xpos);
        c = gpInp->getChar();
        if (c == L'\\') {
            if (gpInp->isEOF()) return false;
            c = gpInp->getChar();
            if (c == L'x' || c == L'X') {
                if (gpInp->isEOF()) return false;
                hyu32 xpos = gpInp->getPos();
                c = gpInp->getChar();
                if (! isHex1(c)) {
                    gpInp->setPos(xpos);
                } else {
                    if (gpInp->isEOF()) return false;
                    c = gpInp->getChar();
                    if (! isHex1(c)) {
                        gpInp->setPos(xpos);
                    }
                    // else XCharH OK
                }
            } // else XCharE OK
        } else if (!bSay && (c == L'\n' || c == L'\r')) {
            // CRLF cannot be in "STRING"
            gpInp->ungetChar();
            return false;
        } else if (bSay && c == L'{') {
            gpInp->ungetChar();
            return true;
        }
    }
}

#if 0
static bool m_skip_sayString(const char* closeStr)
{
    wchar_t c;
    size_t len = HMD_STRLEN(closeStr);
    hyu32 spos = gpInp->getPos();
    for (;;) {
        if (gpInp->remain() >= len) {
            if (!strncmp(closeStr, gpInp->addr(), len))
                break;
        }
        if (gpInp->isEOF())
            break;
        c = gpInp->getChar();
        if (c == L'\\') {
            if (gpInp->isEOF())
                break;
            gpInp->getChar();
        } else if (c == L'{') {
            gpInp->ungetChar();
            break;
        }
    }
    return (spos != gpInp->getPos());
}
#endif



//============================================================
//@ # 空白,改行 
//@ #spcChar         <- " " / "\t" / "\r" / "　" / "\\\r\n" / "\\\r" / "\\\n" / comment
//@ s               <% %spcChar*
SyntaxTree* P_s::uParse(void)
{
    m_ErrorPos startPos = m_curErrPos();
    hyu32 midPos;
    do {
        midPos = m_curPos();
        if (! m_skip_spcChar()) {
            if (m_unclosed_comment) {
                compileError_pos(startPos.parsePos, M_M("unclosed comment"));
                return m_FATAL_PARSER_ERROR;
            }
        }
    } while (midPos != m_curPos());
    return createSyntaxTree(startPos.parsePos);
}


//============================================================
//@ spc             <% %spcChar+
SyntaxTree* P_spc::uParse(void)
{
    m_ErrorPos startPos = m_curErrPos();
    hyu32 midPos;
    do {
        midPos = m_curPos();
        if (! m_skip_spcChar()) {
            if (m_unclosed_comment) {
                compileError_pos(startPos.parsePos, M_M("unclosed comment"));
                return m_FATAL_PARSER_ERROR;
            }
        }
    } while (midPos != m_curPos());
    if (startPos.parsePos != m_curPos())
        return createSyntaxTree(startPos.parsePos);
    m_fail(startPos);
    return m_PARSE_FAILED;
}


//============================================================
//@ z               <% %(spcChar / "\n")*
SyntaxTree* P_z::uParse(void)
{
    m_ErrorPos startPos = m_curErrPos();
    hyu32 midPos;
    wchar_t c;
    do {
        midPos = m_curPos();
        do {
            c = gpInp->getChar();
        } while (c == '\r' || c == '\n');
        gpInp->ungetChar();
        if (! m_skip_spcChar()) {
            if (m_unclosed_comment) {
                compileError_pos(startPos.parsePos, M_M("unclosed comment"));
                return m_FATAL_PARSER_ERROR;
            }
        }
    } while (midPos != m_curPos());
    return createSyntaxTree(startPos.parsePos);
}


//============================================================
//@ # コメント 
//@ comment         <% %comment1 / %comment2
//@ #comment1        <- %"//" (!("\r" / "\n") %.)*
//@ #comment2        <- %"/*" (!"*/" (%comment2 / %.))* %"*/"
SyntaxTree* P_comment::uParse(void)
{
    m_ErrorPos startPos = m_curErrPos();
    if (m_skip_comment1())
        return createSyntaxTree(startPos.parsePos);
    if (m_skip_comment2())
        return createSyntaxTree(startPos.parsePos);
    if (m_unclosed_comment) {
        m_fail(startPos);
        return m_FATAL_PARSER_ERROR;
    }
    return m_PARSE_FAILED;
}


//============================================================
//@ sayString1      <% (!"」" %SChar)+
//@ #SChar <- %XCharH / %XCharE / %XCharS
SyntaxTree* P_sayString1::uParse(void)
{
    m_ErrorPos startPos = m_curErrPos();
    if (m_skip_string(L"」", true))
        if (gpInp->getPos() != startPos.parsePos)
            return createSyntaxTree(startPos.parsePos);
    m_fail(startPos);
    return m_PARSE_FAILED;
}

//============================================================
//@ sayString2      <% (!">>" %SChar)+
//@ #SChar <- %XCharH / %XCharE / %XCharS
SyntaxTree* P_sayString2::uParse(void)
{
    m_ErrorPos startPos = m_curErrPos();
    if (m_skip_string(L">>", true))
        if (gpInp->getPos() != startPos.parsePos)
            return createSyntaxTree(startPos.parsePos);
    m_fail(startPos);
    return m_PARSE_FAILED;
}





//============================================================
//#not_ident <- %"　" / %"「" / %"」"
//#ident_u1 <- %[A-Z] / %[a-z] / %"_" / !not_ident %[\u0080-\uffff]
//#ident_u2 <- %ident_u1 / %[0-9]
//#ident_u3 <- %"?" / %"!"
static bool is_ident_u1(wchar_t wc)
{
    return (wc >= L'A' && wc <= L'Z')
        || (wc >= L'a' && wc <= L'z')
        ||  wc == L'_'
        || ((wc >= 0x0080)
            && (wc != ZEN_SPACE)
            && (wc != ZEN_BRACKET_OPEN)
            && (wc != ZEN_BRACKET_CLOSE));
}
static bool is_ident_u2(wchar_t wc)
{
    if (is_ident_u1(wc))
        return true;
    return (wc >= L'0' && wc <= L'9');
}
static bool is_ident_u3(wchar_t wc)
{
    return (wc == L'?' || wc == L'!');
}

//============================================================
//@ IDENT           <% %ident_u1 %ident_u2* %ident_u3?
SyntaxTree* P_IDENT::uParse(void)
{
    m_ErrorPos startPos = m_curErrPos();
    if (gpInp->isEOF()) {
        m_fail(startPos);
        return m_PARSE_FAILED;
    }
    wchar_t wc = gpInp->getChar();
    if (! is_ident_u1(wc)) {
        m_fail(startPos);
        return m_PARSE_FAILED;
    }
    do {
        if (gpInp->isEOF())
            break;
        wc = gpInp->getChar();
    } while (is_ident_u2(wc));
    if ((! gpInp->isEOF()) && (! is_ident_u3(wc)))
        gpInp->ungetChar();
    return createSyntaxTree(startPos.parsePos);    
}


//============================================================
//@ IDENT_cpp       <% (%[A-Z] / %[a-z] / %"_") (%[A-Z] / %[a-z] / %"_" / %[0-9])*
SyntaxTree* P_IDENT_cpp::uParse(void)
{
    m_ErrorPos startPos = m_curErrPos();
    if (gpInp->skip_ident()) {
        return createSyntaxTree(startPos.parsePos);
    }
    m_fail(startPos);
    return m_PARSE_FAILED;
}



//============================================================
//@ SYMBOL          <% %":" (%ident_a2+ %("?" / "!")? / STRING)
SyntaxTree* P_SYMBOL::uParse(void)
{
    m_ErrorPos startPos = m_curErrPos();
    wchar_t c = gpInp->getChar();
    if (c != L':') {
        m_fail(startPos);
        return m_PARSE_FAILED;
    }
    c = gpInp->getChar();
    if (c == L'"') {
        // STRING
        if (m_skip_string(L"\"", false)) {
            c = gpInp->getChar();
            HMD_ASSERT(c == L'"');
            return createSyntaxTree(startPos.parsePos);
        }
        m_fail(startPos);
        return m_PARSE_FAILED;
    } 
    // else   %ident_a2+ %("?" / "!")?
    gpInp->ungetChar();
    bool b = gpInp->skip_decimalNum();    // allow number char at top
    b |= gpInp->skip_ident();
    if (! b) {
        m_fail(startPos);
        return m_PARSE_FAILED;
    }
    c = gpInp->getChar();
    if (c != L'?' && c != L'!')
        gpInp->ungetChar();

    return createSyntaxTree(startPos.parsePos);
}


//============================================================
//@ INTEGER_10      <% %[0-9]+
SyntaxTree* P_INTEGER_10::uParse(void)
{
    m_ErrorPos startPos = m_curErrPos();
    if (gpInp->skip_decimalNum()) {
        return createSyntaxTree(startPos.parsePos);
    }
    m_fail(startPos);
    return m_PARSE_FAILED;
}


//============================================================
//@ STRING          <% %"\"" (!"\"" %XChar)* %"\""
//@ #XChar <- %XCharH / %XCharE / %XCharA
//@ #SChar <- %XCharH / %XCharE / %XCharS
//@ #XCharH <- %"\\" %("x" / "X") Hex1 Hex1
//@ #XCharE <- %"\\" .
//@ #XCharA <- !("\n" / "\r") %.
//@ #XCharS <- !"{" %.
SyntaxTree* P_STRING::uParse(void)
{
    m_ErrorPos startPos = m_curErrPos();
    wchar_t c = gpInp->getChar();
    if (c != L'"') {
        m_fail(startPos);
        return m_PARSE_FAILED;
    }

    hyu32 top = m_curPos();
    if (m_skip_string(L"\"", false)) {
        SyntaxTree::Childs* childs = new SyntaxTree::Childs(1);
        childs->add(new SyntaxTree(this, top, m_curPos(), NULL));
        wchar_t c = gpInp->getChar();
        HMD_ASSERT(c == L'"');
        return createSyntaxTree(startPos.parsePos, childs);
    }

    compileError_pos(startPos.parsePos, M_M("unclosed STRING"));
    return m_FATAL_PARSER_ERROR;
}

//============================================================
//@ # not token predicate
//@ NotTokenPred <% !ident_u1
SyntaxTree* P_NotTokenPred::uParse(void)
{
    wchar_t c = gpInp->getChar();
    gpInp->ungetChar();
    if ( (c >= L'A' && c <= L'Z')
         || (c >= L'a' && c <= L'z')
         ||  c == L'_'
         || (c >= L'0' && c <= L'9')
         || ((c >= 0x0080) && (c != ZEN_SPACE) && (c != ZEN_BRACKET_OPEN) && (c != ZEN_BRACKET_CLOSE))
         )
        return m_PARSE_FAILED;
    return m_NO_SYNTAX_TREE;
}
