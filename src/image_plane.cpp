#include "../include/image_plane.h"

#include "../include/standard_allocator.h"
#include "../include/gaussian_filter.h"
#include "../include/triangle_filter.h"
#include "../include/box_filter.h"
#include "../include/vector_map_2d.h"

#include <assert.h>
#include <iostream>

manta::ImagePlane::ImagePlane() {
    m_width = 0;
    m_height = 0;
    m_buffer = nullptr;
    m_sampleWeightSums = nullptr;
    m_filter = nullptr;

    m_filterInput = nullptr;
    m_previewTarget = nullptr;

    m_resolutionXInput = nullptr;
    m_resolutionYInput = nullptr;

    m_windowX0Input = nullptr;
    m_windowX1Input = nullptr;
    m_windowY0Input = nullptr;
    m_windowY1Input = nullptr;

    m_windowBottom = 0;
    m_windowTop = 0;
    m_windowLeft = 0;
    m_windowRight = 0;
}

manta::ImagePlane::~ImagePlane() {
    assert(m_buffer == nullptr);
    assert(m_sampleWeightSums == nullptr);
}

void manta::ImagePlane::initialize(int width, int height) {
    assert(width != 0);
    assert(height != 0);

    m_width = width;
    m_height = height;

    const int pixelCount = width * height;

    m_buffer = (math::Vector *)_aligned_malloc(sizeof(math::Vector) * pixelCount, 16);
    m_sampleWeightSums = StandardAllocator::Global()->allocate <math::real>(pixelCount);

    assert(m_buffer != nullptr);

    for (int i = 0; i < pixelCount; i++) {
        m_buffer[i] = math::constants::Zero;
        m_sampleWeightSums[i] = (math::real)0.0;
    }

    m_windowLeft = 0;
    m_windowRight = width - 1;
    m_windowTop = 0;
    m_windowBottom = height - 1;
}

void manta::ImagePlane::destroy() {
    if (m_buffer != nullptr) _aligned_free(m_buffer);
    if (m_sampleWeightSums != nullptr) StandardAllocator::Global()->free(m_sampleWeightSums);

    // Reset member variables
    m_buffer = nullptr;
    m_sampleWeightSums = nullptr;
    m_width = 0;
    m_height = 0;    
}

bool manta::ImagePlane::checkPixel(int x, int y) const {
    return x < m_width && x >= 0 && y < m_height && y >= 0;
}

bool manta::ImagePlane::inWindow(int x, int y) const {
    return x >= m_windowLeft && x <= m_windowRight && y >= m_windowTop && y <= m_windowBottom;
}

inline void manta::ImagePlane::set(const math::Vector &v, int x, int y) {
    assert(x < m_width && x >= 0);
    assert(y < m_height && y >= 0);

    m_buffer[y * m_width + x] = v;
}

manta::math::Vector manta::ImagePlane::sample(int x, int y) const {
    assert(x < m_width && x >= 0);
    assert(y < m_height && y >= 0);

    return m_buffer[y * m_width + x];
}

void manta::ImagePlane::copyFrom(const ImagePlane *source) {
    initialize(m_width, m_height);
    for (int x = 0; x < (m_width); x++) {
        for (int y = 0; y < (m_height); y++) {
            set(sample(x, y), x, y);
        }
    }
}

void manta::ImagePlane::createEmptyFrom(const ImagePlane *source) {
    initialize(source->m_width, source->m_height);
}

void manta::ImagePlane::clear(const math::Vector &v) {
    for (int x = 0; x < m_width; x++) {
        for (int y = 0; y < m_height; y++) {
            set(v, x, y);
        }
    }
}

#define FAST_ABS(x) (((x) > 0) ? (x) : -(x))

void manta::ImagePlane::add(const math::Vector &v, int x, int y) {
    if (x >= m_width || x < 0) return;
    if (y >= m_height || y < 0) return;

    m_buffer[y * m_width + x] = math::add(m_buffer[y * m_width + x], v);

    if (m_previewTarget != nullptr) {
        m_previewTarget->set(m_buffer[y * m_width + x], x, y);
    }
}

