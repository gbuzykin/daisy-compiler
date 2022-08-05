#pragma once

constexpr unsigned long long operator""_KiB(unsigned long long sz) { return sz * 1024; }
constexpr unsigned long long operator""_MiB(unsigned long long sz) { return sz * 1024 * 1024; }
