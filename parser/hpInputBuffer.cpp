
#include "hpInputBuffer.h"
#include "mbcsconv.h"


using namespace Hayat::Parser;


InputBuffer* Hayat::Parser::gpInp = NULL;


InputBuffer::InputBuffer(void)
    : m_linePos(100)
{
    clean();
}

void InputBuffer::clean(void)
{
    m_buffer = NULL;
    m_bufSize = 0;
    m_ptr = m_prevPtr = NULL;
    m_linePos.clear();
    m_linePos.add(0);    // first line
}

void InputBuffer::advance(hyu32 len)
{
    m_ptr += len;
    const char* endaddr = m_buffer + m_bufSize;
    if (m_ptr > endaddr)
        m_ptr = endaddr;
    m_prevPtr = m_ptr;
}

const char* InputBuffer::addr(hyu32 pos)
{
    if (pos < m_bufSize)
        return m_buffer + pos;
    else
        return m_buffer + m_bufSize;
}

hyu32 InputBuffer::copyStr(char* dest, hyu32 destSize, Substr_st& ss)
{
    const char* adr = addr(ss.startPos);
    hyu32 len = ss.len();
    if (ss.startPos > m_bufSize)
        len = 0;
    else if (ss.startPos + len > m_bufSize)
        len = m_bufSize - ss.startPos;
    if (len > destSize) len = destSize - 1;
    HMD_STRNCPY(dest, adr, len+1);
    return len;
}

void InputBuffer::copySummary(char* dest, hyu32 destSize, Substr_st& ss)
{
    const char* adr = addr(ss.startPos);
    hyu32 len = ss.len();
    if (ss.startPos > m_bufSize)
        len = 0;
    else if (ss.startPos + len > m_bufSize)
        len = m_bufSize - ss.startPos;
    if (len < destSize) {
        HMD_STRNCPY(dest, adr, len+1);
        return;
    }
    hyu32 h = (destSize - 6) / 2 ;
    HMD_STRNCPY(dest, adr, h+1);
    HMD_STRNCPY(dest+h, " ... ", 6);
    HMD_STRNCPY(dest+h+5, adr+len-h, h+1);
    dest[h*2+5] = '\0';
}

void InputBuffer::fprintStr(FILE* fp, Substr_st& ss)
{
    const char* adr = addr(ss.startPos);
    hyu32 len = ss.len();
    if (ss.startPos > m_bufSize)
        return;
    if (ss.startPos + len > m_bufSize)
        len = m_bufSize - ss.startPos;
    while (len-- > 0) {
        fputc(*adr++, fp);
    }
}

void InputBuffer::fprintSummary(FILE* fp, Substr_st& ss)
{
    char buf[80];
    copySummary(buf, 80, ss);
    fprintf(fp, buf);
}


wchar_t InputBuffer::getCharAt(const char** ptr)
{
    const char* endaddr = m_buffer + m_bufSize;
    const char* optr = *ptr;
    if ((optr < m_buffer) || (optr >= endaddr))
        return (wchar_t)WEOF;
    wchar_t wc = MBCSConv::getCharAt(ptr);
    if (*ptr == optr) {
        // error occured
        char buf[128];
        hyu32 pos = getPos();
        sprintSourceInfo(buf, 127, pos);
        HMD_PRINTF("%s: invalid char\n", buf);
        ++*ptr;
    }
    return wc;
}

void InputBuffer::ungetChar(bool check)
{
    // if last getChar() was WEOF, then m_ptr == m_prevPtr
    if (check) {
        HMD_ASSERT(m_ptr != m_prevPtr);
    }
    m_ptr = m_prevPtr;
}

bool InputBuffer::isEOF(void)
{
    return (m_ptr >= m_buffer + m_bufSize);
}

