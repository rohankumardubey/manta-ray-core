#ifndef MANTARAY_WORKER_H
#define MANTARAY_WORKER_H

#include "stack_allocator.h"
#include "path_recorder.h"
#include "manta_build_conf.h"
#include "runtime_statistics.h"
#include "intersection_point_manager.h"
#include "stratified_sampler.h"

#include <thread>
#include <atomic>

#if ENABLE_PATH_RECORDING
#define PATH_RECORDER_ARG ,&m_pathRecorder
#define NEW_TREE(name, origin) m_pathRecorder.startNewTree(name, origin)
#define END_TREE()
#define PATH_RECORDER_OUTPUT(fname) m_pathRecorder.writeObjFile(fname)
#else
#define PATH_RECORDER_ARG
#define NEW_TREE(name, origin)
#define END_TREE()
#define PATH_RECORDER_OUTPUT(fname)
#endif

namespace manta {

    struct Job;
    class CameraRayEmitterGroup;
    class Scene;
    class RayTracer;

    class Worker {
    public:
        Worker();
        ~Worker();

        void initialize(mem_size stackSize, RayTracer *rayTracer, int workerId, 
            bool deterministicSeed, const std::string &pathRecorderOutputDirectory,
            unsigned int seed);
        void start(bool multithreaded = true);
        void join();
        void destroy();

        mem_size getMaxMemoryUsage() const { return m_maxMemoryUsage; }
        std::string getTreeName(int pixelIndex, int sample) const;

        const RuntimeStatistics *getStatistics() const { return &m_statistics; }

    protected:
        void work();
        void doJob(const Job *job);

        StackAllocator *m_stack;
        RayTracer *m_rayTracer;

        Sampler *m_sampler;

        std::thread *m_thread;

        IntersectionPointManager m_ipManager;

    protected:
        // Debugging/Path recording
        RuntimeStatistics m_statistics;

        std::string getObjFname();

        PathRecorder m_pathRecorder;
        std::string m_pathRecorderOutputDirectory;
        bool m_deterministicSeed;

    protected:
        // Identification
        int m_workerId;

    protected:
        // Statistics
        mem_size m_maxMemoryUsage;
    };

} /* namespace manta */

#endif /* MANTARAY_WORKER_H */
