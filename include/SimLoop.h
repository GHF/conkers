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

#ifndef SIMLOOP_H_
#define SIMLOOP_H_

#include "GameSys.h"

#include "../PixelToaster/PixelToaster.h"

#include <pthread.h>

#include <atomic>

class SimLoop {
protected:
    GameSys *gameSys;
    const double dt;
    volatile double t;
    volatile bool simRun;
    std::atomic<int> simLock;
    std::atomic<int> simLockReaders;
    pthread_t simThread;
    PixelToaster::Timer timer;

    static void *simLoop(void *arg);
    void loop();
    void acquireSimLock();
    void releaseSimLock();

public:
    SimLoop(GameSys *gameSys, double dt);

    void start();
    void stop();
    void acquireRenderLock();
    void releaseRenderLock();
    double getLastSimTime() const {
        return t;
    }
    double getRealTime() {
        return timer.time();
    }
};

#endif /* SIMLOOP_H_ */
