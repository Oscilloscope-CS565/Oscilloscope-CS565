#include <gtest/gtest.h>
#include <thread>

#include "ioCircularBuffer.h"
#include "ioScaleShiftPipeline.h"

using ioCircularBuffer::CircularBuffer;
using ioScaleShiftPipeline::ScaleShiftPipeline;

TEST(ScaleShiftPipeline, IdentityScaleNoShift) {
    CircularBuffer inBuf(16);
    CircularBuffer outBuf(16);
    ScaleShiftPipeline pipe(&inBuf, &outBuf);
    pipe.scale.store(1.0);
    pipe.shift.store(0.0);
    pipe.blinkDb0.store(false);

    const BYTE in[] = {0, 10, 127, 255};
    pipe.start();
    ASSERT_TRUE(inBuf.write(in, sizeof(in)));
    inBuf.setDone();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pipe.stopJoin();

    BYTE out[sizeof(in)] = {};
    std::size_t got = 0;
    while (got < sizeof(in)) {
        if (!outBuf.read(&out[got], 1)) {
            break;
        }
        ++got;
    }
    ASSERT_EQ(got, sizeof(in));
    for (std::size_t i = 0; i < sizeof(in); ++i) {
        EXPECT_EQ(out[i], in[i]) << "i=" << i;
    }
}

TEST(ScaleShiftPipeline, ScaleAndShiftRounded) {
    CircularBuffer inBuf(16);
    CircularBuffer outBuf(16);
    ScaleShiftPipeline pipe(&inBuf, &outBuf);
    pipe.scale.store(2.0);
    pipe.shift.store(3.0);
    pipe.blinkDb0.store(false);

    BYTE b = 10;
    pipe.start();
    ASSERT_TRUE(inBuf.write(&b, 1));
    inBuf.setDone();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pipe.stopJoin();

    BYTE o = 0;
    ASSERT_TRUE(outBuf.read(&o, 1));
    EXPECT_EQ(o, 23);
}

TEST(ScaleShiftPipeline, ClampHigh255) {
    CircularBuffer inBuf(8);
    CircularBuffer outBuf(8);
    ScaleShiftPipeline pipe(&inBuf, &outBuf);
    pipe.scale.store(10.0);
    pipe.shift.store(0.0);
    pipe.blinkDb0.store(false);

    BYTE b = 200;
    pipe.start();
    ASSERT_TRUE(inBuf.write(&b, 1));
    inBuf.setDone();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pipe.stopJoin();

    BYTE o = 0;
    ASSERT_TRUE(outBuf.read(&o, 1));
    EXPECT_EQ(o, 255);
}

TEST(ScaleShiftPipeline, ClampLow0) {
    CircularBuffer inBuf(8);
    CircularBuffer outBuf(8);
    ScaleShiftPipeline pipe(&inBuf, &outBuf);
    pipe.scale.store(1.0);
    pipe.shift.store(-50.0);
    pipe.blinkDb0.store(false);

    BYTE b = 10;
    pipe.start();
    ASSERT_TRUE(inBuf.write(&b, 1));
    inBuf.setDone();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pipe.stopJoin();

    BYTE o = 0xFF;
    ASSERT_TRUE(outBuf.read(&o, 1));
    EXPECT_EQ(o, 0);
}

TEST(ScaleShiftPipeline, BlinkDb0TogglesBit0) {
    CircularBuffer inBuf(16);
    CircularBuffer outBuf(16);
    ScaleShiftPipeline pipe(&inBuf, &outBuf);
    pipe.scale.store(1.0);
    pipe.shift.store(0.0);
    pipe.blinkDb0.store(true);

    const BYTE in[] = {0xFF, 0xFF, 0xFF, 0xFF};
    pipe.start();
    ASSERT_TRUE(inBuf.write(in, sizeof(in)));
    inBuf.setDone();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pipe.stopJoin();

    BYTE outs[4] = {};
    for (int i = 0; i < 4; ++i) {
        ASSERT_TRUE(outBuf.read(&outs[i], 1)) << "missing byte " << i;
    }
    EXPECT_EQ(outs[0] & 1u, 0u);
    EXPECT_EQ(outs[1] & 1u, 1u);
    EXPECT_EQ(outs[2] & 1u, 0u);
    EXPECT_EQ(outs[3] & 1u, 1u);
    EXPECT_EQ(outs[0] & 0xFEu, 0xFEu);
}
