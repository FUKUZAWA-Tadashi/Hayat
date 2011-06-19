/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCCOMMANDLINE_H_
#define m_HYCCOMMANDLINE_H_

#include "machdep.h"

extern const char** HMD_LOADPATH;
extern hyu32 numLoadPath;

namespace Hayat {
    namespace Compiler {

        void addLoadPath(const char* path, const char* subdir = NULL);
        void finalizeLoadPath(void);


        const char* getParam(int& i, int argc, const char* argv[], const char* pname);


        // argを調べて、 "opt" または "opt=" の形なら 0 を帰し、
        // "opt=数字" の形なら 数字*2 を帰し、
        // "opt=数字!" の形なら 数字*2+1 を帰す。
        // 上のどの形でもなければ、マイナスの数値を帰す。
        int getWarnLevel(const char* opt, const char* arg);
    }
}

#endif /* m_HYCCOMMANDLINE_H_ */
