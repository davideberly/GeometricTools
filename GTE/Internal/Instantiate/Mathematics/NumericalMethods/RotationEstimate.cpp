#include <Mathematics/RotationEstimate.h>

namespace gte
{
    using MF33 = Matrix<3, 3, float>;
    using VF3 = Vector<3, float>;

    template float gte::RotC0Estimate<float, 4>(float);
    template float gte::RotC0Estimate<float, 6>(float);
    template float gte::RotC0Estimate<float, 8>(float);
    template float gte::RotC0Estimate<float, 10>(float);
    template float gte::RotC0Estimate<float, 12>(float);
    template float gte::RotC0Estimate<float, 14>(float);
    template float gte::RotC0Estimate<float, 16>(float);
    template float GetRotC0EstimateMaxError<float, 4>();
    template float GetRotC0EstimateMaxError<float, 6>();
    template float GetRotC0EstimateMaxError<float, 8>();
    template float GetRotC0EstimateMaxError<float, 10>();
    template float GetRotC0EstimateMaxError<float, 12>();
    template float GetRotC0EstimateMaxError<float, 14>();
    template float GetRotC0EstimateMaxError<float, 16>();

    template float gte::RotC1Estimate<float, 4>(float);
    template float gte::RotC1Estimate<float, 6>(float);
    template float gte::RotC1Estimate<float, 8>(float);
    template float gte::RotC1Estimate<float, 10>(float);
    template float gte::RotC1Estimate<float, 12>(float);
    template float gte::RotC1Estimate<float, 14>(float);
    template float gte::RotC1Estimate<float, 16>(float);
    template float GetRotC1EstimateMaxError<float, 4>();
    template float GetRotC1EstimateMaxError<float, 6>();
    template float GetRotC1EstimateMaxError<float, 8>();
    template float GetRotC1EstimateMaxError<float, 10>();
    template float GetRotC1EstimateMaxError<float, 12>();
    template float GetRotC1EstimateMaxError<float, 14>();
    template float GetRotC1EstimateMaxError<float, 16>();

    template float gte::RotC2Estimate<float, 4>(float);
    template float gte::RotC2Estimate<float, 6>(float);
    template float gte::RotC2Estimate<float, 8>(float);
    template float gte::RotC2Estimate<float, 10>(float);
    template float gte::RotC2Estimate<float, 12>(float);
    template float gte::RotC2Estimate<float, 14>(float);
    template float gte::RotC2Estimate<float, 16>(float);
    template float GetRotC2EstimateMaxError<float, 4>();
    template float GetRotC2EstimateMaxError<float, 6>();
    template float GetRotC2EstimateMaxError<float, 8>();
    template float GetRotC2EstimateMaxError<float, 10>();
    template float GetRotC2EstimateMaxError<float, 12>();
    template float GetRotC2EstimateMaxError<float, 14>();
    template float GetRotC2EstimateMaxError<float, 16>();

    template float gte::RotC3Estimate<float, 4>(float);
    template float gte::RotC3Estimate<float, 6>(float);
    template float gte::RotC3Estimate<float, 8>(float);
    template float gte::RotC3Estimate<float, 10>(float);
    template float gte::RotC3Estimate<float, 12>(float);
    template float gte::RotC3Estimate<float, 14>(float);
    template float gte::RotC3Estimate<float, 16>(float);
    template float GetRotC3EstimateMaxError<float, 4>();
    template float GetRotC3EstimateMaxError<float, 6>();
    template float GetRotC3EstimateMaxError<float, 8>();
    template float GetRotC3EstimateMaxError<float, 10>();
    template float GetRotC3EstimateMaxError<float, 12>();
    template float GetRotC3EstimateMaxError<float, 14>();
    template float GetRotC3EstimateMaxError<float, 16>();

