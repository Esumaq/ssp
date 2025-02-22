#include "test_helpers.hpp"
#include <algorithm>
#include <ss/converter.hpp>

TEST_CASE("converter test split") {
    ss::converter c;
    for (const auto& [s, expected, delim] :
         // clang-format off
                {std::make_tuple("a,b,c,d", std::vector{"a", "b", "c", "d"}, ","),
                {"", {}, " "},
                {" x x x x | x ", {" x x x x ", " x "}, "|"},
                {"a::b::c::d", {"a", "b", "c", "d"}, "::"},
                {"x\t-\ty", {"x", "y"}, "\t-\t"},
                {"x", {"x"}, ","}} // clang-format on
    ) {
        auto split = c.split(s, delim);
        CHECK_EQ(split.size(), expected.size());
        for (size_t i = 0; i < split.size(); ++i) {
            auto s = std::string(split[i].first, split[i].second);
            CHECK_EQ(s, expected[i]);
        }
    }
}

TEST_CASE("converter test split with exceptions") {
    ss::converter<ss::throw_on_error> c;
    try {
        for (const auto& [s, expected, delim] :
             // clang-format off
                {std::make_tuple("a,b,c,d", std::vector{"a", "b", "c", "d"}, ","),
                {"", {}, " "},
                {" x x x x | x ", {" x x x x ", " x "}, "|"},
                {"a::b::c::d", {"a", "b", "c", "d"}, "::"},
                {"x\t-\ty", {"x", "y"}, "\t-\t"},
                {"x", {"x"}, ","}} // clang-format on
        ) {
            auto split = c.split(s, delim);
            CHECK_EQ(split.size(), expected.size());
            for (size_t i = 0; i < split.size(); ++i) {
                auto s = std::string(split[i].first, split[i].second);
                CHECK_EQ(s, expected[i]);
            }
        }
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }
}

TEST_CASE("converter test valid conversions") {
    ss::converter c;

    {
        auto tup = c.convert<int>("5");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 5);
    }
    {
        auto tup = c.convert<int, void>("5,junk");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 5);
    }
    {
        auto tup = c.convert<void, int>("junk,5");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 5);
    }
    {
        auto tup = c.convert<int, void, void>("5\njunk\njunk", "\n");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 5);
    }
    {
        auto tup = c.convert<void, int, void>("junk 5 junk", " ");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 5);
    }
    {
        auto tup = c.convert<void, void, int>("junk\tjunk\t5", "\t");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 5);
    }
    {
        auto tup =
            c.convert<void, void, std::optional<int>>("junk\tjunk\t5", "\t");
        REQUIRE(c.valid());
        REQUIRE(tup.has_value());
        CHECK_EQ(tup, 5);
    }
    {
        auto tup = c.convert<int, double, void>("5,6.6,junk");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple(5, 6.6));
    }
    {
        auto tup = c.convert<int, void, double>("5,junk,6.6");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple(5, 6.6));
    }
    {
        auto tup = c.convert<void, int, double>("junk;5;6.6", ";");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple(5, 6.6));
    }
    {
        auto tup =
            c.convert<void, std::optional<int>, double>("junk;5;6.6", ";");
        REQUIRE(c.valid());
        REQUIRE(std::get<0>(tup).has_value());
        CHECK_EQ(tup, std::make_tuple(5, 6.6));
    }
    {
        auto tup =
            c.convert<void, std::optional<int>, double>("junk;5.4;6.6", ";");
        REQUIRE(c.valid());
        REQUIRE_FALSE(std::get<0>(tup).has_value());
        CHECK_EQ(tup, std::make_tuple(std::optional<int>{}, 6.6));
    }
    {
        auto tup =
            c.convert<void, std::variant<int, double>, double>("junk;5;6.6",
                                                               ";");
        REQUIRE(c.valid());
        REQUIRE(std::holds_alternative<int>(std::get<0>(tup)));
        CHECK_EQ(tup, std::make_tuple(std::variant<int, double>{5}, 6.6));
    }
    {
        auto tup =
            c.convert<void, std::variant<int, double>, double>("junk;5.5;6.6",
                                                               ";");
        REQUIRE(c.valid());
        REQUIRE(std::holds_alternative<double>(std::get<0>(tup)));
        CHECK_EQ(tup, std::make_tuple(std::variant<int, double>{5.5}, 6.6));
    }
    {
        auto tup = c.convert<void, std::string_view, double,
                             std::string_view>("junk;s1;6.6;s2", ";");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple(std::string_view{"s1"}, 6.6,
                                      std::string_view{"s2"}));
    }
}

