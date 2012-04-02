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

#include "SimLoop.h"

#include <sys/time.h>

SimLoop::SimLoop(GameSys *gameSys, double dt) :
        gameSys(gameSys), dt(dt), simRun(false), simLock(0), simLockReaders(0) {
}

void *SimLoop::simLoop(void *arg) {
    static_cast<SimLoop *>(arg)->loop();
    return NULL;
}

void SimLoop::loop() {
    while (simRun) {
        double realTime = timer.time();
        while (t < realTime - dt) {
            while (simLockReaders > 0)
                sched_yield();
            acquireSimLock();
            gameSys->sim(t, dt);
            t += dt;
            releaseSimLock();
        }

        const double timeToNextStep = realTime - t;
        if (timeToNextStep > 0.01) {
            sched_yield();
        }
    }
    gameSys->cleanup();
}

void SimLoop::acquireSimLock() {
    while (true) {
        while (simLock != 0)
            sched_yield();
        if (__sync_val_compare_and_swap(&simLock, 0, 1) == 0)
            break;
    }
}

void SimLoop::releaseSimLock() {
    __sync_lock_release(&simLock);
}

void SimLoop::start() {
    simRun = true;
    gameSys->init();
    pthread_create(&simThread, NULL, simLoop, this);
}

void SimLoop::stop() {
    simRun = false;
    pthread_join(simThread, NULL);
}

void SimLoop::acquireRenderLock() {
    __sync_fetch_and_add(&simLockReaders, 1);
    acquireSimLock();
    __sync_fetch_and_sub(&simLockReaders, 1);
}

void SimLoop::releaseRenderLock() {
    releaseSimLock();
}
