#pragma once

#include "common/symbol_loc.h"
#include "uxs/format.h"

namespace daisy {

struct CompilationContext;

namespace logger {

extern unsigned g_debug_level;

enum class MsgType : unsigned { kFatal = 0, kError, kWarning, kNote, kInfo, kDebug };
constexpr MsgType operator+(MsgType type, unsigned level) {
    return static_cast<MsgType>(static_cast<unsigned>(type) + level);
}

template<typename LoggerTy>
class Logger {
 public:
    explicit Logger(MsgType type) : type_(type) {}
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    MsgType getType() const { return type_; }
    const std::string& getMessage() const { return msg_; }

    template<typename... Args>
    LoggerTy& format(uxs::format_string<Args...> fmt, const Args&... args) {
        if (getType() < MsgType::kInfo + g_debug_level) {
            msg_ = uxs::vformat(fmt.get(), uxs::make_format_args(args...));
        }
        return static_cast<LoggerTy&>(*this);
    }

 private:
    MsgType type_;
    std::string msg_;
};

class LoggerSimple : public Logger<LoggerSimple> {
 public:
    explicit LoggerSimple(MsgType type) : Logger<LoggerSimple>(type), header_("daisy-compiler") {}
    LoggerSimple(MsgType type, std::string_view hdr) : Logger<LoggerSimple>(type), header_(hdr) {}
    ~LoggerSimple() { printMessage(getMessage()); }
    void printMessage(std::string_view msg);

 private:
    std::string_view header_;
};

class LoggerExtended : public Logger<LoggerExtended> {
 public:
    LoggerExtended(MsgType type, const SymbolLoc& loc, bool ext_loc_info = false)
        : Logger<LoggerExtended>(type), loc_(loc), print_ext_loc_info_(ext_loc_info) {}
    ~LoggerExtended() { printMessage(getMessage()); }
    void printMessage(std::string_view msg);

 private:
    const SymbolLoc& loc_;
    bool print_ext_loc_info_;
};

inline LoggerSimple debug() { return LoggerSimple(MsgType::kDebug); }
inline LoggerSimple info() { return LoggerSimple(MsgType::kInfo); }
inline LoggerSimple note() { return LoggerSimple(MsgType::kNote); }
inline LoggerSimple warning() { return LoggerSimple(MsgType::kWarning); }
inline LoggerSimple error() { return LoggerSimple(MsgType::kError); }
inline LoggerSimple fatal() { return LoggerSimple(MsgType::kFatal); }

inline LoggerSimple debug(std::string_view hdr) { return LoggerSimple(MsgType::kDebug, hdr); }
inline LoggerSimple info(std::string_view hdr) { return LoggerSimple(MsgType::kInfo, hdr); }
inline LoggerSimple note(std::string_view hdr) { return LoggerSimple(MsgType::kNote, hdr); }
inline LoggerSimple warning(std::string_view hdr) { return LoggerSimple(MsgType::kWarning, hdr); }
inline LoggerSimple error(std::string_view hdr) { return LoggerSimple(MsgType::kError, hdr); }
inline LoggerSimple fatal(std::string_view hdr) { return LoggerSimple(MsgType::kFatal, hdr); }

inline LoggerExtended debug(const SymbolLoc& l, bool ext_loc_info = false) {
    return LoggerExtended(MsgType::kDebug, l, ext_loc_info);
}
inline LoggerExtended info(const SymbolLoc& l, bool ext_loc_info = false) {
    return LoggerExtended(MsgType::kInfo, l, ext_loc_info);
}
inline LoggerExtended note(const SymbolLoc& l, bool ext_loc_info = false) {
    return LoggerExtended(MsgType::kNote, l, ext_loc_info);
}
inline LoggerExtended warning(const SymbolLoc& l) { return LoggerExtended(MsgType::kWarning, l, true); }
inline LoggerExtended error(const SymbolLoc& l) { return LoggerExtended(MsgType::kError, l, true); }
inline LoggerExtended fatal(const SymbolLoc& l) { return LoggerExtended(MsgType::kFatal, l, true); }

}  // namespace logger

}  // namespace daisy
