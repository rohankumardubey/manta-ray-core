#include "../include/convolution_node.h"

#include "../include/complex_map_2d.h"
#include "../include/vector_map_2d_node_output.h"

manta::ConvolutionNode::ConvolutionNode() {
    m_base = nullptr;
    m_filter = nullptr;
    
    m_resize = true;
    m_clip = true;
}

manta::ConvolutionNode::~ConvolutionNode() {
    /* void */
}

void manta::ConvolutionNode::_initialize() {
    /* void */
}

void manta::ConvolutionNode::_evaluate() {
    // Cast inputs
    VectorNodeOutput *a = static_cast<VectorNodeOutput *>(m_base);
    VectorNodeOutput *b = static_cast<VectorNodeOutput *>(m_filter);

    Margins margins;
    VectorMap2D a_mapSafe;
    const VectorMap2D *a_map, *b_map;
    VectorMap2D a_computed, b_computed;

    a->getDataReference((const void **)&a_map);
    b->getDataReference((const void **)&b_map);

    bool computedA = false, computedB = false;

    if (a_map == nullptr) {
        a->calculateAllDimensions(&a_computed);
        a_map = &a_computed;
        computedA = true;
    }
    if (b_map == nullptr) {
        b->calculateAllDimensions(&b_computed);
        b_map = &b_computed;
        computedB = true;
    }

    if (m_resize) {
        a_map->padSafe(&a_mapSafe, &margins);
        a_map = &a_mapSafe;

        if (!m_clip) {
            // If the output is not cropped then we can just take
            // the full safe dimensions
            margins.width = a_map->getWidth();
            margins.height = a_map->getHeight();
            margins.left = 0;
            margins.top = 0;
        }
    }
    else {
        margins.width = a_map->getWidth();
        margins.height = a_map->getHeight();
        margins.left = 0;
        margins.top = 0;
    }

    m_outputMap.initialize(margins.width, margins.height);

    // Perform 3-channel convolution
    ComplexMap2D a_c, a_ft;
    ComplexMap2D b_c, b_ft;

    for (int i = 0; i < b_map->getWidth(); ++i) {
        for (int j = 0; j < b_map->getHeight(); ++j) {
            if (math::getY(b_map->get(i, j)) != 0 && j < 128) {
                int a = 0;
            }
        }
    }

    for (int i = 0; i < 3; i++) {
        a_c.copy(a_map, i);
        a_c.fft_multithreaded(&a_ft, 12); 
        a_c.destroy();

        b_c.copy(b_map, i);
        b_c.fft_multithreaded(&b_ft, 12); 
        b_c.destroy();

        a_ft.cftConvolve(&b_ft, (math::real_d)1.0, (math::real_d)1.0); 
        b_ft.destroy();

        a_ft.fft_multithreaded(&b_ft, 12, true);
        a_ft.destroy();

        for (int u = margins.left; u < margins.left + margins.width; u++) {
            for (int v = margins.top; v < margins.top + margins.height; v++) {
                const int iu = u - margins.left, iv = v - margins.top;

                math::Vector result = m_outputMap.get(iu, iv);
                math::set(result, i, (math::real)b_ft.get(u, v).r);

                assert(!std::isnan(b_ft.get(u, v).r));
                assert(!std::isinf(b_ft.get(u, v).r));

                m_outputMap.set(result, iu, iv);
            }
        }

        b_ft.destroy();
    }

    if (m_resize) {
        a_mapSafe.destroy();
    }

    if (computedA) a_computed.destroy();
    if (computedB) b_computed.destroy();

    m_output.setMap(&m_outputMap);
}

void manta::ConvolutionNode::_destroy() {
    m_outputMap.destroy();
}

void manta::ConvolutionNode::registerInputs() {
    registerInput(&m_base, "base");
    registerInput(&m_filter, "filter");
}

void manta::ConvolutionNode::registerOutputs() {
    setPrimaryOutput("__out");
    registerOutput(&m_output, "__out");
}
