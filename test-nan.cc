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

class B
{
 public:
    inline B() {}

    std::vector<double> a;
    std::string s;

    friend inline auto json_fields(B& aa)
        {
            return std::make_tuple("a", &aa.a, "s", &aa.s);
        }
};

// ----------------------------------------------------------------------

int main()
{
    A a;
    auto d = json::dump(a, 1);
    std::cout << d << std::endl;
    A a1;
    json::parse(d, a1);
    std::cout << json::dump(a1, 1) << std::endl;

    std::string source_b = R"({"a": [null, 11, null], "s": "Null&NaN"})";
    B b;
    json::parse(source_b, b);
    std::copy(b.a.begin(), b.a.end(), std::ostream_iterator<double>(std::cout, " ")); std::cout << std::endl;
    std::cout << json::dump(b, 1) << std::endl;
    return 0;
}

// ----------------------------------------------------------------------
