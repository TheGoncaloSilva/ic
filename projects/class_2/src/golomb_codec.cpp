#include "golomb_codec.h"

/*
##############################################################################
###################           Predictor Class              ###################
##############################################################################
*/

std::string get_type_string(PREDICTOR_TYPE type) {
    std::string predText = "AUTOMATIC (0)";
    if (type == PREDICT1)
        predText = "PREDICT1 (1)";
    else if (type == PREDICT2)
        predText = "PREDICT2 (2)";
    else if (type == PREDICT3)
        predText = "PREDICT3 (3)";
    else
        predText = "UNKNOWN";
    return predText;
}

std::string get_phase_string(PHASE phase) {
    std::string corrText = "AUTOMATIC (0)";
    if (phase == NO_CORRELATION)
        corrText = "NO CORRELATION (1)";
    else if (phase == CORRELATION_MID)
        corrText = "CORRELATION MID (2)";
    else if (phase == CORRELATION_SIDE)
        corrText = "CORRELATION SIDE (3)";
    else
        corrText = "UNKNOWN";
    return corrText;
}

Predictor::Predictor(int nChannels) {
    this->nChannels = nChannels;
}

Predictor::Predictor() {}

Predictor::~Predictor() {}

void Predictor::set_nChannels(int nChannels) {
    this->nChannels = nChannels;
}
int Predictor::get_nChannels() {
    return this->nChannels;
}

int Predictor::predict1(int a1) {
    return a1;
}

int Predictor::predict2(int a1, int a2) {
    return (2 * a1) - a2;
}

int Predictor::predict3(int a1, int a2, int a3) {
    return (3 * a1) - (3 * a2) + a3;
}

double Predictor::calculate_entropy(PREDICTOR_TYPE type, PHASE phase,
                                    std::vector<short>& samples) {
    int total_predictions = 0;
    std::vector<int> predictions;

    // Predict based on the given type and count occurrences of predictions
    for (size_t i = 0; i < samples.size(); ++i) {
        int prediction;
        if (type == PREDICT1) {
            prediction = predict(type, phase, samples, i);
        } else if (type == PREDICT2) {
            prediction = predict(type, phase, samples, i);
        } else if (type == PREDICT3) {
            prediction = predict(type, phase, samples, i);
        } else {
            cerr << "Error: Unknown Predictor type encountered" << endl;
            exit(2);
        }
        predictions.push_back(prediction);
        total_predictions++;
    }

    // Calculate probability distribution and entropy
    double entropy = 0.0;
    for (int value : predictions) {
        double probability =
            std::count(predictions.begin(), predictions.end(), value) /
            static_cast<double>(total_predictions);
        entropy -= probability * std::log2(probability);
    }

    return entropy;
}

int Predictor::predict_no_correlation(PREDICTOR_TYPE type,
                                      std::vector<short>& samples, int index) {
    // this code supports both mono and stereo sound
    int a1, a2, a3;

    if (nChannels == 1) {
        a1 = (index - 1) < 0 ? 0 : samples.at(index - 1);
        a2 = (index - 2) < 0 ? 0 : samples.at(index - 2);
        a3 = (index - 3) < 0 ? 0 : samples.at(index - 3);
    } else {
        // Go two Idx back, because of getting the previous value of the same channel
        a1 = (index - 2) < 0 ? 0 : samples.at(index - 2);
        a2 = (index - 4) < 0 ? 0 : samples.at(index - 4);
        a3 = (index - 6) < 0 ? 0 : samples.at(index - 6);
    }

    if (type == PREDICT1)
        return predict1(a1);
    else if (type == PREDICT2)
        return predict2(a1, a2);
    else
        return predict3(a1, a2, a3);
}

int Predictor::predict_correlated_mid(PREDICTOR_TYPE type,
                                      std::vector<short>& samples, int index) {
    std::vector<int> an;
    for (int i = 1; i <= 5; i += 2) {
        // odd index
        int channel1 = 0;
        if ((index - i) > (int)samples.size())
            channel1 = samples.at(index - i);

        // even index
        int channel2 = 0;
        if ((index - (i - 1)) > (int)samples.size())
            channel2 = samples.at(index - (i - 1));

        an.push_back((channel1 + channel2) / 2);
    }

    if (type == PREDICT1)
        return predict1(an.at(0));
    else if (type == PREDICT2)
        return predict2(an.at(0), an.at(1));
    else
        return predict3(an.at(0), an.at(1), an.at(2));
}

