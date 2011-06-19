/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCELLLIST_H_
#define m_HYCELLLIST_H_


#include "hyMemPool.h"

using namespace Hayat::Common;

namespace Hayat {
    namespace Engine {

        template <typename ContentType> class CellList
        {
        protected:
            CellList*       m_next;
            ContentType     m_content;
        public:
            CellList(void) { m_next = NULL; }
            CellList(const ContentType& content) { m_next = NULL; m_content = content; }
            ~CellList() {}

            typedef bool (*compFunc_t)(const ContentType&, void*);

            void clean(void) { m_next = NULL; }
#ifdef HMD_DEBUG
            // unClean��ԂŎg�����Ƃ���ƒ�~����悤�ɂ���B�������`�F�b�N�p
            void unClean(void) { m_next = (CellList*)1; }
#endif

            void* operator new(size_t size)
            {
#ifndef HMD_DEBUG
                (void)size;
#endif
                HMD_DEBUG_ASSERT(size == sizeof(CellList));
                return (void*) gMemPool->alloc(sizeof(CellList), "CLST");
            }

            void operator delete(void* p)
            {
                gMemPool->free(p);
            }


            ContentType& content(void)
            {
                return m_content;
            }

            void setContent(const ContentType& content)
            {
                m_content = content;
            }

            ContentType& head(void)
            {
                return m_content;
            }

            CellList* tail(void)
            {
                return m_next;
            }

            static CellList* cons(const ContentType& a, CellList* b)
            {
                CellList* c = new CellList(a);
                c->m_next = b;
                return c;
            }

            static ContentType shift(CellList** pList)
            {
                CellList* q = *pList;
                ContentType a = q->head();
                *pList = q->tail();
                delete q;
                return a;
            }

            static void drop(CellList** pList)
            {
                CellList* q = *pList;
                *pList = q->tail();
                delete q;
            }

            static void deleteAll(CellList* pList)
            {
                while (pList != NULL) {
                    CellList* q = pList->m_next;
                    delete pList;
                    pList = q;
                }
            }

            CellList* next(void)
            {
                return m_next;
            }

            void append(CellList* cell)
            {
                CellList* p = this;
                while (p->m_next != NULL)
                    p = p->m_next;
                p->m_next = cell;
            }

            void link(CellList* cell)
            {
                m_next = cell;
            }

            void unlinkNext(void)
            {
                if (m_next != NULL) {
                    m_next = m_next->m_next;
                }
            }

            bool unlink(CellList* cell)
            {
                HMD_ASSERT(cell != NULL);
                CellList* p = this;
                CellList* pn = p->m_next;
                while (pn != NULL) {
                    if (pn == cell) {
                        p->m_next = cell->m_next;
                        return true;
                    }
                    p = pn;
                    pn = p->m_next;
                }
                return false;
            }

            void insert(CellList* cell)
            {
                cell->m_next = m_next;
                m_next = cell;
            }

            void insert(const ContentType& content)
            {
                insert(new CellList(content));
            }


            void add(const ContentType& content)
            {
                append(new CellList(content));
            }

            bool remove(const ContentType& content)
            {
                CellList* p = this;
                CellList* pn = p->m_next;
                while (pn != NULL) {
                    if (pn->m_content == content) {
                        p->m_next = pn->m_next;
                        delete pn;
                        return true;
                    }
                    p = pn;
                    pn = p->m_next;
                }
                return false;
            }

            bool condRemove(compFunc_t compFunc, void* param)
            {
                CellList* p = this;
                CellList* pn = p->m_next;
                while (pn != NULL) {
                    if (compFunc(pn->m_content, param)) {
                        p->m_next = pn->m_next;
                        delete pn;
                        return true;
                    }
                    p = pn;
                    pn = p->m_next;
                }
                return false;
            }

            void removeNext(void)
            {
                if (m_next != NULL) {
                    CellList* nn = m_next->m_next;
                    delete m_next;
                    m_next = nn;
                }
            }

            bool isInclude(const ContentType& c)
            {
                CellList* p = this;
                CellList* pn = p->m_next;
                while (pn != NULL) {
                    if (pn->m_content == c)
                        return true;
                    p = pn;
                    pn = p->m_next;
                }
                return false;
            }
        
#ifdef HMD_DEBUG
            void debugDump(void(*dumpFunc)(const ContentType&))
            {
                CellList* p = this;
                int i = 0;
                while (p != NULL) {
                    dumpFunc(p->m_content);
                    p = p->m_next;
                    if (++i > 50) break;
                }
                HMD_PRINTF("\n");
            }
#endif


        public:
            // �폜���ł���C�e���[�^
            class Iterator {
            protected:
                CellList**  pp;
            public:

                Iterator(void) { pp = NULL; }
                // ���X�g�̐擪�A�h���X���i�[�����A�h���X��n���ď�����
                Iterator(CellList** pCellList) {
                    initialize(pCellList);
                }
                void initialize(CellList** pCellList) { pp = pCellList; }
                // �����������ǂ��� (get()��NULL���ǂ���)
                bool empty(void) { return (*pp) == NULL; }
                // ���݂̃Z����Ԃ�
                CellList* get(void) { return *pp; }
                CellList* operator*() { return *pp; }
                // ���̃Z����Ԃ�
                CellList* next(void) {
                    if (*pp != NULL)
                        pp = &((*pp)->m_next);
                    return *pp;
                }
                // ���݂̃Z���ւ̃|�C���^�̎Q�Ƃ�Ԃ�
                CellList*& ptrRef(void) { return *pp; }
                // �폜:���݂̃Z�������X�g����O��
                CellList* remove(void) {
                    CellList* cell = *pp;
                    if (cell != NULL) {
                        *pp = cell->m_next;
                        cell->m_next = NULL;
                    }
                    return cell;
                }
                // ���݂̃Z���̎��ɑ}������
                // a -> b -> c �Ō��݂̃Z���� b �Ȃ�A
                // a -> b -> x -> c �ɂȂ��Č��݂̃Z���� b �̂܂�
                void insert(CellList* cell) {
                    if (*pp == NULL) {
                        *pp = cell;
                        cell->m_next = NULL;
                    } else {
                        (*pp)->insert(cell);
                    }
                }
                void insert(ContentType content) {
                    insert(new CellList(content));
                }

                // ���݂̃Z���̑O�ɑ}������
                // a -> b -> c �Ō��݂̃Z���� b �Ȃ�A
                // a -> x -> b -> c �ɂȂ��Č��݂̃Z���� b �̂܂�
                void insertPrev(CellList* cell) {
                    cell->m_next = *pp;
                    *pp = cell;
                    pp = &(cell->m_next);
                }
                void insertPrev(ContentType content) {
                    insertPrev(new CellList(content));
                }

            };

            friend class Iterator;

        };

    }
}
#endif /* m_HYCELLLIST_H_ */
