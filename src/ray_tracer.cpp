#include "../include/ray_tracer.h"

#include "../include/material.h"
#include "../include/light_ray.h"
#include "../include/camera_ray_emitter.h"
#include "../include/camera_ray_emitter_group.h"
#include "../include/scene.h"
#include "../include/scene_object.h"
#include "../include/intersection_point.h"
#include "../include/scene_geometry.h"
#include "../include/stack_allocator.h"
#include "../include/memory_management.h"
#include "../include/worker.h"
#include "../include/camera_ray_emitter_group.h"
#include "../include/path_recorder.h"
#include "../include/standard_allocator.h"
#include "../include/stack_list.h"
#include "../include/ray_container.h"
#include "../include/coarse_intersection.h"
#include "../include/image_plane.h"
#include "../include/os_utilities.h"
#include "../include/material_library.h"
#include "../include/vector_node_output.h"
#include "../include/bsdf.h"
#include "../include/sampler.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>

manta::RayTracer::RayTracer() {
    m_multithreadedInput = nullptr;
    m_threadCountInput = nullptr;
    m_renderBlockSizeInput = nullptr;
    m_backgroundColorInput = nullptr;
    m_deterministicSeedInput = nullptr;
    m_materialLibraryInput = nullptr;
    m_sceneInput = nullptr;
    m_cameraInput = nullptr;
    m_filterInput = nullptr;
    m_samplerInput = nullptr;

    m_deterministicSeed = false;
    m_pathRecordingOutputDirectory = "";
    m_backgroundColor = math::constants::Zero;
    m_currentRay = 0;
}

manta::RayTracer::~RayTracer() {
    /* void */
}

void manta::RayTracer::traceAll(const Scene *scene, CameraRayEmitterGroup *group, ImagePlane *target) {
    // Simple performance metrics for now
    auto startTime = std::chrono::system_clock::now();

    // Reset counter
    m_currentRay = 0;

    // Set up the emitter group
    group->configure();

    // Initialize the target
    int resX = group->getResolutionX();
    int resY = group->getResolutionY();

    // TEMP: initialize physical sensor extents
    math::real py = (math::real)1.0;
    math::real px = ((math::real)(resX) / resY);

    target->initialize(resX, resY);

    // Create jobs
    int emitterCount = group->getResolutionX() * group->getResolutionY();
    int start = 0;
    int end = 0;
    bool done = false;

    int horizontalBlocks = resX / m_renderBlockSize + 1;
    int verticalBlocks = resY / m_renderBlockSize + 1;

    for (int i = 0; i < horizontalBlocks; i++) {
        for (int j = 0; j < verticalBlocks; j++) {
            Job newJob;
            newJob.scene = scene;
            newJob.group = group;
            newJob.target = target;
            newJob.startX = i * m_renderBlockSize;
            newJob.startY = j * m_renderBlockSize;
            newJob.endX = (i + 1) * m_renderBlockSize - 1;
            newJob.endY = (j + 1) * m_renderBlockSize - 1;

            if (newJob.startX >= resX) continue;
            if (newJob.startY >= resY) continue;
            if (newJob.endX >= resX) newJob.endX = resX - 1;
            if (newJob.endY >= resY) newJob.endY = resY - 1;

            m_jobQueue.push(newJob);
        }
    }

    // Hide the cursor to avoid annoying blinking artifact
    showConsoleCursor(false);

    // Create and start all threads
    createWorkers();
    startWorkers();
    waitForWorkers();

    target->normalize();

    // Print a single new line to terminate progress display
    // See RayTracer::incrementRayCompletion
    std::cout << std::endl;

    auto endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = endTime - startTime;

#if ENABLE_DETAILED_STATISTICS
    RuntimeStatistics combinedStatistics;
    combinedStatistics.reset();

    // Add all statistics from workers
    for (int i = 0; i < m_threadCount; i++) {
        combinedStatistics.add(m_workers[i].getStatistics());
    }

    // Display statistics
    std::cout <<        "================================================" << std::endl;
    std::cout <<        "Run-time Statistics" << std::endl;
    std::cout <<        "------------------------------------------------" << std::endl;
    int counterCount = RuntimeStatistics::COUNTER_COUNT;
    for (int i = 0; i < counterCount; i++) {
        unsigned __int64 counter = combinedStatistics.counters[i];
        const char *counterName = combinedStatistics.getCounterName((RuntimeStatistics::COUNTER)i);
        std::stringstream ss;
        ss << counterName << ":";
        std::cout << ss.str();
        for (int j = 0; j < (37 - ss.str().length()); j++) {
            std::cout << " ";
        }

        std::cout << counter << std::endl;
    }
#endif /* ENABLE_DETAILED_STATISTICS */

    std::cout <<        "================================================" << std::endl;
    std::cout <<        "Total processing time:               " << diff.count() << " s" << std::endl;
    std::cout <<        "------------------------------------------------" << std::endl;
    std::cout <<        "Standard allocator peak usage:       " << StandardAllocator::Global()->getMaxUsage() / (double)MB << " MB" << std::endl;
    std::cout <<        "Main allocator peak usage:           " << m_stack.getMaxUsage() / (double)MB << " MB" << std::endl;
    unsigned __int64 totalUsage = m_stack.getMaxUsage() + StandardAllocator::Global()->getMaxUsage();
    for (int i = 0; i < m_threadCount; i++) {
        std::stringstream ss;
        ss <<            "Worker " << i << " peak memory usage:";
        std::cout << ss.str();
        for (int j = 0; j < (37 - ss.str().length()); j++) {
            std::cout << " ";
        }
        std::cout << m_workers[i].getMaxMemoryUsage() / (double)MB << " MB" << std::endl;
        totalUsage += m_workers[i].getMaxMemoryUsage();
    }
    std::cout <<        "                                     -----------" << std::endl;
    std::cout <<        "Total memory usage:                  " << totalUsage / (double)MB << " MB" << std::endl;
    std::cout <<        "================================================" << std::endl;

    // Bring back the cursor
    showConsoleCursor(true);
}