bool InputBuffer::cmpStr(const char* str)
{
    if (str == NULL)
        return false;
    hyu32 len = HMD_STRLEN(str);
    if (remain() < len)
        return false;
    return ::strncmp(m_ptr, str, len) == 0;
}
bool InputBuffer::cmpStr(Substr_st& ss, const char* str)
{
    if (str == NULL)
        return false;
    return ::strncmp(m_buffer + ss.startPos, str, ss.len()) == 0;
}
bool InputBuffer::cmpStr(Substr_st& s1, Substr_st& s2)
{
    if (s1.len() != s2.len())
        return false;
    return ::strncmp(m_buffer + s1.startPos, m_buffer + s2.startPos, s1.len()) == 0;
}
    

bool InputBuffer::skip_beforeEol(void)
{
    wchar_t c;
    hyu32 startPos = getPos();
    do {
        c = getChar();
    } while (c != L'\n' && c != L'\r' && c != (wchar_t)WEOF);
    ungetChar();
    return (startPos != getPos());
}
    
bool InputBuffer::skip_eols(void)
{
    wchar_t c;
    hyu32 startPos = getPos();
    do {
        c = getChar();
    } while (c == L'\n' || c == L'\r');
    ungetChar();

    return (startPos != getPos());
}
    
bool InputBuffer::skip__ident(int topCase, int followCase)
{
    hyu32 start = getPos();
    wchar_t c = getChar();
    switch (topCase) {
    case 0: // lower case
        if (c != L'_' && (c < L'a' || c > L'z')) {
            ungetChar();
            return false;
        }
        break;
    case 2: // UPPER case
        if (c < L'A' || c > L'Z') {
            ungetChar();
            return false;
        }
        break;
    default:
        if (!(c >= L'a' && c <= L'z') && !(c >= L'A' && c <= L'Z') && c != L'_') {
            ungetChar();
            return false;
        }
        break;
    }


    bool lflag = false;
    do {
    SkipCond:
        c = getChar();
        if (c >= L'a' && c <= L'z') {
            if (followCase == 2) {
                setPos(start);
                return false;
            }
            lflag = true;
            goto SkipCond;
        }
    } while ((c >= L'A' && c <= L'Z') || c == L'_' || (c >= L'0' && c <= L'9'));

    if ((! lflag) && (followCase == 0)) {
        setPos(start);
        return false;
    }

    ungetChar();
    return true;
}

bool InputBuffer::skip_decimalNum(void)
{
    hyu32 startPos = getPos();
    wchar_t c;
    do {
        c = getChar();
    } while (c >= L'0' && c <= L'9');
    ungetChar();

    return (startPos != getPos());
}

bool InputBuffer::skip_hexChar(void)
{
    wchar_t c = getChar();
    if ((c >= L'0' && c <= L'9')
        || (c >= L'a' && c <= L'f')
        || (c >= L'A' && c <= L'F'))
        return true;
    ungetChar();
    return false;
}
    


bool InputBuffer::pos2linecol(hyu32 pos, hyu32* line, hyu32* col)
{
    if (pos >= m_bufSize)
        return false;

    int lastLine = m_linePos.size();
    hyu32 lastPos = m_linePos[lastLine - 1];
    if (pos < lastPos) {
        // pos is within m_linePos, do binary search
        int i = 0;
        int j = lastLine - 1;
        while (i < j - 1) {
            int k = (i + j) / 2;
            hyu32 kpos = m_linePos[k];
            if (pos < kpos)
                j = k;
            else {
                i = k;
                if (pos == kpos) break;
            }
        }
        *line = i + 1;
        *col = pos - m_linePos[i];
        return true;
    }

    // now, lastPos <= pos <= m_bufSize

    char c1, c2;
    hyu32 chkPos = lastPos;
    while (chkPos < pos) {      // chkPos < m_bufSize
        c1 = m_buffer[chkPos];
        if (chkPos + 1 < m_bufSize) {
            c2 = m_buffer[chkPos+1];
        } else {
            c2 = '\0';
        }

        if (c1 == '\r' || c1 == '\n') {
            if (c1 == '\r' && c2 == '\n') {
                chkPos += 2;
            } else {
                ++ chkPos;
            }
            lastPos = chkPos;
            m_linePos.add(lastPos);
            ++ lastLine;
        } else {
            ++ chkPos;
        }
    }

    if (chkPos == pos)  {
        *line = lastLine;
        *col = pos - lastPos;
    } else {
        *line = lastLine - 1;
        *col = pos - m_linePos[lastLine-2];
    }
    return true;
}

