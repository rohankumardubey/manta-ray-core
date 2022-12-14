#include "../../include/manta.h"

#include "../../include/manta_math.h"
#include "../../include/standard_allocator.h"
#include "../../include/signal_processing.h"
#include "../../include/complex_map_2d.h"
#include "../../include/vector_map_2d.h"
#include "../../include/fraunhofer_diffraction.h"
#include "../../include/cft_estimator_2d.h"
#include "../../include/circular_aperture.h"
#include "../../include/polygonal_aperture.h"
#include "../../include/image_byte_buffer.h"
#include "../../include/jpeg_writer.h"
#include "../../include/texture_node.h"
#include "../../include/intersection_point.h"
#include "../../include/spectrum.h"
#include "../../include/cmf_table.h"
#include "../../include/rgb_space.h"

#include <chrono>
#include <sstream>

#define CMF_PATH "../../demos/cmfs/"
#define WORKSPACE_PATH "../../workspace/"
#define TMP_PATH (WORKSPACE_PATH "tmp/")
#define TEXTURE_PATH "../../demos/textures/"

using namespace manta;

void writeToJpeg(const ComplexMap2D *plane, const std::string &fname, Margins *margins = nullptr) {
    ImageByteBuffer byteBuffer;
    plane->fillByteBuffer(&byteBuffer, margins);

    JpegWriter writer;
    writer.setQuality(95);
    writer.write(&byteBuffer, fname.c_str());

    byteBuffer.free();
}

void writeToJpeg(const VectorMap2D *vectorMap, const std::string &fname) {
    ImageByteBuffer byteBuffer;
    vectorMap->fillByteBuffer(&byteBuffer, true);

    JpegWriter writer;
    writer.setQuality(95);
    writer.write(&byteBuffer, fname.c_str());

    byteBuffer.free();
}

int main() {
    constexpr int SENSOR_RESOLUTION = 1024;
    constexpr int MAX_SAMPLES = 4096;

    constexpr math::real_d apertureRadius = 1.0f;
    constexpr math::real_d sensorWidth = 0.1f; // mm
    constexpr math::real_d sensorElementWidth = sensorWidth / SENSOR_RESOLUTION;

    constexpr math::real_d minWavelength = 380e-6;
    constexpr math::real_d maxWavelength = 780e-6;

    math::real_d minFrequencyStep = sensorElementWidth / maxWavelength;
    math::real_d maxFrequencyStep = sensorElementWidth / minWavelength;

    int estimatorSamples = 0;
    math::real_d sampleWindow = 0.0;
    math::real_d maxFreq = (sensorWidth / 2) / minWavelength;

    sampleWindow = CftEstimator2D::getMinPhysicalDim(minFrequencyStep / 3, apertureRadius * 2);
    estimatorSamples = CftEstimator2D::getMinSamples(maxFreq, sampleWindow, MAX_SAMPLES);

    TextureNode dirtTexture;
    dirtTexture.loadFile(TEXTURE_PATH "pupil.png", true);
    dirtTexture.initialize();
    dirtTexture.evaluate();
    const VectorMap2D *texture = dirtTexture.getMainOutput()->getMap();
    //texture = nullptr;

    PolygonalAperture aperture;
    aperture.setRadius(apertureRadius);
    aperture.setBladeCurvature(1.0f);
    aperture.configure(6, math::constants::PI / 2);

    int maxResolution = (int)((CftEstimator2D::getFreqRange(estimatorSamples, sampleWindow) * maxWavelength + sensorWidth / 2) / sensorElementWidth);

    std::cout << "Parameters =====================" << std::endl;
    std::cout << "Spatial extent: " << sampleWindow << " mm" << std::endl;
    std::cout << "Estimator samples: " << estimatorSamples << std::endl;
    std::cout << "Frequency step: " << CftEstimator2D::getFreqStep(sampleWindow) << " 1/mm" << std::endl;
    std::cout << "Frequency range: " << CftEstimator2D::getFreqRange(estimatorSamples, sampleWindow) << " 1/mm" << std::endl;
    std::cout << "Minimum sensor frequency step (380 nm light): " << minFrequencyStep << "1/mm" << std::endl;
    std::cout << "Max resolution: " << maxResolution << " px" << std::endl;

    FraunhoferDiffraction fraun;
    FraunhoferDiffraction::Settings settings;
    fraun.setDefaultSettings(&settings);

    settings.frequencyMultiplier = (math::real_d)8.0;
    settings.maxSamples = MAX_SAMPLES;
    settings.saveApertureFunction = true;
    settings.textureSamples = 10;
    //settings.minWavelength = 500;
    //settings.maxWavelength = 505;
    settings.wavelengthStep = 5;

    CmfTable table;
    table.loadCsv(CMF_PATH "xyz_cmf_31.csv");
    Spectrum source;
    source.loadCsv(CMF_PATH "d65_lighting.csv");

    std::cout << "Estimating Fraunhofer diffraction" << std::endl;
    auto startTime = std::chrono::system_clock::now();

    fraun.generate(&aperture, texture, SENSOR_RESOLUTION, sensorWidth, &table, &source, &settings);

    auto endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = endTime - startTime;

    std::cout << "Calculation time: " << diff.count() << " s" << std::endl;

    source.destroy();

    std::cout << "Writing to file" << std::endl;
    writeToJpeg(fraun.getDiffractionPattern(), std::string(TMP_PATH) + "sensing_plane.jpg");
    writeToJpeg(fraun.getApertureFunction(), std::string(TMP_PATH) + "aperture_function.jpg");

    fraun.destroy();
}