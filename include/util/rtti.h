#pragma once

#include "uxs/crc32.h"

#include <stdexcept>
#include <utility>

#define RTTI_DECLARE_TYPE_INFO(ty) \
    template<> \
    struct util::type_info<ty> { \
        static constexpr const char* kName = #ty; \
        static constexpr uint64_t kId = util::detail::compose_type_id(kName, __FILE__, __LINE__); \
        static util::type_info_desc desc; \
    }
#define RTTI_IMPLEMENT_TYPE_INFO(ty) \
    util::type_info_desc util::type_info<ty>::desc { \
        util::type_info<ty>::kId, util::type_info<ty>::kName, util::detail::get_super_type_desc<ty>() \
    }

namespace util {

template<typename Ty>
struct type_info;

struct type_info_desc {
    uint64_t type_id;
    const char* name;
    const type_info_desc* super_desc;
};

namespace detail {
constexpr const char* trim_file_path(const char* file_name) {
    for (const char* p = file_name; *p; ++p) {
        if (*p == '/' || *p == '\\') { file_name = p + 1; }
    }
    return file_name;
}
constexpr uint64_t compose_type_id(const char* type_name, const char* file_name, uint32_t n_line) {
    return (static_cast<uint64_t>(uxs::crc32::calc(trim_file_path(file_name)) ^ n_line) << 32) |
           uxs::crc32::calc(type_name);
}
template<typename Ty, typename = std::void_t<typename Ty::super_class_t>>
constexpr const type_info_desc* get_super_type_desc() {
    return &util::type_info<typename Ty::super_class_t>::desc;
}
template<typename Ty, typename... Dummy>
constexpr const type_info_desc* get_super_type_desc(Dummy...) {
    return nullptr;
}
}  // namespace detail

template<typename Ty, typename SuperTy = void>
class rtti_mixin : public SuperTy {
 public:
    using rtti_mixin_t = rtti_mixin;
    using super_class_t = SuperTy;
    const type_info_desc* get_rtti_type_info() const noexcept override { return &type_info<Ty>::desc; };
    template<typename... Args>
    explicit rtti_mixin(Args&&... args) : SuperTy(std::forward<Args>(args)...) {}
};

template<typename Ty>
class rtti_mixin<Ty, void> {
 public:
    using rtti_mixin_t = rtti_mixin;
    virtual const type_info_desc* get_rtti_type_info() const noexcept { return &type_info<Ty>::desc; };
    virtual ~rtti_mixin() = default;
};

namespace detail {
template<typename FromTy, typename ToTy, typename = void>
struct has_relation : std::false_type {};
template<typename FromTy, typename ToTy>
struct has_relation<FromTy, ToTy,  //
                    std::void_t<decltype(static_cast<ToTy*>(std::declval<FromTy*>()))>> : std::true_type {};
template<typename FromTy, typename ToTy>
constexpr bool has_relation_v = has_relation<FromTy, ToTy>::value;
}  // namespace detail

template<typename... Kind, typename Ty, typename = std::void_t<typename Ty::rtti_mixin_t>>
[[nodiscard]] constexpr bool is_kind_of(Ty& ref) noexcept {
    using TyNoCV = std::remove_cv_t<Ty>;
    static_assert(sizeof...(Kind) != 0, "unspecified kind");
    static_assert((std::is_same_v<Kind, std::remove_cv_t<std::remove_reference_t<Kind>>> && ...),
                  "kind should neither be a reference nor has cv-qualifiers");
    static_assert((detail::has_relation_v<TyNoCV, Kind> && ...), "incomplete or incompatible types");
    if constexpr ((!std::is_convertible_v<TyNoCV*, Kind*> && ...)) {
        for (auto* info = ref.get_rtti_type_info(); ((info->type_id != type_info<Kind>::kId) && ...);
             info = info->super_desc) {
            if (info->type_id == type_info<TyNoCV>::kId) { return false; }
        }
    }
    return true;
}

template<typename... Kind, typename Ty, typename = std::void_t<typename Ty::rtti_mixin_t>>
[[nodiscard]] bool is_kind_of(Ty* ref) noexcept {
    return ref && is_kind_of<Kind...>(*ref);
}

template<typename ToTy, typename FromTy, typename = std::enable_if_t<std::is_reference_v<ToTy>>>
[[nodiscard]] ToTy cast(FromTy& ref) {
    using Kind = std::remove_cv_t<std::remove_reference_t<ToTy>>;
    if (!is_kind_of<Kind>(ref)) { throw std::runtime_error("bad cast"); }
    return static_cast<ToTy>(ref);
}

template<typename ToTy, typename FromTy, typename = std::enable_if_t<std::is_pointer_v<ToTy>>>
[[nodiscard]] ToTy cast(FromTy* ptr) noexcept {
    using Kind = std::remove_cv_t<std::remove_pointer_t<ToTy>>;
    return is_kind_of<Kind>(ptr) ? static_cast<ToTy>(ptr) : nullptr;
}

}  // namespace util
