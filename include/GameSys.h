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

#ifndef GAMESYS_H_
#define GAMESYS_H_

#include "GameObject.h"

#include "../PixelToaster/PixelToaster.h"

#include <cairomm/cairomm.h>

#include <vector>
#include <memory>
#include <random>

class GameSys: public PixelToaster::Listener {
protected:
    enum GameState {
        WAITING,
        RUNNING,
        TOPSCORE
    };

    double t;
    double bgColor[3];
    cpSpace *space;

    int screenWidth;
    int screenHeight;
    Cairo::Matrix worldToScreen;
    Cairo::Matrix screenToWorld;

    PixelToaster::Mouse mouse;
    cpBody *mouseBody;
    cpConstraint *mouseJoint;
    cpConstraint *hammerConstraint;

    std::vector<std::shared_ptr<GameObject>> gameObjects;

    cpVect screenCenter;
    cpBB bounds;
    cpShape *walls[4];

    double damageTimer;
    uint64_t score;
    GameState state;

    std::mt19937_64 randomGenerator;

public:
    GameSys(int screenWidth, int screenHeight, const Cairo::Matrix &screenToWorld);

    void init();
    void sim(double t, double dt);
    void cleanup();
    void render(Cairo::RefPtr<Cairo::Context> cr, double t, double dt);

    void onMouseMove(PixelToaster::DisplayInterface &display, PixelToaster::Mouse mouse);
    void onKeyUp(PixelToaster::DisplayInterface &display, PixelToaster::Key key);

    int playerEnemyCollision(cpArbiter *arb, struct cpSpace *space);
};

#endif /* GAMESYS_H_ */
