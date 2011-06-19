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


        // arg�𒲂ׂāA "opt" �܂��� "opt=" �̌`�Ȃ� 0 ���A���A
        // "opt=����" �̌`�Ȃ� ����*2 ���A���A
        // "opt=����!" �̌`�Ȃ� ����*2+1 ���A���B
        // ��̂ǂ̌`�ł��Ȃ���΁A�}�C�i�X�̐��l���A���B
        int getWarnLevel(const char* opt, const char* arg);
    }
}

#endif /* m_HYCCOMMANDLINE_H_ */
