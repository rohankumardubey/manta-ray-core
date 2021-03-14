#ifndef MANTARAY_PREVIEW_NODE_H
#define MANTARAY_PREVIEW_NODE_H

#include "vector_node_output.h"
#include "session.h"

namespace manta {

    class PreviewNode : public piranha::Node {
    public:
        PreviewNode();
        virtual ~PreviewNode();

        VectorNodeOutput *getOutput();

        piranha::native_string getTitle() const { return m_title; }

    private:
        piranha::native_string m_title;

    protected:
        virtual void _initialize();
        virtual void _evaluate();
        virtual void _destroy();

        virtual void registerOutputs();
        virtual void registerInputs();

    protected:
        piranha::pNodeInput m_titleInput;
        piranha::pNodeInput m_dataInput;
    };

} /* namespace manta */

#endif /* MANTARAY_IMAGE_PLANE_PREVIEW_NODE_H */
