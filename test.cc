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
    inline A(int aI, double aF, std::string aS, bool aB) : i(aI), f(aF), s(aS), b(aB) {}

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

class B
{
 public:
    inline B() : s("bb") {}

    inline bool operator == (const B& b) const
        {
            return s == b.s && a == b.a && va == b.va && la == b.la;
        }

    inline void reset()
        {
            s.clear();
            a.reset();
            va.clear();
            la.clear();
        }

    std::string s;
    A a;
    std::vector<A> va;
    std::list<A> la;
};

inline auto json_fields(B& b)
{
    return std::make_tuple("a", &b.a, "la", &b.la, "s", &b.s, "va", &b.va);
}

// ----------------------------------------------------------------------

static void test_simple();
static void test_nested();
static void test_parsing_failure();

// ----------------------------------------------------------------------

int main()
{
    test_simple();
    test_nested();
    test_parsing_failure();
    return 0;
}

// ----------------------------------------------------------------------

void test_simple()
{
    A a1;
    const auto dump1 = json::dump(a1);
    std::cout << "a1: " << dump1 << std::endl;
    A a2;
    json::parse(dump1, a2);
    assert(a1 == a2);

    const char* source_a = R"({"vi": [121, 122, 124], "msi": {"seven": 7, "six": 6}, "ld": [7.7e10, 8.8e-10, 8.8, 9.9e33], "ss": ["a", "b"], "i": 1967, "f": 20.16e15, "s": "doetzchen", "b": true, "?": "ignored comment field"})";
    A a3;
    json::parse(source_a, a3);
    const auto dump3 = json::dump(a3);
    std::cout << "source_a: " << source_a << std::endl;
    std::cout << "a3:       " << dump3 << std::endl;
      //assert(std::string(source_a) == dump3);
}

// ----------------------------------------------------------------------

void test_nested()
{
    B b1;
    b1.s = "b1";
    b1.a.s = "b1.a";
    b1.a.i = 202;
    b1.a.f = 202.202;
    b1.va.emplace_back(505, 505.505, "b1.va.0", true);
    const auto dump1 = json::dump(b1);
    std::cout << "b1: " << dump1 << std::endl;
    B b2;
    json::parse(dump1, b2);
    assert(b1 == b2);

} // test_nested

// ----------------------------------------------------------------------

void test_parsing_failure()
{
    const char* source_err = R"({"vi": [121, 122, 124], "msix": {"seven": 7, "six": 6}, "ld": [7.700000, 8.800000], "ss": ["a", "b"], "i": 1967, "f": 20.160000, "s": "doetzchen", "b": true, "?": "ignored comment field"})";
    A a1;
    try {
        json::parse(source_err, a1);
    }
    catch (json::parsing_error& err) {
        std::cerr << "json:0: error: "<< err.what() << std::endl;
    }

} // test_parsing_failure

// ----------------------------------------------------------------------
