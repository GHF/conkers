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

#include "PlayerObject.h"

using namespace Cairo;

PlayerObject::PlayerObject(cpFloat mass, cpFloat radius, const cpVect &pos) :
        GameObject(mass, cpMomentForCircle(mass, 0, radius, cpvzero), pos), radius(radius) {
    hP = 102;
    maxHP = 102;
}

void PlayerObject::init(cpSpace *space) {
    shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
    cpShapeSetFriction(shape, 0.1);
    cpShapeSetGroup(shape, PLAYER);
    cpShapeSetCollisionType(shape, PLAYER);
    cpShapeSetUserData(shape, this);
}

void PlayerObject::sim(double t, double dt) {

}

void PlayerObject::render(RefPtr<Context> cr, double t, double dt) {
    cr->rotate(M_PI / 2 - cpBodyGetAngle(body)); // draw the health bar without rotation
    cr->move_to(0.0, 0.0);
    cr->set_source_rgba(0.2, 0.2, 0.2, 0.2);
    if (hP != 0.0) {
        cr->arc_negative(0.0, 0.0, radius, 0, 2 * M_PI * (hP / maxHP));
    } else {
        cr->arc(0.0, 0.0, radius, 0, 2 * M_PI);
    }
    cr->fill();
    cr->move_to(0.0, 0.0);
    cr->set_source_rgb(0.0, 0.0, 0.0);
    cr->arc_negative(0.0, 0.0, radius, 2 * M_PI * (hP / maxHP), 0);
    cr->fill();
}
