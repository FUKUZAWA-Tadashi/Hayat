/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCPackage.h"
#include "hyPacking.h"
#include "hyCSymbolTable.h"
#include "hyCStrTable.h"
#include "hyCContext.h"
#include "hyVersion.h"


namespace Hayat {
    namespace Compiler {
        extern SymbolTable gSymTable;
    }
}
using namespace Hayat::Common;
using namespace Hayat::Compiler;


extern void compileError(const char* fmt, ...);

/*static*/ TArray<Package*> Package::m_loadedPackages(0);
/*static*/ StrTable Package::m_basicSignatureTable;

/*static*/ void Package::initialize(void)
{
    m_loadedPackages.initialize(0);
    m_basicSignatureTable.initialize(Signature::NUM_PREDEFINED_PARAMS + 1, Signature::NUM_PREDEFINED_PARAMS * 3);
    for (hyu32 i = 0; i < Signature::NUM_PREDEFINED_PARAMS; ++i) {
        hyu32 len = (Signature::PREDEFINED_PARAMS[i][0] == Signature::NESTNUM_1) ? 2 : 3;
        m_basicSignatureTable.addBytes(Signature::PREDEFINED_PARAMS[i], len);
    }
}

/*static*/ void Package::registration(Package* pkg)
{
    for (int i = (int)m_loadedPackages.size()-1; i >= 0; --i) {
        if (m_loadedPackages.nth(i) == pkg)
            return;     // 同じパッケージが登録済み
    }
    m_loadedPackages.add(pkg);
}

/*static*/ Package* Package::getPackage(SymbolID_t sym)
{
    for (int i = (int)m_loadedPackages.size()-1; i >= 0; --i) {
        Package* p = m_loadedPackages.nth(i);
        if (p->getSymbol() == sym)
            return p;
    }
    return NULL;
}

/*static*/ void Package::finalize(void)
{
    hyu32 n = m_loadedPackages.size();
    for (hys32 i = (hys32)n-1; i >= 0; --i)
        delete m_loadedPackages[i];
    m_loadedPackages.finalize();
    m_basicSignatureTable.finalize();
}

void* Package::operator new(size_t size)
{
    HMD_DEBUG_ASSERT_EQUAL(sizeof(Package), size);
    return (void*) gMemPool->alloc(sizeof(Package));
}

void Package::operator delete(void* p)
{
    gMemPool->free(p);
}



Package::Package(SymbolID_t sym)
    : m_mySymbol(sym), m_linkPackages(0), m_classInfos(0),
      m_signatureTable(), m_unknownClasses(0), m_unknownMethods(0)
{
    m_packageClass = new ClassInfo(sym, this);
    addClassInfo(m_packageClass);
    Context::newPackageClass(m_packageClass);

    SymbolID_t stdlibSym = gSymTable.symbolID("stdlib.hyb");
    if (sym != stdlibSym)
        require(stdlibSym);
}

Package::Package(void)
    : m_mySymbol(SYMBOL_ID_ERROR), m_linkPackages(0), m_classInfos(0), m_signatureTable()
{
    m_packageClass = NULL;
}

Package::~Package()
{
    if (m_packageClass != NULL)
        delete m_packageClass;
}

void Package::require(SymbolID_t sym)
{
    linkPackage(sym);
    if (getPackage(sym) != NULL)
        return;
    const char* packageName = gSymTable.id2str(sym);
    if (packageName == NULL)
        compileError(M_M("illegal require for {%d}"), sym);
    size_t pkgNameLen = HMD_STRLEN(packageName);
    char* path = gMemPool->allocT<char>(pkgNameLen + 1);
    HMD_STRNCPY(path, packageName, pkgNameLen+1);
    path[pkgNameLen-1] = 'p';   // "*.hyb" → "*.hyp"
    hyu8* buf;

    hyu32 fsize = hmd_loadFile_inPath(path, &buf);
    if (fsize == 0)
        compileError(M_M("require failed: cannot load \"%s\""), path);
    const hyu8* endp = readPackage(buf, fsize, path);
    HMD_ASSERT((endp != NULL) && (endp <= buf + fsize));
    hmd_freeFileBuf(buf);
    gMemPool->free(path);

    // requireしたパッケージは内部的にスーパークラス扱い
    Package* pkg = getPackage(sym);
    HMD_DEBUG_ASSERTMSG(pkg != NULL, "package '%s' not found", packageName);
    m_packageClass->addSuperClass(pkg->packageClass());
}

