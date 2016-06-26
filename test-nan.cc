#include "json-struct.hh"

// ----------------------------------------------------------------------

class A
{
 public:
    inline A() : a(77), b(std::numeric_limits<decltype(b)>::quiet_NaN()) {}

    double a;
    double b;

    friend inline auto json_fields(A& aa)
        {
            return std::make_tuple("a", &aa.a, "b", &aa.b);
        }
};

// ----------------------------------------------------------------------

int main()
{
    A a;
    auto d = json::dump(a, 1);
    std::cout << d << std::endl;
    A b;
    json::parse(d, b);
    std::cout << json::dump(b, 1) << std::endl;

    return 0;
}

// ----------------------------------------------------------------------
