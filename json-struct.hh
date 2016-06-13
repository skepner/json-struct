// -*- C++ -*-
// C++14 json serialising header-only library for C++ classes
// (C) Eugene Skepner 2016

#pragma once

#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <functional>

#include "axe.h"

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wweak-vtables"
#endif

// ----------------------------------------------------------------------

namespace json
{
    class parsing_error : public std::runtime_error
    {
     public:
        using std::runtime_error::runtime_error;
    };

    namespace u
    {
        template <std::size_t Skip, std::size_t... Ns, typename... Ts> inline auto tuple_tail(std::index_sequence<Ns...>, std::tuple<Ts...>& t)
        {
            return std::make_tuple(std::get<Ns+Skip>(t)...);
        }

        template <std::size_t Skip, typename... Ts> inline auto tuple_tail(std::tuple<Ts...>& t)
        {
            return tuple_tail<Skip>(std::make_index_sequence<sizeof...(Ts) - Skip>(), t);
        }

          // void_t is a C++17 feature
        template<class ...> using void_t = void; // http://stackoverflow.com/questions/26513095/void-t-can-implement-concepts

          // Inspired by https://jguegant.github.io/blogs/tech/sfinae-introduction.html
        template <typename T, typename = void> struct is_json_fields_defined : public std::false_type {};
        template <typename T> struct is_json_fields_defined<T, void_t<decltype(json_fields(std::declval<T&>()))>> : public std::true_type {};

        template <typename T, typename = void> struct is_emplace_back_defined : public std::false_type {};
        template <typename T> struct is_emplace_back_defined<T, void_t<decltype(std::declval<T>().emplace_back())>> : public std::true_type {};
    }

      // ----------------------------------------------------------------------

    class _no_value : public std::exception
    {
     public:
        using std::exception::exception;
    };

    enum output_only_t { output_only };
    enum output_only_if_true_t { output_only_if_true };
    enum output_only_if_not_empty_t { output_only_if_not_empty };
    enum output_if_true_t { output_if_true };
    enum output_if_not_empty_t { output_if_not_empty };

      // ----------------------------------------------------------------------

    template <typename G, typename S, typename P> class field_t
    {
     public:
        typedef typename G::result_type value_type;
        inline field_t(G&& aG, S&& aS, P&& aPredicate) : mG(aG), mS(aS), mPredicate(aPredicate) {}

        inline value_type get() const { if (!mPredicate()) throw _no_value(); return mG(); }
        inline S& setter() { return mS; }

     private:
        G mG;
        S mS;
        P mPredicate;
    };

      // --------------------

    inline bool _predicate_always() { return true; }

    template <typename T> class _predicate_if_true
    {
     public:
        inline _predicate_if_true(const T* aField) : mField(aField) {}
        inline bool operator()() const { return bool(*mField); }
     private:
        const T* mField;
    };

    template <typename T> class _predicate_if_not_empty
    {
     public:
        inline _predicate_if_not_empty(const T* aField) : mField(aField) {}
        inline bool operator()() const { return !mField->empty(); }
     private:
        const T* mField;
    };

      // --------------------

    template <typename T> class _default_getter
    {
     public:
        typedef T result_type;
        inline _default_getter(const T* aField) : mField(aField) {}
        inline const T& operator()() const { return *mField; }
     private:
        const T* mField;
    };

    template <typename T> class _default_setter
    {
     public:
        inline _default_setter(T* aField) : mField(aField) {}
        inline void operator()(const T& val) const { *mField = val; }
     private:
        T* mField;
    };

    template <typename T, typename Arg = T> class _no_setter
    {
     public:
        inline _no_setter(T* = nullptr) {}
        inline void operator()(const Arg&) const {}
    };

    template <typename G, typename S, typename P> inline auto _field_t_make(G&& aG, S&& aS, P&& aPredicate)
    {
        return field_t<G, S, P>(std::forward<G>(aG), std::forward<S>(aS), std::forward<P>(aPredicate));
    }

      // ----------------------------------------------------------------------

