#pragma once

#include <cstddef>
#include <tuple>
#include <utility>

namespace util {
template<typename FnTy>
class raii_cleaner {
 public:
    explicit raii_cleaner(FnTy fn) : clean_fn_(fn) {}
    ~raii_cleaner() { clean_fn_(); }

 private:
    FnTy clean_fn_;
};
template<typename... Ts>
class raii_cleaner<std::tuple<Ts&...>> {
    static_assert(sizeof...(Ts) != 0, "at least one parameter must be specified");

 public:
    explicit raii_cleaner(std::tuple<Ts&...> v) : v_(v) {}
    ~raii_cleaner() { clean_impl(std::index_sequence_for<Ts...>()); }

 private:
    std::tuple<Ts&...> v_;

    template<std::size_t... Is>
    void clean_impl(std::index_sequence<Is...>) {
        (std::get<Is>(v_).clear(), ...);
    }
};
template<typename Ty>
raii_cleaner(Ty) -> raii_cleaner<Ty>;
}  // namespace util