int Predictor::predict_correlated_side(PREDICTOR_TYPE type,
                                       std::vector<short>& samples, int index) {
    std::vector<int> an;
    for (int i = 1; i <= 5; i += 2) {
        // odd index
        int channel1 = 0;
        if ((index - i) > (int)samples.size())
            channel1 = samples.at(index - i);

        // even index
        int channel2 = 0;
        if ((index - (i - 1)) > (int)samples.size())
            channel2 = samples.at(index - (i - 1));

        an.push_back((channel1 - channel2) / 2);
    }

    if (type == PREDICT1)
        return predict1(an.at(0));
    else if (type == PREDICT2)
        return predict2(an.at(0), an.at(1));
    else
        return predict3(an.at(0), an.at(1), an.at(2));
}

int Predictor::predict(PREDICTOR_TYPE type, PHASE phase,
                       std::vector<short>& samples, int index) {
    if (!check_type(type) || type == AUTOMATIC) {
        cerr << "Error: Unknown/Invalid Predictor " << unsigned(type)
             << " encountered while decoding Block" << endl;
        exit(2);
    }

    if (!check_phase(phase) || phase == P_AUTOMATIC) {
        cerr << "Error: Unknown/Invalid Phase " << unsigned(phase)
             << " encountered while decoding Block" << endl;
        exit(2);
    }

    if (phase == NO_CORRELATION || nChannels == 1)
        return predict_no_correlation(type, samples, index);
    else if (phase == CORRELATION_MID)
        return predict_correlated_mid(type, samples, index);
    else  // correlation SIDE
        return predict_correlated_side(type, samples, index);
}

bool Predictor::check_type(PREDICTOR_TYPE type) {
    if (type == AUTOMATIC || type == PREDICT1 || type == PREDICT2 ||
        type == PREDICT3)
        return true;
    return false;
}

bool Predictor::check_phase(PHASE phase) {
    if (phase == NO_CORRELATION || phase == CORRELATION_MID ||
        phase == CORRELATION_SIDE || phase == P_AUTOMATIC)
        return true;
    return false;
}

std::tuple<PREDICTOR_TYPE, PHASE> Predictor::benchmark(
    std::vector<short>& samples, PREDICTOR_TYPE bestPredictor,
    PHASE bestPhase) {
    double min_entropy =
        std::numeric_limits<double>::max();  // Initialize to max

    PREDICTOR_TYPE initialPredictor = bestPredictor;
    PHASE initialPhase = bestPhase;

    // I know, kinda gross... This code should be optimized
    for (int p = NO_CORRELATION; p <= CORRELATION_SIDE; ++p) {
        PHASE phase;
        if (initialPhase != P_AUTOMATIC)
            phase = initialPhase;
        else
            phase = static_cast<PHASE>(p);

        if (initialPredictor == AUTOMATIC || initialPredictor == PREDICT1) {
            double entropy1 = calculate_entropy(PREDICT1, phase, samples);
            if (entropy1 <= min_entropy) {
                min_entropy = entropy1;
                bestPredictor = PREDICT1;
                bestPhase = phase;
            }
        }

        if (initialPredictor == AUTOMATIC || initialPredictor == PREDICT2) {
            double entropy2 = calculate_entropy(PREDICT2, phase, samples);
            if (entropy2 <= min_entropy) {
                min_entropy = entropy2;
                bestPredictor = PREDICT2;
                bestPhase = phase;
            }
        }

        if (initialPredictor == AUTOMATIC || initialPredictor == PREDICT3) {
            double entropy3 = calculate_entropy(PREDICT3, phase, samples);
            if (entropy3 <= min_entropy) {
                min_entropy = entropy3;
                bestPredictor = PREDICT3;
                bestPhase = phase;
            }
        }

        // No need to go further, since only no_correlation is supported for mono audio
        //  or the phase wasn't automatic
        if (nChannels == 1 || initialPhase != P_AUTOMATIC)
            break;
    }

    // Use the initial phase if bestPredictor is set to AUTOMATIC
    if (initialPhase != P_AUTOMATIC && initialPredictor == AUTOMATIC &&
        nChannels > 1) {
        bestPhase = initialPhase;
    }

    // Use the initial predictor if bestPhase is set to P_AUTOMATIC
    if (initialPhase == P_AUTOMATIC && initialPredictor != AUTOMATIC &&
        nChannels > 1) {
        bestPredictor = initialPredictor;
    }

    return std::make_tuple(bestPredictor, bestPhase);
}

