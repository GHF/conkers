/*
 * GameObject.h
 *
 *  Created on: Apr 2, 2012
 *      Author: Xo
 */

#ifndef GAMEOBJECT_H_
#define GAMEOBJECT_H_

#include <chipmunk.h>
#include <cairomm/cairomm.h>

class GameObject {
protected:
    cpBody *body;

public:
    GameObject(cpFloat mass, cpFloat moment, const cpVect &pos = cpvzero) :
            body(NULL) {
        body = cpBodyNew(mass, moment);
        cpBodySetPos(body, pos);
    }

    virtual ~GameObject() {
        if (body != NULL) {
            cpBodyFree(body);
        }
    }

    cpBody *getBody() {
        return body;
    }

    const cpBody *getBody() const {
        return body;
    }

    virtual void sim(double t, double dt) = 0;
    virtual void render(Cairo::RefPtr<Cairo::Context> cr, double t, double dt) = 0;
};

#endif /* GAMEOBJECT_H_ */
