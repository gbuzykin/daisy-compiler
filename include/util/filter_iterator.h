#pragma once

#include "rtti.h"
#include "uxs/iterator.h"

namespace util {

template<typename Iter, typename ValTy,
         typename = std::void_t<typename std::remove_reference_t<decltype(*std::declval<Iter>())>::rtti_mixin_t>>
class filter_iterator
    : public uxs::iterator_facade<filter_iterator<Iter, ValTy>, ValTy, std::forward_iterator_tag, ValTy&, ValTy*,
                                  typename std::iterator_traits<Iter>::difference_type> {
    static_assert(detail::has_relation_v<std::remove_cv_t<typename std::iterator_traits<Iter>::value_type>,
                                         std::remove_cv_t<ValTy>>,
                  "incomplete or incompatible types");

 private:
    using super = uxs::iterator_facade<filter_iterator<Iter, ValTy>, ValTy, std::forward_iterator_tag, ValTy&, ValTy*,
                                       typename std::iterator_traits<Iter>::difference_type>;

 public:
    using iterator_type = Iter;
    using filter_kind = std::remove_cv_t<ValTy>;
    using reference = typename super::reference;

    filter_iterator() noexcept {}
    filter_iterator(iterator_type curr, iterator_type end) noexcept : curr_(curr), end_(end) {
        while (curr_ != end_ && !is_kind_of<filter_kind>(*curr_)) { ++curr_; }
    }

    void increment() noexcept {
        while (++curr_ != end_ && !is_kind_of<filter_kind>(*curr_)) {}
    }

    bool is_equal_to(const filter_iterator& it) const noexcept {
        uxs_iterator_assert(end_ == it.end_);
        return curr_ == it.curr_;
    }

    reference dereference() const noexcept { return static_cast<reference>(*curr_); }

    iterator_type base() const { return curr_; }

 private:
    iterator_type curr_, end_;
};

template<typename ValTy, typename Iter>
filter_iterator<Iter, ValTy> make_filter_iterator(Iter from, Iter to) noexcept {
    return filter_iterator<Iter, ValTy>(from, to);
}

template<typename ValTy, typename Iter>
uxs::iterator_range<filter_iterator<Iter, ValTy>> make_filter_range(Iter from, Iter to) noexcept {
    return {filter_iterator<Iter, ValTy>(from, to), filter_iterator<Iter, ValTy>(to, to)};
}

template<typename ValTy, typename Iter>
uxs::iterator_range<filter_iterator<Iter, ValTy>> make_filter_range(const std::pair<Iter, Iter>& p) noexcept {
    return {filter_iterator<Iter, ValTy>(p.first, p.second), filter_iterator<Iter, ValTy>(p.second, p.second)};
}

template<typename ValTy, typename Range>
auto make_filter_range(Range&& r) noexcept -> uxs::iterator_range<filter_iterator<decltype(std::end(r)), ValTy>> {
    return {filter_iterator<decltype(std::end(r)), ValTy>(std::begin(r), std::end(r)),
            filter_iterator<decltype(std::end(r)), ValTy>(std::end(r), std::end(r))};
}

}  // namespace util
