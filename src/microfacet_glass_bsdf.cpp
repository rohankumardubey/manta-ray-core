#include "../include/microfacet_glass_bsdf.h"

#include "../include/intersection_point.h"
#include "../include/microfacet_distribution.h"
#include "../include/object_reference_node_output.h"

manta::MicrofacetGlassBSDF::MicrofacetGlassBSDF() {
    m_distribution = nullptr;
    m_distributionInput = nullptr;
    m_mediaInterface = nullptr;
    m_mediaInterfaceInput = nullptr;
}

manta::MicrofacetGlassBSDF::~MicrofacetGlassBSDF() {
    /* void */
}

manta::math::Vector manta::MicrofacetGlassBSDF::sampleF(
    const IntersectionPoint *surfaceInteraction,
    const math::Vector2 &u,
    const math::Vector &i,
    math::Vector *o,
    math::real *pdf,
    RayFlags *flags,
    StackAllocator *stackAllocator) 
{
    // Choose which distribution to sample and remap the stratified samples
    math::real d = u.x;
    math::Vector2 nu = (d > (math::real)0.5)
        ? math::Vector2((u.x - (math::real)0.5) * (math::real)2.0, u.y)
        : math::Vector2(u.x * (math::real)2.0, u.y);

    // Generate microfacet normal
    math::Vector m = m_distribution->generateMicrosurfaceNormal(surfaceInteraction, nu);

    const math::real ior = m_mediaInterface->ior(surfaceInteraction->m_direction);
    const math::real F = m_mediaInterface->fresnelTerm(i, m, surfaceInteraction->m_direction);
    math::real o_dot_m;

    math::Vector f;
    math::real rPdf = 0.0, tPdf = 0.0;
    if (d > (math::real)0.5) {
        // Reflection
        math::Vector ri = math::reflect(i, m);
        o_dot_m = math::getScalar(math::dot(ri, m));

        *o = ri;

        math::real cosThetaO = math::getZ(*o);
        math::real cosThetaI = math::getZ(i);

        if (
            o_dot_m <= (math::real)0.0 ||
            cosThetaO <= (math::real)0.0 ||
            cosThetaI <= (math::real)0.0) 
        {
            *pdf = 0.0;
            return math::constants::Zero;
        }

        rPdf = (m_distribution->calculatePDF(m, surfaceInteraction) / ::abs(4 * o_dot_m));

        // Calculate reflectivity
        f = math::loadScalar(
            m_distribution->calculateDistribution(m, surfaceInteraction) *
            m_distribution->bidirectionalShadowMasking(i, *o, m, surfaceInteraction) *
            F / (4 * cosThetaI * cosThetaO));
    }
    else {
        // Refraction
        math::Vector rt;
        if (!refract(i, m, ior, &rt)) {
            *pdf = (math::real)0.0;
            return math::constants::Zero;
        }

        if ((math::getZ(rt) > 0) == (math::getZ(i) > 0)) {
            *pdf = (math::real)0.0;
            return math::constants::Zero;
        }

        o_dot_m = math::getScalar(math::dot(rt, m));
        math::real i_dot_m = math::getScalar(math::dot(i, m));

        *o = rt;

        math::real cosThetaI = math::getZ(i);
        math::real cosThetaO = math::getZ(rt);

        if (
            i_dot_m <= (math::real)0.0 ||
            o_dot_m == (math::real)0.0 ||
            cosThetaO == (math::real)0.0 ||
            cosThetaI == (math::real)0.0)
        {
            *pdf = (math::real)0.0;
            return math::constants::Zero;
        }

        const math::real no = m_mediaInterface->no(surfaceInteraction->m_direction);
        const math::real ni = m_mediaInterface->ni(surfaceInteraction->m_direction);
        math::real jacobian = no * no * ::abs(o_dot_m);
        math::real jacobian_div = ni * ::abs(i_dot_m) + no * ::abs(o_dot_m);
        jacobian /= (jacobian_div * jacobian_div);

        tPdf = m_distribution->calculatePDF(m, surfaceInteraction) * jacobian;

        // Calculate transmittance
        math::real Ft_num = (1 / (ior * ior)) * 
            m_distribution->calculateDistribution(m, surfaceInteraction) *
            m_distribution->bidirectionalShadowMasking(i, rt, m, surfaceInteraction) * (1 - F);
        Ft_num *= ::abs(o_dot_m * i_dot_m);

        math::real Ft_div = (i_dot_m + (1 / ior) * ::abs(o_dot_m));
        Ft_div *= Ft_div;
        Ft_div *= ::abs(cosThetaO * cosThetaI);

        f = math::mul(
            math::loadScalar(Ft_num / Ft_div),
            math::loadScalar((ni * ni) / (no * no)));
    }

    *pdf = (rPdf + tPdf) / (math::real)2.0;
    return f;
}

manta::math::Vector manta::MicrofacetGlassBSDF::f(
    const IntersectionPoint *surfaceInteraction,
    const math::Vector &i,
    const math::Vector &o,
    StackAllocator *stackAllocator)
{
    return math::constants::Zero;
}

manta::math::real manta::MicrofacetGlassBSDF::pdf(
    const IntersectionPoint *surfaceInteraction,
    const math::Vector &i,
    const math::Vector &o) 
{
    return math::real(0.0);
}

void manta::MicrofacetGlassBSDF::_evaluate() {
    BXDF::_evaluate();

    m_distribution = getObject<MicrofacetDistribution>(m_distributionInput);
    m_mediaInterface = getObject<MediaInterface>(m_mediaInterfaceInput);
}

void manta::MicrofacetGlassBSDF::registerInputs() {
    BXDF::registerInputs();

    registerInput(&m_distributionInput, "distribution");
    registerInput(&m_mediaInterfaceInput, "media_interface");
}
