#include <gtest/gtest.h>
#include "http_utils.h"

// --- to_fixed1 tests ---

TEST(ToFixed1, FormatsZero) {
    EXPECT_EQ(to_fixed1(0.0), "0.0");
}

TEST(ToFixed1, FormatsWholeNumber) {
    EXPECT_EQ(to_fixed1(42.0), "42.0");
}

TEST(ToFixed1, RoundsToOneDecimal) {
    EXPECT_EQ(to_fixed1(3.14159), "3.1");
}

TEST(ToFixed1, RoundsUp) {
    EXPECT_EQ(to_fixed1(2.95), "3.0");
}

TEST(ToFixed1, NegativeNumber) {
    EXPECT_EQ(to_fixed1(-1.23), "-1.2");
}

TEST(ToFixed1, LargeNumber) {
    EXPECT_EQ(to_fixed1(99999.9), "99999.9");
}

// --- CreateHTTPResponse tests ---

TEST(CreateHTTPResponse, ContainsStatusLine) {
    auto response = CreateHTTPResponse("hello");
    EXPECT_TRUE(response.find("HTTP/1.1 200 OK\r\n") != std::string::npos);
}

TEST(CreateHTTPResponse, DefaultContentTypeIsHtml) {
    auto response = CreateHTTPResponse("hello");
    EXPECT_TRUE(response.find("Content-Type: text/html\r\n") != std::string::npos);
}

TEST(CreateHTTPResponse, CustomContentType) {
    auto response = CreateHTTPResponse("{}", "application/json");
    EXPECT_TRUE(response.find("Content-Type: application/json\r\n") != std::string::npos);
}

TEST(CreateHTTPResponse, CorrectContentLength) {
    std::string body = "test body content";
    auto response = CreateHTTPResponse(body);
    std::string expected = "Content-Length: " + std::to_string(body.length()) + "\r\n";
    EXPECT_TRUE(response.find(expected) != std::string::npos);
}

TEST(CreateHTTPResponse, ContainsCorsHeader) {
    auto response = CreateHTTPResponse("hello");
    EXPECT_TRUE(response.find("Access-Control-Allow-Origin: *\r\n") != std::string::npos);
}

TEST(CreateHTTPResponse, ContainsCacheControl) {
    auto response = CreateHTTPResponse("hello");
    EXPECT_TRUE(response.find("Cache-Control: no-cache\r\n") != std::string::npos);
}

TEST(CreateHTTPResponse, HeaderBodySeparation) {
    auto response = CreateHTTPResponse("hello");
    // Headers and body separated by \r\n\r\n
    EXPECT_TRUE(response.find("\r\n\r\nhello") != std::string::npos);
}

TEST(CreateHTTPResponse, EmptyBody) {
    auto response = CreateHTTPResponse("");
    EXPECT_TRUE(response.find("Content-Length: 0\r\n") != std::string::npos);
    EXPECT_TRUE(response.find("\r\n\r\n") != std::string::npos);
}
