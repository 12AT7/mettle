#ifndef INC_METTLE_OUTPUT_TO_PRINTABLE_HPP
#define INC_METTLE_OUTPUT_TO_PRINTABLE_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <locale>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

#include "string.hpp"
#include "traits.hpp"
#include "type_name.hpp"
#include "../detail/algorithm.hpp"
#include "../detail/tuple_algorithm.hpp"

namespace mettle {

  template<typename T>
  std::string to_printable(T begin, T end);

  // Non-generic overloads

  inline std::string to_printable(std::nullptr_t) {
    return "nullptr";
  }

  template<typename Char, typename Traits, typename Alloc>
  inline std::string to_printable(
    const std::basic_string<Char, Traits, Alloc> &s
  ) requires string_convertible<std::basic_string<Char, Traits, Alloc>> {
    return represent_string(s);
  }

  template<typename Char, typename Traits>
  inline std::string to_printable(
    const std::basic_string_view<Char, Traits> &s
  ) requires string_convertible<std::basic_string_view<Char, Traits>> {
    return represent_string(s);
  }

  inline std::string to_printable(char c) {
    return represent_string(std::string(1, c), '\'');
  }

  inline std::string to_printable(wchar_t c) {
    return represent_string(std::wstring(1, c), '\'');
  }

#if __cpp_char8_t
  inline std::string to_printable(char8_t c) {
    return represent_string(std::u8string(1, c), '\'');
  }
#endif

  inline std::string to_printable(char16_t c) {
    return represent_string(std::u16string(1, c), '\'');
  }

  inline std::string to_printable(char32_t c) {
    return represent_string(std::u32string(1, c), '\'');
  }

  inline std::string to_printable(unsigned char c) {
    std::ostringstream ss;
    ss << "0x" << std::setw(2) << std::setfill('0') << std::hex
       << static_cast<unsigned int>(c);
    return ss.str();
  }

  inline std::string to_printable(signed char c) {
    std::ostringstream ss;
    ss << (c >= 0 ? '+' : '-') << "0x" << std::setw(2) << std::setfill('0')
       << std::hex << std::abs(static_cast<int>(c));
    return ss.str();
  }

  inline std::string to_printable(std::byte b) {
    return to_printable(static_cast<unsigned char>(b));
  }

  template<character T>
  inline std::string to_printable(const T *s) {
    if(!s) return to_printable(nullptr);
    return represent_string(s);
  }

  template<typename Ret, typename ...Args>
  inline auto to_printable(Ret (*)(Args...)) {
    return type_name<Ret(Args...)>();
  }

  // Containers

  namespace detail {
    template<typename T>
    std::string stringify_tuple(const T &tuple);
  }

  template<typename T, typename U>
  std::string to_printable(const std::pair<T, U> &pair) {
    return detail::stringify_tuple(pair);
  }

  template<typename ...T>
  std::string to_printable(const std::tuple<T...> &tuple) {
    return detail::stringify_tuple(tuple);
  }

  template<typename T, std::size_t N>
  std::string to_printable(const T (&v)[N]) requires(!character<T>) {
    return to_printable(std::begin(v), std::end(v));
  }

  // The main `to_printable` implementation. This needs to be after the
  // non-generic versions above so that those overloads get picked up by the
  // calls inside this function.

  template<typename T>
  inline auto to_printable(const T &t) {
    if constexpr(std::is_pointer_v<T>) {
      using ValueType = std::remove_pointer_t<T>;
      if constexpr(!std::is_const_v<ValueType>) {
        return to_printable(const_cast<const ValueType *>(t));
      } else {
        if(!t) return to_printable(nullptr);
        std::ostringstream ss;
        if constexpr (std::is_same_v<ValueType, const unsigned char> ||
                      std::is_same_v<ValueType, const signed char>) {
          // Don't print signed/unsigned char* as regular strings.
          ss << static_cast<const void *>(t);
        } else {
          ss << t;
        }
        return ss.str();
      }
    } else if constexpr(std::is_enum_v<T>) {
      return type_name<T>() + "(" + std::to_string(
        static_cast<typename std::underlying_type<T>::type>(t)
      ) + ")";
    } else if constexpr(std::is_same_v<std::remove_cv_t<T>, bool>) {
      return t ? "true" : "false";
    } else if constexpr(printable<T>) {
      return t;
    } else if constexpr(any_exception<T>) {
      std::ostringstream ss;
      ss << type_name(t) << "(" << to_printable(t.what()) << ")";
      return ss.str();
    } else if constexpr(iterable<T>) {
      return to_printable(std::begin(t), std::end(t));
    } else {
      return type_name<T>();
    }
  }

  template<typename T>
  inline auto to_printable(const volatile T &t) {
    return to_printable(const_cast<const T &>(t));
  }

  // These need to be last in order for the `to_printable()` calls inside to
  // pick up all the above implementations (plus any others via ADL).

  template<typename T>
  std::string to_printable(T begin, T end) {
    std::ostringstream ss;
    ss << "[" << detail::iter_joined(begin, end, [](auto &&item) {
      return to_printable(item);
    }) << "]";
    return ss.str();
  }

  namespace detail {
    template<typename T>
    std::string stringify_tuple(const T &tuple) {
      std::ostringstream ss;
      ss << "[" << tuple_joined(tuple, [](auto &&item) {
        return to_printable(item);
      }) << "]";
      return ss.str();
    }
  }

} // namespace mettle

#endif
