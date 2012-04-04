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

#include "ButterEnemyObject.h"

using namespace std;
using namespace Cairo;

ButterEnemyObject::ButterEnemyObject(shared_ptr<GameObject> player, cpFloat mass, cpFloat size, const cpVect &pos) :
        GameObject(mass, cpMomentForBox(mass, size, size), pos), width(size), height(size), player(player) {
}

void ButterEnemyObject::init(cpSpace *space) {
    shape = cpSpaceAddShape(space, cpBoxShapeNew(body, width, height));
    cpShapeSetFriction(shape, 0.1);
    cpShapeSetCollisionType(shape, ENEMY);
    cpShapeSetUserData(shape, this);

    angleConstraint = cpDampedRotarySpringNew(space->staticBody, body, M_PI / 4, 1000.0, 0.8);
    cpSpaceAddConstraint(space, angleConstraint);
}

void ButterEnemyObject::sim(double t, double dt) {
    if (!alive)
        return;

    cpBodyResetForces(body);
    const cpVect playerPos = cpBodyGetPos(player->getBody());
    const cpVect rocketAccel = (playerPos - cpBodyGetPos(body)) * (0.75 * cpBodyGetMass(body));
    cpBodyApplyForce(body, rocketAccel, cpvzero);
}

void ButterEnemyObject::render(RefPtr<Context> cr, double t, double dt) {
    const double alpha = cpflerp(0.0, 0.6, cpfclamp01(expireTime - t));
    cr->set_source_rgba(0.0, 0.0, 0.0, alpha);
    const double lineWidth = 1.5;
    cr->set_line_width(lineWidth);
    cr->rectangle(-width * 0.5 + lineWidth * 0.5,
            -height * 0.5 + lineWidth * 0.5,
            width - lineWidth,
            height - lineWidth);
    cr->stroke();
}

void ButterEnemyObject::damagingHit(GameObject *other, const cpVect &relVel, double t) {
    GameObject::damagingHit(other, relVel, t);
    if (!alive) {
        // apply gravity
        cpBodyResetForces(body);
        const cpVect gravity = cpv(0, -100);
        cpBodyApplyForce(body, gravity * cpBodyGetMass(body), cpvzero);
    }
}