      // default getter and setter
    template <typename T> inline auto field(T* field)
    {
        return _field_t_make(_default_getter<T>(field), _default_setter<T>(field), &_predicate_always);
    }

      // default getter and setter, output if true
    template <typename T> inline auto field(T* field, output_if_true_t)
    {
        return _field_t_make(_default_getter<T>(field), _default_setter<T>(field), _predicate_if_true<T>(field));
    }

      // default getter and setter, output if not empty
    template <typename T> inline auto field(T* field, output_if_not_empty_t)
    {
        return _field_t_make(_default_getter<T>(field), _default_setter<T>(field), _predicate_if_not_empty<T>(field));
    }

      // --------------------

      // default getter, no setter - output only
    template <typename T> inline auto field(T* field, output_only_t)
    {
        return _field_t_make(_default_getter<T>(field), _no_setter<T>(field), &_predicate_always);
    }

      // default getter, no setter - output only if true
    template <typename T> inline auto field(T* field, output_only_if_true_t)
    {
        return _field_t_make(_default_getter<T>(field), _no_setter<T>(field), _predicate_if_true<T>(field));
    }

      // default getter, no setter - output only if not empty
    template <typename T> inline auto field(T* field, output_only_if_not_empty_t)
    {
        return _field_t_make(_default_getter<T>(field), _no_setter<T>(field), _predicate_if_not_empty<T>(field));
    }

      // --------------------

      // provided getter, no setter - output only
    template <typename T, typename GF> inline auto field(T* field, GF aGetter)
    {
        typedef decltype(std::bind(aGetter, field)()) value_type;
        return _field_t_make(std::bind(aGetter, field), _no_setter<T, value_type>(field), &_predicate_always);
    }

      // provided getter, no setter - output only if true
    template <typename T, typename GF> inline auto field(T* field, GF aGetter, output_only_if_true_t)
    {
        typedef decltype(std::bind(aGetter, field)()) value_type;
        return _field_t_make(std::bind(aGetter, field), _no_setter<T, value_type>(field), _predicate_if_true<T>(field));
    }

      // same as above, another option type for convenience
    template <typename T, typename GF> inline auto field(T* field, GF aGetter, output_if_true_t)
    {
        return field(field, aGetter, output_only_if_true);
    }

      // provided getter, no setter - output only if not empty
    template <typename T, typename GF> inline auto field(T* field, GF aGetter, output_only_if_not_empty_t)
    {
        typedef decltype(std::bind(aGetter, field)()) value_type;
        return _field_t_make(std::bind(aGetter, field), _no_setter<T, value_type>(field), _predicate_if_not_empty<T>(field));
    }

      // same as above, another option type for convenience
    template <typename T, typename GF> inline auto field(T* field, GF aGetter, output_if_not_empty_t)
    {
        return field(field, aGetter, output_only_if_not_empty);
    }

      // --------------------

      // provided getter and setter
    template <typename T, typename GF, typename SF> inline auto field(T* field, GF aGetter, SF aSetter)
    {
        return _field_t_make(std::bind(aGetter, field), std::bind(aSetter, field, std::placeholders::_1), &_predicate_always);
    }

      // provided getter and setter, output if true
    template <typename T, typename GF, typename SF> inline auto field(T* field, GF aGetter, SF aSetter, output_if_true_t)
    {
        return _field_t_make(std::bind(aGetter, field), std::bind(aSetter, field, std::placeholders::_1), _predicate_if_true<T>(field));
    }

      // provided getter and setter, output if not empty
    template <typename T, typename GF, typename SF> inline auto field(T* field, GF aGetter, SF aSetter, output_if_not_empty_t)
    {
        return _field_t_make(std::bind(aGetter, field), std::bind(aSetter, field, std::placeholders::_1), _predicate_if_not_empty<T>(field));
    }

      // ----------------------------------------------------------------------

    template <typename T> class _const_getter
    {
     public:
        typedef T result_type;
          //inline _const_getter(const T& aData) : m(aData) {}
        inline _const_getter(T&& aData) : m(std::move(aData)) {}
        inline const T& operator()() const { return m; }
     private:
        T m;
    };

