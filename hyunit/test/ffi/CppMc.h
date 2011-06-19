/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_CPPMC_H_
#define m_CPPMC_H_

class CppMc {
 public:
    hys32 x, y, z;

    void setXYZ(hys32 x_) {
        x = x_;
        y = x_ + 1;
        z = x_ + 2;
    }

    hys32 getY(void) {
        return y;
    }
};

#endif /* m_CPPMC_H_ */
