#include <cmath>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <set>

namespace json = web::json;
namespace http = web::http;
namespace listener = web::http::experimental::listener;

auto handle_calculation(const http::http_request &request, const std::map<std::wstring, std::wstring> &query_split) {
    const auto &handle_error = [&](const std::wstring &message) {
        json::value answer;
        answer[U("error")] = json::value::string(message);
        request.reply(http::status_codes::BadRequest, answer);
    };

    const auto &handle_success = [&](const double &result) {
        json::value answer;
        answer[U("result")] = json::value::number(result);
        request.reply(http::status_codes::OK, answer);
    };

    double num1 = NAN, num2 = NAN;
    try {
        num1 = std::stod(query_split.at(U("num1")));
        num2 = std::stod(query_split.at(U("num2")));
    } catch (const std::exception &) {
        handle_error(U("Invalid input values."));
        return;
    }

    const auto operation = web::uri::split_path(request.request_uri().path()).back();

    const std::unordered_map<std::wstring, std::function<void()>> operations = {
            {U("add"), [&] { handle_success(num1 + num2); }},
            {U("subtract"), [&] { handle_success(num1 - num2); }},
            {U("multiply"), [&] { handle_success(num1 * num2); }},
            {U("divide"), [&] { num2 != 0 ? handle_success(num1 / num2)
                                          : handle_error(U("Cannot divide by zero")); }},
    };

    operations.contains(operation)
            ? std::invoke(operations.at(operation))
            : handle_error(U("Invalid operation"));
}

auto handle_trigonometry(const http::http_request &request, const std::map<std::wstring, std::wstring> &query_split) {
    const auto &handle_error = [&](const std::wstring &message) {
        json::value answer;
        answer[U("error")] = json::value::string(message);
        request.reply(http::status_codes::BadRequest, answer);
    };

    const auto &handle_success = [&](const double &result) {
        json::value answer;
        answer[U("result")] = json::value::number(result);
        request.reply(http::status_codes::OK, answer);
    };

    double num1 = NAN, num2 = NAN;
    try {
        num1 = std::stod(query_split.at(U("num1")));
        num2 = std::stod(query_split.at(U("num2")));
    } catch (const std::exception &) {
        handle_error(U("Invalid input values."));
        return;
    }

    const auto operation = web::uri::split_path(request.request_uri().path()).back();

    if (num2 == 0) {
        handle_error(U("Cannot divide by zero"));
        return;
    }

    const std::unordered_map<std::wstring, std::function<void()>> operations = {
            {U("sin"), [&] { handle_success(std::sin(num1 / num2)); }},
            {U("cos"), [&] { handle_success(std::cos(num1 / num2)); }},
            {U("tan"), [&] { handle_success(std::tan(num1 / num2)); }},
            {U("ctg"), [&] { handle_success(1 / std::tan(num1 / num2)); }},
    };

    operations.contains(operation)
            ? std::invoke(operations.at(operation))
            : handle_error(U("Invalid operation"));
}

auto handle_appropriate_path(const http::http_request &request, const std::map<std::wstring, std::wstring> &http_vars) {
    const auto path_split{web::uri::split_path(request.request_uri().path())};
    const std::set<std::wstring> path_vars{path_split.cbegin(), path_split.cend()};

    path_vars.contains(U("calculation"))
            ? handle_calculation(request, http_vars)
            : handle_trigonometry(request, http_vars);
}

auto handle_get(const http::http_request &request) {
    const auto &query_split{web::uri::split_query(request.request_uri().query())};
    handle_appropriate_path(request, query_split);
}

auto handle_post(http::http_request request) {
    if (request.headers().content_type() != U("application/json")) {
        json::value answer;
        answer[U("error")] = json::value::string(U("Invalid content type"));
        request.reply(http::status_codes::BadRequest, answer);
        return;
    }

    const auto http_post_json = request.extract_json().get();
    std::map<std::wstring, std::wstring> http_post_vars;

    if (http_post_json.is_object()) {
        for (const auto &[key, value]: http_post_json.as_object()) {
            http_post_vars[key] = value.is_string() ? value.as_string() : std::to_wstring(value.as_double());
        }
    }

    handle_appropriate_path(request, http_post_vars);
}

auto main() -> int {
    std::vector uris{
            U("http://localhost:8080/calculation/add"),
            U("http://localhost:8080/calculation/subtract"),
            U("http://localhost:8080/calculation/multiply"),
            U("http://localhost:8080/calculation/divide"),

            U("http://localhost:8080/trigonometry/sin"),
            U("http://localhost:8080/trigonometry/cos"),
            U("http://localhost:8080/trigonometry/tan"),
            U("http://localhost:8080/trigonometry/ctg"),
    };

    std::vector<listener::http_listener> listeners;
    for (const auto &uri: uris) {
        listeners.emplace_back(uri);
        auto &listener = listeners.back();

        listener.support(http::methods::GET, handle_get);
        listener.support(http::methods::POST, handle_post);

        listener
                .open()
                .then([&]() { std::wcout << U("Starting to listen on ") << uri << U("\n"); })
                .wait();
    }

    std::cout << "Press Enter to exit." << std::endl;
    std::string line;
    std::getline(std::cin, line);

    return 0;
}
