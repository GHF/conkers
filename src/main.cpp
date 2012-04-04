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

#include <algorithm>
#include <iostream>
#include <vector>

#include <cstdlib>
#include <stdint.h>

int main(int argc, const char * const argv[]) {
    using namespace PixelToaster;
    using namespace Cairo;
    using namespace std;

    int width = 1280;
    int height = 800;

    if (argc == 3) {
        width = std::max(atoi(argv[1]), 300);
        height = std::max(atoi(argv[2]), 300);
    }

//    Display display("CONKERS - by Xo Wang", width, height, Output::Default, Mode::FloatingPoint);
    Display display("CONKERS - by Xo Wang", width, height, Output::Default, Mode::TrueColor);

//    vector<FloatingPointPixel> backBuffer(width * height);
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
    cr->scale(0.01, 0.01);
    const Matrix worldToScreen = cr->get_matrix();

    GameSys gameSys(width, height, worldToScreen);
    display.listener(&gameSys);

    SimLoop simLoop(&gameSys, 1.0 / 120);
    simLoop.start();

    while (display.open()) {
        cr->save();
        simLoop.acquireRenderLock();
        const double dt = simLoop.getRealTime() - simLoop.getLastSimTime();
        gameSys.render(cr, simLoop.getLastSimTime(), dt);
        simLoop.releaseRenderLock();
        cr->restore();

//        if ((uintptr_t(pixels.data()) & 0xF != 0) || (uintptr_t(backBuffer.data()) & 0xF != 0)) {
//            fprintf(stderr, "pixel buffer is not aligned\n");
//            break;
//        }
//        const __m128 scaling = _mm_set1_ps(1.0 / 255.0);
//        for (size_t i = 0; i < pixels.size(); i += 4) {
//            const __m128i bytes = _mm_load_si128((__m128i *) &pixels[i]);
//            const __m128i shorts0 = _mm_unpacklo_epi8(bytes, _mm_set1_epi8(0));
//            const __m128i shorts1 = _mm_unpackhi_epi8(bytes, _mm_set1_epi8(0));
//            const __m128i longs0 = _mm_unpacklo_epi16(shorts0, _mm_set1_epi16(0));
//            const __m128i longs1 = _mm_unpackhi_epi16(shorts0, _mm_set1_epi16(0));
//            const __m128i longs2 = _mm_unpacklo_epi16(shorts1, _mm_set1_epi16(0));
//            const __m128i longs3 = _mm_unpackhi_epi16(shorts1, _mm_set1_epi16(0));
//            const __m128 fp0 = _mm_cvtepi32_ps(longs0);
//            const __m128 fp1 = _mm_cvtepi32_ps(longs1);
//            const __m128 fp2 = _mm_cvtepi32_ps(longs2);
//            const __m128 fp3 = _mm_cvtepi32_ps(longs3);
//            _mm_store_ps((float *) &backBuffer[i], _mm_mul_ps(fp0, scaling));
//            _mm_store_ps((float *) &backBuffer[i + 1], _mm_mul_ps(fp1, scaling));
//            _mm_store_ps((float *) &backBuffer[i + 2], _mm_mul_ps(fp2, scaling));
//            _mm_store_ps((float *) &backBuffer[i + 3], _mm_mul_ps(fp3, scaling));
//        }
//
//        display.update(backBuffer);
        display.update(pixels);
    }

    simLoop.stop();

    return EXIT_SUCCESS;
}
