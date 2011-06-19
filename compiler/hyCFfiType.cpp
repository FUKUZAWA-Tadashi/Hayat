/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */



#include "hyCFfiType.h"
#include "hyCSymbolTable.h"
#include "hpInputBuffer.h"

using namespace Hayat::Common;
using namespace Hayat::Compiler;
using namespace Hayat::Parser;


FfiType::m_InitStruct FfiTypeMgr::m_ini[] = {
    // [0]は型指定無しの場合、又はc++クラス定義の無いユーザー定義型の場合
    // [1]はNilClass又は値無しの場合
    {NULL,      "Value",   "",                  "%s"},
    {"NilClass",     NULL,      NULL,           "NIL_VALUE"},
    {"Bool",    "bool",    ".toBool()",         "Value::fromBool(%s)"},
    {"Int",     "hys32",     ".toInt()",        "Value::fromInt(%s)"},
    {"Float",   "hyf32",     ".toFloat()",      "Value::fromFloat(%s)"},
    {"String",  "const char*", ".toString()",   "Value::fromString(%s)"},
    {"Object",  "Object*",   ".toObj()",        "Value::fromObj(%s)"},
    {"Symbol",  "SymbolID_t", ".toSymbol()",    "Value::fromSymbol(%s)"},
    {"Class",   "const HClass*", ".toClass()",  "Value::fromClass(%s)"},
    {"Array",   "ValueArray*", ".toCppObj<ValueArray>(HSym_Array)", "Value::fromObj(%s->getObj())"},
    {"List",    "ValueList*", ".toList()",      "Value::fromList(%s)"},
    {"StringBuffer", "StringBuffer*", ".toCppObj<StringBuffer>(HSym_StringBuffer)", "Value::fromObj(%s->getObj())"},
};




FfiTypeMgr::FfiTypeMgr(void)
    : m_interfaces(0)
{
}

void FfiTypeMgr::initialize(void)
{
    if (m_interfaces.size() > 0)
        return;
    for (hyu32 i = 0; i < sizeof(m_ini)/sizeof(m_ini[0]); i++) {
        m_interfaces.add(new FfiType(m_ini[i]));
    }
}

void FfiTypeMgr::finalize(void)
{
    hyu32 n = m_interfaces.size();
    for (hyu32 i = 0; i < n; i++) {
        delete m_interfaces[i];
    }
    m_interfaces.finalize();
}

FfiTypeMgr::~FfiTypeMgr()
{
    finalize();
}

void FfiTypeMgr::createInterface (const char* hClassName, const char* cppClassName)
{
    FfiType* p = find(hClassName);
    if (p != NULL)
        return;

    p = new FfiType(hClassName, cppClassName);
    m_interfaces.add(p);
}

FfiType* FfiTypeMgr::find(Substr_st key)
{
    if (key.len() == 0)
        return m_interfaces[0];
    hyu32 n = m_interfaces.size();
    for (hyu32 i = 1; i < n; i++) {
        if (gpInp->cmpStr(key, m_interfaces[i]->name()))
            return m_interfaces[i];
    }
    return NULL;
}

FfiType* FfiTypeMgr::find(const char* key)
{
    if (key == NULL)
        return m_interfaces[0];
    hyu32 n = m_interfaces.size();
    for (hyu32 i = 1; i < n; i++) {
        if (HMD_STRCMP(key, m_interfaces[i]->name()) == 0)
            return m_interfaces[i];
    }
    return NULL;
}

const FfiType& FfiTypeMgr::get(Substr_st key)
{
    FfiType* p = find(key);
    if (p != NULL)
        return *p;
    return *m_interfaces[0];
}

const FfiType& FfiTypeMgr::get(const char* key)
{
    FfiType* p = find(key);
    if (p != NULL)
        return *p;
    return *m_interfaces[0];
}

void* FfiType::operator new(size_t size)
{
    HMD_DEBUG_ASSERT_EQUAL(sizeof(FfiType), size);
    return (void*) gMemPool->alloc(sizeof(FfiType));
}

void FfiType::operator delete(void* p)
{
    gMemPool->free(p);
}

FfiType::FfiType(const char* hClassName, const char* cppClassName)
{
    static const char* TO_TMPL = ".toCppObj<%s>()";
    static const char* FROM_TMPL = "Value::fromObj(Object::fromCppObj(%s))";
    static const hyu32 LEN_TO_TMPL = 16;
    //static const hyu32 LEN_FROM_TMPL = 39;
    HMD_DEBUG_ASSERT(HMD_STRLEN(TO_TMPL) <= LEN_TO_TMPL);
    //HMD_DEBUG_ASSERT(HMD_STRLEN(FROM_TMPL) <= LEN_FROM_TMPL);

    hyu32 len = HMD_STRLEN(cppClassName);
    m_type = gMemPool->allocT<char>(len + 2);
    m_to = gMemPool->allocT<char>(len + LEN_TO_TMPL);
    m_memAllocFlag = true;
    
    m_name = hClassName;
    HMD_STRNCPY(m_type, cppClassName, len+1);
    HMD_SNPRINTF(m_to, len+LEN_TO_TMPL, TO_TMPL, m_type);
    // HMD_SNPRINTF(m_from, nnn+LEN_FROM_TMPL, FROM_TMPL, mmm);
	m_from = (char*)FROM_TMPL;
    m_type[len] = '*'; m_type[len+1] = '\0';
}

FfiType::FfiType(const m_InitStruct& ini)
{
    m_name = (char*) ini.name;
    m_type = (char*) ini.type;
    m_to = (char*) ini.to;
    m_from = (char*) ini.from;
    m_memAllocFlag = false;
}

FfiType::~FfiType()
{
    if (m_memAllocFlag) {
        // m_name is not allcated
        gMemPool->free(m_type);
        gMemPool->free(m_to);
        m_memAllocFlag = false;
    }
}
