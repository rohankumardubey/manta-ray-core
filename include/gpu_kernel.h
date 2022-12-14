#ifndef MANTARAY_GPU_KERNEL_H
#define MANTARAY_GPU_KERNEL_H

#include "manta_build_conf.h"
#if INCLUDE_OPENCL_IMPL

namespace manta {

    // Forward declarations
    class GPUMemory;

    class GPUKernel {
    public:
        GPUKernel() {}
        virtual ~GPUKernel() {}

        virtual void setArgument(int argument, GPUMemory *memory) = 0;
        virtual void run(unsigned int elements) = 0;
        virtual void free() = 0;
    };

} /* namespace manta */

#endif /* INCLUDE_OPENCL_IMPL */
#endif /* MANTARAY_GPU_KERNEL_H */
