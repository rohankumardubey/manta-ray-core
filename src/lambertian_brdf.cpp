#include "../include/lambertian_brdf.h"

manta::LambertianBRDF::LambertianBRDF() {
    /* void */
}

manta::LambertianBRDF::~LambertianBRDF() {
    /* void */
}

manta::math::Vector manta::LambertianBRDF::sampleF(const IntersectionPoint *surfaceInteraction, 
        const math::Vector &i, math::Vector *o, math::real *pdf, StackAllocator *stackAllocator) const {    
    // Uniformly sample a hemisphere
    math::real r1 = math::uniformRandom(math::constants::TWO_PI);
    math::real r2 = math::uniformRandom();
    math::real r2s = (math::real)sqrt(1 - r2 * r2);

    math::Vector direction = math::loadVector(
        cos(r1) * r2s,
        sin(r1) * r2s,
        r2);

    *o = direction;
    *pdf = (math::real)1.0 / math::constants::TWO_PI;

    return math::loadScalar((math::real)1.0 / math::constants::PI);
}

manta::math::Vector manta::LambertianBRDF::f(const IntersectionPoint *surfaceInteraction, 
    const math::Vector &i, const math::Vector &o, StackAllocator *stackAllocator) const 
{
    return math::loadScalar((math::real)1.0 / math::constants::PI);
}

manta::math::real manta::LambertianBRDF::pdf(
    const IntersectionPoint *surfaceInteraction, const math::Vector &i, const math::Vector &o) const 
{
    return (math::real)1.0 / math::constants::TWO_PI;
}