TEST_CASE("converter test valid conversions with exceptions") {
    ss::converter<ss::throw_on_error> c;

    try {
        auto tup = c.convert<int>("5");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 5);
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        auto tup = c.convert<int, void>("5,junk");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 5);
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        auto tup = c.convert<void, int>("junk,5");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 5);
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        auto tup = c.convert<int, void, void>("5\njunk\njunk", "\n");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 5);
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        auto tup = c.convert<void, int, void>("junk 5 junk", " ");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 5);
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        auto tup = c.convert<void, void, int>("junk\tjunk\t5", "\t");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 5);
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        auto tup =
            c.convert<void, void, std::optional<int>>("junk\tjunk\t5", "\t");
        REQUIRE(c.valid());
        REQUIRE(tup.has_value());
        CHECK_EQ(tup, 5);
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        auto tup = c.convert<int, double, void>("5,6.6,junk");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple(5, 6.6));
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        auto tup = c.convert<int, void, double>("5,junk,6.6");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple(5, 6.6));
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        auto tup = c.convert<void, int, double>("junk;5;6.6", ";");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple(5, 6.6));
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        auto tup =
            c.convert<void, std::optional<int>, double>("junk;5;6.6", ";");
        REQUIRE(c.valid());
        REQUIRE(std::get<0>(tup).has_value());
        CHECK_EQ(tup, std::make_tuple(5, 6.6));
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        auto tup =
            c.convert<void, std::optional<int>, double>("junk;5.4;6.6", ";");
        REQUIRE(c.valid());
        REQUIRE_FALSE(std::get<0>(tup).has_value());
        CHECK_EQ(tup, std::make_tuple(std::optional<int>{}, 6.6));
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        auto tup =
            c.convert<void, std::variant<int, double>, double>("junk;5;6.6",
                                                               ";");
        REQUIRE(c.valid());
        REQUIRE(std::holds_alternative<int>(std::get<0>(tup)));
        CHECK_EQ(tup, std::make_tuple(std::variant<int, double>{5}, 6.6));
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        auto tup =
            c.convert<void, std::variant<int, double>, double>("junk;5.5;6.6",
                                                               ";");
        REQUIRE(c.valid());
        REQUIRE(std::holds_alternative<double>(std::get<0>(tup)));
        CHECK_EQ(tup, std::make_tuple(std::variant<int, double>{5.5}, 6.6));
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        auto tup = c.convert<void, std::string_view, double,
                             std::string_view>("junk;s1;6.6;s2", ";");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple(std::string_view{"s1"}, 6.6,
                                      std::string_view{"s2"}));
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }
}

TEST_CASE("converter test invalid conversions") {
    ss::converter c;

    c.convert<int>("");
    REQUIRE_FALSE(c.valid());

    c.convert<int>("1", "");
    REQUIRE_FALSE(c.valid());

    c.convert<int>("10", "");
    REQUIRE_FALSE(c.valid());

    c.convert<int, void>("");
    REQUIRE_FALSE(c.valid());

    c.convert<int, void>(",junk");
    REQUIRE_FALSE(c.valid());

    c.convert<void, int>("junk,");
    REQUIRE_FALSE(c.valid());

    c.convert<int>("x");
    REQUIRE_FALSE(c.valid());

    c.convert<int, void>("x");
    REQUIRE_FALSE(c.valid());

    c.convert<int, void>("x,junk");
    REQUIRE_FALSE(c.valid());

    c.convert<void, int>("junk,x");
    REQUIRE_FALSE(c.valid());

    c.convert<void, std::variant<int, double>, double>("junk;.5.5;6", ";");
    REQUIRE_FALSE(c.valid());
}