    template <typename T> inline auto comment(T&& aComment)
    {
        return _field_t_make(_const_getter<T>(std::forward<T>(aComment)), _no_setter<T>(), &_predicate_always);
    }

    inline auto comment(const char* aComment)
    {
        return _field_t_make(_const_getter<std::string>(std::string(aComment)), _no_setter<std::string>(), &_predicate_always);
    }

      // ----------------------------------------------------------------------

    namespace r
    {
        typedef std::string::iterator iterator;

        class failure : public std::exception
        {
         public:
            virtual ~failure() noexcept {}
            template<class T> failure(T&& aMsg, iterator i1, iterator i2) : msg(std::forward<T>(aMsg)), text_start(i1), text(i1, std::min(i1 + 40, i2)) {}
            failure(const failure&) = default;
            virtual const char* what() const noexcept { return msg.c_str(); }
            std::string message(std::string::iterator buffer_start) const { return msg + " at offset " + std::to_string(text_start - buffer_start) + " when parsing '" + text + "'"; }

         private:
            std::string msg;
            iterator text_start;
            std::string text;
        };

        template<typename R1, typename R2> class r_atomic_t AXE_RULE
        {
          public:
            r_atomic_t(R1&& r1, R2&& r2) : r1_(std::forward<R1>(r1)), r2_(std::forward<R2>(r2)) {}

            inline axe::result<iterator> operator()(iterator i1, iterator i2) const
            {
                auto match = r1_(i1, i2);
                if (match.matched) {   // if r1_ matched r2_ must match too
                    match = r2_(match.position, i2);
                    if (!match.matched)
                        throw failure(std::string("matching failed for [") + axe::get_name(r1_) + "] followed by [" + axe::get_name(r2_) + "]", match.position, i2);
                      // throw failure(std::string("R1 >= R2 rule failed with\n   R1: ") + axe::get_name(r1_) + "\n   R2: " + axe::get_name(r2_), match.position, i2);
                    return axe::make_result(true, match.position);
                }
                return match;
            }

          private:
            R1 r1_;
            R2 r2_;
        };

        template<typename R1, typename R2> inline r_atomic_t<typename std::enable_if<AXE_IS_RULE(R1), R1>::type, typename std::enable_if<AXE_IS_RULE(R2), R2>::type> operator >= (R1&& r1, R2&& r2)
        {
            return r_atomic_t<R1, R2>(std::forward<R1>(r1), std::forward<R2>(r2));
        }

          // ----------------------------------------------------------------------

        const auto space = axe::r_any(" \t\n\r");
        const auto object_begin = axe::r_named(*space & axe::r_lit('{') & *space, "object_begin");
        const auto object_end = axe::r_named(*space & axe::r_lit('}') & *space, "object end");
        const auto array_begin = axe::r_named(*space & axe::r_lit('[') & *space, "array begin");
        const auto array_end = axe::r_named(*space & axe::r_lit(']') & *space, "array end");
        const auto comma = axe::r_named(*space & axe::r_lit(',') & *space, "comma");
        const auto doublequotes = axe::r_named(axe::r_lit('"'), "doublequotes");
        const auto colon = axe::r_named(*space & axe::r_lit(':') & *space, "colon");
        const auto string_content = *("\\\"" | (axe::r_any() - axe::r_any("\"\n\r")));
        inline auto skey(const char* key) { return axe::r_named(axe::r_named(doublequotes & key, "object key") >= doublequotes, "object key + doublequotes") >= colon; };

          // ----------------------------------------------------------------------
          // forward decalrations

        template <typename T> class parser_object_t;
        template <typename T, typename std::enable_if<u::is_json_fields_defined<T>{}>::type* = nullptr> parser_object_t<T> parser_value(T& value);

        template <typename T> class parser_array_t;
          // Note use method for arrays only if there is no json_fields(T&) defined
        template <typename T, typename std::enable_if<u::is_emplace_back_defined<T>{} && !u::is_json_fields_defined<T>{}>::type* = nullptr> parser_array_t<T> parser_value(T& value);

