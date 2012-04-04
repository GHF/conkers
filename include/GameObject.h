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

#ifndef GAMEOBJECT_H_
#define GAMEOBJECT_H_

#include <chipmunk.h>
#include <cairomm/cairomm.h>

#include <algorithm>

class GameObject {
    friend class GameSys;

protected:
    cpBody *body;
    double expireTime;
    bool alive;
    double hP;
    double maxHP;

public:
    enum CollisionGroup {
        PLAYER = 1, ENEMY = 2, ENVIRONMENT = 3
    };

    GameObject(cpFloat mass, cpFloat moment, const cpVect &pos = cpvzero) :
            body(NULL), expireTime(INFINITY), alive(true), hP(68), maxHP(100) {
        body = cpBodyNew(mass, moment);
        cpBodySetPos(body, pos);
        cpBodySetUserData(body, this);
    }

    virtual ~GameObject() {
        if (body != NULL) {
            cpSpaceRemoveBody(body->space_private, body);
            cpBodyFree(body);
        }
    }

    virtual cpBody *getBody() {
        return body;
    }

    virtual const cpBody *getBody() const {
        return body;
    }

    virtual double timeToLive(double t) {
        return std::max(expireTime - t, 0.0);
    }

    virtual bool isAlive() const {
        return alive;
    }

    virtual double getHP() const {
        return hP;
    }

    virtual double getMaxHP() const {
        return maxHP;
    }

    virtual void init(cpSpace *space) = 0;
    virtual void sim(double t, double dt) = 0;
    virtual void render(Cairo::RefPtr<Cairo::Context> cr, double t, double dt) = 0;

    virtual void damagingHit(GameObject *other, const cpVect &relVel, double t) {
        if (!alive || (other != NULL && !other->isAlive()))
            return;
        hP = cpfclamp(hP - cpvlength(relVel) * 0.10, 0, maxHP);
        if (hP == 0.0) {
            alive = false;
            expireTime = t + 1.0;
        }
    }
};

#endif /* GAMEOBJECT_H_ */