void InputBuffer::buildSourceInfo(SourceInfo* dest, hyu32 pos)
{
    pos2linecol(pos, &(dest->line), &(dest->col));
    dest->fname = NULL;
}

void InputBuffer::sprintSourceInfo(char* dest, hyu32 destSize, hyu32 pos)
{
    SourceInfo si;
    buildSourceInfo(&si, pos);
    si.printns(dest, destSize);
}



#if 0
void InputBuffer::debugOutLinePos(void)
{
    char buf[80];
    pos2linecol(m_bufSize, &line, &col);      // build all m_linePos
    hyu32 prevPos = 0;
    for (int i = 0; i < m_linePos.size(); i++) {
        hyu32 pos = m_linePos[i];
        sprintSourceInfo(buf, 80, pos);
        HMD_PRINTF("%s: ", buf);
        Substr ss(prevPos, pos);
        prevPos = pos;
        copySummary(buf, 80, ss);
        HMD_PRINTF("'%s'\n", buf);
    }
}
#endif


FileInputBuffer::FileInputBuffer(const char* filename)
    : InputBuffer(), m_paths(0), m_includePaths(1), m_locate(1)
{
    m_bufSize = hmd_fileSize(filename);
    m_buffer = gMemPool->allocT<const char>(m_bufSize + 8, "fibf");
    hyu32 nread = hmd_loadFile((hyu8*)m_buffer, filename, m_bufSize);
    ((char*)m_buffer)[nread] = '\0';
    m_ptr = m_prevPtr = m_buffer;
    m_LocInfo* li = new m_LocInfo(0, filename);
    m_locate.add(li);
}

void FileInputBuffer::finalize(void)
{
    if (m_buffer != NULL)
        gMemPool->free((void*)m_buffer);
    m_buffer = NULL;
    m_bufSize = 0;
    m_ptr = m_prevPtr = NULL;
    m_linePos.finalize();
    TArrayIterator<m_LocInfo*> itr(&m_locate);
    while (itr.hasMore())
        delete itr.next();
    m_locate.finalize();
    TArrayIterator<const char*> itr2(&m_paths);
    while (itr2.hasMore())
        gMemPool->free((void*)itr2.next());
    m_paths.finalize();
    TArrayIterator<const char*> itr3(&m_includePaths);
    while (itr3.hasMore())
        gMemPool->free((void*)itr3.next());
    m_includePaths.finalize();
}

void FileInputBuffer::addIncludePath(const char* path)
{
    hyu32 psize = HMD_STRLEN(path) + 1;
    char* p = gMemPool->allocT<char>(psize, "incp");
    HMD_STRNCPY(p, path, psize);
    m_includePaths.add(p);
}

char* FileInputBuffer::expandPath(const char* filename, hyu32* pSize)
{
    char* path;
    hyu32 fnlen = HMD_STRLEN(filename);
    hyu32 pathCapa = fnlen + 1;
    path = gMemPool->allocT<char>(pathCapa, "epth");
    HMD_STRNCPY(path, filename, pathCapa);
    hyu32 size = hmd_fileSize(path);
    if (size > 0) {
        if (pSize != NULL) *pSize = size;
        return path;
    }

    TArrayIterator<const char*> itr(&m_includePaths);
    while (itr.hasMore()) {
        const char* inc = itr.next();
        size_t ilen = HMD_STRLEN(inc);
        if (pathCapa < fnlen + ilen + 2) {
            pathCapa = fnlen + ilen + 2;
            path = gMemPool->reallocT<char>(path, pathCapa);
        }
        HMD_STRNCPY(path, inc, pathCapa);
        if (path[ilen-1] != PATH_DELIM) {
            path[ilen] = PATH_DELIM;
            path[ilen+1] = '\0';
        }
        HMD_STRSCAT(path, filename, pathCapa);
        size = hmd_fileSize(path);
        if (size > 0) {
            if (pSize != NULL) *pSize = size;
            return path;
        }
    }

    gMemPool->free(path);
    if (pSize != NULL) *pSize = 0;
    return NULL;
}