void Package::linkPackage(SymbolID_t sym)
{
    for (int i = (int)m_linkPackages.size()-1; i >= 0; --i) {
        if (m_linkPackages.nth(i) == sym)
            return;         // 既にリンク済み
    }
    m_linkPackages.add(sym);
}

hyu16 Package::getSignatureID(const hyu8* sigBytes, hyu32 len)
{
    hyu16 id = m_basicSignatureTable.checkIdBytes(sigBytes, len);
    if (id != StrTable::NOT_FOUND_ID)
        return id;
    return m_signatureTable.getIdBytes(sigBytes, len) + Signature::NUM_PREDEFINED_PARAMS;
}

const Signature::Sig_t* Package::getSignatureBytes(hyu16 signatureID)
{
    if (signatureID < Signature::NUM_PREDEFINED_PARAMS)
        return (const Signature::Sig_t*)m_basicSignatureTable.getBytes(signatureID);
    return (const Signature::Sig_t*)m_signatureTable.getBytes(signatureID - Signature::NUM_PREDEFINED_PARAMS);
}

void Package::fwriteSignatureTable(FILE* fp)
{
    TArray<hyu8> out(32);
    m_signatureTable.writeBytes(&out);
    fwrite(out.top(), 1, out.size(), fp);
}

void Package::fwriteArityTable(FILE* fp)
{
    hyu32 n = m_signatureTable.numStr();
    if (n == 0)
        return;
    TArray<hyu8> arities(n+4);
    for (hyu32 i = 0; i < n; ++i)
        arities.add(Signature::arity((const Signature::Sig_t*)m_signatureTable.getBytes((hyu16)i)));
    arities.align(4, 0xfa);
    fwrite(arities.top(), 1, arities.size(), fp);
}


void Package::fwriteLinks(FILE* fp)
{
    hyu8 buf[4];

    // リンクバイトコード個数
    hyu32 n = m_linkPackages.size();
    HMD_ASSERT(n < 65536);
    Endian::pack<hyu16>(buf, (hyu16)n);
    size_t nwrite = fwrite(buf, sizeof(hyu16), 1, fp);
    HMD_ASSERT(nwrite == 1);

    // 未使用
    nwrite = fwrite(buf, sizeof(hyu16), 1, fp);
    HMD_ASSERT(nwrite == 1);

    // パッケージシンボル
    Endian::pack<SymbolID_t>(buf, m_mySymbol);
    nwrite = fwrite(buf, sizeof(SymbolID_t), 1, fp);
    HMD_ASSERT(nwrite == 1);

    for (hyu32 i = 0; i < n; i++) {
        // リンクバイトコードのシンボル require順
        Endian::pack<SymbolID_t>(buf, m_linkPackages[i]);
        nwrite = fwrite(buf, sizeof(SymbolID_t), 1, fp);
        HMD_ASSERT(nwrite == 1);
    }
    if (sizeof(SymbolID_t) == 2) {
        if ((n & 1) == 0) {
            nwrite = fwrite("**", 1, 2, fp);
            HMD_ASSERT(nwrite == 2);
        }
    }

    StrTable nameTable;
    nameTable.initialize();
    for (hyu32 i = 0; i < n; i++) {
        nameTable.getOffs(gSymTable.id2str(m_linkPackages[i]));
    }
    nameTable.writeFile(fp);
}

hys32 Package::classInfo2ID(ClassInfo* ci)
{
    hys32 id;
    if (m_classInfos.find(ci, &id))
        return id;
    return -1;
}