/*
##############################################################################
###################         Golomb Encoder Code            ###################
##############################################################################
*/

GEncoder::GEncoder(std::string outFileName, int m = -1,
                   PREDICTOR_TYPE pred = AUTOMATIC, PHASE phase = P_AUTOMATIC)
    : writer('w', outFileName), golomb(DEFAULT_GOLOMB_M, writer) {

    this->m = m;
    this->outputFileName = outFileName;

    if (!predictorClass.check_type(pred)) {
        cerr << "Error: Unknown Predictor " << unsigned(pred)
             << " encountered while creating Encoder" << endl;
        exit(2);
    }
    this->predictor = pred;

    if (!predictorClass.check_phase(phase)) {
        cerr << "Error: Unknown Phase " << unsigned(phase)
             << " encountered while creating Encoder" << endl;
        exit(2);
    }
    this->phase = phase;
}

GEncoder::~GEncoder() {
    golomb.~Golomb();
    writer.~BitStream();
}

void GEncoder::setFile(File file) {
    this->fileStruct = file;
}

int GEncoder::test_calculate_m(std::vector<short>& values) {
    return calculate_m(values);
}

std::vector<unsigned short> GEncoder::test_abs_value_vector(
    std::vector<short>& values) {
    return abs_value_vector(values);
}

int GEncoder::test_lossy_error(int error, PREDICTOR_TYPE pred, PHASE phase,
                               int currentIndex, std::vector<short>& samples) {
    return lossy_error(error, pred, phase, currentIndex, samples);
}

std::vector<unsigned short> GEncoder::abs_value_vector(
    std::vector<short>& values) {
    std::vector<unsigned short> result;
    result.reserve(values.size());

    for (const auto& elem : values) {
        result.push_back(std::abs(elem));
    }

    return result;
}

int GEncoder::calculate_m(std::vector<short>& values) {
    /* Calculate alpha */
    double alpha = 1.0;
    if (!values.empty()) {
        std::vector<unsigned short> abs_values = abs_value_vector(values);
        double mean = static_cast<double>(std::accumulate(
                          abs_values.begin(), abs_values.end(), 0)) /
                      abs_values.size();
        alpha = exp(-1.0 / mean);  // Calculate alpha using mean
    }
    // Calculate 'm' based on the formula
    double aux = (-1 / log(alpha));
    int m = std::ceil(aux);

    // guarantee that minimum value is 1
    m = std::max(1, m);

    return std::min(m, 16383);  // cap golomb max size (14 bits)
}

void GEncoder::write_file() {

    // Write file header_values
    writer.writeNBits(fileStruct.blockSize, BITS_BLOCK_SIZE);
    writer.writeNBits(fileStruct.sampleRate, BITS_SAMPLE_RATE);
    writer.writeNBits(fileStruct.nFrames, BITS_N_FRAMES);
    writer.writeNBits(fileStruct.nChannels, BITS_N_CHANNELS);
    writer.writeNBits(fileStruct.bitRate, BITS_BIT_RATE);
    writer.writeNBits(fileStruct.lossy, BITS_LOSSY);
    writer.writeNBits(fileStruct.approach, BITS_APPROACH);
    golomb.setApproach(fileStruct.approach);

    // Write Blocks (data)
    std::cout << "\nWriting data to file..." << endl;

    int count = 1;
    for (auto& block : fileStruct.blocks) {
        // check if block size is correct
        if (block.data.size() != fileStruct.blockSize) {
            cerr << "Error: Block size mismatch" << endl;
            exit(2);
        }

        // Write Block header
        writer.writeNBits(block.m, BITS_M);
        writer.writeNBits(block.phase, BITS_PHASE);
        writer.writeNBits(block.predictor, BITS_PREDICTOR);

        golomb.setM(block.m);  // DONT FORGET THIS

        // Write Block data
        std::cout << " - Writing Block " << std::setw(3) << count++ << "/"
                  << std::setw(3) << fileStruct.blocks.size()
                  << " to file with m = " << std::setw(3) << unsigned(block.m)
                  << ", p = " << std::setw(1) << unsigned(block.predictor)
                  << " and phase = " << std::setw(1) << unsigned(block.phase)
                  << "  " << endl;  //<< "\r" << std::flush;

        for (short& sample : block.data)
            golomb.encode(sample);
    }
    std::cout << "\nAll data written to file\n\n";
}

