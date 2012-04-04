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
#include "ButterEnemyObject.h"

#include <chipmunk.h>

#include <algorithm>
#include <cmath>

using namespace std;
using namespace Cairo;
using namespace PixelToaster;

GameSys::GameSys(int screenWidth, int screenHeight, const Matrix &worldToScreen) :
        t(0.0),
                bgColor( { 1.0, 1.0, 1.0 }),
                screenWidth(screenWidth),
                screenHeight(screenHeight),
                worldToScreen(worldToScreen),
                screenToWorld(worldToScreen),
                screenCenter(cpvzero),
                bounds(cpBBNew(-105, -90, 105, 90)),
                damageTimer(-INFINITY),
                score(0),
                state(WAITING) {
    screenToWorld.invert();
}

static int playerEnemyCollision(cpArbiter *arb, struct cpSpace *space, void *data) {
    return static_cast<GameSys *>(data)->playerEnemyCollision(arb, space);
}

void GameSys::init() {
    space = cpSpaceNew();
    cpSpaceSetDamping(space, 0.3);

    shared_ptr<PlayerObject> player(new PlayerObject(10.0, 4.0));
    gameObjects.push_back(player);

    shared_ptr<HammerObject> hammer(new HammerObject(20.0, 7.0, 7.0, cpv(0, -12.0)));
    gameObjects.push_back(hammer);

    for (size_t i = 0; i < 10; i++) {
        shared_ptr<ButterEnemyObject> enemy(new ButterEnemyObject(player, 2.0, 8.0, cpv(10 + i, 17)));
        gameObjects.push_back(enemy);
    }

    hammerConstraint = cpPinJointNew(player->getBody(), hammer->getBody(), cpvzero, cpv(0, 3.0));
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
    walls[0] = cpSegmentShapeNew(space->staticBody, cpv(bounds.l, bounds.t + 1), cpv(bounds.l, bounds.b - 1), 0);
    walls[1] = cpSegmentShapeNew(space->staticBody, cpv(bounds.l - 1, bounds.b), cpv(bounds.r + 1, bounds.b), 0);
    walls[2] = cpSegmentShapeNew(space->staticBody, cpv(bounds.r, bounds.t + 1), cpv(bounds.r, bounds.b - 1), 0);
    walls[3] = cpSegmentShapeNew(space->staticBody, cpv(bounds.l - 1, bounds.t), cpv(bounds.r + 1, bounds.t), 0);
    const cpFloat wallFriction = 0.6;
    for (cpShape *wall : walls) {
        cpShapeSetFriction(wall, wallFriction);
        cpShapeSetUserData(wall, NULL);
        cpShapeSetCollisionType(wall, GameObject::ENVIRONMENT);
        cpSpaceAddShape(space, wall);
    }

    cpSpaceAddCollisionHandler(space,
            GameObject::ENVIRONMENT,
            GameObject::ENEMY,
            ::playerEnemyCollision,
            NULL,
            NULL,
            NULL,
            this);

    cpSpaceAddCollisionHandler(space,
            GameObject::ENEMY,
            GameObject::ENEMY,
            ::playerEnemyCollision,
            NULL,
            NULL,
            NULL,
            this);

    cpSpaceAddCollisionHandler(space,
            GameObject::PLAYER,
            GameObject::ENEMY,
            ::playerEnemyCollision,
            NULL,
            NULL,
            NULL,
            this);
}

void GameSys::cleanup() {
    gameObjects.resize(0); // delete all objects before freeing space
    cpSpaceFree(space);
    cpConstraintFree(mouseJoint);
    cpConstraintFree(hammerConstraint);
    cpBodyFree(mouseBody);
}

void GameSys::sim(double t, double dt) {
    this->t = t;

    size_t numEnemiesWanted = score / 1000 + (score % 100) / 10;
    numEnemiesWanted = min(numEnemiesWanted, size_t(100));
    numEnemiesWanted = max(numEnemiesWanted, size_t(1));
    uniform_real_distribution<> xDistribution(bounds.l, bounds.r);
    uniform_real_distribution<> yDistribution(bounds.b, bounds.t);
    if (state == RUNNING && gameObjects.size() - 2 < numEnemiesWanted) {
        if (generate_canonical<double, 16>(randomGenerator) < numEnemiesWanted * 0.1 * dt) {
            cpVect pos = cpv(xDistribution(randomGenerator), yDistribution(randomGenerator));
            while (cpvdistsq(pos, cpBodyGetPos(gameObjects[0]->getBody())) < 289) {
                pos = cpv(xDistribution(randomGenerator), yDistribution(randomGenerator));
            }
            shared_ptr<ButterEnemyObject> enemy(new ButterEnemyObject(gameObjects[0], 2.0, 8.0, pos));
            enemy->init(space);
            cpSpaceAddBody(space, enemy->getBody());
            gameObjects.push_back(enemy);
        }
    }

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

    vector<shared_ptr<GameObject>>::iterator newEnd = remove_if(gameObjects.begin() + 2,
            gameObjects.end(),
            [=](shared_ptr<GameObject> gameObject) -> bool {
                return !gameObject->isAlive() && gameObject->timeToLive(t) <= 0.0;
            });
    gameObjects.resize(newEnd - gameObjects.begin());

    if (state == RUNNING && gameObjects[0]->isAlive() == false) {
        state = TOPSCORE;
        for (size_t i = 2; i < gameObjects.size(); i++) {
            gameObjects[i]->alive = false;
            gameObjects[i]->expireTime = t + 1.0;
        }
    }
}

