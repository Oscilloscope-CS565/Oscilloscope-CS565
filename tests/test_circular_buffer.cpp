#include <gtest/gtest.h>
#include <thread>
#include <vector>

#include "ioCircularBuffer.h"

using ioCircularBuffer::CircularBuffer;

TEST(CircularBuffer, SingleByteRoundTrip) {
    CircularBuffer buf(4);
    BYTE in = 0xAB;
    ASSERT_TRUE(buf.write(&in, 1));
    BYTE out = 0;
    ASSERT_TRUE(buf.read(&out, 1));
    EXPECT_EQ(out, 0xAB);
    EXPECT_EQ(buf.getCount(), 0u);
}

TEST(CircularBuffer, MultipleBytesPreservesOrder) {
    CircularBuffer buf(8);
    const BYTE data[] = {1, 2, 3, 4, 5};
    ASSERT_TRUE(buf.write(data, sizeof(data)));
    BYTE readBack[sizeof(data)] = {};
    ASSERT_TRUE(buf.read(readBack, sizeof(data)));
    for (size_t i = 0; i < sizeof(data); ++i) {
        EXPECT_EQ(readBack[i], data[i]) << "index " << i;
    }
}

TEST(CircularBuffer, WrapAroundHeadTail) {
    CircularBuffer buf(3);
    for (int round = 0; round < 5; ++round) {
        BYTE a = static_cast<BYTE>(10 + round);
        BYTE b = static_cast<BYTE>(20 + round);
        ASSERT_TRUE(buf.write(&a, 1));
        ASSERT_TRUE(buf.write(&b, 1));
        BYTE ra = 0, rb = 0;
        ASSERT_TRUE(buf.read(&ra, 1));
        ASSERT_TRUE(buf.read(&rb, 1));
        EXPECT_EQ(ra, a);
        EXPECT_EQ(rb, b);
    }
}

TEST(CircularBuffer, SetDoneUnblocksReaderOnEmpty) {
    CircularBuffer buf(2);
    bool read_ok = false;
    std::thread consumer([&]() {
        BYTE x = 0;
        read_ok = buf.read(&x, 1);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    buf.setDone();
    consumer.join();
    EXPECT_FALSE(read_ok);
}

TEST(CircularBuffer, ProducerConsumerConcurrent) {
    constexpr std::size_t kCapacity = 64;
    constexpr int kTotal = 10000;
    CircularBuffer buf(kCapacity);
    std::vector<int> errors;
    std::thread producer([&]() {
        for (int i = 0; i < kTotal; ++i) {
            BYTE b = static_cast<BYTE>(i & 0xFF);
            if (!buf.write(&b, 1)) {
                errors.push_back(-1);
                return;
            }
        }
        buf.setDone();
    });
    int received = 0;
    while (true) {
        BYTE b = 0;
        if (!buf.read(&b, 1)) {
            break;
        }
        int expected = received & 0xFF;
        if (static_cast<int>(b) != expected) {
            errors.push_back(received);
            break;
        }
        ++received;
    }
    producer.join();
    EXPECT_TRUE(errors.empty());
    EXPECT_EQ(received, kTotal);
}