        template <typename T> class parser_set_t;
        template <typename T> auto parser_value(std::set<T>& value);
        template <typename T> class parser_map_t;
        template <typename T> auto parser_value(std::map<std::string, T>& value);

          // ----------------------------------------------------------------------

        inline iterator s_to_number(iterator i1, iterator i2, int& target) { std::size_t pos = 0; target = std::stoi(std::string(i1, i2), &pos, 0); return i1 + static_cast<std::string::difference_type>(pos); }
        inline iterator s_to_number(iterator i1, iterator i2, long& target) { std::size_t pos = 0; target = std::stol(std::string(i1, i2), &pos, 0); return i1 + static_cast<std::string::difference_type>(pos); }
        inline iterator s_to_number(iterator i1, iterator i2, long long& target) { std::size_t pos = 0; target = std::stoll(std::string(i1, i2), &pos, 0); return i1 + static_cast<std::string::difference_type>(pos); }
        inline iterator s_to_number(iterator i1, iterator i2, unsigned long& target) { std::size_t pos = 0; target = std::stoul(std::string(i1, i2), &pos, 0); return i1 + static_cast<std::string::difference_type>(pos); }
        inline iterator s_to_number(iterator i1, iterator i2, unsigned long long& target) { std::size_t pos = 0; target = std::stoull(std::string(i1, i2), &pos, 0); return i1 + static_cast<std::string::difference_type>(pos); }
        inline iterator s_to_number(iterator i1, iterator i2, float& target) { std::size_t pos = 0; target = std::stof(std::string(i1, i2), &pos); return i1 + static_cast<std::string::difference_type>(pos); }
        inline iterator s_to_number(iterator i1, iterator i2, double& target) { std::size_t pos = 0; target = std::stod(std::string(i1, i2), &pos); return i1 + static_cast<std::string::difference_type>(pos); }
        inline iterator s_to_number(iterator i1, iterator i2, long double& target) { std::size_t pos = 0; target = std::stold(std::string(i1, i2), &pos); return i1 + static_cast<std::string::difference_type>(pos); }

        template <typename T, typename = void> struct is_s_to_number_defined : public std::false_type {};
        template <typename T> struct is_s_to_number_defined<T, u::void_t<decltype(s_to_number(iterator(), iterator(), std::declval<T&>()))>> : public std::true_type {};

        template <typename T> class parser_number_t AXE_RULE
        {
          public:
            inline parser_number_t(T& v) : m(v) {}
            inline axe::result<iterator> operator()(iterator i1, iterator i2) const
            {
                try {
                    const auto end = s_to_number(i1, i2, m);
                    return axe::make_result(end > i1, end, i1);
                }
                catch (std::invalid_argument&) {
                    return axe::make_result(false, i1);
                }
                catch (std::out_of_range&) {
                    axe::throw_failure("out of range", i1, i2);
                }
            }
          private:
            T& m;
        };

        template <typename T, typename std::enable_if<is_s_to_number_defined<T>{}>::type* = nullptr> auto parser_value(T& value)
        {
            return parser_number_t<T>(value);
        }

        inline auto parser_value(std::string& target)
        {
            return doublequotes >= (string_content >> target) >= doublequotes;
        }

        class parser_bool_t AXE_RULE
        {
          public:
            inline parser_bool_t(bool& v) : m(v) {}
            inline axe::result<iterator> operator()(iterator i1, iterator i2) const
            {
                auto yes = axe::e_ref([this](auto, auto) { m = true; });
                auto no = axe::e_ref([this](auto, auto) { m = false; });
                return ((axe::r_str("true") >> yes) | (axe::r_str("1") >> yes) | (axe::r_str("false") >> no) | (axe::r_str("0") >> no) | axe::r_fail("true or 1 or false or 0 expected"))(i1, i2);
            }
          private:
            bool& m;
        };

        inline auto parser_value(bool& target)
        {
            return parser_bool_t(target);
        }

          // ----------------------------------------------------------------------
          // field_t
          // ----------------------------------------------------------------------

