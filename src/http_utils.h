#pragma once

#include <string>
#include <cstdio>

// Format a double to 1 decimal place
inline std::string to_fixed1(double val) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.1f", val);
    return buf;
}

// Create an HTTP 200 response with given content and content type
inline std::string CreateHTTPResponse(const std::string& content, const std::string& content_type = "text/html") {
    std::string response;
    response.reserve(256 + content.length());
    response += "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + content_type + "\r\n";
    response += "Content-Length: " + std::to_string(content.length()) + "\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Cache-Control: no-cache\r\n";
    response += "\r\n";
    response += content;
    return response;
}
