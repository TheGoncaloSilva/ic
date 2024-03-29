// Import of standard or provided libraries
#include <gtest/gtest.h>
#include <cmath>
#include <string>

// Import of local libraries
#include <bit_stream.h>
#include <golomb.h>

#define TEST_FILE_NAME "test_bitStream.bin"

using namespace std;

struct File {
    uint16_t blockSize;
    uint8_t nChannels;
    uint16_t sampleRate;
    uint32_t nFrames;
    uint16_t quantizationBits;
    bool lossy;  // true if lossy, false if lossless
    uint16_t timesPlayed;
};

TEST(BitStream, multipleValues) {
    File inF;
    inF.blockSize = 1024;
    inF.nChannels = 2;
    inF.sampleRate = 44100;
    inF.nFrames = 100;
    inF.quantizationBits = 8;
    inF.lossy = true;
    inF.timesPlayed = 12324;

    BitStream writer('w', TEST_FILE_NAME);

    writer.writeNBits(inF.blockSize, 16);
    writer.writeNBits(inF.nChannels, 8);
    writer.writeNBits(inF.sampleRate, 16);
    writer.writeNBits(inF.nFrames, 32);
    writer.writeNBits(inF.quantizationBits, 8);
    writer.writeNBits(inF.lossy, 2);
    writer.writeNBits(inF.timesPlayed, 16);

    writer.~BitStream();

    File outF;
    BitStream reader('r', TEST_FILE_NAME);

    outF.blockSize = reader.readNBits(16);
    outF.nChannels = reader.readNBits(8);
    outF.sampleRate = reader.readNBits(16);
    outF.nFrames = reader.readNBits(32);
    outF.quantizationBits = reader.readNBits(8);
    outF.lossy = reader.readNBits(2);
    outF.timesPlayed = reader.readNBits(16);

    reader.~BitStream();

    EXPECT_EQ(inF.blockSize, outF.blockSize);
    EXPECT_EQ(inF.nChannels, outF.nChannels);
    EXPECT_EQ(inF.sampleRate, outF.sampleRate);
    EXPECT_EQ(inF.nFrames, outF.nFrames);
    EXPECT_EQ(inF.quantizationBits, outF.quantizationBits);
    EXPECT_EQ(inF.lossy, outF.lossy);
    EXPECT_EQ(inF.timesPlayed, outF.timesPlayed);

    remove(TEST_FILE_NAME);
};

TEST(BitStream, testWGolomb) {
    File inF;
    inF.blockSize = 1024;
    inF.nChannels = 2;
    inF.sampleRate = 44100;
    inF.nFrames = 100;
    inF.quantizationBits = 8;
    inF.lossy = true;
    inF.timesPlayed = 12324;

    BitStream writer('w', TEST_FILE_NAME);

    writer.writeNBits(inF.blockSize, 16);
    writer.writeNBits(inF.nChannels, 8);
    writer.writeNBits(inF.sampleRate, 16);
    writer.writeNBits(inF.nFrames, 32);
    writer.writeNBits(inF.quantizationBits, 8);
    writer.writeNBits(inF.lossy, 2);
    writer.writeNBits(inF.timesPlayed, 16);

    Golomb golomb(4, writer);
    std::list<int> values;
    for (int i = 0; i <= 20; i++) {
        golomb.encode(i);
        values.push_back(i);
    }
    int blockM = 4;
    int sequence = 7;
    writer.writeNBits(blockM, 8);
    writer.writeNBits(sequence, 4);

    golomb.~Golomb();

    File outF;
    BitStream reader('r', TEST_FILE_NAME);

    outF.blockSize = reader.readNBits(16);
    outF.nChannels = reader.readNBits(8);
    outF.sampleRate = reader.readNBits(16);
    outF.nFrames = reader.readNBits(32);
    outF.quantizationBits = reader.readNBits(8);
    outF.lossy = reader.readNBits(2);
    outF.timesPlayed = reader.readNBits(16);

    EXPECT_EQ(inF.blockSize, outF.blockSize);
    EXPECT_EQ(inF.nChannels, outF.nChannels);
    EXPECT_EQ(inF.sampleRate, outF.sampleRate);
    EXPECT_EQ(inF.nFrames, outF.nFrames);
    EXPECT_EQ(inF.quantizationBits, outF.quantizationBits);
    EXPECT_EQ(inF.lossy, outF.lossy);
    EXPECT_EQ(inF.timesPlayed, outF.timesPlayed);

    Golomb golomb2(4, reader);
    for (int i = 0; i <= 20; i++) {
        int decoded = golomb2.decode();
        int original = values.front();
        values.pop_front();
        EXPECT_EQ(decoded, original);
    }

    EXPECT_EQ(blockM, reader.readNBits(8));
    EXPECT_EQ(sequence, reader.readNBits(4));

    golomb2.~Golomb();

    remove(TEST_FILE_NAME);
};