        template <typename S, typename V> class parser_field_t AXE_RULE
        {
          public:
            inline parser_field_t(S& aS) : mS(aS) {}
            inline axe::result<iterator> operator()(iterator i1, iterator i2) const
            {
                V v;
                auto r = parser_value(v)(i1, i2);
                mS(v);
                return r;
            }
          private:
            S mS;
        };

        template <typename G, typename S, typename P> inline auto parser_value(field_t<G, S, P>& a)
        {
            typedef typename field_t<G, S, P>::value_type value_type;
            return parser_field_t<S, value_type>(a.setter());
        }

          // ----------------------------------------------------------------------
          // object -> struct
          // ----------------------------------------------------------------------

        template <typename T> inline auto parser_object_item(const char* key, T& value)
        {
            return skey(key) >= parser_value(value);
        }

        template <typename T> inline auto parser_object_item(const char* key, T* value)
        {
            return skey(key) >= parser_value(*value);
        }

        template <typename T> inline auto make_items_parser_tuple(std::tuple<const char*, T*>&& a)
        {
            return parser_object_item(std::get<0>(a), std::get<1>(a)); // *
        }

          // Necessary for tuple<const char *, json::field_t<>>
        template <typename T> inline auto make_items_parser_tuple(std::tuple<const char*, T>&& a)
        {
            return parser_object_item(std::get<0>(a), std::get<1>(a));
        }

        template <typename... Ts> inline auto make_items_parser_tuple(std::tuple<Ts...>&& a)
        {
            return parser_object_item(std::get<0>(a), std::get<1>(a)) | make_items_parser_tuple(u::tuple_tail<2>(a));
        }

        template <typename T> class parser_object_t AXE_RULE
        {
          public:
            inline parser_object_t(T& v) : m(v) {}
            inline axe::result<iterator> operator()(iterator i1, iterator i2) const
            {
                auto item = make_items_parser_tuple(json_fields(m));
                return (object_begin >= ~( item & *(comma >= item) ) >= object_end)(i1, i2);
            }
          private:
            T& m;
        };

        template <typename T, typename std::enable_if<u::is_json_fields_defined<T>{}>::type*> parser_object_t<T> parser_value(T& value)
        {
            return parser_object_t<T>(value);
        }

          // ----------------------------------------------------------------------
          // array -> vector, list, set
          // ----------------------------------------------------------------------

        template <typename T> class parser_list_t AXE_RULE
        {
          public:
            inline parser_list_t(T& v) : m(v) {}
            inline parser_list_t(const parser_list_t<T>& v) = default;
            virtual inline ~parser_list_t() = default;
            virtual void add() const = 0;
            inline axe::result<iterator> operator()(iterator i1, iterator i2) const
            {
                auto clear_target = axe::e_ref([this](auto, auto) { m.clear(); });
                auto insert_item = axe::e_ref([this](auto, auto) { add(); });
                auto item = parser_value(keep) >> insert_item;
                return ((array_begin >> clear_target) >= ~( item & *(comma >= item) ) >= array_end)(i1, i2);
            }
          protected:
            T& m;
            mutable typename T::value_type keep;
        };

        template <typename T> class parser_set_t : public parser_list_t<T>
        {
         public:
            inline parser_set_t(T& v) : parser_list_t<T>(v) {}
            virtual inline void add() const { this->m.insert(this->keep); }
        };

        template <typename T> auto parser_value(std::set<T>& value)
        {
            return parser_set_t<std::set<T>>(value);
        }

        template <typename T> class parser_array_t : public parser_list_t<T>
        {
         public:
            inline parser_array_t(T& v) : parser_list_t<T>(v) {}
            virtual inline void add() const { this->m.push_back(this->keep); }
        };

          // Note use method for arrays only if there is no json_fields(T&) defined
        template <typename T, typename std::enable_if<u::is_emplace_back_defined<T>{} && !u::is_json_fields_defined<T>{}>::type*> parser_array_t<T> parser_value(T& value)
        {
            return parser_array_t<T>(value);
        }

          // ----------------------------------------------------------------------
          // object -> map<string, T>
          // ----------------------------------------------------------------------

        template <typename T> inline auto parser_map_item(std::string& key, T& value)
        {
            return doublequotes >= (string_content >> key) >= doublequotes >= colon >= parser_value(value);
        }

