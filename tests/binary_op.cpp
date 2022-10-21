#include <iostream>
#include <type_traits>

#define XSTR(s) STR(s)
#define STR(s) #s

template <typename Ty> void print_num(Ty x) {
  if constexpr (std::is_signed_v<Ty>) {
    std::cout << static_cast<int>(x) << "i";
  } else {
    std::cout << static_cast<unsigned>(x) << "u";
  }
  std::cout << sizeof(Ty) * 8;
}

template <typename Ty1, typename Ty2> void print(Ty1 x, Ty2 y) {
  std::cout << "#if (";
  print_num(x);
  std::cout << " " XSTR(OP) " ";
  print_num(y);
  std::cout << ") != ";
  print_num(x OP y);
  std::cout << std::endl;
  std::cout << "#error invalid result" << std::endl;
  std::cout << "#endif" << std::endl;
}

int main() {
  print(2678, 99871);
  print(2678u, 99871);
  print(static_cast<int8_t>(47), 99871);
  print(static_cast<uint8_t>(47), 99871);
  print(2678u, 99871u);
  print(static_cast<int8_t>(47), 99871u);
  print(static_cast<uint8_t>(47), 99871u);

  //-------------------------------------------

  print(-2678, -99871);
  print(-2678u, -99871);
  print(static_cast<int8_t>(-static_cast<int8_t>(47)), -99871);
  print(static_cast<uint8_t>(-static_cast<uint8_t>(47)), -99871);
  print(-2678u, -99871u);
  print(static_cast<int8_t>(-static_cast<int8_t>(47)), -99871u);
  print(static_cast<uint8_t>(-static_cast<uint8_t>(47)), -99871u);

  //-------------------------------------------

  print(-2678, 99871);
  print(-2678u, 99871);
  print(static_cast<int8_t>(-static_cast<int8_t>(47)), 99871);
  print(static_cast<uint8_t>(-static_cast<uint8_t>(47)), 99871);
  print(-2678u, 99871u);
  print(static_cast<int8_t>(-static_cast<int8_t>(47)), 99871u);
  print(static_cast<uint8_t>(-static_cast<uint8_t>(47)), 99871u);

  //-------------------------------------------

  print(2678, -99871);
  print(static_cast<int8_t>(47), -99871);
  print(static_cast<uint8_t>(47), -99871);
  print(static_cast<int8_t>(47), -99871u);
  print(static_cast<uint8_t>(47), -99871u);

  return 0;
}
