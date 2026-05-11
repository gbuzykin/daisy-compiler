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

template <typename Ty1, typename Ty2> void print(Ty2 x) {
  std::cout << "#if ";
  print_num(x);
  std::cout << " != ";
  print_num(static_cast<Ty1>(x));
  std::cout << std::endl;
  std::cout << "#error invalid result" << std::endl;
  std::cout << "#endif" << std::endl;
}

int main() {
  print<int>(99871u);
  print<int>(static_cast<int8_t>(47));
  print<int>(static_cast<uint8_t>(47));
  print<unsigned>(static_cast<int8_t>(47));
  print<unsigned>(static_cast<uint8_t>(47));

  //-------------------------------------------

  print<int>(-99871u);
  print<int>(static_cast<int8_t>(-static_cast<int8_t>(47)));
  print<unsigned>(static_cast<int8_t>(-static_cast<int8_t>(47)));

  return 0;
}
