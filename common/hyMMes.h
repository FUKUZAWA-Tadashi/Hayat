/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef _HYMMES_H_
#define _HYMMES_H_

#include "hyMemPool.h"


#define M_M(mes) (::Hayat::Common::MMes::getMMes(mes))


namespace Hayat {
    namespace Common {
        class MMes {
        public:
            static const char*  getMMes(const char* mes);

            // load path にある翻訳メッセージファイル file を読み込む
            static void         readTable(const char* file, MemPool* pool = NULL);

            static bool         takeTable(const char* buf, int bufSize);
            static void         initialize(void);
            static void         finalize(void);
            
        protected:
            typedef struct {
                const char* orig;
                const char* trans;
            } mm_pair_st;

            static mm_pair_st*  m_mes_tbl;
            static int          m_tbl_size;
            static hyu8*        m_fileBuf;
            static MemPool*     m_fileBuf_pool;

            static int          m_cmpfn(mm_pair_st& ent, const char*& v);
        };
    }
}


#endif /* _HYMMES_H_ */