bool FileInputBuffer::include(const char* filename)
{
    char* path;
    hyu32 size;
    path = expandPath(filename, &size);
    if (path == NULL)
        return false;
    m_paths.add(path);

    hyu32 newBufSize = m_bufSize + size + 2;
    hyu32 curPos = getPos();

    int x = m_searchLocate(curPos);
    m_LocInfo* li = m_getLocInfo(curPos);
    HMD_ASSERT(x >= 0);
    HMD_ASSERT(li != NULL);

    while (m_linePos[m_linePos.size() - 1] >= curPos)
        m_linePos.chop(m_linePos.size() - 1);
    // TODO: remove memoized SyntaxTree after curPos

    const char* newBuf = gMemPool->reallocT<const char>(m_buffer, newBufSize + 8);
    if (newBuf == NULL)
        return false;
    m_ptr = m_prevPtr = newBuf + curPos;
    memmove((void*)(m_ptr + size + 2), (void*)m_ptr, m_bufSize - curPos);
    *(char*)(m_ptr + size) = '\r';       // insert CRLF after file end
    *(char*)(m_ptr + size + 1) = '\n';
    hyu32 nread = hmd_loadFile((hyu8*)m_ptr, path, size);
    HMD_ASSERT(nread == size);
    m_buffer = newBuf;
    m_bufSize = newBufSize;

    li->pos += size + 2;
    m_LocInfo* nf = new m_LocInfo(curPos, path);
    m_locate.insert(x+1) = nf;
    m_locate.insert(x+2) = li;

    for (x=x+3; (hyu32)x < m_locate.size(); ++x) {
        m_LocInfo* p = m_locate[x];
        p->pos += size + 2;
    }

    return true;
}

int FileInputBuffer::m_searchLocate(hyu32 pos)
{
    if (pos > m_bufSize)
        return -1;
    int i = 0;
    int j = m_locate.size();
    while (i < j - 1) {
        int k = (i + j) / 2;
        if (pos >= m_locate[k]->pos)
            i = k;
        else
            j = k;
    }
    return i;
}

FileInputBuffer::m_LocInfo* FileInputBuffer::m_getLocInfo(hyu32 pos)
{
    int i = m_searchLocate(pos);
    if (i < 0)
        return NULL;
    hyu32 line, col;
    if (! pos2linecol(pos, &line, &col))
        return NULL;

    m_LocInfo* p = new m_LocInfo(*(m_locate[i]));
    hyu32 pl, pc;
    pos2linecol(p->pos, &pl, &pc);
    line -= pl;         // lines from locate(i)
    p->line += line;
    if (line == 0)
        p->col += col - pc;
    else
        p->col = col;
    p->pos = pos;
    return p;
}


FileInputBuffer::m_LocInfo::m_LocInfo(hyu32 ipos, const char* filename)
    : pos(ipos), fname(filename)
{
    line = 1;
    col = 0;
}

/*
FileInputBuffer::m_LocInfo::m_LocInfo(FileInputBuffer::m_LocInfo& inf)
{
    pos = inf.pos;
    fname = inf.fname;
    line = inf.line;
    col = inf.col;
}
*/

FileInputBuffer::m_LocInfo::~m_LocInfo()
{
}

void FileInputBuffer::buildSourceInfo(SourceInfo* dest, hyu32 pos)
{
    m_LocInfo* p = m_getLocInfo(pos);
    if (p == NULL) {
        dest->line = dest->col = 0;
        dest->fname = "(out of file)";
        return;
    }
    dest->line = p->line;
    dest->col = p->col;
    dest->fname = p->fname;
    delete p;
}





StringInputBuffer::StringInputBuffer(const char* str)
    : InputBuffer()
{
    m_buffer = str;
    m_bufSize = HMD_STRLEN(str);
    m_ptr = m_prevPtr = m_buffer;
}

StringInputBuffer::~StringInputBuffer()
{
    clean();
}

bool StringInputBuffer::include(const char* str)
{
    HMD_ASSERT(false);      // not implemented yet
    return false;
}
