// #include <ctime>
#include "json-struct.hh"

// ----------------------------------------------------------------------

static void test1();

// ----------------------------------------------------------------------

class A
{
 public:
    inline A() : i(1) {}
    inline A(int aI) : i(aI) {}

    int i;

    friend inline auto json_fields(A& a, bool for_output)
        {
            if (for_output && a.i % 2)
                throw json::no_value();
            return std::make_tuple("i", &a.i);
        }
};

// ----------------------------------------------------------------------

class B
{
 public:

    std::vector<A> va;

    friend inline auto json_fields(B& b)
        {
            return std::make_tuple("va", &b.va, "msg", json::comment("partial array output test"));
        }
};


int main()
{
    test1();

    return 0;
}

// ----------------------------------------------------------------------

void test1()
{
    int num_elements = 10;

      // auto start = std::clock();
    B b;
    for (int i = 1; i <= num_elements; ++i)
        b.va.emplace_back(i);
      // std::cout << "making data " << (std::clock() - start) / double(CLOCKS_PER_SEC) << std::endl;

    auto d = json::dump(b, 1);
    std::cout << d << std::endl;
    B b1;
    json::parse(d, b1);
}

// ----------------------------------------------------------------------