        template <typename T> class parser_map_t AXE_RULE
        {
          public:
            inline parser_map_t(T& v) : m(v) {}
            inline axe::result<iterator> operator()(iterator i1, iterator i2) const
            {
                auto clear_target = axe::e_ref([this](auto, auto) { m.clear(); });
                auto insert_item = axe::e_ref([this](auto, auto) { return m.insert(std::make_pair(keep_key, keep_value)); });
                auto item = parser_map_item(keep_key, keep_value) >> insert_item;
                return ((object_begin >> clear_target) >= ~( item & *(comma >= item) ) >= object_end)(i1, i2);
            }
          private:
            T& m;
            mutable std::string keep_key;
            mutable typename T::mapped_type keep_value;
        };

        template <typename T> auto parser_value(std::map<std::string, T>& value)
        {
            return parser_map_t<std::map<std::string, T>>(value);
        }
    }

          // ----------------------------------------------------------------------

    template <typename T> inline void parse(std::string source, T& target)
    {
        auto parser = r::parser_value(target);
        try {
            parser(std::begin(source), std::end(source));
        }
        catch (r::failure& err) {
            throw parsing_error(err.message(std::begin(source)));
        }
        catch (axe::failure<char>& err) {
            throw parsing_error(err.message());
        }
    }

      // ----------------------------------------------------------------------

    namespace w
    {
          // ---- value_to_string ------------------------------------------------------------------

        template <typename T> inline typename std::enable_if<std::is_floating_point<T>::value, std::string>::type value_to_string(T val)
        {
            std::ostringstream s;
            s << std::setprecision(std::numeric_limits<T>::max_digits10) << val;
            return s.str();
        }

        template <typename T> inline typename std::enable_if<std::is_integral<T>::value, std::string>::type value_to_string(T val)
        {
            return std::to_string(val);
        }

        inline std::string value_to_string(std::string val)
        {
            return '"' + val + '"';
        }

        inline std::string value_to_string(bool val)
        {
            return val ? "true" : "false";
        }

          // ----------------------------------------------------------------------

        class output
        {
         protected:
            std::string buffer;
            bool insert_comma;

            inline void comma(bool ic) { if (insert_comma) add_comma(); insert_comma = ic; }
            virtual inline void add_comma() { buffer.append(1, ','); }
            virtual inline void indent_simple() = 0;
            virtual inline void indent_extend() = 0;
            virtual inline void indent_reduce() = 0;
            virtual inline void no_indent() = 0;

         public:
            inline output() : insert_comma(false) {}
            inline output(const output&) = default;
            inline virtual ~output() = default;
            inline operator std::string () const { return buffer; }

            inline output& open(char c) { comma(false); indent_extend(); buffer.append(1, c); return *this; }
            inline output& close(char c) { insert_comma = false; comma(true); indent_reduce(); buffer.append(1, c); return *this; }

            inline output& append(const char* val) { buffer.append(1, '"'); buffer.append(val); buffer.append(1, '"'); return *this; }

            template <typename T, typename std::enable_if<std::is_integral<T>{} || std::is_floating_point<T>{} || std::is_convertible<T*, std::string*>{} || std::is_convertible<T*, bool*>{}>::type* = nullptr> inline output& append(T val)
                {
                    comma(true);
                    indent_simple();
                    buffer.append(value_to_string(val));
                    return *this;
                }

            template <typename T> inline output& append(const char* key, const T& val)
                {
                    comma(false);
                    indent_simple();
                    append(key);
                    buffer.append(": ");
                    no_indent();
                    append(val);
                    insert_comma = true;
                    return *this;
                }

              // ---- struct ------------------------------------------------------------------

            template <typename T> inline output& append(std::tuple<const char*, T*>&& val)
                {
                    return append(std::get<0>(val), *std::get<1>(val));
                }

            template <typename G, typename S, typename P> inline output& append(std::tuple<const char*, field_t<G, S, P>>&& val)
                {
                    return append(std::get<0>(val), std::get<1>(val));
                }

