/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYBINARYSEARCH_H_
#define m_HYBINARYSEARCH_H_


template<typename T> bool binarySearch(const T* table, int numContents, const T& val, int* pIndex)
{
    int i = 0;
    int j = numContents;
    int k;
    

    while (i < j) {
        k = (i + j) / 2;
        const T& v = table[k];
        if (v == val){
            *pIndex = k;
            return true;
        }
        if (v < val) {
            i = k + 1;
        } else {
            j = k;
        }
    }
    *pIndex = i;        // == j
    return false;
}


template<typename T> bool binarySearchRange(const T* table, int numContents, const T& val, int* pIndexMin, int* pIndexMax)
{
    int i = 0;
    int j = numContents;
    int k;
    

    while (i < j) {
        k = (i + j) / 2;
        const T& v = table[k];
        if (v == val){
            // found
            i = k - 1;
            while ((i >= 0) && (table[i] == val))
                --i;
            j = k + 1;
            while ((j < numContents) && (table[j] == val))
                ++j;
            *pIndexMin = i + 1;
            *pIndexMax = j - 1;
            return true;
        }
        if (v < val) {
            i = k + 1;
        } else {
            j = k;
        }
    }
    *pIndexMin = *pIndexMax = i;        // == j
    return false;
}

template<typename T, typename U> bool binarySearchFn(T* table, int numContents, int (*fn)(T&, U&), U& val, int* pIndex)
{
    int i = 0;
    int j = numContents;
    int k;
    

    while (i < j) {
        k = (i + j) / 2;
        T& v = table[k];
        int f = fn(v, val);
        if (f == 0){
            *pIndex = k;
            return true;
        }
        if (f < 0) {
            i = k + 1;
        } else {
            j = k;
        }
    }
    *pIndex = i;        // == j
    return false;
}



#endif /* m_HYBINARYSEARCH_H_ */
