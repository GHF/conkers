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
#include "HammerObject.h"

#include <chipmunk.h>

#include <cmath>

using namespace std;
using namespace Cairo;
using namespace PixelToaster;

GameSys::GameSys(int screenWidth, int screenHeight, const Matrix &worldToScreen) :
        bgColor( { 1.0, 1.0, 1.0 }),
                screenWidth(screenWidth),
                screenHeight(screenHeight),
                worldToScreen(worldToScreen),
                screenToWorld(worldToScreen),
                screenCenter(cpvzero),
                bounds(cpBBNew(-105, -90, 105, 90)) {
    screenToWorld.invert();
}

void GameSys::init() {
    cpVect gravity = cpv(0, -100);
    space = cpSpaceNew();
    cpSpaceSetGravity(space, gravity);
    cpSpaceSetDamping(space, 0.3);

    shared_ptr<PlayerObject> player(new PlayerObject(10.0, 3.0));
    gameObjects.push_back(player);

    shared_ptr<HammerObject> hammer(new HammerObject(20.0, 7.0, 5.0, cpv(0, -12.0)));
    gameObjects.push_back(hammer);

    hammerConstraint = cpPinJointNew(player->getBody(), hammer->getBody(), cpvzero, cpv(0, 2.3));
    cpSpaceAddConstraint(space, hammerConstraint);

    for (shared_ptr<GameObject> gameObject : gameObjects) {
        gameObject->init(space);
        cpSpaceAddBody(space, gameObject->getBody());
    }

    // add a body that tracks the mouse and constrain the player to it
    mouseBody = cpBodyNew(INFINITY, INFINITY);
    mouseJoint = cpPivotJointNew2(mouseBody,
            player->getBody(),
            cpvzero,
            cpBodyWorld2Local(player->getBody(), cpvzero));
    mouseJoint->maxForce = 100000.0;
    cpSpaceAddConstraint(space, mouseJoint);

    // set up walls
    walls[0] = cpSegmentShapeNew(space->staticBody, cpv(bounds.l, bounds.t), cpv(bounds.l, bounds.b), 0);
    walls[1] = cpSegmentShapeNew(space->staticBody, cpv(bounds.l, bounds.b), cpv(bounds.r, bounds.b), 0);
    walls[2] = cpSegmentShapeNew(space->staticBody, cpv(bounds.r, bounds.t), cpv(bounds.r, bounds.b), 0);
    walls[3] = cpSegmentShapeNew(space->staticBody, cpv(bounds.l, bounds.t), cpv(bounds.r, bounds.t), 0);
    const cpFloat wallFriction = 0.6;
    for (cpShape *wall : walls) {
        cpShapeSetFriction(wall, wallFriction);
        cpSpaceAddShape(space, wall);
    }
}

void GameSys::cleanup() {
    cpSpaceFree(space);
    cpConstraintFree(mouseJoint);
    cpConstraintFree(hammerConstraint);
    cpBodyFree(mouseBody);
}

void GameSys::sim(double t, double dt) {
    for (shared_ptr<GameObject> gameObject : gameObjects) {
        gameObject->sim(t, dt);
    }

    cpVect mousePos = cpv(mouse.x, mouse.y);
    screenToWorld.transform_point(mousePos.x, mousePos.y);
    mousePos = mousePos + screenCenter;
    // IIR LPF on mouse movements
    cpVect newMousePoint = cpvlerp(mouseBody->p, mousePos, 0.99);
    mouseBody->v = (newMousePoint - mouseBody->p) * dt;
    mouseBody->p = newMousePoint;

    // amount to shift the screen this frame
    cpVect playerScreenPos = cpBodyGetPos(gameObjects.front()->getBody()) - screenCenter;
    worldToScreen.transform_point(playerScreenPos.x, playerScreenPos.y);
    cpVect screenError = cpv(playerScreenPos.x - screenWidth / 2, playerScreenPos.y - screenHeight / 2);
    if (fabs(screenError.x) < screenWidth / 4) {
        screenError.x = 0;
    }
    if (fabs(screenError.y) < screenHeight / 4) {
        screenError.y = 0;
    }
    screenError.x = cpfmod(screenError.x, screenWidth / 4);
    screenError.y = cpfmod(screenError.y, screenHeight / 4);
    screenToWorld.transform_distance(screenError.x, screenError.y);
    screenError.x = copysign(screenError.x * screenError.x, screenError.x);
    screenError.y = copysign(screenError.y * screenError.y, screenError.y);

    screenCenter = screenCenter + screenError * (0.75 * dt);

    cpSpaceStep(space, dt);
}

void GameSys::render(RefPtr<Context> cr, double t, double dt) {
    cr->set_source_rgb(bgColor[0], bgColor[1], bgColor[2]);
    cr->paint();

    // center screen within window
    cr->translate(-screenCenter.x, -screenCenter.y);

    const double gridSpacing = 15.0;
    cr->set_source_rgb(0.7, 0.7, 0.7);
    cr->set_line_width(0.1);
    for (double x = bounds.l; x < bounds.r; x += gridSpacing) {
        cr->move_to(x, bounds.t);
        cr->line_to(x, bounds.b);
        cr->stroke();
    }
    for (double y = bounds.b; y < bounds.t; y += gridSpacing) {
        cr->move_to(bounds.l, y);
        cr->line_to(bounds.r, y);
        cr->stroke();
    }

    cr->set_source_rgb(0.0, 0.0, 0.0);
    cr->set_line_width(0.2);
    cr->move_to(bounds.l, bounds.t);
    cr->line_to(bounds.l, bounds.b);
    cr->line_to(bounds.r, bounds.b);
    cr->line_to(bounds.r, bounds.t);
    cr->line_to(bounds.l, bounds.t);
    cr->stroke();

    cpBody * const playerBody = hammerConstraint->a;
    cpBody * const hammerBody = hammerConstraint->b;
    cpVect anchor1 = cpPinJointGetAnchr1(hammerConstraint);
    cpVect anchor2 = cpPinJointGetAnchr2(hammerConstraint);
    const cpVect playerPos = cpBodyLocal2World(playerBody, anchor1)
            + cpBodyGetVelAtLocalPoint(playerBody, anchor1) * dt;
    const cpVect hammerPos = cpBodyLocal2World(hammerBody, anchor2)
            + cpBodyGetVelAtLocalPoint(hammerBody, anchor2) * dt;
    cr->set_line_width(0.7);
    cr->set_source_rgba(0.0, 0.0, 0.0, 0.5);
    cr->move_to(playerPos.x, playerPos.y);
    cr->line_to(hammerPos.x, hammerPos.y);
    cr->stroke();

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