TEST(BitStream, testWGolombIntensive) {
    File inF;
    inF.blockSize = 1024;
    inF.nChannels = 2;
    inF.sampleRate = 44100;
    inF.nFrames = 100;
    inF.quantizationBits = 8;
    inF.lossy = true;
    inF.timesPlayed = 12324;

    const int DEFAULT_M = 100;
    const int DEFAULT_P = 1;
    const int GOLOMB_M = 4;

    BitStream writer('w', TEST_FILE_NAME);

    writer.writeNBits(inF.blockSize, 16);
    writer.writeNBits(inF.nChannels, 8);
    writer.writeNBits(inF.sampleRate, 16);
    writer.writeNBits(inF.nFrames, 32);
    writer.writeNBits(inF.quantizationBits, 8);
    writer.writeNBits(inF.lossy, 2);
    writer.writeNBits(inF.timesPlayed, 16);

    Golomb golomb(GOLOMB_M, writer);
    std::list<int> values;
    // simulate blocks
    for (int i = 0; i <= 20; i++) {
        writer.writeNBits(DEFAULT_M, 8);  // m value
        writer.writeNBits(DEFAULT_P, 4);  // predictor value
        for (int j = 0; j < 4; j++) {
            golomb.encode(j);
            values.push_back(j);
        }
    }

    golomb.~Golomb();

    File outF;
    BitStream reader('r', TEST_FILE_NAME);

    outF.blockSize = reader.readNBits(16);
    outF.nChannels = reader.readNBits(8);
    outF.sampleRate = reader.readNBits(16);
    outF.nFrames = reader.readNBits(32);
    outF.quantizationBits = reader.readNBits(8);
    outF.lossy = reader.readNBits(2);
    outF.timesPlayed = reader.readNBits(16);

    EXPECT_EQ(inF.blockSize, outF.blockSize);
    EXPECT_EQ(inF.nChannels, outF.nChannels);
    EXPECT_EQ(inF.sampleRate, outF.sampleRate);
    EXPECT_EQ(inF.nFrames, outF.nFrames);
    EXPECT_EQ(inF.quantizationBits, outF.quantizationBits);
    EXPECT_EQ(inF.lossy, outF.lossy);
    EXPECT_EQ(inF.timesPlayed, outF.timesPlayed);

    Golomb golomb2(GOLOMB_M, reader);
    // Read blocks and check data
    for (int i = 0; i <= 20; i++) {  // number of blocks
        int m = reader.readNBits(8);
        int predictor = reader.readNBits(4);
        EXPECT_EQ(m, DEFAULT_M);
        EXPECT_EQ(predictor, DEFAULT_P);
        for (int j = 0; j < 4; j++) {  // number of samples per block
            int decoded = golomb2.decode();
            int original = values.front();
            values.pop_front();
            EXPECT_EQ(decoded, original);
        }
    }

    golomb2.~Golomb();

    remove(TEST_FILE_NAME);
};

TEST(BitStream, testWGolombIntensive_2) {

    for (int GOLOMB_M = 2; GOLOMB_M <= 20; GOLOMB_M++) {

        File inF;
        inF.blockSize = 1024;
        inF.nChannels = 2;
        inF.sampleRate = 44100;
        inF.nFrames = 100;
        inF.quantizationBits = 8;
        inF.lossy = true;
        inF.timesPlayed = 12324;

        const int DEFAULT_M = 100;
        const int DEFAULT_P = 1;

        BitStream writer('w', TEST_FILE_NAME);

        writer.writeNBits(inF.blockSize, 16);
        writer.writeNBits(inF.nChannels, 8);
        writer.writeNBits(inF.sampleRate, 16);
        writer.writeNBits(inF.nFrames, 32);
        writer.writeNBits(inF.quantizationBits, 8);
        writer.writeNBits(inF.lossy, 2);
        writer.writeNBits(inF.timesPlayed, 16);

        Golomb golomb(GOLOMB_M, writer);
        // simulate blocks
        for (int i = 0; i < 20; i++) {
            writer.writeNBits(DEFAULT_M, 8);  // m value
            writer.writeNBits(DEFAULT_P, 4);  // predictor value
            for (int j = 0; j < 10; j++)
                golomb.encode(10);
        }

        golomb.~Golomb();

        File outF;
        BitStream reader('r', TEST_FILE_NAME);

        outF.blockSize = reader.readNBits(16);
        outF.nChannels = reader.readNBits(8);
        outF.sampleRate = reader.readNBits(16);
        outF.nFrames = reader.readNBits(32);
        outF.quantizationBits = reader.readNBits(8);
        outF.lossy = reader.readNBits(2);
        outF.timesPlayed = reader.readNBits(16);

        EXPECT_EQ(inF.blockSize, outF.blockSize);
        EXPECT_EQ(inF.nChannels, outF.nChannels);
        EXPECT_EQ(inF.sampleRate, outF.sampleRate);
        EXPECT_EQ(inF.nFrames, outF.nFrames);
        EXPECT_EQ(inF.quantizationBits, outF.quantizationBits);
        EXPECT_EQ(inF.lossy, outF.lossy);
        EXPECT_EQ(inF.timesPlayed, outF.timesPlayed);

        Golomb golomb2(GOLOMB_M, reader);
        // Read blocks and check data
        for (int i = 0; i < 20; i++) {  // number of blocks
            int m = reader.readNBits(8);
            int predictor = reader.readNBits(4);
            EXPECT_EQ(m, DEFAULT_M);
            EXPECT_EQ(predictor, DEFAULT_P);
            // number of samples per block
            for (int j = 0; j < 10; j++) {
                int decoded = golomb2.decode();
                EXPECT_EQ(decoded, 10);
            }
        }

        golomb2.~Golomb();

        remove(TEST_FILE_NAME);
    }
};