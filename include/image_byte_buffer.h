#ifndef MANTARAY_IMAGE_BYTE_BUFFER_H
#define MANTARAY_IMAGE_BYTE_BUFFER_H

#include "manta_math.h"

namespace manta {

    // Forward declarations
    class ImagePlane;

    class ImageByteBuffer {
    public:
        struct Color {
            unsigned char r, g, b, a;
        };

    public:
        ImageByteBuffer();
        ~ImageByteBuffer();

        void initialize(const ImagePlane *sceneBuffer, bool correctGamma);
        void initialize(const unsigned char *buffer, int width, int height, int pitch);
        void initialize(const math::Vector *buffer, int width, int height, bool correctGamma);
        void initialize(const math::real *buffer, int width, int height, bool correctGamma);
        void initialize(int width, int height);
        void free();

        unsigned char *getBuffer() const { return m_buffer; }

        int getPitch() const { return m_pitch; }
        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }

        void setPixel(int row, int column, const Color &c);
        void convertToColor(const math::Vector &v, bool correctGamma, Color *c) const;

    protected:
        int m_pitch;
        int m_width;
        int m_height;

        unsigned char *m_buffer;
    };

} /* namespace manta */

#endif /* MANTARAY_IMAGE_BYTE_BUFFER_H */
