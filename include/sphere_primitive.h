#ifndef MANTARAY_SPHERE_PRIMITIVE_H
#define MANTARAY_SPHERE_PRIMITIVE_H

#include "scene_geometry.h"

#include "runtime_statistics.h"

namespace manta {

    class SpherePrimitive : public SceneGeometry {
    public:
        enum DetectionMode {
            CONVEX_ONLY,
            CONCAVE_ONLY,
            STANDARD
        };

    public:
        SpherePrimitive();
        virtual ~SpherePrimitive();

        virtual bool findClosestIntersection(LightRay *ray, CoarseIntersection *intersection, 
            math::real minDepth, math::real maxDepth, StackAllocator *s /**/ STATISTICS_PROTOTYPE) const;
        virtual math::Vector getClosestPoint(const CoarseIntersection *hint, 
            const math::Vector &p) const;
        virtual void getVicinity(const math::Vector &p, math::real radius, IntersectionList *list, 
            SceneObject *object) const;
        virtual void fineIntersection(const math::Vector &r, IntersectionPoint *p, 
            const CoarseIntersection *hint) const;
        virtual bool fastIntersection(LightRay *ray) const;
        virtual bool occluded(const math::Vector &p0, const math::Vector &d, math::real maxDepth /**/ STATISTICS_PROTOTYPE) const { return false; }

        void detectIntersection(const LightRay *ray, IntersectionPoint *convex, 
            IntersectionPoint *concave) const;

        math::real getRadius() const { return m_radius; }
        void setRadius(math::real radius) { m_radius = radius; }

    protected:
        virtual void _initialize();
        virtual void _evaluate();

        virtual void registerInputs();

        piranha::pNodeInput m_radiusInput;

    protected:
        math::real m_radius;
    };

} /* namespace manta */

#endif /* MANTARAY_SPHERE_PRIMITIVE_H */