TEST_CASE("converter test invalid conversions with exceptions") {
    ss::converter<ss::throw_on_error> c;

    REQUIRE_EXCEPTION(c.convert<int>(""));
    REQUIRE_EXCEPTION(c.convert<int>("1", ""));
    REQUIRE_EXCEPTION(c.convert<int>("10", ""));
    REQUIRE_EXCEPTION(c.convert<int, void>(""));
    REQUIRE_EXCEPTION(c.convert<int, void>(",junk"));
    REQUIRE_EXCEPTION(c.convert<void, int>("junk,"));
    REQUIRE_EXCEPTION(c.convert<int>("x"));
    REQUIRE_EXCEPTION(c.convert<int, void>("x"));
    REQUIRE_EXCEPTION(c.convert<int, void>("x,junk"));
    REQUIRE_EXCEPTION(c.convert<void, int>("junk,x"));
    REQUIRE_EXCEPTION(
        c.convert<void, std::variant<int, double>, double>("junk;.5.5;6", ";"));
}

TEST_CASE("converter test ss:ax restriction (all except)") {
    ss::converter c;

    c.convert<ss::ax<int, 0>>("0");
    REQUIRE_FALSE(c.valid());

    c.convert<ss::ax<int, 0, 1, 2>>("1");
    REQUIRE_FALSE(c.valid());

    c.convert<void, char, ss::ax<int, 0, 1, 2>>("junk,c,1");
    REQUIRE_FALSE(c.valid());

    c.convert<ss::ax<int, 1>, char>("1,c");
    REQUIRE_FALSE(c.valid());
    {
        int tup = c.convert<ss::ax<int, 1>>("3");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 3);
    }
    {
        std::tuple<char, int> tup = c.convert<char, ss::ax<int, 1>>("c,3");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple('c', 3));
    }
    {
        std::tuple<int, char> tup = c.convert<ss::ax<int, 1>, char>("3,c");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple(3, 'c'));
    }
}

TEST_CASE("converter test ss:ax restriction (all except) with exceptions") {
    ss::converter<ss::throw_on_error> c;

    REQUIRE_EXCEPTION(c.convert<ss::ax<int, 0>>("0"));
    REQUIRE_EXCEPTION(c.convert<ss::ax<int, 0, 1, 2>>("1"));
    REQUIRE_EXCEPTION(c.convert<void, char, ss::ax<int, 0, 1, 2>>("junk,c,1"));
    REQUIRE_EXCEPTION(c.convert<ss::ax<int, 1>, char>("1,c"));

    try {
        {
            int tup = c.convert<ss::ax<int, 1>>("3");
            CHECK_EQ(tup, 3);
        }
        {
            std::tuple<char, int> tup = c.convert<char, ss::ax<int, 1>>("c,3");
            CHECK_EQ(tup, std::make_tuple('c', 3));
        }
        {
            std::tuple<int, char> tup = c.convert<ss::ax<int, 1>, char>("3,c");
            CHECK_EQ(tup, std::make_tuple(3, 'c'));
        }
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }
}

TEST_CASE("converter test ss:nx restriction (none except)") {
    ss::converter c;

    c.convert<ss::nx<int, 1>>("3");
    REQUIRE_FALSE(c.valid());

    c.convert<char, ss::nx<int, 1, 2, 69>>("c,3");
    REQUIRE_FALSE(c.valid());

    c.convert<ss::nx<int, 1>, char>("3,c");
    REQUIRE_FALSE(c.valid());

    {
        auto tup = c.convert<ss::nx<int, 3>>("3");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 3);
    }
    {
        auto tup = c.convert<ss::nx<int, 0, 1, 2>>("2");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 2);
    }
    {
        auto tup = c.convert<char, void, ss::nx<int, 0, 1, 2>>("c,junk,1");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple('c', 1));
    }
    {
        auto tup = c.convert<ss::nx<int, 1>, char>("1,c");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple(1, 'c'));
    }
}

TEST_CASE("converter test ss:nx restriction (none except) with exceptions") {
    ss::converter<ss::throw_on_error> c;

    REQUIRE_EXCEPTION(c.convert<ss::nx<int, 1>>("3"));
    REQUIRE_EXCEPTION(c.convert<char, ss::nx<int, 1, 2, 69>>("c,3"));
    REQUIRE_EXCEPTION(c.convert<ss::nx<int, 1>, char>("3,c"));

    try {
        {
            auto tup = c.convert<ss::nx<int, 3>>("3");
            REQUIRE(c.valid());
            CHECK_EQ(tup, 3);
        }
        {
            auto tup = c.convert<ss::nx<int, 0, 1, 2>>("2");
            REQUIRE(c.valid());
            CHECK_EQ(tup, 2);
        }
        {
            auto tup = c.convert<char, void, ss::nx<int, 0, 1, 2>>("c,junk,1");
            REQUIRE(c.valid());
            CHECK_EQ(tup, std::make_tuple('c', 1));
        }
        {
            auto tup = c.convert<ss::nx<int, 1>, char>("1,c");
            REQUIRE(c.valid());
            CHECK_EQ(tup, std::make_tuple(1, 'c'));
        }
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }
}

