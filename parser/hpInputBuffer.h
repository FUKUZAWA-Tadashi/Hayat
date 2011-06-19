
#ifndef m_INPUTBUFFER_H_
#define m_INPUTBUFFER_H_

#include "hyArray.h"
#include "hpSubstr.h"
#include "hpSourceInfo.h"
#include "mbcsconv.h"


using namespace Hayat::Common;

namespace Hayat {
    namespace Parser {

        class InputBuffer {
        public:
            InputBuffer(void);
            virtual ~InputBuffer() {}

            void	clean(void);
            hyu32	getPos(void) { return (hyu32)(m_ptr - m_buffer); }
            void	setPos(hyu32 pos) { m_ptr = m_prevPtr = addr(pos); }
            hyu32	remain(void) { return m_bufSize - getPos(); }
            void	advance(hyu32 len);
            const char*	addr(void) { return m_ptr; }
            const char*	addr(hyu32 pos);
	
            virtual wchar_t getCharAt(const char** ptr);

            wchar_t getChar(void) {
                m_prevPtr = m_ptr;
                return getCharAt(&m_ptr);
            }
            wchar_t getCharAt(hyu32 pos) {
                const char* ptr = addr(pos);
                return getCharAt(&ptr);
            }
            void	ungetChar(bool check = false); // only once after getChar()

            bool	isEOF(void);

            hyu32	copyStr(char* dest, hyu32 destSize, Substr_st& ss);
            void	copySummary(char* dest, hyu32 destSize, Substr_st& ss);
            void	fprintStr(FILE* fp, Substr_st& ss);
            void	fprintSummary(FILE* fp, Substr_st& ss);

            // if match, cmpStr returns true
            bool	cmpStr(const char* str);
            bool	cmpStr(Substr_st& ss, const char* str);
            bool	cmpStr(Substr_st& s1, Substr_st& s2);

            bool	pos2linecol(hyu32 pos, hyu32* line, hyu32* col);
            virtual void buildSourceInfo(SourceInfo* dest, hyu32 pos);
            void	sprintSourceInfo(char* dest, hyu32 destSize, hyu32 pos);

            virtual void	addIncludePath(const char*) = 0;
            virtual bool	include(const char*) = 0;


            // skip before \r or \n or EOF
            bool skip_beforeEol(void);
            // skip \r and \n
            bool skip_eols(void);

            ///////// in ident:
            // a-z : lower
            // A-Z : upper
            // m_ : lower at top, not match identC at second, otherwise any
            // 0-9 : cannot be at top, not match identC at second, otherwise any

            // topCase: 0=lower, 1=any, 2=UPPER
            // followCase: 0=at least 1 lower, 1=any, 2=no lower
            bool skip__ident(int topCase, int followCase);
            bool skip_ident(void) { return skip__ident(1,1); }
            bool skip_identL(void) { return skip__ident(0,1); } // lower
            bool skip_identU(void) { return skip__ident(2,2); } // upper only
            bool skip_identC(void) { return skip__ident(2,0); } // capital
            // skip 0-9
            bool skip_decimalNum(void);
            // skip one char 0-9,a-f,A-F
            bool skip_hexChar(void);



#if 0
            void debugOutLinePos(void);
#endif

        protected:
            const char*	m_buffer;
            hyu32		m_bufSize;
            const char*	m_ptr;
            const char*	m_prevPtr;

            TArray<hyu32>	m_linePos;
        };


        class FileInputBuffer : public InputBuffer {
        public:
            FileInputBuffer(const char* filename);
            ~FileInputBuffer() { finalize(); }

            void finalize(void);
            void addIncludePath(const char* path);
            bool include(const char* filename);
            char* expandPath(const char* filename, hyu32* pSize = NULL);
            /*virtual*/ void buildSourceInfo(SourceInfo* dest, hyu32 pos);
        protected:
            class m_LocInfo {
            public:
                hyu32 pos;
                const char* fname;
                hyu32 line;
                hyu32 col;
                m_LocInfo(hyu32, const char*);
                /* m_LocInfo(m_LocInfo&); */
                ~m_LocInfo();
                static void* operator new(size_t size) { return gMemPool->alloc(size, "_Loc"); }
                static void operator delete(void* p) { gMemPool->free(p); }
            };
            int m_searchLocate(hyu32 pos);
            m_LocInfo* m_getLocInfo(hyu32 pos);
        protected:
            TArray<const char*> m_paths;
            TArray<const char*> m_includePaths;
            TArray<m_LocInfo*>   m_locate;
        };


        class StringInputBuffer : public InputBuffer {
        public:
            StringInputBuffer(const char* str);
            ~StringInputBuffer();

            void addIncludePath(const char*) {}
            bool include(const char* str);
        };



        // global pointer to InputBuffer
        extern InputBuffer* gpInp;
    }
}

#endif /* m_INPUTBUFFER_H_ */
