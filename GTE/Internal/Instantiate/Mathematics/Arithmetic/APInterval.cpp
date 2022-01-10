#include <Mathematics/APInterval.h>

namespace gte
{
    using BSN = BSNumber<UIntegerAP32>;
    template class APInterval<BSN>;
    template APInterval<BSN> operator+(APInterval<BSN> const&);
    template APInterval<BSN> operator-(APInterval<BSN> const&);
    template APInterval<BSN> gte::operator+(BSN const&, APInterval<BSN> const&);
    template APInterval<BSN> gte::operator+(APInterval<BSN> const&, BSN const&);
    template APInterval<BSN> operator+(APInterval<BSN> const&, APInterval<BSN> const&);
    template APInterval<BSN>& gte::operator+=(APInterval<BSN>&, BSN const&);
    template APInterval<BSN>& operator+=(APInterval<BSN>&, APInterval<BSN> const&);
    template APInterval<BSN> gte::operator-(BSN const&, APInterval<BSN> const&);
    template APInterval<BSN> gte::operator-(APInterval<BSN> const&, BSN const&);
    template APInterval<BSN> operator-(APInterval<BSN> const&, APInterval<BSN> const&);
    template APInterval<BSN>& gte::operator-=(APInterval<BSN>&, BSN const&);
    template APInterval<BSN>& operator-=(APInterval<BSN>&, APInterval<BSN> const&);
    template APInterval<BSN> gte::operator*(BSN const&, APInterval<BSN> const&);
    template APInterval<BSN> gte::operator*(APInterval<BSN> const&, BSN const&);
    template APInterval<BSN> operator*(APInterval<BSN> const&, APInterval<BSN> const&);
    template APInterval<BSN>& gte::operator*=(APInterval<BSN>&, BSN const&);
    template APInterval<BSN>& gte::operator*=(APInterval<BSN>&, APInterval<BSN> const&);

    using BSR = BSRational<UIntegerAP32>;
    template class APInterval<BSR>;
    template APInterval<BSR> operator+(APInterval<BSR> const&);
    template APInterval<BSR> operator-(APInterval<BSR> const&);
    template APInterval<BSR> gte::operator+(BSR const&, APInterval<BSR> const&);
    template APInterval<BSR> gte::operator+(APInterval<BSR> const&, BSR const&);
    template APInterval<BSR> operator+(APInterval<BSR> const&, APInterval<BSR> const&);
    template APInterval<BSR>& gte::operator+=(APInterval<BSR>&, BSR const&);
    template APInterval<BSR>& operator+=(APInterval<BSR>&, APInterval<BSR> const&);
    template APInterval<BSR> gte::operator-(BSR const&, APInterval<BSR> const&);
    template APInterval<BSR> gte::operator-(APInterval<BSR> const&, BSR const&);
    template APInterval<BSR> operator-(APInterval<BSR> const&, APInterval<BSR> const&);
    template APInterval<BSR>& gte::operator-=(APInterval<BSR>&, BSR const&);
    template APInterval<BSR>& operator-=(APInterval<BSR>&, APInterval<BSR> const&);
    template APInterval<BSR> gte::operator*(BSR const&, APInterval<BSR> const&);
    template APInterval<BSR> gte::operator*(APInterval<BSR> const&, BSR const&);
    template APInterval<BSR> operator*(APInterval<BSR> const&, APInterval<BSR> const&);
    template APInterval<BSR>& gte::operator*=(APInterval<BSR>&, BSR const&);
    template APInterval<BSR>& operator*=(APInterval<BSR>&, APInterval<BSR> const&);
    template APInterval<BSR> gte::operator/(BSR const&, APInterval<BSR> const&);
    template APInterval<BSR> gte::operator/(APInterval<BSR> const&, BSR const&);
    template APInterval<BSR> operator/(APInterval<BSR> const&, APInterval<BSR> const&);
    template APInterval<BSR>& gte::operator/=(APInterval<BSR>&, BSR const&);
    template APInterval<BSR>& operator/=(APInterval<BSR>&, APInterval<BSR> const&);
}
