#include <cmath>

#include "json-struct.hh"

// ----------------------------------------------------------------------

// http://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
template<typename T> inline typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type float_equal(T x, T y, int ulp=1)
{
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::abs(x-y) < std::numeric_limits<T>::epsilon() * std::abs(x+y) * ulp
    // unless the result is subnormal
                           || std::abs(x-y) < std::numeric_limits<T>::min();
}

// ----------------------------------------------------------------------

class A
{
 public:
    inline A() { reset(); }

    inline bool operator == (const A& a) const
        {
            return i == a.i && float_equal(f, a.f) && s == a.s && b == a.b && vi == a.vi && ss == a.ss && msi == a.msi;
        }

    inline void reset()
        {
            i = std::numeric_limits<decltype(i)>::max();
            f = std::numeric_limits<decltype(f)>::max();
            s.clear();
            b = false;
            vi.clear();
            ss.clear();
            msi.clear();
        }

    int i;
    double f;
    std::string s;
    bool b;
    std::vector<int> vi;
    std::list<double> ld;
    std::set<std::string> ss;
    std::map<std::string, int> msi;
};

inline auto json_fields(A& a)
{
    return std::make_tuple("vi", &a.vi, "msi", &a.msi, "ld", &a.ld, "ss", &a.ss, "i", &a.i, "f", &a.f, "s", &a.s, "b", &a.b, "?", json::comment("ignored comment field"));
}

// ----------------------------------------------------------------------

static void test_simple();

// ----------------------------------------------------------------------

int main()
{
    test_simple();
    return 0;
}

// ----------------------------------------------------------------------

void test_simple()
{
    A a1;
    const auto dump1 = to_json(a1);
    std::cout << dump1 << std::endl;
    A a2;
    json::parse(dump1, a2);
    assert(a1 == a2);

    const char* source_a = R"({"vi": [121, 122, 124], "msi": {"seven": 7, "six": 6}, "ld": [7.700000, 8.800000], "ss": ["a", "b"], "i": 1967, "f": 20.160000, "s": "doetzchen", "b": true, "?": "ignored comment field"})";
    A a3;
    json::parse(source_a, a3);
    const auto dump3 = to_json(a3);
    std::cout << source_a << std::endl;
    std::cout << dump3 << std::endl;
    assert(std::string(source_a) == dump3);
}

// ----------------------------------------------------------------------
