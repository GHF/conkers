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

#include <chipmunk.h>

#include <cmath>

using namespace Cairo;
using namespace PixelToaster;

GameSys::GameSys(const Matrix &worldToScreen, double gameScale) :
        bgColor(), space(NULL), screenToWorld(worldToScreen), gameScale(gameScale) {
    screenToWorld.invert();
}

void GameSys::init() {
    cpVect gravity = cpv(0, -100);
    space = cpSpaceNew();
    cpSpaceSetGravity(space, gravity);
}

void GameSys::cleanup() {
    cpSpaceFree(space);
}

void GameSys::sim(double t, double dt) {
    cpSpaceStep(space, dt);
}

void GameSys::render(RefPtr<Context> cr, double t, double dt) {
    cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->paint();

    cr->set_source_rgb(0.0, 0.0, 0.0);
    double mouseX = mouse.x, mouseY = mouse.y;
    screenToWorld.transform_point(mouseX, mouseY);
    cr->arc(mouseX, mouseY, 0.05, 0, 2 * M_PI);
    cr->fill();

    cr->scale(gameScale, gameScale);
    for (GameObject &gameObject : gameObjects) {
        const cpBody * const body = gameObject.getBody();
        const cpVect pos = cpBodyGetPos(body) + cpBodyGetVel(body) * dt;
        const cpFloat angle = cpBodyGetAngle(body) + cpBodyGetAngVel(body) * dt;

        cr->save();

        cr->translate(pos.x, pos.y);
        cr->rotate(angle);
        gameObject.render(cr, t, dt);

        cr->restore();
    }
}

void GameSys::onMouseMove(DisplayInterface &display, Mouse mouse) {
    this->mouse = mouse;
}
