#ifndef MANTARAY_REMAP_NODE_OUTPUT_H
#define MANTARAY_REMAP_NODE_OUTPUT_H

#include "vector_node_output.h"

#include "manta_math.h"
#include "intersection_point.h"

namespace manta {

    class RemapNodeOutput : public VectorNodeOutput {
    public:
        RemapNodeOutput() {
            setDimensions(1);
            setDimensionSize(0, 1);

            m_start = math::constants::One;
            m_end = math::constants::Zero;

            m_input = nullptr;
        }

        virtual ~RemapNodeOutput() {
            /* void */
        }

        virtual void sample(const IntersectionPoint *surfaceInteraction, void *target) const;
        virtual void discreteSample2D(int x, int y, void *target) const;
        virtual void fullOutput(const void **target) const;

        void setInput(piranha::pNodeInput input) { m_input = input; }
        piranha::pNodeInput getInput() const { return m_input; }

        void setStart(const math::Vector &start) { m_start = start; }
        math::Vector getStart() const { return m_start; }

        void setEnd(const math::Vector &end) { m_end = end; }
        math::Vector getEnd() const { return m_end; }

        piranha::pNodeInput *getInputConnection() { return &m_input; }

    protected:
        math::Vector remap(const math::Vector &s) const;

    protected:
        piranha::pNodeInput m_input;

        math::Vector m_start;
        math::Vector m_end;
    };

} /* namespace manta */

#endif /* MANTARAY_REMAP_NODE_OUTPUT_H */