void manta::RayTracer::tracePixel(int px, int py, const Scene *scene, CameraRayEmitterGroup *group, ImagePlane *target) {
    // Set up the emitter group
    group->initialize();

    // Initialize the target
    int resX = group->getResolutionX();
    int resY = group->getResolutionY();

    target->initialize(resX, resY);

    // Create the singular job for the pixel
    Job job;
    job.scene = scene;
    job.group = group;
    job.target = target;
    job.startX = px;
    job.endX = px;
    job.startY = py;
    job.endY = py;

    m_jobQueue.push(job);

    // Create and start all threads
    createWorkers();
    startWorkers();
    waitForWorkers();
}

void manta::RayTracer::traceRayEmitter(const CameraRayEmitter *emitter, RayContainer *container, 
    const Scene *scene, IntersectionPointManager *manager, StackAllocator *s 
    /**/ PATH_RECORDER_DECL /**/ STATISTICS_PROTOTYPE) const 
{
    //emitter->getSampler()->startPixelSession();

    //for (int i = 0; i < rayCount; i++) {
    //    LightRay *ray = &rays[i];
    //    ray->calculateTransformations();

    //    math::Vector intensity = traceRay(scene, ray, 0, manager, nullptr, s /**/ PATH_RECORDER_VAR /**/ STATISTICS_PARAM_INPUT);
    //    ray->setIntensity(intensity);
    //}

    //container->calculateIntensity();
}

void manta::RayTracer::configure(mem_size stackSize, mem_size workerStackSize, int threadCount, int renderBlockSize, bool multithreaded) {
    m_stack.initialize(stackSize);
    m_renderBlockSize = renderBlockSize;
    m_multithreaded = multithreaded;
    m_threadCount = threadCount;
    m_workerStackSize = workerStackSize;
}

