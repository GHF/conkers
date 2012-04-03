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
#include "GameSys.h"

#include "../PixelToaster/PixelToaster.h"

#include <pthread.h>
#include <cairomm/cairomm.h>

#include <iostream>
#include <vector>

#include <cstdlib>
#include <stdint.h>

int main(int argc, const char * const argv[]) {
    using namespace PixelToaster;
    using namespace Cairo;
    using namespace std;

    const int width = 960;
    const int height = 600;

    Display display(argv[0], width, height, Output::Default, Mode::TrueColor);

    vector<TrueColorPixel> pixels(width * height);

    RefPtr<ImageSurface> surface = ImageSurface::create((unsigned char *) pixels.data(),
            FORMAT_ARGB32,
            width,
            height,
            ImageSurface::format_stride_for_width(FORMAT_ARGB32, width));
    RefPtr<Context> cr = Context::create(surface);

    const int minDim = std::min(width, height);
    cr->translate((width - minDim) / 2, (height - minDim) / 2);
    cr->scale(minDim, -minDim);
    cr->translate(0.5, -0.5);
    const Matrix worldToScreen = cr->get_matrix();

    GameSys gameSys(worldToScreen, 0.01);
    display.listener(&gameSys);

    SimLoop simLoop(&gameSys, 1.0 / 60);
    simLoop.start();

    while (display.open()) {
        cr->save();
        simLoop.acquireRenderLock();
        const double dt = simLoop.getRealTime() - simLoop.getLastSimTime();
        gameSys.render(cr, simLoop.getLastSimTime(), dt);
        simLoop.releaseRenderLock();
        cr->restore();
        display.update(pixels);
    }

    simLoop.stop();

    return EXIT_SUCCESS;
}