    template void gte::RotationEstimate<float, 4>(VF3 const&, MF33&);
    template void gte::RotationEstimate<float, 6>(VF3 const&, MF33&);
    template void gte::RotationEstimate<float, 8>(VF3 const&, MF33&);
    template void gte::RotationEstimate<float, 10>(VF3 const&, MF33&);
    template void gte::RotationEstimate<float, 12>(VF3 const&, MF33&);
    template void gte::RotationEstimate<float, 14>(VF3 const&, MF33&);
    template void gte::RotationEstimate<float, 16>(VF3 const&, MF33&);

    template void gte::RotationDerivativeEstimate<float, 4>(VF3 const&, std::array<MF33, 3>&);
    template void gte::RotationDerivativeEstimate<float, 6>(VF3 const&, std::array<MF33, 3>&);
    template void gte::RotationDerivativeEstimate<float, 8>(VF3 const&, std::array<MF33, 3>&);
    template void gte::RotationDerivativeEstimate<float, 10>(VF3 const&, std::array<MF33, 3>&);
    template void gte::RotationDerivativeEstimate<float, 12>(VF3 const&, std::array<MF33, 3>&);
    template void gte::RotationDerivativeEstimate<float, 14>(VF3 const&, std::array<MF33, 3>&);
    template void gte::RotationDerivativeEstimate<float, 16>(VF3 const&, std::array<MF33, 3>&);

    template void gte::RotationAndDerivativeEstimate<float, 4>(VF3 const&, MF33&, std::array<MF33, 3>&);
    template void gte::RotationAndDerivativeEstimate<float, 6>(VF3 const&, MF33&, std::array<MF33, 3>&);
    template void gte::RotationAndDerivativeEstimate<float, 8>(VF3 const&, MF33&, std::array<MF33, 3>&);
    template void gte::RotationAndDerivativeEstimate<float, 10>(VF3 const&, MF33&, std::array<MF33, 3>&);
    template void gte::RotationAndDerivativeEstimate<float, 12>(VF3 const&, MF33&, std::array<MF33, 3>&);
    template void gte::RotationAndDerivativeEstimate<float, 14>(VF3 const&, MF33&, std::array<MF33, 3>&);
    template void gte::RotationAndDerivativeEstimate<float, 16>(VF3 const&, MF33&, std::array<MF33, 3>&);
}

namespace gte
{
    using MD33 = Matrix<3, 3, double>;
    using VD3 = Vector<3, double>;

    template double gte::RotC0Estimate<double, 4>(double);
    template double gte::RotC0Estimate<double, 6>(double);
    template double gte::RotC0Estimate<double, 8>(double);
    template double gte::RotC0Estimate<double, 10>(double);
    template double gte::RotC0Estimate<double, 12>(double);
    template double gte::RotC0Estimate<double, 14>(double);
    template double gte::RotC0Estimate<double, 16>(double);
    template double GetRotC0EstimateMaxError<double, 4>();
    template double GetRotC0EstimateMaxError<double, 6>();
    template double GetRotC0EstimateMaxError<double, 8>();
    template double GetRotC0EstimateMaxError<double, 10>();
    template double GetRotC0EstimateMaxError<double, 12>();
    template double GetRotC0EstimateMaxError<double, 14>();
    template double GetRotC0EstimateMaxError<double, 16>();

    template double gte::RotC1Estimate<double, 4>(double);
    template double gte::RotC1Estimate<double, 6>(double);
    template double gte::RotC1Estimate<double, 8>(double);
    template double gte::RotC1Estimate<double, 10>(double);
    template double gte::RotC1Estimate<double, 12>(double);
    template double gte::RotC1Estimate<double, 14>(double);
    template double gte::RotC1Estimate<double, 16>(double);
    template double GetRotC1EstimateMaxError<double, 4>();
    template double GetRotC1EstimateMaxError<double, 6>();
    template double GetRotC1EstimateMaxError<double, 8>();
    template double GetRotC1EstimateMaxError<double, 10>();
    template double GetRotC1EstimateMaxError<double, 12>();
    template double GetRotC1EstimateMaxError<double, 14>();
    template double GetRotC1EstimateMaxError<double, 16>();

