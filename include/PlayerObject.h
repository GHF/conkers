/*
 *  Copyright (C) 2012 Xo Wang
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL XO
 *  WANG BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 *  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  Except as contained in this notice, the name of Xo Wang shall not be
 *  used in advertising or otherwise to promote the sale, use or other dealings
 *  in this Software without prior written authorization from Xo Wang.
 */

#ifndef PLAYEROBJECT_H_
#define PLAYEROBJECT_H_

#include "GameObject.h"

class PlayerObject: public GameObject {
protected:
    cpFloat radius;
    cpShape *shape;

public:
    PlayerObject(cpFloat mass, cpFloat radius, const cpVect &pos = cpvzero);
    ~PlayerObject() {
        if (shape != NULL) {
            cpSpaceRemoveShape(shape->space_private, shape);
            cpShapeFree(shape);
        }
    }

    void init(cpSpace *space);
    void sim(double t, double dt);
    void render(Cairo::RefPtr<Cairo::Context> cr, double t, double dt);
};

#endif /* PLAYEROBJECT_H_ */
