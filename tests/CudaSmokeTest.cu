#include <cuda_runtime.h>

#include <iostream>

namespace
{
    bool checkCuda(cudaError_t result, const char* operation)
    {
        if (result == cudaSuccess)
        {
            return true;
        }

        std::cerr
            << operation
            << " failed: "
            << cudaGetErrorString(result)
            << '\n';

        return false;
    }

    __global__ void incrementValue(int* value)
    {
        *value += 1;
    }
}

int main()
{
    int deviceCount = 0;

    if (!checkCuda(cudaGetDeviceCount(&deviceCount), "cudaGetDeviceCount"))
    {
        return 1;
    }

    if (deviceCount == 0)
    {
        std::cerr << "No CUDA-capable device was found.\n";
        return 1;
    }

    constexpr int deviceIndex = 0;

    if (!checkCuda(cudaSetDevice(deviceIndex), "cudaSetDevice"))
    {
        return 1;
    }

    cudaDeviceProp deviceProperties{};

    if (!checkCuda(
            cudaGetDeviceProperties(&deviceProperties, deviceIndex),
            "cudaGetDeviceProperties"))
    {
        return 1;
    }

    int hostValue = 41;
    int* deviceValue = nullptr;

    if (!checkCuda(
            cudaMalloc(
                reinterpret_cast<void**>(&deviceValue),
                sizeof(int)
            ),
            "cudaMalloc"))
    {
        return 1;
    }

    if (!checkCuda(
            cudaMemcpy(
                deviceValue,
                &hostValue,
                sizeof(int),
                cudaMemcpyHostToDevice
            ),
            "cudaMemcpy host to device"))
    {
        cudaFree(deviceValue);
        return 1;
    }

    incrementValue<<<1, 1>>>(deviceValue);

    if (!checkCuda(cudaGetLastError(), "incrementValue launch") ||
        !checkCuda(cudaDeviceSynchronize(), "incrementValue execution"))
    {
        cudaFree(deviceValue);
        return 1;
    }

    if (!checkCuda(
            cudaMemcpy(
                &hostValue,
                deviceValue,
                sizeof(int),
                cudaMemcpyDeviceToHost
            ),
            "cudaMemcpy device to host"))
    {
        cudaFree(deviceValue);
        return 1;
    }

    cudaFree(deviceValue);

    if (hostValue != 42)
    {
        std::cerr << "CUDA kernel returned an unexpected value.\n";
        return 1;
    }

    std::cout << "CUDA smoke test passed\n"
              << "Device: " << deviceProperties.name << '\n'
              << "Compute capability: "
              << deviceProperties.major
              << '.'
              << deviceProperties.minor
              << '\n';

    return 0;
}