TEST_CASE("converter test ss:ir restriction (in range)") {
    ss::converter c;

    c.convert<ss::ir<int, 0, 2>>("3");
    REQUIRE_FALSE(c.valid());

    c.convert<char, ss::ir<int, 4, 69>>("c,3");
    REQUIRE_FALSE(c.valid());

    c.convert<ss::ir<int, 1, 2>, char>("3,c");
    REQUIRE_FALSE(c.valid());

    {
        auto tup = c.convert<ss::ir<int, 1, 5>>("3");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 3);
    }
    {
        auto tup = c.convert<ss::ir<int, 0, 2>>("2");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 2);
    }
    {
        auto tup = c.convert<char, void, ss::ir<int, 0, 1>>("c,junk,1");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple('c', 1));
    }
    {
        auto tup = c.convert<ss::ir<int, 1, 20>, char>("1,c");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple(1, 'c'));
    }
}

TEST_CASE("converter test ss:ir restriction (in range) with exceptions") {
    ss::converter<ss::throw_on_error> c;

    REQUIRE_EXCEPTION(c.convert<ss::ir<int, 0, 2>>("3"));
    REQUIRE_EXCEPTION(c.convert<char, ss::ir<int, 4, 69>>("c,3"));
    REQUIRE_EXCEPTION(c.convert<ss::ir<int, 1, 2>, char>("3,c"));

    try {
        {
            auto tup = c.convert<ss::ir<int, 1, 5>>("3");
            REQUIRE(c.valid());
            CHECK_EQ(tup, 3);
        }
        {
            auto tup = c.convert<ss::ir<int, 0, 2>>("2");
            REQUIRE(c.valid());
            CHECK_EQ(tup, 2);
        }
        {
            auto tup = c.convert<char, void, ss::ir<int, 0, 1>>("c,junk,1");
            REQUIRE(c.valid());
            CHECK_EQ(tup, std::make_tuple('c', 1));
        }
        {
            auto tup = c.convert<ss::ir<int, 1, 20>, char>("1,c");
            REQUIRE(c.valid());
            CHECK_EQ(tup, std::make_tuple(1, 'c'));
        }
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }
}

TEST_CASE("converter test ss:oor restriction (out of range)") {
    ss::converter c;

    c.convert<ss::oor<int, 1, 5>>("3");
    REQUIRE_FALSE(c.valid());

    c.convert<ss::oor<int, 0, 2>>("2");
    REQUIRE_FALSE(c.valid());

    c.convert<char, ss::oor<int, 0, 1>, void>("c,1,junk");
    REQUIRE_FALSE(c.valid());

    c.convert<ss::oor<int, 1, 20>, char>("1,c");
    REQUIRE_FALSE(c.valid());

    {
        auto tup = c.convert<ss::oor<int, 0, 2>>("3");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 3);
    }

    {
        auto tup = c.convert<char, void, ss::oor<int, 4, 69>>("c,junk,3");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple('c', 3));
    }

    {
        auto tup = c.convert<ss::oor<int, 1, 2>, char>("3,c");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple(3, 'c'));
    }
}

