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

template <typename Ty> void print(Ty x, unsigned sh) {
  std::cout << "#if (";
  print_num(x);
  std::cout << " " XSTR(OP) " " << sh << ") != ";
  print_num(x OP sh);
  std::cout << std::endl;
  std::cout << "#error invalid result" << std::endl;
  std::cout << "#endif" << std::endl;
}

int main() {
  print(2678, 5);
  print(2678u, 5);
  print(-2678, 5);
  print(-2678u, 5);

  return 0;
}
