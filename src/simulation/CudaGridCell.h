#pragma once

#include <cstdint>

namespace hzl::simulation
{
    using CudaCellKey = std::uint64_t;

    constexpr int cudaCellCoordinateBits = 21;
    constexpr int cudaCellCoordinateBias = 1 << 20;
    constexpr CudaCellKey cudaCellCoordinateMask =
        (CudaCellKey{ 1 } << cudaCellCoordinateBits) - 1;

#ifdef __CUDACC__
    __host__ __device__ inline CudaCellKey encodeCudaCellCoordinates(
        int cellX,
        int cellY,
        int cellZ
    )
    {
        const CudaCellKey encodedX =
            static_cast<CudaCellKey>(cellX + cudaCellCoordinateBias) &
            cudaCellCoordinateMask;

        const CudaCellKey encodedY =
            static_cast<CudaCellKey>(cellY + cudaCellCoordinateBias) &
            cudaCellCoordinateMask;

        const CudaCellKey encodedZ =
            static_cast<CudaCellKey>(cellZ + cudaCellCoordinateBias) &
            cudaCellCoordinateMask;

        return (encodedX << (cudaCellCoordinateBits * 2)) |
               (encodedY << cudaCellCoordinateBits) |
               encodedZ;
    }
#endif
}
