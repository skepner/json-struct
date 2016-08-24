#include "json-struct.hh"

// ----------------------------------------------------------------------

class A
{
 public:
    inline A() : d1(0.15), d2(1.6), d3(M_PI), d4(1.6e56), f1(0.15F), f2(1.6F), f3(static_cast<float>(M_PI)) {}

    double d1, d2, d3, d4;
    float f1, f2, f3;

    friend inline auto json_fields(A& aa)
        {
            return std::make_tuple(
                "d1", &aa.d1,
                "d2", &aa.d2,
                "d3", &aa.d3,
                "d4", &aa.d4,
                "f1", &aa.f1,
                "f2", &aa.f2,
                "f3", &aa.f3
                                   );
        }
};

// ----------------------------------------------------------------------

int main()
{
    A a;
    auto d = json::dump(a, 1);
    std::cout << d << std::endl;
    return 0;
}

// ----------------------------------------------------------------------
