#include "../include/cft_estimator_2d.h"
#include "../include/complex_map_2d.h"

#include <assert.h>

manta::CftEstimator2D::CftEstimator2D() {
    m_physicalWidth = (math::real_d)0.0;
    m_physicalHeight = (math::real_d)0.0;

    m_horizontalSamples = 0;
    m_verticalSamples = 0;

    m_discreteApproximation = nullptr;
}

manta::CftEstimator2D::~CftEstimator2D() {
    /* void */
}

void manta::CftEstimator2D::initialize(
    ComplexMap2D *cft,
    math::real_d physicalWidth,
    math::real_d physicalHeight)
{
    m_physicalWidth = physicalWidth;
    m_physicalHeight = physicalHeight;

    m_verticalSamples = cft->getWidth();
    m_horizontalSamples = cft->getHeight();

    m_discreteApproximation = cft;
}

void manta::CftEstimator2D::destroy() {
    /* void */
}

manta::math::Complex manta::CftEstimator2D::sample(
    math::real_d freq_x,
    math::real_d freq_y,
    math::real_d w) const
{
    math::real_d k_x, k_y;

    k_x = freq_x * m_physicalWidth;
    k_y = freq_y * m_physicalHeight;

    int i = (int)k_x;
    int j = (int)k_y;

    // Wrap indices
    i = (i < 0) ? m_horizontalSamples + i : i;
    j = (j < 0) ? m_verticalSamples + j : j;

    if (i >= m_horizontalSamples) return math::Complex();
    if (j >= m_verticalSamples) return math::Complex();
    if (i < 0) return math::Complex();
    if (j < 0) return math::Complex();

    return m_discreteApproximation->sampleDiscrete(i, j);
}

manta::math::Complex manta::CftEstimator2D::sampleDiscrete(int i, int j) const {
    return m_discreteApproximation->sampleDiscrete(i, j);
}

manta::math::real_d manta::CftEstimator2D::getFrequencyX(int i) const {
    if (i >= m_horizontalSamples / 2) i -= m_horizontalSamples;

    return i / m_physicalWidth;
}

manta::math::real_d manta::CftEstimator2D::getFrequencyY(int j) const {
    if (j >= m_verticalSamples / 2) j -= m_verticalSamples;

    return j / m_physicalHeight;
}

manta::math::real_d manta::CftEstimator2D::getHorizontalFreqRange() const {
    return getFreqRange(m_horizontalSamples, m_physicalWidth);
}

manta::math::real_d manta::CftEstimator2D::getVerticalFreqRange() const {
    return getFreqRange(m_verticalSamples, m_physicalHeight);
}

manta::math::real_d manta::CftEstimator2D::getHorizontalFreqStep() const {
    return getFreqStep(m_physicalWidth);
}

manta::math::real_d manta::CftEstimator2D::getVerticalFreqStep() const {
    return getFreqStep(m_physicalHeight);
}

manta::math::real_d manta::CftEstimator2D::getFreqRange(
    int horizontalSamples,
    math::real_d physicalWidth) 
{
    math::real_d maxFreq = (horizontalSamples / 2) / physicalWidth;
    return maxFreq;
}

manta::math::real_d manta::CftEstimator2D::getFreqStep(math::real_d physicalWidth) {
    return 1 / physicalWidth;
}

manta::math::real_d manta::CftEstimator2D::getMinPhysicalDim(
    math::real_d freqStep, math::real_d minDim) 
{
    math::real_d perfect = 1 / freqStep;
    return (perfect > minDim) ? perfect : minDim;
}

int manta::CftEstimator2D::getMinSamples(
    math::real_d maxFreq, math::real_d physicalDim, int maxSamples) 
{
    // Calculate a rough approximation based on the maximum frequency
    int firstApprox = (int)(maxFreq * physicalDim * 2);

    // Round to a power of 2
    int minSamples = 1;
    while (minSamples < firstApprox && (minSamples < maxSamples || maxSamples < 0)) {
        minSamples <<= 1;
    }

    return minSamples;
}
