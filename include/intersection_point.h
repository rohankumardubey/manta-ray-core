#ifndef INTERSECTION_POINT_H
#define INTERSECTION_POINT_H

#include <manta_math.h>

namespace manta {

	struct IntersectionPoint {
	public:
		math::Vector m_position;
		math::Vector m_normal;
		math::real m_depth;

		bool m_intersection;
	};

} /* namespace manta */

#endif /* INTERSECTION_POINT_H */