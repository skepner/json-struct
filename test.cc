// #include <ctime>
#include "json-struct.hh"

// ----------------------------------------------------------------------

static void test1();

// ----------------------------------------------------------------------

class A
{
 public:
    inline A() : i(0) {}
    inline A(int aI) : i(aI), f(aI ? 1.0 / double(aI) : -0.3456), s(std::to_string(aI)), ls(10, '-') {}

    int i;
    double f;
    std::string s;
    std::string ls;

    friend inline auto json_fields(A& a)
        {
            return std::make_tuple("i", &a.i, "f", &a.f, "s", &a.s, "ls", &a.ls);
        }
};

// ----------------------------------------------------------------------

int main()
{
    test1();

    return 0;
}

// ----------------------------------------------------------------------

void test1()
{
    // std::string src = R"({"i": 0, "f": .1, "s": "aa", "ls": "bbb"})";
    // A a0;
    // json::parse(src, a0);
    // std::cout << json::dump(a0, 1) << std::endl;

    int num_elements = 5;

    auto start = std::clock();
    std::vector<A> va;
    for (int i = 3; i <= num_elements; ++i)
        va.emplace_back(i);
    std::cout << "making data " << (std::clock() - start) / double(CLOCKS_PER_SEC) << std::endl;

    start = std::clock();
    std::string da = json::dump(va, 1);
    std::cout << "dumping data " << (std::clock() - start) / double(CLOCKS_PER_SEC) << std::endl;
    std::cout << "dump of " << va.size() << ": " << da.size() << std::endl;
    std::cout << da << std::endl;

    start = std::clock();
    std::vector<A> vb;
    json::parse(da, vb);
    std::cout << "parsing data " << (std::clock() - start) / double(CLOCKS_PER_SEC) << std::endl;

    start = std::clock();
    std::string db = json::dump(vb, 1);
    std::cout << "dumping data " << (std::clock() - start) / double(CLOCKS_PER_SEC) << std::endl;
    std::cout << "dump of " << vb.size() << ": " << db.size() << std::endl;
    std::cout << db << std::endl;

    assert(da == db);
}

// ----------------------------------------------------------------------
