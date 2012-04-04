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

#include "HammerObject.h"

using namespace Cairo;

HammerObject::HammerObject(cpFloat mass, cpFloat width, cpFloat height, const cpVect &pos) :
        GameObject(mass, cpMomentForBox(mass, width, height), pos), width(width), height(height) {
}

void HammerObject::init(cpSpace *space) {
    shape = cpSpaceAddShape(space, cpBoxShapeNew(body, width, height));
    cpShapeSetFriction(shape, 0.8);
    cpShapeSetGroup(shape, PLAYER);
}

void HammerObject::sim(double t, double dt) {

}

void HammerObject::render(RefPtr<Context> cr, double t, double dt) {
    cr->set_source_rgba(0.0, 0.0, 0.0, 1.0);
    cr->rectangle(-width * 0.5, -height * 0.5, width, height);
    cr->fill();
}