void manta::ImagePlane::processSamples(ImageSample *samples, int sampleCount, StackAllocator *stack) {
    struct Block {
        math::Vector value;
        math::real weight;
        int x, y;
    };

    Block *blocks = (Block *)stack->allocate(sizeof(Block), 16);
    Block *currentBlock = blocks;

    int blockCount = 0;
    for (int i = 0; i < sampleCount; i++) {
        const ImageSample &sample = samples[i];
        const math::Vector2 extents = m_filter->getExtents();
        const int left = (int)(floor(sample.imagePlaneLocation.x - extents.x));
        const int right = (int)(ceil(sample.imagePlaneLocation.x + extents.x) + (math::real)0.5);
        const int top = (int)(floor(sample.imagePlaneLocation.y - extents.y));
        const int bottom = (int)(ceil(sample.imagePlaneLocation.y + extents.y) + (math::real)0.5); 

        for (int x = left; x <= right; x++) {
            for (int y = top; y <= bottom; y++) {
                if (!checkPixel(x, y)) continue;

                const math::Vector2 p(
                    sample.imagePlaneLocation.x - (math::real)x, 
                    sample.imagePlaneLocation.y - (math::real)y
                );

                if (FAST_ABS(p.x) > extents.x || FAST_ABS(p.y) > extents.y) continue;

                const math::Vector weight = m_filter->evaluate(p);

                currentBlock->value = math::mul(weight, sample.intensity);
                currentBlock->weight = math::getScalar(weight);
                currentBlock->x = x;
                currentBlock->y = y;
                currentBlock++;
                blockCount++;
            }
        }
    }

    std::unique_lock<std::mutex> lock(m_lock);
    for (int i = 0; i < blockCount; i++) {
        const Block &block = blocks[i];
        
        math::Vector &value = m_buffer[block.y * m_width + block.x];
        math::real &weightSum = m_sampleWeightSums[block.y * m_width + block.x];

        value = math::add(value, block.value);
        weightSum += block.weight;
    }

    if (m_previewTarget != nullptr) {
        for (int i = 0; i < blockCount; ++i) {
            const Block &block = blocks[i];

            const math::Vector value = m_buffer[block.y * m_width + block.x];
            const math::real weightSum = m_sampleWeightSums[block.y * m_width + block.x];

            if (weightSum != 0) {
                m_previewTarget->set(math::div(value, math::loadScalar(weightSum)), block.x, block.y);
            }
        }
    }

    stack->free(blocks);
}

void manta::ImagePlane::normalize() {
    for (int x = 0; x < m_width; x++) {
        for (int y = 0; y < m_height; y++) {
            math::Vector *value = &m_buffer[y * m_width + x];
            math::real weightSum = m_sampleWeightSums[y * m_width + x];

            *value = (weightSum == 0)
                ? *value
                : math::div(*value, math::loadScalar(weightSum));

            constexpr math::Vector DebugRed = { { (math::real)1.0, (math::real)0.0, (math::real)0.0 } };
            constexpr math::Vector DebugBlue = { { (math::real)0.0, (math::real)0.0, (math::real)1.0 } };
            constexpr math::Vector DebugGreen = { { (math::real)0.0, (math::real)1.0, (math::real)0.0 } };

            if (std::isnan(math::getX(*value)) || std::isnan(math::getY(*value)) || std::isnan(math::getZ(*value))) {
                *value = DebugRed;
            }
            else if (std::isinf(math::getX(*value)) || std::isinf(math::getY(*value)) || std::isinf(math::getZ(*value))) {
                *value = DebugGreen;
            }
            else if (math::getX(*value) < 0 || math::getY(*value) < 0 || math::getZ(*value) < 0) {
                *value = DebugBlue;
            }
        }
    }
}

void manta::ImagePlane::_evaluate() {
    m_filter = getObject<Filter>(m_filterInput);

    piranha::native_int width, height;
    m_resolutionXInput->fullCompute((void *)&width);
    m_resolutionYInput->fullCompute((void *)&height);
    initialize(width, height);

    piranha::native_int x0, x1, y0, y1;
    m_windowX0Input->fullCompute((void *)&x0);
    m_windowX1Input->fullCompute((void *)&x1);
    m_windowY0Input->fullCompute((void *)&y0);
    m_windowY1Input->fullCompute((void *)&y1);

    m_windowLeft = std::min(x0, x1);
    m_windowRight = std::max(x0, x1);
    m_windowTop = std::min(y0, y1);
    m_windowBottom = std::max(y0, y1);

    m_output.setReference(this);
}

void manta::ImagePlane::_initialize() {
    /* void */
}

void manta::ImagePlane::_destroy() {
    destroy();
}

void manta::ImagePlane::registerInputs() {
    registerInput(&m_filterInput, "filter");
    registerInput(&m_resolutionXInput, "width");
    registerInput(&m_resolutionYInput, "height");
    registerInput(&m_windowX0Input, "w_x0");
    registerInput(&m_windowX1Input, "w_x1");
    registerInput(&m_windowY0Input, "w_y0");
    registerInput(&m_windowY1Input, "w_y1");
}

void manta::ImagePlane::registerOutputs() {
    ObjectReferenceNode::registerOutputs();
}
