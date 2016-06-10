// -*- C++ -*-
// C++14 json serialising header-only library for C++ classes
// (C) Eugene Skepner 2016

#pragma once

#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>

#include "axe.h"

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wweak-vtables"
#endif

// ----------------------------------------------------------------------

// template <typename T> inline auto json_fields(T&);

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
    }

      // ----------------------------------------------------------------------

    class comment
    {
     public:
        inline comment(std::string a) : m(a) {}
        inline operator std::string() const { return m; }
     private:
        std::string m;
    };

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

        inline iterator s_to_number(iterator i1, iterator i2, int& target) { std::size_t pos = 0; target = std::stoi(std::string(i1, i2), &pos, 0); return i1 + static_cast<std::string::difference_type>(pos); }
        inline iterator s_to_number(iterator i1, iterator i2, long& target) { std::size_t pos = 0; target = std::stol(std::string(i1, i2), &pos, 0); return i1 + static_cast<std::string::difference_type>(pos); }
        inline iterator s_to_number(iterator i1, iterator i2, long long& target) { std::size_t pos = 0; target = std::stoll(std::string(i1, i2), &pos, 0); return i1 + static_cast<std::string::difference_type>(pos); }
        inline iterator s_to_number(iterator i1, iterator i2, unsigned long& target) { std::size_t pos = 0; target = std::stoul(std::string(i1, i2), &pos, 0); return i1 + static_cast<std::string::difference_type>(pos); }
        inline iterator s_to_number(iterator i1, iterator i2, unsigned long long& target) { std::size_t pos = 0; target = std::stoull(std::string(i1, i2), &pos, 0); return i1 + static_cast<std::string::difference_type>(pos); }
        inline iterator s_to_number(iterator i1, iterator i2, float& target) { std::size_t pos = 0; target = std::stof(std::string(i1, i2), &pos); return i1 + static_cast<std::string::difference_type>(pos); }
        inline iterator s_to_number(iterator i1, iterator i2, double& target) { std::size_t pos = 0; target = std::stod(std::string(i1, i2), &pos); return i1 + static_cast<std::string::difference_type>(pos); }
        inline iterator s_to_number(iterator i1, iterator i2, long double& target) { std::size_t pos = 0; target = std::stold(std::string(i1, i2), &pos); return i1 + static_cast<std::string::difference_type>(pos); }

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

        template <typename T> auto parser_value(T& value) -> decltype(s_to_number(iterator(), iterator(), value), parser_number_t<T>(value))
        {
            return parser_number_t<T>(value);
        }

        inline auto parser_value(std::string& target)
        {
            return doublequotes >= (string_content >> target) >= doublequotes;
        }

          // ignored comment field
        inline auto parser_value(comment&)
        {
            return doublequotes >= string_content >= doublequotes;
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
          // forward decalrations

        template <typename T> class parser_object_t;
        template <typename T> auto parser_value(T& value) -> decltype(json_fields(value), parser_object_t<T>(value));
        template <typename T> class parser_array_t;
        template <typename T> auto parser_value(T& value) -> decltype(std::declval<T>().emplace_back(), parser_array_t<T>(value));
        template <typename T> class parser_set_t;
        template <typename T> auto parser_value(std::set<T>& value) -> decltype(parser_set_t<std::set<T>>(value));
        template <typename T> class parser_map_t;
        template <typename T> auto parser_value(std::map<std::string, T>& value) -> decltype(parser_map_t<std::map<std::string, T>>(value));

          // ----------------------------------------------------------------------
          // object -> struct
          // ----------------------------------------------------------------------

        template <typename T> inline auto parser_object_item(const char* key, T& value)
        {
            return skey(key) >= parser_value(value);
        }

        template <typename T> inline auto make_items_parser_tuple(std::tuple<const char*, T*>&& a)
        {
            return parser_object_item(std::get<0>(a), *std::get<1>(a));
        }

        inline auto make_items_parser_tuple(std::tuple<const char*, comment>&& a)
        {
            return parser_object_item(std::get<0>(a), std::get<1>(a));
        }

        template <typename... Ts> inline auto make_items_parser_tuple(std::tuple<Ts...>&& a)
        {
            return parser_object_item(std::get<0>(a), *std::get<1>(a)) | make_items_parser_tuple(u::tuple_tail<2>(a));
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

        template <typename T> auto parser_value(T& value) -> decltype(json_fields(value), parser_object_t<T>(value))
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

        template <typename T> auto parser_value(std::set<T>& value) -> decltype(parser_set_t<std::set<T>>(value))
        {
            return parser_set_t<std::set<T>>(value);
        }

        template <typename T> class parser_array_t : public parser_list_t<T>
        {
         public:
            inline parser_array_t(T& v) : parser_list_t<T>(v) {}
            virtual inline void add() const { this->m.push_back(this->keep); }
        };

        template <typename T> auto parser_value(T& value) -> decltype(std::declval<T>().emplace_back(), parser_array_t<T>(value))
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

        template <typename T> auto parser_value(std::map<std::string, T>& value) -> decltype(parser_map_t<std::map<std::string, T>>(value))
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
        class object_begin {};
        class object_end {};
        class array_begin {};
        class array_end {};
        class comma {};
        class colon {};

        class output
        {
         public:
            inline output() : insert_comma(false) {}
            inline operator std::string () const { return buffer; }
            inline output& operator + (const char* s) { buffer.append(s); return *this; }
            inline output& operator + (char c) { buffer.append(1, c); return *this; }
            inline output& operator + (bool b) { if (b) buffer.append("true"); else buffer.append("false"); return *this; }
            inline output& operator + (object_begin) { buffer.append(1, '{'); insert_comma = false; return *this; }
            inline output& operator + (object_end) { buffer.append(1, '}'); insert_comma = true; return *this; }
            inline output& operator + (array_begin) { buffer.append(1, '['); insert_comma = false; return *this; }
            inline output& operator + (array_end) { buffer.append(1, ']'); insert_comma = true; return *this; }
            inline output& operator + (comma) { if (insert_comma) buffer.append(", "); insert_comma = true; return *this; }
            inline output& operator + (colon) { buffer.append(": "); return *this; }
         private:
            std::string buffer;
            bool insert_comma;
        };

        template <typename T> inline auto operator + (output& o, const T& value) -> decltype(std::to_string(value), std::declval<output&>())
        {
            return o + std::to_string(value).c_str();
        }

        inline output& operator + (output& o, std::string value)
        {
            return o + '"' + value.c_str() + '"';
        }

        inline output& operator + (output& o, comment value)
        {
            return o + static_cast<std::string>(value);
        }

          // ----------------------------------------------------------------------
          // vector, list
          // ----------------------------------------------------------------------

        template <typename T> class array
        {
         public:
            typedef typename T::const_iterator ci;
            inline array(const T& value) : mBegin(std::begin(value)), mEnd(std::end(value)) {}
         private:
            ci mBegin, mEnd;
            friend output& operator + (output& o, const array<T>& value)
                {
                    for (auto i = value.mBegin; i != value.mEnd; ++i)
                        o + comma() + *i;
                    return o;
                }
        };

        template <typename T> inline auto operator + (output& o, const T& value) -> decltype(std::declval<T>().front(), std::declval<output&>())
        {
            return o + array_begin() + array<T>(value) + array_end();
        }

        template <typename T> inline output& operator + (output& o, const std::set<T>& value)
        {
            return o + array_begin() + array<std::set<T>>(value) + array_end();
        }

          // ----------------------------------------------------------------------
          // map
          // ----------------------------------------------------------------------

        template <typename T> class map_writer
        {
         public:
            typedef typename std::map<std::string, T>::const_iterator ci;
            inline map_writer(const std::map<std::string, T>& value) : mBegin(std::begin(value)), mEnd(std::end(value)) {}
         private:
            ci mBegin, mEnd;
            friend output& operator + (output& o, const map_writer<T>& value)
                {
                    for (auto i = value.mBegin; i != value.mEnd; ++i)
                        o + comma() + i->first + colon() + i->second;
                    return o;
                }
        };

        template <typename T> inline output& operator + (output& o, const std::map<std::string, T>& value)
        {
            return o + object_begin() + map_writer<T>(value) + object_end();
        }
          // ----------------------------------------------------------------------

        template <typename T> inline output& operator + (output& o, std::tuple<const char*, T*>&& value)
        {
            return o + comma() + '"' + std::get<0>(value) + '"' + colon() + *std::get<1>(value);
        }

        inline output& operator + (output& o, std::tuple<const char*, comment>&& value)
        {
            return o + comma() + '"' + std::get<0>(value) + '"' + colon() + std::get<1>(value);
        }

        template <typename... Ts> inline output& operator + (output& o, std::tuple<Ts...>&& value)
        {
            return o + std::make_tuple(std::get<0>(value), std::get<1>(value)) + u::tuple_tail<2u>(value);
        }
    }
}

// ----------------------------------------------------------------------

template <typename T> inline auto operator + (json::w::output& o, const T& value) -> decltype(json_fields(std::declval<T&>()), std::declval<json::w::output&>())
{
    return o + json::w::object_begin() + json_fields(const_cast<T&>(value)) + json::w::object_end();
}

template <typename T> inline std::string to_json(const T& a)
{
    auto o = json::w::output();
    return o + a;
}

// ----------------------------------------------------------------------

#ifdef __clang__
#pragma GCC diagnostic pop
#endif

// ----------------------------------------------------------------------