static void renderText(RefPtr<Context> cr, const string &s, double size, double x, double y, bool centered = true) {
    cr->select_font_face("Gotham Rounded Bold", FONT_SLANT_NORMAL, FONT_WEIGHT_NORMAL);
    cr->set_font_size(size);
    TextExtents te;
    cr->get_text_extents(s, te);
    if (centered) {
        cr->move_to(x - te.width / 2 - te.x_bearing, y - te.height / 2 - te.y_bearing);
    } else {
        cr->move_to(x - te.x_bearing, -y - te.y_bearing);
    }
    cr->show_text(s);
}

void GameSys::render(RefPtr<Context> cr, double t, double dt) {
    bgColor[1] = cpflerp(0.0, 1.0, cpfclamp01(5 * (t - damageTimer)));
    bgColor[2] = bgColor[1];
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
    cr->set_line_width(1.0);
    cr->set_source_rgb(0.0, 0.0, 0.0);
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

    if (state == WAITING) {
        cr->scale(1.0, -1.0);
        cr->set_source_rgb(0.0, 0.0, 0.0);
        renderText(cr, "Press SPACE to start", 10, 0, -30);
        renderText(cr, "CONKERS", 30, 0, 0);
        renderText(cr, "Xo Wang & Nathan Hays", 12, 0, 30);
    } else if (state == RUNNING) {
        double scoreLeft = 20;
        double scoreTop = 20;
        screenToWorld.transform_point(scoreLeft, scoreTop);
        // show score
        char scoreText[9];
        snprintf(scoreText, 9, "%08ld", (long) score);
        cr->translate(screenCenter.x, screenCenter.y);
        cr->scale(1.0, -1.0);
        cr->set_source_rgb(0.0, 0.0, 0.0);
        renderText(cr, scoreText, 7, scoreLeft, scoreTop, false);
    } else if (state == TOPSCORE) {
        char scoreText[9];
        snprintf(scoreText, 9, "%08ld", (long) score);
        cr->translate(screenCenter.x, screenCenter.y);
        cr->scale(1.0, -1.0);
        cr->set_source_rgb(0.0, 0.0, 0.0);
        renderText(cr, "GAME OVER", 17, 0, -35);
        renderText(cr, "2. 00000000", 10, 0, 0);
        renderText(cr, "3. 00000000", 10, 0, 12);
        renderText(cr, "4. 00000000", 10, 0, 24);
        renderText(cr, "restart game to play again :(", 6, 0, 40);
        cr->set_source_rgba(0.0, 0.0, 0.0, 0.2 + 0.8 * sin(t * M_PI));
        renderText(cr, string("1. ") + scoreText, 10, 0, -12);
    }
}

void GameSys::onMouseMove(DisplayInterface &display, Mouse mouse) {
    this->mouse = mouse;
}

void GameSys::onKeyUp(DisplayInterface &display, Key key) {
    switch (key) {
    case Key::Space: {
        if (state == WAITING) {
            state = RUNNING;
            gameObjects.resize(2);
        }
        break;
    }

    default:
        break;
    }
}

int GameSys::playerEnemyCollision(cpArbiter *arb, struct cpSpace *space) {
    CP_ARBITER_GET_SHAPES(arb, aShape, enemyShape);
    CP_ARBITER_GET_BODIES(arb, aBody, enemyBody);

    const cpVect relVel = cpBodyGetVel(aBody) - cpBodyGetVel(enemyBody);
    GameObject *aObject = static_cast<GameObject *>(cpShapeGetUserData(aShape));
    GameObject *enemy = static_cast<GameObject *>(cpShapeGetUserData(enemyShape));
    if (cpShapeGetUserData(aShape) == gameObjects[0].get()) { // player & enemy
        if (state == RUNNING) {
            if (enemy->isAlive()) {
                if (damageTimer < t) {
                    damageTimer = t + 0.05;
                } else {
                    damageTimer += 0.05;
                }
            }
            aObject->damagingHit(enemy, relVel, t);
        }
    } else if (cpShapeGetUserData(aShape) == gameObjects[1].get()) { // hammer & enemy

    } else if (cpShapeGetUserData(aShape) == NULL) { // wall & enemy

    } else { // probably enemy & enemy, so send collision to both
        if (state == RUNNING) {
            aObject->damagingHit(enemy, relVel, t);
        }
    }

    if (state == RUNNING) {
        enemy->damagingHit(aObject, -relVel, t);
        score += cpvlength(relVel);
    }

    return 1;
}