/*static*/ ClassInfo* Package::ID2classInfo(SymbolID_t pkgSym, hys32 classID)
{
    Package* pkg = getPackage(pkgSym);
    HMD_DEBUG_ASSERTMSG(pkg != NULL, M_M("no package %s{%d} found"), gSymTable.id2str(pkgSym), pkgSym);
    return pkg->ID2classInfo(classID);

    HMD_DEBUG_ASSERT(classID >= 0 && (hyu32)classID < pkg->m_classInfos.size());
    return pkg->m_classInfos[classID];
}

ClassInfo* Package::ID2classInfo(hys32 classID)
{
    HMD_DEBUG_ASSERT(classID >= 0 && (hyu32)classID < m_classInfos.size());
    return m_classInfos[classID];
}


void Package::serialize(TArray<hyu8>* out)
{
    packOut<SymbolID_t>(out, m_mySymbol);
    out->align(4,0xdf);
    m_signatureTable.writeBytes(out);
    hyu32 n = m_linkPackages.size();
    HMD_ASSERT(n <= 0xffff);
    packOut<hyu16>(out, (hyu16)n);
    for (hyu32 i = 0; i < n; ++i)
        packOut<SymbolID_t>(out, m_linkPackages[i]);
    n = m_classInfos.size();
    HMD_ASSERT(n <= 0xffff);
    packOut<hyu16>(out, (hyu16)n);
    for (hyu32 i = 0; i < n; ++i)
        packOut<SymbolID_t>(out, m_classInfos[i]->classSymbol());
    for (hyu32 i = 0; i < n; ++i)
        m_classInfos[i]->serialize(out);
}

const hyu8* Package::deserialize(const hyu8* inp)
{
    HMD_DEBUG_ASSERT(m_mySymbol == SYMBOL_ID_ERROR);
    HMD_DEBUG_ASSERT(m_linkPackages.size() == 0);
    HMD_DEBUG_ASSERT(m_classInfos.size() == 0);
    m_mySymbol = Endian::unpackP<SymbolID_t>(&inp);
    inp += (4 - sizeof(m_mySymbol));
    m_signatureTable.readBytes(&inp);
    hyu16 numpkgs = Endian::unpackP<hyu16>(&inp);
    TArray<SymbolID_t> pkgs(numpkgs);
    for (hyu16 i = 0; i < numpkgs; ++i) {
        pkgs.add(Endian::unpackP<SymbolID_t>(&inp));
    }
    hyu16 n = Endian::unpackP<hyu16>(&inp);
    for (hyu16 i = 0; i < n; ++i) {
        SymbolID_t sym = Endian::unpackP<SymbolID_t>(&inp);
        ClassInfo* ci = new ClassInfo(sym, this);
        m_classInfos.add(ci);
        if (m_packageClass == NULL)
            m_packageClass = ci;
    }
    // m_packageClass ができてから、requireする
    for (hyu16 i = 0; i < numpkgs; ++i) {
        require(pkgs[i]);
    }
    for (hyu16 i = 0; i < n; ++i)
        inp = m_classInfos[i]->deserialize(inp);
    return inp;
}

void Package::fwritePackage(FILE* fp)
{
    hyu8 id[8] = { 0xc1, 0xe1, (Endian::MY_ENDIAN == Endian::LITTLE) ? 0 : 1, 0,
                 MAJOR_VERSION, BYTECODE_VERSION, COMPILER_REVISION, 0 };
    fwrite(id, sizeof(hyu8), 8, fp);
    TArray<hyu8> out(256);
    serialize(&out);
    size_t nwrite = fwrite(out.top(), 1, out.size(), fp);
    HMD_ASSERT(nwrite == out.size());
}

/*static*/ const hyu8* Package::readPackage(const hyu8* buf, hyu32 bufSize, const char* fname)
{
    if (buf[0] != 0xc1 || buf[1] != 0xe1)
        compileError(M_M("%s: not Hayat package"), fname);
    if (((buf[2] == 0) && (Endian::MY_ENDIAN != Endian::LITTLE))
        || ((buf[2] != 0) && (Endian::MY_ENDIAN == Endian::LITTLE)))
        compileError(M_M("%s: Endian mismatch"), fname);
    if (buf[4] != MAJOR_VERSION
        || buf[5] != BYTECODE_VERSION)
        compileError(M_M("%s: version mismatch"), fname);
    Package* pkg = new Package();
    registration(pkg);
    return pkg->deserialize(buf+8);
}


