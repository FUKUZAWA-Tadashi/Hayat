/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYBYTECODE_H_
#define m_HYBYTECODE_H_

#include "hyClass.h"
#include "hySignature.h"
#include "hyBytecodeIf.h"
#include "hyDebugSourceInfo.h"


namespace Hayat {
    namespace Engine {

        class CodePtr;
        class MethodPtr;
        class Context;
        class StringBuffer;

    class Bytecode : public Hayat::Common::BytecodeIf {
            friend class GC;
            friend class CodeManager;

        public:
            Bytecode(void);
            virtual ~Bytecode();

            void* operator new(size_t size);
            void operator delete(void* p);

            bool            readFile(const char* fname, void (*loadedCallback)(const char*) = NULL);
            void            setBytecode(const hyu8* buf, hyu32 size);
            // size分のメモリを自分で確保し、bufからコピーして、setBytecodeする
            void            takeBytecode(const hyu8* buf, hyu32 size);
            void            finalize(void);

            void            initCodePtr(CodePtr* ptr) const;
            const hyu8*       getString(hyu32 offs) const;
            bool            bHaveString(const hyu8* str) const {
                return (str >= m_stringTable && str < m_stringTable + m_stringTableSize); }

            const Signature::Sig_t*   getSignatureBytes(hyu16 id) const;
            hyu32             getSignatureBytesLen(hyu16 id) const;
            hyu8              getSignatureArity(hyu16 id) const;
            const HClass*   mainClass(void) const { return m_mainClass; }
            bool            haveBytecode(void) const { return (m_bytecodeBuf != NULL); }

            const hyu8*     codeStartAddr(void) const { return m_bytecodeBuf; }
            const hyu8*     codeEndAddr(void) const { return m_bytecodeEndAddr; }
            bool            isInCodeAddr(const hyu8* ptr) const { return (m_bytecodeBuf <= ptr) && (ptr < m_bytecodeEndAddr); }

            SymbolID_t      bytecodeSymbol(void) const { return (SymbolID_t)m_mySymbol; }
            void            changeBytecodeSymbol(SymbolID_t asName);

            // リンクバイトコードも含めてトップレベルクラスを検索する
            const HClass*   searchClass(SymbolID_t classSym) const;
            // リンクバイトコードも含めてトップレベルメソッドを検索する
            bool            searchMethod(MethodPtr* pMethodPtr, SymbolID_t methodSym, Signature* pSig) const;
            // リンクバイトコードも含めてトップレベル定数を検索
            Value*          getConstVarAddr(SymbolID_t varSym) const;

            // リンクバイトコードに対して再帰的に initLinkBytecode(context, true)を実行し、
            // setInitialized()を実行していない状態でかつbExecがtrueなら、
            // execBytecodeTop()を実行してsetInitialized()を実行する。
            void            initLinkBytecode(Context* context, bool bExec = false);
            void            setInitialized(void) { m_initializedFlag = true; }


        protected:
            const HClass*   m_searchClass(SymbolID_t classSym) const;
            bool            m_searchMethod(MethodPtr* pMethodPtr, SymbolID_t methodSym, Signature* pSig) const;
            Value*          m_getConstVarAddr(SymbolID_t varSym) const;

        public:
            void            m_GC_mark_staticVar(void);
        
        protected:
            hyu8*             m_fileBuf;

            const HClass*     m_mainClass;

            const hyu8*       m_bytecodeBuf;
            hyu32             m_bytecodeSize;
            const hyu8*       m_bytecodeEndAddr;

            hyu32             m_stringTableSize;
            const hyu8*       m_stringTable;

            hyu32             m_signatureTableNum;
            const hyu32*      m_signatureTableOffs;
            const hyu8*       m_signatureTable;
            const hyu8*       m_signatureArityTable;

            // シンボルは、通常はファイル名の拡張子を ".hyb" にして
            // シンボル化したもの。 "filename.hy" → :"filename.hyb"
            SymbolID_t      m_mySymbol;
            hyu16             m_numLinkBytecodes;
            const SymbolID_t*   m_linkBytecodes;

            bool            m_initializedFlag; // バイトコード初期化をしたか




#ifdef HY_ENABLE_BYTECODE_RELOAD
        public:
            // バイトコードリロード時の旧クラスのデータを新クラスに移す
            void            copyClassDataTo(Bytecode* dest, bool bCopyClassVar, bool bCopyConstVar);
        protected:
            BMap<const HClass*, const HClass*>      m_generationMap;
#endif




#ifdef HMD_DEBUG
        protected:
            DebugInfos  m_debugInfos;
            static bool m_bReadDebugInf;

        public:
            void    debugGetInfo(StringBuffer* sb, const hyu8* ptr) const;

            void    debugGetInfo(char* buf, hyu32 bufSize, const hyu8* ptr) const;

            // バイトコード*.hybの読み込みと同時に、デバッグ情報*.hdbも
            // 読み込む場合は setFlagReadDebugInfo(true) とする。
            // *.hdbは、Debug::setDebugMemPool()で指定したメモリ領域に読む。
            // 指定がなければgMemPoolに読む。
            static void setFlagReadDebugInfo(bool flag) { m_bReadDebugInf = flag; }
#else
        public:
            void    debugGetInfo(StringBuffer* sb, const hyu8* ptr) const;
            void    debugGetInfo(char* buf, hyu32 bufSize, const hyu8* ptr) const;
            static void setFlagReadDebugInfo(bool) {}
#endif


        };

    }
}

#endif /* m_HYBYTECODE_H_ */