void manta::RayTracer::destroy() {
    destroyWorkers();
}

void manta::RayTracer::incrementRayCompletion(const Job *job, int increment) {
    m_outputLock.lock();
    
    int emitterCount = job->group->getResolutionX() * job->group->getResolutionY();
    m_currentRay += increment;

    // Print in increments of 1000 or the last 1000 one by one
    if (m_currentRay % 1000 == 0 || m_currentRay >= (emitterCount - 1000)) {
        std::cout << "Ray " << m_currentRay << "/" << emitterCount << "                      \r" << std::flush;
    }

    m_outputLock.unlock();
}

void manta::RayTracer::_evaluate() {
    int threadCount;
    int renderBlockSize;
    bool multithreaded;
    bool deterministicSeed;
    CameraRayEmitterGroup *camera;
    Scene *scene;
    Filter *filter;

    static_cast<piranha::NodeOutput *>(m_multithreadedInput)->fullCompute((void *)&multithreaded);
    static_cast<piranha::NodeOutput *>(m_threadCountInput)->fullCompute((void *)&threadCount);
    static_cast<piranha::NodeOutput *>(m_renderBlockSizeInput)->fullCompute((void *)&renderBlockSize);
    static_cast<piranha::NodeOutput *>(m_deterministicSeedInput)->fullCompute((void *)&deterministicSeed);
    static_cast<VectorNodeOutput *>(m_backgroundColorInput)->sample(nullptr, (void *)&m_backgroundColor);

    m_materialManager = getObject<MaterialLibrary>(m_materialLibraryInput);
    camera = getObject<CameraRayEmitterGroup>(m_cameraInput);
    scene = getObject<Scene>(m_sceneInput);
    filter = getObject<Filter>(m_filterInput);
    m_sampler = getObject<Sampler>(m_samplerInput);

    configure(200 * MB, 50 * MB, threadCount, renderBlockSize, multithreaded);
    setDeterministicSeedMode(deterministicSeed);

    ImagePlane imagePlane;
    imagePlane.setFilter(filter);
    traceAll(scene, camera, &imagePlane);

    VectorMap2D *vectorMap = new VectorMap2D();
    vectorMap->copy(&imagePlane);
    imagePlane.destroy();

    m_output.setMap(vectorMap);
}

void manta::RayTracer::_initialize() {
    /* void */
}

void manta::RayTracer::registerInputs() {
    registerInput(&m_multithreadedInput, "multithreaded");
    registerInput(&m_threadCountInput, "threads");
    registerInput(&m_renderBlockSizeInput, "block_size");
    registerInput(&m_backgroundColorInput, "background");
    registerInput(&m_deterministicSeedInput, "deterministic_seed");
    registerInput(&m_materialLibraryInput, "materials");
    registerInput(&m_sceneInput, "scene");
    registerInput(&m_cameraInput, "camera");
    registerInput(&m_filterInput, "filter");
    registerInput(&m_samplerInput, "sampler");
}

void manta::RayTracer::registerOutputs() {
    registerOutput(&m_output, "image");
}

void manta::RayTracer::createWorkers() {
    m_workers = new Worker[m_threadCount];

    std::mt19937 seedGenerator;

    for (int i = 0; i < m_threadCount; i++) {
        m_workers[i].initialize(m_workerStackSize, this, i, m_deterministicSeed, m_pathRecordingOutputDirectory, seedGenerator());
    }
}

void manta::RayTracer::startWorkers() {
    int workerCount = m_threadCount;

    std::cout << "Starting " << workerCount << " workers" << std::endl;

    for (int i = 0; i < workerCount; i++) {
        m_workers[i].start(m_multithreaded);
    }
}

void manta::RayTracer::waitForWorkers() {
    int workerCount = m_threadCount;

    for (int i = 0; i < workerCount; i++) {
        m_workers[i].join();
    }
}

