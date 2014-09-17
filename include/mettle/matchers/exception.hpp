#ifndef INC_METTLE_MATCHERS_EXCEPTION_HPP
#define INC_METTLE_MATCHERS_EXCEPTION_HPP

#include "core.hpp"
#include "result.hpp"

namespace mettle {

namespace detail {
  template<typename Exception, typename Matcher>
  class thrown_impl_base : public matcher_tag {
  public:
    template<typename T>
    thrown_impl_base(T &&t) : matcher(std::forward<T>(t)) {}

    std::string desc() const {
      std::ostringstream ss;
      ss << "threw " << type_name<Exception>() << "(" << matcher.desc() << ")";
      return ss.str();
    }
  protected:
    match_result match(const Exception &e) const {
      match_result m = matcher(e);
      std::ostringstream ss;
      ss << "threw " << type_name<Exception>();
      if(!m.message.empty())
        ss << "(" << m.message << ")";
      return {m.matched, ss.str()};
    }
  private:
    Matcher matcher;
  };


  template<typename Exception, typename Matcher>
  struct thrown_impl : thrown_impl_base<Exception, Matcher> {
    using base = thrown_impl_base<Exception, Matcher>;

    using base::base;

    template<typename U>
    match_result operator ()(U &&value) const {
      try {
        value();
        return {false, "threw nothing"};
      }
      catch(const Exception &e) {
        return base::match(e);
      }
      catch(const std::exception &e) {
        std::ostringstream ss;
        ss << "threw " << to_printable(e);
        return {false, ss.str()};
      }
      catch(...) {
        return {false, "threw unknown exception"};
      }
    }
  };

  template<typename Matcher>
  struct thrown_impl<std::exception, Matcher>
    : thrown_impl_base<std::exception, Matcher> {
    using base = thrown_impl_base<std::exception, Matcher>;

    using base::base;

    template<typename U>
    match_result operator ()(U &&value) const {
      try {
        value();
        return {false, "threw nothing"};
      }
      catch(const std::exception &e) {
        return base::match(e);
      }
      catch(...) {
        return {false, "threw unknown exception"};
      }
    }
  };

}

template<typename Exception, typename T>
auto thrown_raw(T &&thing) {
  auto matcher = ensure_matcher(std::forward<T>(thing));
  return detail::thrown_impl<Exception, decltype(matcher)>(std::move(matcher));
}

template<typename Exception, typename T>
auto thrown(T &&thing) {
  return thrown_raw<Exception>(
    make_matcher(
      ensure_matcher(std::forward<T>(thing)),
      [](const auto &value, auto &&matcher) -> match_result {
        std::ostringstream ss;
        ss << "what: " << to_printable(value.what());
        return {matcher( std::string(value.what()) ), ss.str()};
      }, "what: "
    )
  );
}

template<typename Exception>
auto thrown() {
  return thrown_raw<Exception>(anything());
}

inline auto thrown() {
  return make_matcher([](auto &&value) -> match_result {
    try {
      value();
      return {false, "threw nothing"};
    }
    catch(...) {
      return true;
    }
  }, "threw exception");
}

} // namespace mettle

#endif