            template <typename... Ts> inline output& append(std::tuple<Ts...>&& val)
                {
                    append(std::make_tuple(std::get<0>(val), std::get<1>(val)));
                    return append(u::tuple_tail<2u>(val));
                }

            template <typename T, typename std::enable_if<u::is_json_fields_defined<T>{}>::type* = nullptr> inline auto append(const T& val)
                {
                    open('{');
                    append(json_fields(const_cast<T&>(val)));
                    return close('}');
                }

              // ---- vector, list, set ------------------------------------------------------------------

         private:
            template <typename T> class warray
            {
             public:
                inline warray(const T& val) : mValue(val) {}
                inline output& append_to(output& o)
                    {
                        o.open('[');
                        if (!mValue.empty()) {
                            for (auto& i: mValue) {
                                o.append(i);
                            }
                        }
                        else {
                            o.no_indent();
                        }
                        return o.close(']');
                    }
             private:
                const T& mValue;
            };

         public:
            template <typename T> inline output& append(const std::set<T>& val)
                {
                    return warray<std::set<T>>(val).append_to(*this);
                }

            template <typename T> inline output& append(const std::vector<T>& val)
                {
                    return warray<std::vector<T>>(val).append_to(*this);
                }

            template <typename T> inline output& append(const std::list<T>& val)
                {
                    return warray<std::list<T>>(val).append_to(*this);
                }

              // ---- map ------------------------------------------------------------------

         private:
            template <typename T> class wmap
            {
             public:
                inline wmap(const std::map<std::string, T>& val) : mValue(val) {}
                inline output& append_to(output& o)
                    {
                        o.open('{');
                        if (!mValue.empty()) {
                            for (auto& i: mValue) {
                                o.append(i.first.c_str(), i.second);
                            }
                        }
                        else {
                            o.no_indent();
                        }
                        return o.close('}');
                    }
             private:
                const std::map<std::string, T>& mValue;
            };

         public:
            template <typename T> inline output& append(const std::map<std::string, T>& val)
                {
                    return wmap<T>(val).append_to(*this);
                }

              // ---- field_t<G, S, P> ------------------------------------------------------------------

            template <typename G, typename S, typename P> inline output& append(const char* key, const field_t<G, S, P>& val)
                {
                    try {
                        return append(key, val.get()); // val.get() may throw _no_value
                    }
                    catch (_no_value&) {
                        return *this; // avoid writing key/value pair
                    }
                }
        };

          // ----------------------------------------------------------------------

        class output_compact : public output
        {
         public:
            inline output_compact() : output(), insert_space(false) {}

         protected:
            bool insert_space;

            virtual inline void indent_simple() { if (insert_space) buffer.append(1, ' '); insert_space = true; }
            virtual inline void indent_extend() { insert_space = false; }
            virtual inline void indent_reduce() { insert_space = true; }
            virtual inline void no_indent() { insert_space = false; }
        };

          // ----------------------------------------------------------------------

        class output_pretty : public output
        {
         public:
            inline output_pretty(size_t aIndent) : output(), indent(aIndent), prefix(1, '\n'), insert_prefix(false) {}

         protected:
            virtual inline void indent_simple() { if (insert_prefix) buffer.append(prefix); insert_prefix = true; }
            virtual inline void indent_extend() { indent_simple(); prefix.append(indent, ' '); }
            virtual inline void indent_reduce() { prefix.erase(prefix.size() - indent); indent_simple(); }
            virtual inline void no_indent() { insert_prefix = false; }

         private:
            size_t indent;
            std::string prefix;
            bool insert_prefix;
        };
    }

      // ----------------------------------------------------------------------

    template <typename T> inline std::string dump(const T& a, int indent = 0)
    {
        if (indent <= 0) {
            auto o = json::w::output_compact();
            return o.append(a);
        }
        else {
            auto o = json::w::output_pretty(static_cast<size_t>(indent));
            return o.append(a);
        }
    }
}

// ----------------------------------------------------------------------

#ifdef __clang__
#pragma GCC diagnostic pop
#endif

// ----------------------------------------------------------------------