void manta::RayTracer::destroyWorkers() {
    int workerCount = m_threadCount;

    for (int i = 0; i < workerCount; i++) {
        m_workers[i].destroy();
    }

    delete[] m_workers;
    m_workers = nullptr;
}

void manta::RayTracer::depthCull(const Scene *scene, const LightRay *ray, SceneObject **closestObject, IntersectionPoint *point, StackAllocator *s /**/ STATISTICS_PROTOTYPE) const {
    int objectCount = scene->getSceneObjectCount();

    CoarseIntersection closestIntersection;
    math::real closestDepth = math::constants::REAL_MAX;

    // Find the closest intersection
    bool found = false;
    for (int i = 0; i < objectCount; i++) {
        SceneObject *object = scene->getSceneObject(i);
        SceneGeometry *geometry = object->getGeometry();

        if (!geometry->fastIntersection(ray)) continue;

        if (geometry->findClosestIntersection(ray, &closestIntersection, (math::real)0.0, closestDepth, s /**/ STATISTICS_PARAM_INPUT)) {
            found = true;
            closestIntersection.sceneObject = object;
            closestDepth = closestIntersection.depth;
        }
    }

    if (found) {
        math::Vector position = math::add(ray->getSource(), math::mul(ray->getDirection(), math::loadScalar(closestIntersection.depth)));
        closestIntersection.sceneGeometry->fineIntersection(position, point, &closestIntersection);
        point->m_valid = true;
        *closestObject = closestIntersection.sceneObject;

        refineContact(ray, closestIntersection.depth, point, closestObject, s);
    }
    else {
        point->m_valid = false;
        *closestObject = nullptr;
    }
}

void manta::RayTracer::refineContact(const LightRay *ray, math::real depth, 
    IntersectionPoint *point, SceneObject **closestObject, StackAllocator *s) const 
{
    // Simple bias
    math::real d = depth;
    d -= (math::real)5E-3;
    if (d < (math::real)0.0) {
        d = (math::real)0.0;
    }
    math::Vector dist = math::loadScalar(d);
    point->m_position = math::add(ray->getSource(), math::mul(ray->getDirection(), dist));

    if (math::getScalar(math::dot(point->m_faceNormal, ray->getDirection())) > 0.0) {
        point->m_faceNormal = math::negate(point->m_faceNormal);
        point->m_vertexNormal = math::negate(point->m_vertexNormal);
        point->m_direction = MediaInterface::DIRECTION_OUT;
    }
    else {
        point->m_direction = MediaInterface::DIRECTION_IN;
    }
}

