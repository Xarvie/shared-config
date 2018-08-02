#ifndef RAPIDJSON_STRTOD_
#define RAPIDJSON_STRTOD_
#include "ieee754.h"
#include "biginteger.h"
#include "diyfp.h"
#include "pow10.h"
RAPIDJSON_NAMESPACE_BEGIN
namespace internal {
inline double FastPath(double significand, int exp) {
    if (exp < -308)
        return 0.0;
    else if (exp >= 0)
        return significand * internal::Pow10(exp);
    else
        return significand / internal::Pow10(-exp);
}
inline double StrtodNormalPrecision(double d, int p) {
    if (p < -308) {
        d = FastPath(d, -308);
        d = FastPath(d, p + 308);
    }
    else
        d = FastPath(d, p);
    return d;
}
template <typename T>
inline T Min3(T a, T b, T c) {
    T m = a;
    if (m > b) m = b;
    if (m > c) m = c;
    return m;
}
inline int CheckWithinHalfULP(double b, const BigInteger& d, int dExp) {
    const Double db(b);
    const uint64_t bInt = db.IntegerSignificand();
    const int bExp = db.IntegerExponent();
    const int hExp = bExp - 1;
    int dS_Exp2 = 0, dS_Exp5 = 0, bS_Exp2 = 0, bS_Exp5 = 0, hS_Exp2 = 0, hS_Exp5 = 0;
    if (dExp >= 0) {
        dS_Exp2 += dExp;
        dS_Exp5 += dExp;
    }
    else {
        bS_Exp2 -= dExp;
        bS_Exp5 -= dExp;
        hS_Exp2 -= dExp;
        hS_Exp5 -= dExp;
    }
    if (bExp >= 0)
        bS_Exp2 += bExp;
    else {
        dS_Exp2 -= bExp;
        hS_Exp2 -= bExp;
    }
    if (hExp >= 0)
        hS_Exp2 += hExp;
    else {
        dS_Exp2 -= hExp;
        bS_Exp2 -= hExp;
    }
    int common_Exp2 = Min3(dS_Exp2, bS_Exp2, hS_Exp2);
    dS_Exp2 -= common_Exp2;
    bS_Exp2 -= common_Exp2;
    hS_Exp2 -= common_Exp2;
    BigInteger dS = d;
    dS.MultiplyPow5(static_cast<unsigned>(dS_Exp5)) <<= static_cast<unsigned>(dS_Exp2);
    BigInteger bS(bInt);
    bS.MultiplyPow5(static_cast<unsigned>(bS_Exp5)) <<= static_cast<unsigned>(bS_Exp2);
    BigInteger hS(1);
    hS.MultiplyPow5(static_cast<unsigned>(hS_Exp5)) <<= static_cast<unsigned>(hS_Exp2);
    BigInteger delta(0);
    dS.Difference(bS, &delta);
    return delta.Compare(hS);
}
inline bool StrtodFast(double d, int p, double* result) {
    if (p > 22  && p < 22 + 16) {
        d *= internal::Pow10(p - 22);
        p = 22;
    }
    if (p >= -22 && p <= 22 && d <= 9007199254740991.0) { 
        *result = FastPath(d, p);
        return true;
    }
    else
        return false;
}
inline bool StrtodDiyFp(const char* decimals, size_t length, size_t decimalPosition, int exp, double* result) {
    uint64_t significand = 0;
    size_t i = 0;   
    for (; i < length; i++) {
        if (significand  >  RAPIDJSON_UINT64_C2(0x19999999, 0x99999999) ||
            (significand == RAPIDJSON_UINT64_C2(0x19999999, 0x99999999) && decimals[i] > '5'))
            break;
        significand = significand * 10u + static_cast<unsigned>(decimals[i] - '0');
    }
    if (i < length && decimals[i] >= '5') 
        significand++;
    size_t remaining = length - i;
    const unsigned kUlpShift = 3;
    const unsigned kUlp = 1 << kUlpShift;
    int64_t error = (remaining == 0) ? 0 : kUlp / 2;
    DiyFp v(significand, 0);
    v = v.Normalize();
    error <<= -v.e;
    const int dExp = static_cast<int>(decimalPosition) - static_cast<int>(i) + exp;
    int actualExp;
    DiyFp cachedPower = GetCachedPower10(dExp, &actualExp);
    if (actualExp != dExp) {
        static const DiyFp kPow10[] = {
            DiyFp(RAPIDJSON_UINT64_C2(0xa0000000, 00000000), -60),  
            DiyFp(RAPIDJSON_UINT64_C2(0xc8000000, 00000000), -57),  
            DiyFp(RAPIDJSON_UINT64_C2(0xfa000000, 00000000), -54),  
            DiyFp(RAPIDJSON_UINT64_C2(0x9c400000, 00000000), -50),  
            DiyFp(RAPIDJSON_UINT64_C2(0xc3500000, 00000000), -47),  
            DiyFp(RAPIDJSON_UINT64_C2(0xf4240000, 00000000), -44),  
            DiyFp(RAPIDJSON_UINT64_C2(0x98968000, 00000000), -40)   
        };
        int  adjustment = dExp - actualExp - 1;
        RAPIDJSON_ASSERT(adjustment >= 0 && adjustment < 7);
        v = v * kPow10[adjustment];
        if (length + static_cast<unsigned>(adjustment)> 19u) 
            error += kUlp / 2;
    }
    v = v * cachedPower;
    error += kUlp + (error == 0 ? 0 : 1);
    const int oldExp = v.e;
    v = v.Normalize();
    error <<= oldExp - v.e;
    const unsigned effectiveSignificandSize = Double::EffectiveSignificandSize(64 + v.e);
    unsigned precisionSize = 64 - effectiveSignificandSize;
    if (precisionSize + kUlpShift >= 64) {
        unsigned scaleExp = (precisionSize + kUlpShift) - 63;
        v.f >>= scaleExp;
        v.e += scaleExp; 
        error = (error >> scaleExp) + 1 + static_cast<int>(kUlp);
        precisionSize -= scaleExp;
    }
    DiyFp rounded(v.f >> precisionSize, v.e + static_cast<int>(precisionSize));
    const uint64_t precisionBits = (v.f & ((uint64_t(1) << precisionSize) - 1)) * kUlp;
    const uint64_t halfWay = (uint64_t(1) << (precisionSize - 1)) * kUlp;
    if (precisionBits >= halfWay + static_cast<unsigned>(error)) {
        rounded.f++;
        if (rounded.f & (DiyFp::kDpHiddenBit << 1)) { 
            rounded.f >>= 1;
            rounded.e++;
        }
    }
    *result = rounded.ToDouble();
    return halfWay - static_cast<unsigned>(error) >= precisionBits || precisionBits >= halfWay + static_cast<unsigned>(error);
}
inline double StrtodBigInteger(double approx, const char* decimals, size_t length, size_t decimalPosition, int exp) {
    const BigInteger dInt(decimals, length);
    const int dExp = static_cast<int>(decimalPosition) - static_cast<int>(length) + exp;
    Double a(approx);
    int cmp = CheckWithinHalfULP(a.Value(), dInt, dExp);
    if (cmp < 0)
        return a.Value();  
    else if (cmp == 0) {
        if (a.Significand() & 1)
            return a.NextPositiveDouble();
        else
            return a.Value();
    }
    else 
        return a.NextPositiveDouble();
}
inline double StrtodFullPrecision(double d, int p, const char* decimals, size_t length, size_t decimalPosition, int exp) {
    RAPIDJSON_ASSERT(d >= 0.0);
    RAPIDJSON_ASSERT(length >= 1);
    double result;
    if (StrtodFast(d, p, &result))
        return result;
    while (*decimals == '0' && length > 1) {
        length--;
        decimals++;
        decimalPosition--;
    }
    while (decimals[length - 1] == '0' && length > 1) {
        length--;
        decimalPosition--;
        exp++;
    }
    const int kMaxDecimalDigit = 780;
    if (static_cast<int>(length) > kMaxDecimalDigit) {
        int delta = (static_cast<int>(length) - kMaxDecimalDigit);
        exp += delta;
        decimalPosition -= static_cast<unsigned>(delta);
        length = kMaxDecimalDigit;
    }
    if (int(length) + exp < -324)
        return 0.0;
    if (StrtodDiyFp(decimals, length, decimalPosition, exp, &result))
        return result;
    return StrtodBigInteger(result, decimals, length, decimalPosition, exp);
}
} 
RAPIDJSON_NAMESPACE_END
#endif 