TEST_CASE("converter test ss:oor restriction (out of range) with exceptions") {
    ss::converter<ss::throw_on_error> c;

    REQUIRE_EXCEPTION(c.convert<ss::oor<int, 1, 5>>("3"));
    REQUIRE_EXCEPTION(c.convert<ss::oor<int, 0, 2>>("2"));
    REQUIRE_EXCEPTION(c.convert<char, ss::oor<int, 0, 1>, void>("c,1,junk"));
    REQUIRE_EXCEPTION(c.convert<ss::oor<int, 1, 20>, char>("1,c"));

    try {
        {
            auto tup = c.convert<ss::oor<int, 0, 2>>("3");
            REQUIRE(c.valid());
            CHECK_EQ(tup, 3);
        }

        {
            auto tup = c.convert<char, void, ss::oor<int, 4, 69>>("c,junk,3");
            REQUIRE(c.valid());
            CHECK_EQ(tup, std::make_tuple('c', 3));
        }

        {
            auto tup = c.convert<ss::oor<int, 1, 2>, char>("3,c");
            REQUIRE(c.valid());
            CHECK_EQ(tup, std::make_tuple(3, 'c'));
        }
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }
}

const inline std::vector<int> extracted_vector = {1, 2, 3};

// custom extract
template <>
inline bool ss::extract(const char* begin, const char* end,
                        std::vector<int>& value) {
    if (begin == end) {
        return false;
    }
    value = extracted_vector;
    return true;
}

TEST_CASE("converter test ss:ne restriction (not empty)") {
    ss::converter c;

    c.convert<ss::ne<std::string>>("");
    REQUIRE_FALSE(c.valid());

    c.convert<int, ss::ne<std::string>>("3,");
    REQUIRE_FALSE(c.valid());

    c.convert<ss::ne<std::string>, int>(",3");
    REQUIRE_FALSE(c.valid());

    c.convert<void, ss::ne<std::string>, int>("junk,,3");
    REQUIRE_FALSE(c.valid());

    c.convert<ss::ne<std::vector<int>>>("");
    REQUIRE_FALSE(c.valid());

    {
        auto tup = c.convert<ss::ne<std::string>>("s");
        REQUIRE(c.valid());
        CHECK_EQ(tup, "s");
    }
    {
        auto tup = c.convert<std::optional<int>, ss::ne<std::string>>("1,s");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple(1, "s"));
    }
    {
        auto tup = c.convert<ss::ne<std::vector<int>>>("{1 2 3}");
        REQUIRE(c.valid());
        CHECK_EQ(tup, extracted_vector);
    }
}

TEST_CASE("converter test ss:ne restriction (not empty) with exceptions") {
    ss::converter<ss::throw_on_error> c;

    REQUIRE_EXCEPTION(c.convert<ss::ne<std::string>>(""));
    REQUIRE_EXCEPTION(c.convert<int, ss::ne<std::string>>("3,"));
    REQUIRE_EXCEPTION(c.convert<ss::ne<std::string>, int>(",3"));
    REQUIRE_EXCEPTION(c.convert<void, ss::ne<std::string>, int>("junk,,3"));
    REQUIRE_EXCEPTION(c.convert<ss::ne<std::vector<int>>>(""));

    try {
        {
            auto tup = c.convert<ss::ne<std::string>>("s");
            REQUIRE(c.valid());
            CHECK_EQ(tup, "s");
        }
        {
            auto tup =
                c.convert<std::optional<int>, ss::ne<std::string>>("1,s");
            REQUIRE(c.valid());
            CHECK_EQ(tup, std::make_tuple(1, "s"));
        }
        {
            auto tup = c.convert<ss::ne<std::vector<int>>>("{1 2 3}");
            REQUIRE(c.valid());
            CHECK_EQ(tup, extracted_vector);
        }
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }
}

TEST_CASE(
    "converter test ss:lt ss::lte ss::gt ss::gte restriction (in range)") {
    ss::converter c;

    c.convert<ss::lt<int, 3>>("3");
    REQUIRE_FALSE(c.valid());

    c.convert<ss::lt<int, 2>>("3");
    REQUIRE_FALSE(c.valid());

    c.convert<ss::gt<int, 3>>("3");
    REQUIRE_FALSE(c.valid());

    c.convert<ss::gt<int, 4>>("3");
    REQUIRE_FALSE(c.valid());

    c.convert<ss::lte<int, 2>>("3");
    REQUIRE_FALSE(c.valid());

    c.convert<ss::gte<int, 4>>("3");
    REQUIRE_FALSE(c.valid());

    {
        auto tup = c.convert<ss::lt<int, 4>>("3");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 3);
    }

    {
        auto tup = c.convert<ss::gt<int, 2>>("3");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 3);
    }

    {
        auto tup = c.convert<ss::lte<int, 4>>("3");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 3);
    }

    {
        auto tup = c.convert<ss::lte<int, 3>>("3");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 3);
    }

    {
        auto tup = c.convert<ss::gte<int, 2>>("3");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 3);
    }

    {
        auto tup = c.convert<ss::gte<int, 3>>("3");
        REQUIRE(c.valid());
        CHECK_EQ(tup, 3);
    }
}