manta::math::Vector manta::RayTracer::traceRay(
    const Scene *scene, LightRay *ray, int degree, IntersectionPointManager *manager, Sampler *sampler,
    StackAllocator *s /**/ PATH_RECORDER_DECL /**/ STATISTICS_PROTOTYPE) const 
{
    constexpr int MAX_BOUNCES = 4;

    LightRay *currentRay = ray;
    LightRay localRay;
    math::Vector beta = math::constants::One;
    math::Vector L = math::constants::Zero;

    for (int bounces = 0; bounces < MAX_BOUNCES; bounces++) {
        SceneObject *sceneObject = nullptr;
        Material *material = nullptr;
        IntersectionPoint point;
        point.m_lightRay = currentRay;
        point.m_id = manager->generateId();
        point.m_threadId = manager->getThreadId();
        point.m_manager = manager;

        depthCull(scene, currentRay, &sceneObject, &point, s /**/ STATISTICS_PARAM_INPUT);

        bool foundIntersection = (sceneObject != nullptr);

        if (foundIntersection) {
            material = (point.m_material == -1) 
                ? sceneObject->getDefaultMaterial()
                : material = m_materialManager->getMaterial(point.m_material);

            math::Vector emission = material->getEmission(point);

            L = math::add(
                L,
                math::mul(beta, emission)
            );
        }
        else {
            L = math::add(
                L,
                math::mul(beta, m_backgroundColor)
            );
            break;
        }

        // Get the BSDF associated with this material
        BSDF *bsdf = material->getBSDF();
        if (bsdf == nullptr) break;

        // Generate a new path
        math::Vector outgoingDir = math::negate(currentRay->getDirection());
        math::Vector incomingDir, t_incomingDir;

        math::Vector normal = math::getScalar(math::dot(outgoingDir, point.m_vertexNormal)) > 0
            ? point.m_vertexNormal
            : math::negate(point.m_vertexNormal);

        // Generate basis vectors
        math::Vector u = math::constants::YAxis;
        math::Vector v;
        math::Vector w = normal;
        if (abs(math::getX(w)) < 0.1f) {
            u = math::constants::XAxis;
        }
        u = math::normalize(math::cross(u, w));
        v = math::cross(w, u);

        // Transform incident ray
        math::Vector t_dir = math::loadVector(
            math::getScalar(math::dot(outgoingDir, u)),
            math::getScalar(math::dot(outgoingDir, v)),
            math::getScalar(math::dot(outgoingDir, w)));

        math::Vector2 s_u = (sampler != nullptr)
            ? sampler->generate2d()
            : math::Vector2(math::uniformRandom(), math::uniformRandom());
        math::real pdf;
        math::Vector f = bsdf->sampleF(&point, s_u, t_dir, &t_incomingDir, &pdf, s);
        f = math::mul(f, material->getFilterColor(point));
        f = math::mask(f, math::constants::MaskOffW);

        if (pdf == (math::real)0.0) break;
        if (math::getScalar(math::maxComponent(f)) < (math::real)1E-4) break;

        incomingDir = math::add(
            math::mul(u, math::loadScalar(math::getX(t_incomingDir))),
            math::mul(v, math::loadScalar(math::getY(t_incomingDir)))
        );
        incomingDir = math::add(
            incomingDir,
            math::mul(math::loadScalar(math::getZ(t_incomingDir)), w)
        );

        beta = math::mul(
            math::mul(f, beta),
            math::abs(math::dot(incomingDir, point.m_vertexNormal))
        );
        beta = math::div(
            beta,
            math::loadScalar(pdf)
        );

        if (std::isnan(math::getX(beta)) || std::isnan(math::getY(beta)) || std::isnan(math::getZ(beta))) {
            int a = 0;
        }

        localRay.setDirection(incomingDir);
        if (math::getScalar(math::dot(incomingDir, w)) >= 0) {
            localRay.setSource(point.m_position);
        }
        else {
            // The light ray is undergoing transmission
            localRay.setSource(math::add(point.m_position, math::mul(localRay.getDirection(), math::loadScalar((math::real)1E-2))));
        }
        localRay.calculateTransformations();
        currentRay = &localRay;

        if (bounces > 3) {
            math::real q = std::max((math::real)0.05, 1 - math::getScalar(math::maxComponent(beta)));
            if (math::uniformRandom() < q) break;
            beta = math::div(beta, math::loadScalar(1 - q));
        }
    }

    return L;
}

void manta::RayTracer::traceRays(const Scene *scene, const RayContainer &rayContainer, 
    IntersectionPointManager *manager, StackAllocator *s 
    /**/ PATH_RECORDER_DECL /**/ STATISTICS_PROTOTYPE) const 
{
    int nRays = rayContainer.getRayCount();

    for (int i = 0; i < nRays; i++) {
        INCREMENT_COUNTER(RuntimeStatistics::RAYS_CAST);

        LightRay *ray = &rayContainer.getRays()[i];
        ray->calculateTransformations();

        traceRay(scene, ray, rayContainer.getDegree(), manager, nullptr, s /**/ PATH_RECORDER_VAR /**/ STATISTICS_PARAM_INPUT);
    }
}