    template double gte::RotC2Estimate<double, 4>(double);
    template double gte::RotC2Estimate<double, 6>(double);
    template double gte::RotC2Estimate<double, 8>(double);
    template double gte::RotC2Estimate<double, 10>(double);
    template double gte::RotC2Estimate<double, 12>(double);
    template double gte::RotC2Estimate<double, 14>(double);
    template double gte::RotC2Estimate<double, 16>(double);
    template double GetRotC2EstimateMaxError<double, 4>();
    template double GetRotC2EstimateMaxError<double, 6>();
    template double GetRotC2EstimateMaxError<double, 8>();
    template double GetRotC2EstimateMaxError<double, 10>();
    template double GetRotC2EstimateMaxError<double, 12>();
    template double GetRotC2EstimateMaxError<double, 14>();
    template double GetRotC2EstimateMaxError<double, 16>();

    template double gte::RotC3Estimate<double, 4>(double);
    template double gte::RotC3Estimate<double, 6>(double);
    template double gte::RotC3Estimate<double, 8>(double);
    template double gte::RotC3Estimate<double, 10>(double);
    template double gte::RotC3Estimate<double, 12>(double);
    template double gte::RotC3Estimate<double, 14>(double);
    template double gte::RotC3Estimate<double, 16>(double);
    template double GetRotC3EstimateMaxError<double, 4>();
    template double GetRotC3EstimateMaxError<double, 6>();
    template double GetRotC3EstimateMaxError<double, 8>();
    template double GetRotC3EstimateMaxError<double, 10>();
    template double GetRotC3EstimateMaxError<double, 12>();
    template double GetRotC3EstimateMaxError<double, 14>();
    template double GetRotC3EstimateMaxError<double, 16>();

    template void gte::RotationEstimate<double, 4>(VD3 const&, MD33&);
    template void gte::RotationEstimate<double, 6>(VD3 const&, MD33&);
    template void gte::RotationEstimate<double, 8>(VD3 const&, MD33&);
    template void gte::RotationEstimate<double, 10>(VD3 const&, MD33&);
    template void gte::RotationEstimate<double, 12>(VD3 const&, MD33&);
    template void gte::RotationEstimate<double, 14>(VD3 const&, MD33&);
    template void gte::RotationEstimate<double, 16>(VD3 const&, MD33&);

    template void gte::RotationDerivativeEstimate<double, 4>(VD3 const&, std::array<MD33, 3>&);
    template void gte::RotationDerivativeEstimate<double, 6>(VD3 const&, std::array<MD33, 3>&);
    template void gte::RotationDerivativeEstimate<double, 8>(VD3 const&, std::array<MD33, 3>&);
    template void gte::RotationDerivativeEstimate<double, 10>(VD3 const&, std::array<MD33, 3>&);
    template void gte::RotationDerivativeEstimate<double, 12>(VD3 const&, std::array<MD33, 3>&);
    template void gte::RotationDerivativeEstimate<double, 14>(VD3 const&, std::array<MD33, 3>&);
    template void gte::RotationDerivativeEstimate<double, 16>(VD3 const&, std::array<MD33, 3>&);

    template void gte::RotationAndDerivativeEstimate<double, 4>(VD3 const&, MD33&, std::array<MD33, 3>&);
    template void gte::RotationAndDerivativeEstimate<double, 6>(VD3 const&, MD33&, std::array<MD33, 3>&);
    template void gte::RotationAndDerivativeEstimate<double, 8>(VD3 const&, MD33&, std::array<MD33, 3>&);
    template void gte::RotationAndDerivativeEstimate<double, 10>(VD3 const&, MD33&, std::array<MD33, 3>&);
    template void gte::RotationAndDerivativeEstimate<double, 12>(VD3 const&, MD33&, std::array<MD33, 3>&);
    template void gte::RotationAndDerivativeEstimate<double, 14>(VD3 const&, MD33&, std::array<MD33, 3>&);
    template void gte::RotationAndDerivativeEstimate<double, 16>(VD3 const&, MD33&, std::array<MD33, 3>&);
}
