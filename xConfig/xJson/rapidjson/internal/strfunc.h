#ifndef RAPIDJSON_INTERNAL_STRFUNC_H_
#define RAPIDJSON_INTERNAL_STRFUNC_H_
#include "../stream.h"
RAPIDJSON_NAMESPACE_BEGIN
namespace internal {
template <typename Ch>
inline SizeType StrLen(const Ch* s) {
    const Ch* p = s;
    while (*p) ++p;
    return SizeType(p - s);
}
template<typename Encoding>
bool CountStringCodePoint(const typename Encoding::Ch* s, SizeType length, SizeType* outCount) {
    GenericStringStream<Encoding> is(s);
    const typename Encoding::Ch* end = s + length;
    SizeType count = 0;
    while (is.src_ < end) {
        unsigned codepoint;
        if (!Encoding::Decode(is, &codepoint))
            return false;
        count++;
    }
    *outCount = count;
    return true;
}
} 
RAPIDJSON_NAMESPACE_END
#endif 
