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

## Options for field serialization

### Output only

Field is serialized but never imported on deserialization, its value in json ignored.

    struct A
    {
        int i;
    };

    inline auto json_fields(A& a)
    {
        return std::make_tuple("i", json::field(&a.i, json::output_only));
    }

### Output only if true

Field is serialized only if it has operator bool defined (otherwise -
compilation error) and it returns true. Field is never imported on
deserialization, its value in json ignored.

    struct A
    {
        bool b;
    };

    inline auto json_fields(A& a)
    {
        return std::make_tuple("b", json::field(&a.b, json::output_only_if_true));
    }


### Output only if not empty

Field is serialized only if it has empty member function defined
(e.g. std::vector, otherwise - compilation error) and it returns
false. Field is never imported on deserialization, its value in json
ignored.

    struct A
    {
        bool b;
    };

    inline auto json_fields(A& a)
    {
        return std::make_tuple("b", json::field(&a.b, json::output_only_if_not_empty));
    }


### Output if true

Field is serialized only if it has operator bool defined (otherwise -
compilation error) and it returns true. Field is imported in a regular way.

    struct A
    {
        bool b;
    };

    inline auto json_fields(A& a)
    {
        return std::make_tuple("b", json::field(&a.b, json::output_if_true));
    }


### Output if not empty

Field is serialized only if it has empty member function defined
(e.g. std::vector, otherwise - compilation error) and it returns
false. Field is imported in a regular way.

    struct A
    {
        bool b;
    };

    inline auto json_fields(A& a)
    {
        return std::make_tuple("b", json::field(&a.b, json::output_if_not_empty));
    }


### Field with getter and setter

If a field cannot be serialized/deserialized directly but provides
getter (for serialization) and setter (for deserialization) member
functions instead.

    struct F
    {
        std::string getter() { return "value-of-F"; }
        void setter(std::string) { /* e.g. value ignored */ }
    }

    struct A
    {
        F f;
    };

    inline auto json_fields(A& a)
    {
        return std::make_tuple("f", json::field(&a.f, &F::getter, &F::setter));
    }

json::output_if_true or json::output_if_not_empty can be appended to
the json::field call if necessary:

    json::field(&a.f, &F::getter, &F::setter, json::output_if_true)
    json::field(&a.f, &F::getter, &F::setter, json::output_if_not_empty)

### Field with just getter

If a field cannot be serialized directly but provides getter. Field is
never imported on deserialization, its value in json ignored.

    struct F
    {
        std::string getter() { return "value-of-F"; }
    }

    struct A
    {
        F f;
    };

    inline auto json_fields(A& a)
    {
        return std::make_tuple("f", json::field(&a.f, &F::getter));
    }

json::output_if_true or json::output_if_not_empty can be appended to
the json::field call if necessary:

    json::field(&a.f, &F::getter, json::output_if_true)
    json::field(&a.f, &F::getter, json::output_if_not_empty)

# TODO

- sorting keys on parsing by their length: longer first