TEST_CASE("converter test ss:lt ss::lte ss::gt ss::gte restriction (in range) "
          "with exception") {
    ss::converter<ss::throw_on_error> c;

    REQUIRE_EXCEPTION(c.convert<ss::lt<int, 3>>("3"));
    REQUIRE_EXCEPTION(c.convert<ss::lt<int, 2>>("3"));
    REQUIRE_EXCEPTION(c.convert<ss::gt<int, 3>>("3"));
    REQUIRE_EXCEPTION(c.convert<ss::gt<int, 4>>("3"));
    REQUIRE_EXCEPTION(c.convert<ss::lte<int, 2>>("3"));
    REQUIRE_EXCEPTION(c.convert<ss::gte<int, 4>>("3"));

    try {
        {
            auto tup = c.convert<ss::lt<int, 4>>("3");
            REQUIRE(c.valid());
            CHECK_EQ(tup, 3);
        }

        {
            auto tup = c.convert<ss::gt<int, 2>>("3");
            REQUIRE(c.valid());
            CHECK_EQ(tup, 3);
        }

        {
            auto tup = c.convert<ss::lte<int, 4>>("3");
            REQUIRE(c.valid());
            CHECK_EQ(tup, 3);
        }

        {
            auto tup = c.convert<ss::lte<int, 3>>("3");
            REQUIRE(c.valid());
            CHECK_EQ(tup, 3);
        }

        {
            auto tup = c.convert<ss::gte<int, 2>>("3");
            REQUIRE(c.valid());
            CHECK_EQ(tup, 3);
        }

        {
            auto tup = c.convert<ss::gte<int, 3>>("3");
            REQUIRE(c.valid());
            CHECK_EQ(tup, 3);
        }
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }
}

TEST_CASE("converter test error mode") {
    ss::converter<ss::string_error> c;
    c.convert<int>("junk");
    CHECK_FALSE(c.valid());
    CHECK_FALSE(c.error_msg().empty());
}

TEST_CASE("converter test throw on error mode") {
    ss::converter<ss::throw_on_error> c;
    REQUIRE_EXCEPTION(c.convert<int>("junk"));
}

TEST_CASE("converter test converter with quotes spacing and escaping") {
    {
        ss::converter c;

        auto tup = c.convert<std::string, std::string, std::string>(
            R"("just","some","strings")");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple("\"just\"", "\"some\"", "\"strings\""));
    }

    {
        ss::converter<ss::quote<'"'>> c;

        auto tup = c.convert<std::string, std::string, double, char>(
            buff(R"("just",some,"12.3","a")"));
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple("just", "some", 12.3, 'a'));
    }

    {
        ss::converter<ss::trim<' '>> c;

        auto tup = c.convert<std::string, std::string, double, char>(
            buff(R"(    just  ,  some   ,  12.3 ,a     )"));
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple("just", "some", 12.3, 'a'));
    }

    {
        ss::converter<ss::escape<'\\'>> c;

        auto tup =
            c.convert<std::string, std::string>(buff(R"(ju\,st,strings)"));
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple("ju,st", "strings"));
    }

    {
        ss::converter<ss::escape<'\\'>, ss::trim<' '>, ss::quote<'"'>> c;

        auto tup = c.convert<std::string, std::string, double, std::string>(
            buff(R"(  ju\,st  ,  "so,me"  ,   12.34     ,   "str""ings")"));
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple("ju,st", "so,me", 12.34, "str\"ings"));
    }
}

