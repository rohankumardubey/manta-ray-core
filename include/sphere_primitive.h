#ifndef SPHERE_PRIMITIVE_H
#define SPHERE_PRIMITIVE_H

#include <scene_geometry.h>

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
		~SpherePrimitive();

		virtual math::real coarseIntersection(const LightRay *ray, IntersectionList *l, SceneObject *object, math::real depthHint, math::real epsilon) const;
		virtual void fineIntersection(const LightRay *ray, IntersectionPoint *p, CoarseIntersection *hint) const;
		virtual bool fastIntersection(const LightRay *ray) const;

		void detectIntersection(const LightRay *ray, IntersectionPoint *convex, IntersectionPoint *concave) const;

		math::real getRadius() const { return m_radius; }
		void setRadius(math::real radius) { m_radius = radius; }

	protected:
		math::real m_radius;
	};

} /* namespace manta */

#endif /* SPHERE_PRIMITIVE_H */
