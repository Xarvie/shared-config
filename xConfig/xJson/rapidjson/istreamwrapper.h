#ifndef RAPIDJSON_ISTREAMWRAPPER_H_
#define RAPIDJSON_ISTREAMWRAPPER_H_
#include "stream.h"
#include <iosfwd>
#ifdef __clang__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(padded)
#endif
#ifdef _MSC_VER
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(4351) 
#endif
RAPIDJSON_NAMESPACE_BEGIN
template <typename StreamType>
class BasicIStreamWrapper {
public:
    typedef typename StreamType::char_type Ch;
    BasicIStreamWrapper(StreamType& stream) : stream_(stream), count_(), peekBuffer_() {}
    Ch Peek() const { 
        typename StreamType::int_type c = stream_.peek();
        return RAPIDJSON_LIKELY(c != StreamType::traits_type::eof()) ? static_cast<Ch>(c) : '\0';
    }
    Ch Take() { 
        typename StreamType::int_type c = stream_.get();
        if (RAPIDJSON_LIKELY(c != StreamType::traits_type::eof())) {
            count_++;
            return static_cast<Ch>(c);
        }
        else
            return '\0';
    }
    size_t Tell() const { return count_; }
    Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
    void Put(Ch) { RAPIDJSON_ASSERT(false); }
    void Flush() { RAPIDJSON_ASSERT(false); }
    size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }
    const Ch* Peek4() const {
        RAPIDJSON_ASSERT(sizeof(Ch) == 1); 
        int i;
        bool hasError = false;
        for (i = 0; i < 4; ++i) {
            typename StreamType::int_type c = stream_.get();
            if (c == StreamType::traits_type::eof()) {
                hasError = true;
                stream_.clear();
                break;
            }
            peekBuffer_[i] = static_cast<Ch>(c);
        }
        for (--i; i >= 0; --i)
            stream_.putback(peekBuffer_[i]);
        return !hasError ? peekBuffer_ : 0;
    }
private:
    BasicIStreamWrapper(const BasicIStreamWrapper&);
    BasicIStreamWrapper& operator=(const BasicIStreamWrapper&);
    StreamType& stream_;
    size_t count_;  
    mutable Ch peekBuffer_[4];
};
typedef BasicIStreamWrapper<std::istream> IStreamWrapper;
typedef BasicIStreamWrapper<std::wistream> WIStreamWrapper;
#if defined(__clang__) || defined(_MSC_VER)
RAPIDJSON_DIAG_POP
#endif
RAPIDJSON_NAMESPACE_END
#endif 