void Package::checkClass(SymbolID_t classSym, hyu32 parsePos)
{
    int idx;
    if (binarySearch<SymbolID_t>(m_knownClasses.top(), m_knownClasses.size(), classSym, &idx))
        return;
    if (m_packageClass->checkClass(classSym))
        m_knownClasses.insert(idx) = classSym;
    else
        m_unknownClasses[classSym] = parsePos;
}


bool Package::checkMethod(SymbolID_t methodSym, hyu16 signatureID, hyu32 parsePos)
{
    //HMD_PRINTF("checkMethod(%s,%d)\n",gSymTable.id2str(methodSym),signatureID);
    const Signature::Sig_t* rightSig = getSignatureBytes(signatureID);
    if (m_packageClass->checkMethod(methodSym, rightSig))
        return true;
    hyu32 numPkg = m_loadedPackages.size();
    for (hyu32 j = 0; j < numPkg; ++j) {
        if (m_loadedPackages[j]->packageClass()->checkMethod(methodSym, rightSig))
            return true;
    }
    // 該当メソッド無し

    int min, max;
    if (binarySearchRange(m_unknownMethods.keys().top(), m_unknownMethods.size(), methodSym, &min, &max)) {
        TArray<UM_t>& uma = m_unknownMethods.values();
        for (int i = min; i <= max; ++i) {
            if (uma[i].signatureID == signatureID && uma[i].pos == parsePos)
                return false;         // 同じものが登録済み
        }
        ++max;
    }

    m_unknownMethods.keys().insert(max) = methodSym;
    UM_t& um = m_unknownMethods.values().insert(max);
    um.pos = parsePos;
    um.signatureID = signatureID;

    return false;
}

bool Package::finalCheck(int checkLevel)
{
    if (checkLevel <= 0) return false;
    const char* ckmsg = (checkLevel > 1) ? "\n****** COMPILE ERROR ******\n\n" : "Warning: ";
    bool bNoError = true;

    TArray<SymbolID_t>& unknownClassSyms = m_unknownClasses.keys();
    hyu32 num = unknownClassSyms.size();
    for (hyu32 i = 0; i < num; ++i) {
        SymbolID_t sym = unknownClassSyms[i];
        hyu32 numPkg = m_loadedPackages.size();
        hyu32 j;
        for (j = 0; j < numPkg; ++j) {
            if (m_loadedPackages[j]->packageClass()->checkClass(sym))
                break;
        }
        if (j >= numPkg) {
            HMD_PRINTF(ckmsg);
            SourceInfo si;
            gpInp->buildSourceInfo(&si, m_unknownClasses.values()[i]);
            HMD_PRINTF("%s:%d:%d: ", si.fname, si.line, si.col);
            HMD_PRINTF("unknown class name '%s' used\n", gSymTable.id2str(sym));
            bNoError = false;
        }
    }

    TArray<SymbolID_t>& unknownMethodSyms = m_unknownMethods.keys();
    num = unknownMethodSyms.size();
    for (hyu32 i = 0; i < num; ++i) {
        SymbolID_t sym = unknownMethodSyms[i];
        UM_t& um = m_unknownMethods.values()[i];
        const Signature::Sig_t* rightSig = getSignatureBytes(um.signatureID);
        hyu32 numPkg = m_loadedPackages.size();
        hyu32 j;
        for (j = 0; j < numPkg; ++j) {
            if (m_loadedPackages[j]->packageClass()->checkMethod(sym, rightSig))
                break;
        }
        if (j >= numPkg) {
            HMD_PRINTF(ckmsg);
            SourceInfo si;
            gpInp->buildSourceInfo(&si, um.pos);
            HMD_PRINTF("%s:%d:%d: ", si.fname, si.line, si.col);
            HMD_PRINTF("local variable or method '%s' not defined or signature mismatch\n", gSymTable.id2str(sym));
            
            bNoError = false;
        }
    }

    return bNoError;
}
