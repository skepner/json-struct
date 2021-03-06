#include <cmath>
#include <ctime>

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

static void test_simple();
static void test_nested();
static void test_parsing_failure();
static void test_recursive_struct();
static void test_field_getter_setter();

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

 private:
    int i;
    double f;
    std::string s;
    bool b;
    std::vector<int> vi;
    std::list<double> ld;
    std::set<std::string> ss;
    std::map<std::string, int> msi;

    friend inline auto json_fields(A& a)
        {
            return std::make_tuple("vi", &a.vi, "msi", &a.msi, "ld", &a.ld, "ss", &a.ss, "i", &a.i, "f", &a.f, "s", &a.s, "b", &a.b, "?", json::comment("ignored comment field"));
        }

    friend void test_simple();
    friend void test_nested();
};

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

int main()
{
    test_field_getter_setter();

    test_simple();
    test_nested();
    test_parsing_failure();
    test_recursive_struct();

    return 0;
}

// ----------------------------------------------------------------------

void test_simple()
{
    int indent = 2;
    A a1;
    std::cout << "a1: " << json::dump(a1, indent) << std::endl;
    a1.vi.push_back(93);
    a1.ld.push_back(9.3);
    a1.ss.insert("neun");
    a1.msi["elf"] = 11;
    std::cout << "a1: " << json::dump(a1, indent) << std::endl;
    a1.vi.push_back(94);
    a1.ld.push_back(9.4);
    a1.ss.insert("zehn");
    a1.msi["zwölf"] = 12;

    const auto dump1 = json::dump(a1, indent);
    std::cout << "a1: " << dump1 << std::endl;

    A a2;
    json::parse(dump1, a2);
    assert(a1 == a2);

    const char* source_a = R"({"vi": [121, 122, 124], "msi": {"seven": 7, "six": 6}, "ld": [7.7e10, 8.8e-10, 8.8, 9.9e33], "ss": ["a", "b"], "i": 1967, "f": 20.16e15, "s": "doetzchen", "b": true, "?": "ignored comment field"})";
    A a3;
    json::parse(source_a, a3);
    const auto dump3 = json::dump(a3, indent);
    std::cout << "source_a: " << source_a << std::endl;
    std::cout << "a3:       " << dump3 << std::endl;
      //assert(std::string(source_a) == dump3);
}

// ----------------------------------------------------------------------

void test_nested()
{
    int indent = 2;
    B b1;
    b1.s = "b1";
    b1.a.s = "b1.a";
    b1.a.i = 202;
    b1.a.f = 202.202;
    b1.va.emplace_back(505, 505.505, "b1.va.0", true);
    const auto dump1 = json::dump(b1, indent);
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

class R
{
 public:

    int i;
    std::vector<R> sub;

    friend inline auto json_fields(R& r)
        {
            return std::make_tuple("i", &r.i, "sub", &r.sub);
        }

};

void test_recursive_struct()
{
    const char* source_r = R"({"i": 1, "sub": [{"i": 11}, {"i": 12, "sub": [{"i": 121}]}]})";
    R r;
    json::parse(source_r, r);
    std::cout << "r: " << json::dump(r, 2) << std::endl;

} // test_recursive_struct

// ----------------------------------------------------------------------

class GS_Field
{
 public:
    inline GS_Field() { reset(); }
    inline void reset()
        {
            mTime.tm_sec = mTime.tm_min = mTime.tm_hour = 0;
            mTime.tm_year = mTime.tm_mon = 0;
            mTime.tm_mday = 1;
        }

    inline std::string display() const
        {
            char buf[16];
            std::strftime(buf, sizeof(buf), "%Y-%m-%d", &mTime);
            return buf;
        }

    inline void parse(std::string aText)
        {
            if (aText.size() == 10) {
                strptime(aText.c_str(), "%Y-%m-%d", &mTime);
            }
            else if (aText.size() == 7) {
                strptime(aText.c_str(), "%Y-%m", &mTime);
            }
            else if (aText.size() == 0) {
                reset();
            }
            else {
                throw std::runtime_error(std::string("cannot parse date from ") + aText);
            }
        }

    inline std::map<std::string, int> split() const
        {
            return {{"year", mTime.tm_year + 1900}, {"month", mTime.tm_mon + 1}, {"day", mTime.tm_mday}};
        }

    inline void combine(const std::map<std::string, int>& source)
        {
            try { mTime.tm_year = source.at("year") - 1900; } catch (std::out_of_range&) {}
            try { mTime.tm_mon  = source.at("month") - 1; } catch (std::out_of_range&) {}
            try { mTime.tm_mday = source.at("day"); } catch (std::out_of_range&) {}
        }

 private:
    std::tm mTime;
};

class GS
{
 public:
    double f;
    int i;
    bool b;
    std::vector<int> vi;
    std::vector<int> vi_no_read;
    GS_Field time;
    GS_Field date;
    GS_Field no_read;

    friend inline auto json_fields(GS& gs)
        {
            return std::make_tuple("f", &gs.f
                                   , "time", json::field(&gs.time, &GS_Field::display, &GS_Field::parse)
                                   , "date", json::field(&gs.date, &GS_Field::split, &GS_Field::combine)
                                   , "no_read", json::field(&gs.no_read, &GS_Field::split)
                                   , "b", json::field(&gs.b, json::output_if_true) // json::output_if(&gs.b)
                                   , "i", &gs.i
                                   , "vi_no_read", json::field(&gs.vi_no_read, json::output_only_if_not_empty)
                                   , "vi", json::field(&gs.vi, json::output_if_not_empty)
                                   );
        }

};

void test_field_getter_setter()
{
    GS gs1;
    gs1.i = 111;
    gs1.time.parse("2016-06-12");
    gs1.date.parse("2016-03-22");
    std::string dump1 = json::dump(gs1, 2);
    std::cout << "gs1: " << dump1 << std::endl;
    GS gs2;
    json::parse(dump1, gs2);
    std::cout << "gs2: " << json::dump(gs2, 0) << std::endl;

    std::string source_gs3 = R"({"f": 0, "time": "2016-06-12", "date": {"day": 22, "month": 4, "year": 2000}, "no_read": {"day": 2, "month": 2, "year": 1901}, "b": true, "i": 111, "vi": [56, 57], "vi_no_read": [321]})";
    GS gs3;
    json::parse(source_gs3, gs3);
    std::cout << "s 3: " << source_gs3 << std::endl;
    std::cout << "gs3: " << json::dump(gs3, 0) << std::endl;

    std::cout << std::endl << std::endl;

} // test_field_getter_setter

// ----------------------------------------------------------------------
