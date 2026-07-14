// This file intentionally violates Chromium clang-format style.
// It exists ONLY to demonstrate the lint-clang-format PR check (NEVA-11205)
// and must NOT be merged. It is not referenced by any BUILD.gn target.

#include <vector>
#include <string>

namespace neva {

int  Sum(int a,int b){
  if(a>b)
    return a+b ;
  return a-b;
}

class   DemoWidget {
 public:
  DemoWidget() {}
  void SetName( const std::string &name ) {
    std::vector<int>  values = {1,2,3};
    for(auto v : values) { total_ += v; }
    name_ = name;
  }

 private:
  std::string name_;
  int total_=0;
};

}  // namespace neva
