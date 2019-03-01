#include <phong_distribution.h>

#include <vector_material_node.h>

#include <assert.h>

manta::PhongDistribution::PhongDistribution() {
	m_power = (math::real)1.0;
	m_minMapPower = (math::real)1.0;
	m_powerNode = nullptr;
}

manta::PhongDistribution::~PhongDistribution() {

}

void manta::PhongDistribution::initialize(const IntersectionPoint *surfaceInteraction, MaterialNodeMemory *memory, StackAllocator *stackAllocator) const {
	MicrofacetDistribution::initialize(surfaceInteraction, memory, stackAllocator);

	PhongMemory *phongMemory = reinterpret_cast<PhongMemory *>((void *)memory->memory);

	if (m_powerNode != nullptr) {
		// Sample the power input and save it in the state container
		math::real power = math::getScalar(m_powerNode->sample(surfaceInteraction));
		phongMemory->power = power * (m_power - m_minMapPower) + m_minMapPower;
	}
	else {
		phongMemory->power = m_power;
	}
}

manta::math::Vector manta::PhongDistribution::generateMicrosurfaceNormal(MaterialNodeMemory *mem) const {
	PhongMemory *memory = reinterpret_cast<PhongMemory *>((void *)mem->memory);

	math::real power = memory->power;

	math::real r1 = math::uniformRandom();
	math::real r2 = math::uniformRandom();

	math::real rho_m = 2 * math::constants::PI * r2;
	math::real cos_theta_m = ::pow(r1, (math::real)1.0 / (power + (math::real)2.0));
	math::real sin_theta_m = ::sqrt((math::real)1.0 - cos_theta_m * cos_theta_m);

	assert(!std::isnan(cos_theta_m));
	assert(!std::isnan(sin_theta_m));

	math::Vector t1 = math::loadVector(sin_theta_m, sin_theta_m, cos_theta_m);
	math::Vector t2 = math::loadVector(cos(rho_m), sin(rho_m), (math::real)1.0);

	return math::mul(t1, t2);
}

manta::math::real manta::PhongDistribution::calculateDistribution(const math::Vector &m, MaterialNodeMemory *mem) const {
	PhongMemory *memory = reinterpret_cast<PhongMemory *>((void *)mem->memory);

	math::real cos_theta_m = math::getZ(m);
	
	return ((memory->power + (math::real)2.0) / math::constants::TWO_PI) * ::pow(cos_theta_m, memory->power);
}

manta::math::real manta::PhongDistribution::calculateG1(const math::Vector &v, const math::Vector &m, MaterialNodeMemory *mem) const {
	PhongMemory *memory = reinterpret_cast<PhongMemory *>((void *)mem->memory);
	
	math::real v_dot_m = ::abs(math::getScalar(math::dot(v, m)));
	if (v_dot_m < 0) return (math::real)0.0;

	math::real v_dot_m2 = v_dot_m * v_dot_m;

	if (v_dot_m2 >= (math::real)1.0) return 1.0;

	math::real a = ::sqrt(((math::real)0.5 * memory->power + 1) / ((math::real)1.0 - v_dot_m2)) * v_dot_m;
	a = ::abs(a);

	if (a < (math::real)0.0) {
		a = (math::real)0.0;
	}

	math::real secondTerm = (math::real)1.0;
	if (a < (math::real)1.6) {
		secondTerm = (math::real)((3.535 * a + 2.181 * a * a) / (1 + 2.767 * a + 2.577 * a * a));
	}

	return secondTerm;
}