TEST_CASE("converter test converter with quotes spacing and escaping with "
          "exceptions") {
    try {
        ss::converter<ss::throw_on_error> c;

        auto tup = c.convert<std::string, std::string, std::string>(
            R"("just","some","strings")");
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple("\"just\"", "\"some\"", "\"strings\""));
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        ss::converter<ss::quote<'"'>> c;

        auto tup = c.convert<std::string, std::string, double, char>(
            buff(R"("just",some,"12.3","a")"));
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple("just", "some", 12.3, 'a'));
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        ss::converter<ss::throw_on_error, ss::trim<' '>> c;

        auto tup = c.convert<std::string, std::string, double, char>(
            buff(R"(    just  ,  some   ,  12.3 ,a     )"));
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple("just", "some", 12.3, 'a'));
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        ss::converter<ss::throw_on_error, ss::escape<'\\'>> c;

        auto tup =
            c.convert<std::string, std::string>(buff(R"(ju\,st,strings)"));
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple("ju,st", "strings"));
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }

    try {
        ss::converter<ss::throw_on_error, ss::escape<'\\'>, ss::trim<' '>,
                      ss::quote<'"'>>
            c;

        auto tup = c.convert<std::string, std::string, double, std::string>(
            buff(R"(  ju\,st  ,  "so,me"  ,   12.34     ,   "str""ings")"));
        REQUIRE(c.valid());
        CHECK_EQ(tup, std::make_tuple("ju,st", "so,me", 12.34, "str\"ings"));
    } catch (ss::exception& e) {
        FAIL(std::string{e.what()});
    }
}

TEST_CASE("converter test invalid split conversions") {
    ss::converter<ss::string_error, ss::escape<'\\'>, ss::trim<' '>,
                  ss::quote<'"'>>
        c;

    {
        // mismatched quote
        c.convert<std::string, std::string, double, char>(
            buff(R"(  "just  , some ,   "12.3","a"  )"));
        CHECK_FALSE(c.valid());
        CHECK_FALSE(c.unterminated_quote());
        CHECK_FALSE(c.error_msg().empty());
    }

    {
        // unterminated quote
        c.convert<std::string, std::string, double, std::string>(
            buff(R"(  ju\,st  ,  "so,me"  ,   12.34     ,   "str""ings)"));
        CHECK_FALSE(c.valid());
        CHECK(c.unterminated_quote());
        CHECK_FALSE(c.error_msg().empty());
    }

    {
        // unterminated escape
        c.convert<std::string, std::string, double, std::string>(
            buff(R"(just,some,2,strings\)"));
        CHECK_FALSE(c.valid());
        CHECK_FALSE(c.unterminated_quote());
        CHECK_FALSE(c.error_msg().empty());
    }

    {
        // unterminated escape while quoting
        c.convert<std::string, std::string, double, std::string>(
            buff(R"(just,some,2,"strings\)"));
        CHECK_FALSE(c.valid());
        CHECK_FALSE(c.unterminated_quote());
        CHECK_FALSE(c.error_msg().empty());
    }

    {
        // unterminated escaped quote
        c.convert<std::string, std::string, double, std::string>(
            buff(R"(just,some,2,"strings\")"));
        CHECK_FALSE(c.valid());
        CHECK(c.unterminated_quote());
        CHECK_FALSE(c.error_msg().empty());
    }
}

TEST_CASE("converter test invalid split conversions with exceptions") {
    ss::converter<ss::escape<'\\'>, ss::trim<' '>, ss::quote<'"'>,
                  ss::throw_on_error>
        c;

    // mismatched quote
    REQUIRE_EXCEPTION(c.convert<std::string, std::string, double, char>(
        buff(R"(  "just  , some ,   "12.3","a"  )")));
    CHECK_FALSE(c.unterminated_quote());

    // unterminated quote
    REQUIRE_EXCEPTION(c.convert<std::string, std::string, double, std::string>(
        buff(R"(  ju\,st  ,  "so,me"  ,   12.34     ,   "str""ings)")));
    CHECK(c.unterminated_quote());

    // unterminated escape
    REQUIRE_EXCEPTION(c.convert<std::string, std::string, double, std::string>(
        buff(R"(just,some,2,strings\)")));
    CHECK_FALSE(c.unterminated_quote());

    // unterminated escape while quoting
    REQUIRE_EXCEPTION(c.convert<std::string, std::string, double, std::string>(
        buff(R"(just,some,2,"strings\)")));
    CHECK_FALSE(c.unterminated_quote());

    // unterminated escaped quote
    REQUIRE_EXCEPTION(c.convert<std::string, std::string, double, std::string>(
        buff(R"(just,some,2,"strings\")")));
    CHECK(c.unterminated_quote());
}

