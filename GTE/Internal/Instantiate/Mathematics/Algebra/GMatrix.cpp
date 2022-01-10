#include <Mathematics/GMatrix.h>

namespace gte
{
    template class GMatrix<float>;
    template GMatrix<float> operator+(GMatrix<float> const&);
    template GMatrix<float> operator-(GMatrix<float> const&);
    template GMatrix<float> operator+(GMatrix<float> const&, GMatrix<float> const&);
    template GMatrix<float> operator-(GMatrix<float> const&, GMatrix<float> const&);
    template GMatrix<float> gte::operator*(GMatrix<float> const&, float);
    template GMatrix<float> gte::operator*(float, GMatrix<float> const&);
    template GMatrix<float> gte::operator/(GMatrix<float> const&, float);
    template GMatrix<float>& operator+=(GMatrix<float>&, GMatrix<float> const&);
    template GMatrix<float>& operator-=(GMatrix<float>&, GMatrix<float> const&);
    template GMatrix<float>& gte::operator*=(GMatrix<float>&, float);
    template GMatrix<float>& gte::operator/=(GMatrix<float>&, float);
    template float L1Norm(GMatrix<float> const&);
    template float L2Norm(GMatrix<float> const&);
    template float LInfinityNorm(GMatrix<float> const&);
    template GMatrix<float> Inverse(GMatrix<float> const&, bool*);
    template float Determinant(GMatrix<float> const&);
    template GMatrix<float> Transpose(GMatrix<float> const&);
    template GVector<float> operator*(GMatrix<float> const&, GVector<float> const&);
    template GVector<float> operator*(GVector<float> const&, GMatrix<float> const&);
    template GMatrix<float> operator*(GMatrix<float> const&, GMatrix<float> const&);
    template GMatrix<float> MultiplyAB(GMatrix<float> const&, GMatrix<float> const&);
    template GMatrix<float> MultiplyABT(GMatrix<float> const&, GMatrix<float> const&);
    template GMatrix<float> MultiplyATB(GMatrix<float> const&, GMatrix<float> const&);
    template GMatrix<float> MultiplyATBT(GMatrix<float> const&, GMatrix<float> const&);
    template GMatrix<float> MultiplyMD(GMatrix<float> const&, GVector<float> const&);
    template GMatrix<float> MultiplyDM(GVector<float> const&, GMatrix<float> const&);
    template GMatrix<float> OuterProduct(GVector<float> const&, GVector<float> const&);
    template void MakeDiagonal(GVector<float> const& D, GMatrix<float>& M);

    template class GMatrix<double>;
    template GMatrix<double> operator+(GMatrix<double> const&);
    template GMatrix<double> operator-(GMatrix<double> const&);
    template GMatrix<double> operator+(GMatrix<double> const&, GMatrix<double> const&);
    template GMatrix<double> operator-(GMatrix<double> const&, GMatrix<double> const&);
    template GMatrix<double> gte::operator*(GMatrix<double> const&, double);
    template GMatrix<double> gte::operator*(double, GMatrix<double> const&);
    template GMatrix<double> gte::operator/(GMatrix<double> const&, double);
    template GMatrix<double>& operator+=(GMatrix<double>&, GMatrix<double> const&);
    template GMatrix<double>& operator-=(GMatrix<double>&, GMatrix<double> const&);
    template GMatrix<double>& gte::operator*=(GMatrix<double>&, double);
    template GMatrix<double>& gte::operator/=(GMatrix<double>&, double);
    template double L1Norm(GMatrix<double> const&);
    template double L2Norm(GMatrix<double> const&);
    template double LInfinityNorm(GMatrix<double> const&);
    template GMatrix<double> Inverse(GMatrix<double> const&, bool*);
    template double Determinant(GMatrix<double> const&);
    template GMatrix<double> Transpose(GMatrix<double> const&);
    template GVector<double> operator*(GMatrix<double> const&, GVector<double> const&);
    template GVector<double> operator*(GVector<double> const&, GMatrix<double> const&);
    template GMatrix<double> operator*(GMatrix<double> const&, GMatrix<double> const&);
    template GMatrix<double> MultiplyAB(GMatrix<double> const&, GMatrix<double> const&);
    template GMatrix<double> MultiplyABT(GMatrix<double> const&, GMatrix<double> const&);
    template GMatrix<double> MultiplyATB(GMatrix<double> const&, GMatrix<double> const&);
    template GMatrix<double> MultiplyATBT(GMatrix<double> const&, GMatrix<double> const&);
    template GMatrix<double> MultiplyMD(GMatrix<double> const&, GVector<double> const&);
    template GMatrix<double> MultiplyDM(GVector<double> const&, GMatrix<double> const&);
    template GMatrix<double> OuterProduct(GVector<double> const&, GVector<double> const&);
    template void MakeDiagonal(GVector<double> const& D, GMatrix<double>& M);
}
