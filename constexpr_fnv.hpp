#pragma once

#include <cstdint>
#include <type_traits>

namespace forced_value::detail {
    // until "template <auto val>" is implemented
    // this is the best workaround
    // but beware, template non-type arguments cannot be substituted into val
    template<class T, T val>
    constexpr T v = val;
}
#define v(expr) (forced_value::detail::v<decltype((expr)), (expr)>)

namespace fnv::detail {
    namespace constant::b32 {
        constexpr uint32_t prime = 16777619u;
        constexpr uint32_t offset_basis = 2166136261u;
    }

    namespace constant::b64 {
        constexpr uint64_t prime = 1099511628211uLL;
        constexpr uint64_t offset_basis = 14695981039346656037uLL;
    }

    template<class T, class U>
    constexpr T fnv1a_compute(const U input, const T prime, T hash) {
        hash ^= input;
        hash *= prime;

        return hash;
    }

    template<class T, class U, std::size_t N>
    constexpr T fnv1a_strlit(const U(&input)[N], const T prime, T hash) {
        for (std::size_t i = 0; i < (N - 1); i++) {
            hash = fnv1a_compute(input[i], prime, hash);
        }

        return hash;
    }

    template<class T = uint32_t, class U>
    constexpr T fnv1a_strptr(const U *input, const std::size_t len, const T prime, T hash) {
        for (std::size_t i = 0; i < len; i++) {
            hash = fnv1a_compute(input[i], prime, hash);
        }

        return hash;
    }
}

namespace fnv {
    template<class T = uint32_t, class U, std::size_t N>
    constexpr T fnv1a(const U(&input)[N]) {
        static_assert(std::is_same<U, char>() || std::is_same<U, wchar_t>(), "unsupported input type");
        static_assert(std::is_same<T, uint64_t>() || std::is_same<T, uint32_t>(), "unsupported hash type");

        if constexpr (std::is_same<T, uint64_t>()) {
            return detail::fnv1a_strlit<uint64_t>(input, detail::constant::b64::prime, detail::constant::b64::offset_basis);
        } else {
            return detail::fnv1a_strlit<uint32_t>(input, detail::constant::b32::prime, detail::constant::b32::offset_basis);
        }
    }

    template<class T = uint32_t, class U>
    constexpr T fnv1a(const U *input, std::size_t len) {
        static_assert(std::is_same<U, char>() || std::is_same<U, wchar_t>(), "unsupported input type");
        static_assert(std::is_same<T, uint64_t>() || std::is_same<T, uint32_t>(), "unsupported hash type");

        if constexpr (std::is_same<T, uint64_t>()) {
            return detail::fnv1a_strptr<uint64_t>(input, len, detail::constant::b64::prime, detail::constant::b64::offset_basis);
        } else {
            return detail::fnv1a_strptr<uint32_t>(input, len, detail::constant::b32::prime, detail::constant::b32::offset_basis);
        }
    }



    // WARNING: the hash is not consistent across different platform
    template<class U, class T = uint32_t>
    constexpr T make_type_unique_hash() {
#ifdef _MSC_VER
        // __FUNCSIG__: T <calling-convention> fnv::make_type_unique_hash<U, T>(void)
        constexpr auto val = fnv1a<T>(__FUNCSIG__);
        return val; 
#else // GCC, CLANG
        // __PRETTY_FUNCTION__ for GCC: constexpr T fnv::make_type_unique_hash() [with U = <Insert U name here>; T = <Insert T name here>]
        // __PRETTY_FUNCTION__ for Clang: T fnv::make_type_unique_hash() [U = <Insert U name here>, T = <Insert T name here>]
        return fnv1a<T>(__PRETTY_FUNCTION__);
#endif
    }
}

namespace fnv::literals {
    constexpr uint32_t operator "" _fnv1a_32(const char *ptr, const std::size_t len) {
        return detail::fnv1a_strptr(ptr, len, detail::constant::b32::prime, detail::constant::b32::offset_basis);
    }

    constexpr uint64_t operator "" _fnv1a_64(const char *ptr, const std::size_t len) {
        return detail::fnv1a_strptr(ptr, len, detail::constant::b64::prime, detail::constant::b64::offset_basis);
    }

    constexpr uint32_t operator "" _fnv1a_32(const wchar_t *ptr, const std::size_t len) {
        return detail::fnv1a_strptr(ptr, len, detail::constant::b32::prime, detail::constant::b32::offset_basis);
    }

    constexpr uint64_t operator "" _fnv1a_64(const wchar_t *ptr, const std::size_t len) {
        return detail::fnv1a_strptr(ptr, len, detail::constant::b64::prime, detail::constant::b64::offset_basis);
    }
}
