/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYSTRINGBOX_H_
#define m_HYSTRINGBOX_H_

/*
  String の保管場所を管理する。
  バイトコードのリロードで、置換されるバイトコード中のStringが
  参照されていた場合、ここに移動される。

  この中に保管された文字列は、フルGC時のみGCの対象となる。
*/


#include "hyArray.h"
#include "hyBitArray.h"


class Test_StringBox; // for unittest


using namespace Hayat::Common;

namespace Hayat {
    namespace Engine {

        class  StringBox {
            friend class ::Test_StringBox; // for unittest

        protected:
            TArray<const char*> m_strStorage;
            BitArray            m_markFlags;

        public:
            StringBox(void);
            ~StringBox() { finalize(); }

            void finalize(void);

            const char* store(const char* str);

            void unmark(void);  // GC準備
            void mark(const char* str); // GC mark
            void sweep(void);

        };

    }
}

#endif /* m_HYSTRINGBOX_H_ */
