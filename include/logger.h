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

    template<typename... Ts>
    LoggerTy& format(std::string_view fmt, const Ts&... args) {
        msg_ = uxs::format(fmt, args...);
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
    LoggerExtended(MsgType type, const SymbolLoc& loc) : Logger<LoggerExtended>(type), loc_(loc) {}
    ~LoggerExtended() { printMessage(getMessage()); }
    void printMessage(std::string_view msg);

 private:
    const SymbolLoc& loc_;
};

class LoggerExtendedUnwind : public Logger<LoggerExtendedUnwind> {
 public:
    LoggerExtendedUnwind(MsgType type, const SymbolLoc& loc) : Logger<LoggerExtendedUnwind>(type), loc_(loc) {}
    ~LoggerExtendedUnwind() { printMessage(getMessage()); }
    void printMessage(std::string_view msg);

 private:
    const SymbolLoc& loc_;
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

inline LoggerExtended debug(const SymbolLoc& l) { return LoggerExtended(MsgType::kDebug, l); }
inline LoggerExtendedUnwind debugUnwind(const SymbolLoc& l) { return LoggerExtendedUnwind(MsgType::kDebug, l); }
inline LoggerExtended info(const SymbolLoc& l) { return LoggerExtended(MsgType::kInfo, l); }
inline LoggerExtended note(const SymbolLoc& l) { return LoggerExtended(MsgType::kNote, l); }
inline LoggerExtendedUnwind warning(const SymbolLoc& l) { return LoggerExtendedUnwind(MsgType::kWarning, l); }
inline LoggerExtendedUnwind error(const SymbolLoc& l) { return LoggerExtendedUnwind(MsgType::kError, l); }
inline LoggerExtendedUnwind fatal(const SymbolLoc& l) { return LoggerExtendedUnwind(MsgType::kFatal, l); }

}  // namespace logger

}  // namespace daisy
