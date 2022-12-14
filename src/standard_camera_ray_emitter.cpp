#include "../include/standard_camera_ray_emitter.h"

#include "../include/sampler.h"
#include "../include/light_ray.h"
#include "../include/stack_allocator.h"

manta::StandardCameraRayEmitter::StandardCameraRayEmitter() {
    /* void */
}

manta::StandardCameraRayEmitter::~StandardCameraRayEmitter() {
    /* void */
}

void manta::StandardCameraRayEmitter::generateRay(LightRay *ray) const {
    math::Vector2 p_u = m_sampler->generate2d();

    math::Vector target = transformToImagePlane(
        math::Vector2(p_u.x - (math::real)0.5, p_u.y - (math::real)0.5));
    math::Vector dir = math::sub(target, m_position);
    dir = math::normalize(dir);

    ray->setDirection(dir);
    ray->setSource(m_position);
    ray->setIntensity(math::constants::Zero);
    ray->setCameraWeight((math::real)1.0);
    ray->setImagePlaneLocation(
        math::Vector2(
            p_u.x - (math::real)0.5 + (math::real)m_pixelX,
            -(p_u.y - (math::real)0.5) + (math::real)m_pixelY));
}
