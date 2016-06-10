# json-struct
C++14 json serialising header-only library for C++ classes

## Dependencies:

[AXE](https://github.com/skepner/axe)

## Usage

For a class/struct to become serializable to json define non-member
function json_fields(T&) accepting non-const(!) reference to that
class and returning std::tuple listing field name (as a literal
string) followe by the pointer to the field, e.g.

    class A
    {
     public:
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
        return std::make_tuple(
            "vi", &a.vi,
            "msi", &a.msi,
            "ld", &a.ld,
            "ss", &a.ss,
            "i", &a.i,
            "f", &a.f,
            "s", &a.s,
            "b", &a.b,
            "?", json::comment("ignored comment field"));
    }

Special tuple value json::comment(<literal string>) is for fields
appearing in the output, but ignoring by the parser. Note just strings
are supported.

Then instance of the class can be serialized to json using

    std::string dump = json::dump(a);

or deserialized from a json string using

    json::parse(dump, a);

Nested classes are supported, json_fields() must be provided for all
classes, e.g.

    struct A
    {
        int i;
    };

    struct B
    {
        float f;
        A a;
        std::vector<A> va;
    };

    inline auto json_fields(A& a)
    {
        return std::make_tuple("i", &a.i);
    }

    inline auto json_fields(B& b)
    {
        return std::make_tuple("f", &b.f, "a", &b.a, "va", &b.va);
    }

## TODO

- recursive classes (e.g. tree)
- pretty json output
