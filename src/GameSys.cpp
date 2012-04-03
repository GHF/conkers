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

#include "GameSys.h"
#include "PlayerObject.h"

#include <chipmunk.h>

#include <cmath>

using namespace std;
using namespace Cairo;
using namespace PixelToaster;

GameSys::GameSys(const Matrix &worldToScreen) :
        bgColor(), screenToWorld(worldToScreen) {
    screenToWorld.invert();
}

void GameSys::init() {
    cpVect gravity = cpv(0, -100);
    space = cpSpaceNew();
    cpSpaceSetGravity(space, gravity);

    shared_ptr<PlayerObject> player(new PlayerObject(10.0, 2.0));
    gameObjects.push_back(player);

    for (shared_ptr<GameObject> gameObject : gameObjects) {
        gameObject->init();
        cpSpaceAddBody(space, gameObject->getBody());
    }

    // add a body that tracks the mouse and constrain the player to it
    mouseBody = cpBodyNew(INFINITY, INFINITY);
    mouseJoint = cpPivotJointNew2(mouseBody,
            player->getBody(),
            cpvzero,
            cpBodyWorld2Local(player->getBody(), cpvzero));
    mouseJoint->maxForce = 50000.0f;
    mouseJoint->errorBias = cpfpow(1.0f - 0.15f, 60.0f);
    cpSpaceAddConstraint(space, mouseJoint);
}

void GameSys::cleanup() {
    cpSpaceFree(space);
    cpConstraintFree(mouseJoint);
    cpBodyFree(mouseBody);
}

void GameSys::sim(double t, double dt) {
    for (shared_ptr<GameObject> gameObject : gameObjects) {
        gameObject->sim(t, dt);
    }

    cpVect mousePos = cpv(mouse.x, mouse.y);
    screenToWorld.transform_point(mousePos.x, mousePos.y);
    // IIR LPF on mouse movements
    cpVect newMousePoint = cpvlerp(mouseBody->p, mousePos, 0.95);
    mouseBody->v = (newMousePoint - mouseBody->p) * dt;
    mouseBody->p = newMousePoint;

    cpSpaceStep(space, dt);
}

void GameSys::render(RefPtr<Context> cr, double t, double dt) {
    cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->paint();

    // render each game object
    for (shared_ptr<GameObject> gameObject : gameObjects) {
        const cpBody * const body = gameObject->getBody();
        // do linear interpolation from the current physics step to right now
        const cpVect pos = cpBodyGetPos(body) + cpBodyGetVel(body) * dt;
        const cpFloat angle = cpBodyGetAngle(body) + cpBodyGetAngVel(body) * dt;

        cr->save();

        // transform into local coordinates to make drawing easy
        cr->translate(pos.x, pos.y);
        cr->rotate(angle);
        gameObject->render(cr, t, dt);

        cr->restore();
    }
}

void GameSys::onMouseMove(DisplayInterface &display, Mouse mouse) {
    this->mouse = mouse;
}
