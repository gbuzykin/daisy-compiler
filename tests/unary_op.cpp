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

template <typename Ty> void print(Ty x) {
  std::cout << "#if ";
  std::cout << XSTR(OP);
  print_num(x);
  std::cout << " != ";
  print_num(OP x);
  std::cout << std::endl;
  std::cout << "#error invalid result" << std::endl;
  std::cout << "#endif" << std::endl;
}

int main() {
  print(2678);
  print(2678u);
  print(-2678);
  print(-2678u);

  return 0;
}
