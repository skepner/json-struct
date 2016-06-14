// #include <ctime>
#include "json-struct.hh"

// ----------------------------------------------------------------------

static void test1();

// ----------------------------------------------------------------------

class A
{
 public:
    inline A() : i(0) {}
    inline A(int aI) : i(aI), f(aI), s(std::to_string(aI)), ls(5000, '-') {}

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
    int num_elements = 3000;

    auto start = std::clock();
    std::vector<A> va;
    for (int i = 1; i <= num_elements; ++i)
        va.emplace_back(i);
    std::cout << "making data " << (std::clock() - start) / double(CLOCKS_PER_SEC) << std::endl;

    start = std::clock();
    std::string da = json::dump(va, 1);
    std::cout << "dumping data " << (std::clock() - start) / double(CLOCKS_PER_SEC) << std::endl;
    std::cout << "dump of " << va.size() << ": " << da.size() << std::endl;

    start = std::clock();
    std::vector<A> vb;
    json::parse(da, vb);
    std::cout << "parsing data " << (std::clock() - start) / double(CLOCKS_PER_SEC) << std::endl;

    start = std::clock();
    std::string db = json::dump(vb, 1);
    std::cout << "dumping data " << (std::clock() - start) / double(CLOCKS_PER_SEC) << std::endl;
    std::cout << "dump of " << vb.size() << ": " << db.size() << std::endl;

    assert(da == db);
}

// ----------------------------------------------------------------------
