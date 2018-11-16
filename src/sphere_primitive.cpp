#include <sphere_primitive.h>

#include <light_ray.h>
#include <intersection_point.h>

manta::SpherePrimitive::SpherePrimitive() {
	m_radius = (math::real)0.0;
}

manta::SpherePrimitive::~SpherePrimitive() {

}

void manta::SpherePrimitive::detectIntersection(const LightRay *ray, IntersectionPoint *p) const {
	math::Vector d_pos = math::sub(ray->getSource(), m_position);
	math::Vector d_dot_dir = math::dot(d_pos, ray->getDirection());
	math::Vector mag2 = math::magnitudeSquared3(d_pos);

	math::Vector radius2 = math::loadScalar(m_radius * m_radius);
	math::Vector det = math::sub(math::mul(d_dot_dir, d_dot_dir), math::sub(mag2, radius2));

	if (math::getScalar(det) < (math::real)0.0) {
		p->m_intersection = false;
	}
	else {
		det = math::sqrt(det);
		math::Vector t1 = math::sub(det, d_dot_dir);
		math::Vector t2 = math::sub(math::negate(det), d_dot_dir);

		math::real t1_s = math::getScalar(t1);
		math::real t2_s = math::getScalar(t2);

		p->m_intersection = t1_s > (math::real)0.0 || t2_s > (math::real)0.0;

		if (p->m_intersection) {
			math::Vector t;
			if (t2_s < t1_s && t2_s >= 0.0f) {
				t = t2;
				p->m_depth = t2_s;
			}
			else {
				t = t1;
				p->m_depth = t1_s;
			}

			p->m_position = math::add(ray->getSource(), math::mul(ray->getDirection(), t));

			// Calculate the normal
			math::Vector normal = math::sub(p->m_position, m_position);
			normal = math::normalize(normal);

			p->m_normal = normal;
		}
	}
}

void manta::SpherePrimitive::detectIntersection(const LightRay * ray, IntersectionPoint * convex, IntersectionPoint * concave) const {
	math::Vector d_pos = math::sub(ray->getSource(), m_position);
	math::Vector d_dot_dir = math::dot(d_pos, ray->getDirection());
	math::Vector mag2 = math::magnitudeSquared3(d_pos);

	math::Vector radius2 = math::loadScalar(m_radius * m_radius);
	math::Vector det = math::sub(math::mul(d_dot_dir, d_dot_dir), math::sub(mag2, radius2));

	if (math::getScalar(det) < (math::real)0.0) {
		if (convex != nullptr) convex->m_intersection = false;
		if (concave != nullptr) concave->m_intersection = false;
	}
	else {
		det = math::sqrt(det);
		math::Vector t1 = math::sub(det, d_dot_dir);
		math::Vector t2 = math::sub(math::negate(det), d_dot_dir);

		math::real t1_s = math::getScalar(t1);
		math::real t2_s = math::getScalar(t2);

		// Initialize these to false
		if (convex != nullptr) convex->m_intersection = false;
		if (concave != nullptr) concave->m_intersection = false;

		if (t1_s > (math::real)0.0) {
			math::Vector pos = math::add(ray->getSource(), math::mul(ray->getDirection(), t1));

			// Calculate the normal
			math::Vector normal = math::sub(pos, m_position);
			normal = math::normalize(normal);

			if (math::getScalar(math::dot(ray->getDirection(), normal)) > 0) {
				if (concave != nullptr) {
					concave->m_intersection = true;
					concave->m_depth = t1_s;
					concave->m_position = pos;
					concave->m_normal = math::negate(normal);
				}
			}
			else {
				if (convex != nullptr) {
					convex->m_intersection = true;
					convex->m_depth = t1_s;
					convex->m_position = pos;
					convex->m_normal = normal;
				}
			}
		}
			
		if (t2_s > (math::real)0.0) {
			math::Vector pos = math::add(ray->getSource(), math::mul(ray->getDirection(), t2));

			// Calculate the normal
			math::Vector normal = math::sub(pos, m_position);
			normal = math::normalize(normal);

			if (math::getScalar(math::dot(ray->getDirection(), normal)) > 0) {
				if (concave != nullptr) {
					concave->m_intersection = true;
					concave->m_depth = t1_s;
					concave->m_position = pos;
					concave->m_normal = math::negate(normal);
				}
			}
			else {
				if (convex != nullptr) {
					convex->m_intersection = true;
					convex->m_depth = t1_s;
					convex->m_position = pos;
					convex->m_normal = normal;
				}
			}
		}
	}
}