int GEncoder::lossy_error(int error, PREDICTOR_TYPE pred, PHASE phase,
                          int currentIndex, std::vector<short>& samples) {
    // We cannot cut samples directly, since that would speed up the audio
    //  , so we need to cut the number of bits per sample required to represent
    //  the sample. We do that by quantizing the error and then the samples
    //  should be updated to guarantee that our audio pattern didn't get
    //  corrupted.

    // skip compiler warnings
    pred = pred;
    phase = phase;
    samples = samples;

    const int bitsPerSample = 16;  // size of short

    int defaultBitRate =  // in kbps
        (fileStruct.sampleRate * bitsPerSample * fileStruct.nChannels) / 1000;

    double averageBitsToEliminate =
        double(defaultBitRate - fileStruct.bitRate) /
        double(fileStruct.sampleRate * fileStruct.nChannels);

    //int bitsToEliminatePerSecond = defaultBitRate - fileStruct.bitRate;

    if (averageBitsToEliminate == 0.0 ||
        (currentIndex == 0 && fileStruct.nChannels == 1) ||
        (currentIndex < 2 && fileStruct.nChannels == 2))
        return error;

    // or error >> 1
    double tmpShift = double(defaultBitRate / fileStruct.bitRate);
    int shiftAmount = std::ceil(tmpShift);
    int adjustedError = error >> shiftAmount;

    //cout << "bitsToEliminate = " << averageBitsToEliminate << endl;
    //cout << "defaultBitRate = " << defaultBitRate << endl;
    //cout << "new bitRate = " << fileStruct.bitRate << endl;
    //cout << "bitsToEliminatePerSecond = " << bitsToEliminatePerSecond << endl;
    //cout << "scalingFactor = " << double(scalingFactor) << endl;

    // Here past samples should be adjusted with the error,
    //  according to predictor and phase chosen

    return adjustedError;
}

Block GEncoder::process_block(std::vector<short>& block, int blockId,
                              int nBlocks) {

    PREDICTOR_TYPE pred = predictor;
    PHASE ph = phase;
    if (pred == AUTOMATIC || ph == P_AUTOMATIC) {
        std::cout << " - "
                  << "Benchmarking predictor for Block " << std::setw(3)
                  << blockId + 1 << "/" << nBlocks << "\r" << std::flush;
        std::tuple<int, int> ret = predictorClass.benchmark(block, pred, ph);
        pred = static_cast<PREDICTOR_TYPE>(std::get<0>(ret));
        ph = static_cast<PHASE>(std::get<1>(ret));
    }

    Block encodedBlock;
    encodedBlock.predictor = pred;
    encodedBlock.phase = ph;

    for (int i = 0; i < (int)block.size(); i++) {
        int prediction = predictorClass.predict(pred, ph, block, i);
        int error = block.at(i) - prediction;

        if (fileStruct.lossy)
            error = lossy_error(error, pred, ph, i, block);

        encodedBlock.data.push_back(error);
    }

    // Use attributed m or calculate one
    int bM = m;
    if (bM < 1)
        bM = calculate_m(encodedBlock.data);
    encodedBlock.m = bM;

    return encodedBlock;
}

void GEncoder::encode_file(File file, std::vector<short>& inSamples,
                           size_t nBlocks) {
    this->fileStruct = file;
    this->predictorClass.set_nChannels(file.nChannels);
    this->golomb.setApproach(fileStruct.approach);

    std::cout << "Entering encoding phase" << std::endl;
    // Divide in blocks and process each one
    for (int i = 0; i < (int)nBlocks; i++) {
        std::vector<short> block;
        for (int j = 0; j < file.blockSize; j++)
            block.push_back(inSamples[i * file.blockSize + j]);

        Block encodedBlock = process_block(block, i, nBlocks);

        // Add encoded block to file
        fileStruct.blocks.push_back(encodedBlock);
    }
    if (predictor == AUTOMATIC)
        std::cout << "\n";

    std::cout << "All blocks encoded. Writing to file" << endl;

    write_file();
}

/*
##############################################################################
###################         Golomb Decoder Code            ###################
##############################################################################
*/

GDecoder::GDecoder(std::string inFileName)
    : reader('r', inFileName), golomb(DEFAULT_GOLOMB_M, reader) {

    this->inputFileName = inFileName;
}

GDecoder::~GDecoder() {
    golomb.~Golomb();
    reader.~BitStream();
}

File& GDecoder::read_file() {
    std::cout << "Reading file " << inputFileName;

    // Read file header
    fileStruct.blockSize = reader.readNBits(BITS_BLOCK_SIZE);
    fileStruct.sampleRate = reader.readNBits(BITS_SAMPLE_RATE);
    fileStruct.nFrames = reader.readNBits(BITS_N_FRAMES);
    fileStruct.nChannels = reader.readNBits(BITS_N_CHANNELS);
    fileStruct.bitRate = reader.readNBits(BITS_BIT_RATE);
    fileStruct.lossy = reader.readNBits(BITS_LOSSY);
    fileStruct.approach =
        static_cast<APPROACH>(reader.readNBits(BITS_APPROACH));

    cout << "Music Processing information: \n"
         << "\n - Block Size: " << fileStruct.blockSize
         << "\n - Number of Channels: " << unsigned(fileStruct.nChannels)
         << "\n - Sample Rate: " << fileStruct.sampleRate
         << "\n - Total Number of Frames: " << unsigned(fileStruct.nFrames)
         << "\n - Golomb Approach: " << approach_to_string(fileStruct.approach)
         << "\n - Encode type: " << (fileStruct.lossy ? "lossy" : "lossless")
         << "\n - Bit Rate: " << fileStruct.bitRate << "\n"
         << endl;

    // Write Blocks (data)
    int nBlocks{static_cast<int>(
        ceil(static_cast<double>(fileStruct.nFrames) / fileStruct.blockSize))};

    nBlocks *= (int)fileStruct.nChannels;

    std::cout << " with " << unsigned(nBlocks) << " blocks" << endl;

    if (!check_approach(fileStruct.approach)) {
        cerr << "Error: Invalid approach type " << fileStruct.approach << endl;
        exit(1);
    }
    golomb.setApproach(fileStruct.approach);
    predictorClass.set_nChannels(fileStruct.nChannels);

    //std::ofstream outputFile("decoded_values_distribution.txt");

    for (int bId = 0; bId < nBlocks; bId++) {
        Block block;
        // Read Block header
        block.m = reader.readNBits(BITS_M);
        block.phase = static_cast<PHASE>(reader.readNBits(BITS_PHASE));
        block.predictor =
            static_cast<PREDICTOR_TYPE>(reader.readNBits(BITS_PREDICTOR));

        // Read Block data
        this->golomb.setM(block.m);
        std::cout << " - Reading Block " << std::setw(3) << bId + 1
                  << " with m = " << std::setw(3) << unsigned(block.m)
                  << ", predictor = " << std::setw(1)
                  << unsigned(block.predictor)
                  << " and phase = " << std::setw(1) << unsigned(block.phase)
                  << endl;

        // check m
        if (block.m < 1) {
            cerr << "Error: Encountered invalid m = " << unsigned(block.m)
                 << endl;
            exit(2);
        }

        for (uint16_t i = 0; i < fileStruct.blockSize; i++) {
            int data = golomb.decode();
            block.data.push_back((short)data);
            //outputFile << data << "\n";
        }

        fileStruct.blocks.push_back(block);
    }
    std::cout << "All data read from file" << std::endl;

    //outputFile.close();

    return fileStruct;
}

std::vector<short> GDecoder::decode_block(Block& block) {
    std::vector<short> samples;
    PREDICTOR_TYPE pred = static_cast<PREDICTOR_TYPE>(block.predictor);
    PHASE ph = static_cast<PHASE>(block.phase);

    for (int i = 0; i < (int)block.data.size(); i++) {
        int prediction = predictorClass.predict(pred, ph, samples, i);
        // error + prediction
        int sample = block.data.at(i) + prediction;
        samples.push_back(sample);
    }

    return samples;
}

std::vector<short> GDecoder::decode_file() {
    std::vector<short> outSamples;
    std::cout << "Decoding file with " << unsigned(fileStruct.blocks.size())
              << " Blocks" << endl;
    int count = 1;
    for (Block& block : fileStruct.blocks) {
        std::cout << " - Decoding Block: " << count++ << "\r" << std::flush;
        std::vector<short> blockSamples = decode_block(block);
        outSamples.insert(outSamples.end(), blockSamples.begin(),
                          blockSamples.end());
    }
    std::cout << "\nAll Blocks decoded\n" << endl;

    return outSamples;
}