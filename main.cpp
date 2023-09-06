#include <cpprest/http_listener.h>
#include <cpprest/json.h>

namespace json = web::json;
namespace http = web::http;
namespace listener = web::http::experimental::listener;

auto handle_calculation(const std::map<std::wstring, std::wstring> &params, const http::http_request &request) {
    const auto &handle_error = [&](const std::wstring &message) {
        json::value answer;
        answer[U("error")] = json::value::string(message);
        request.reply(http::status_codes::BadRequest, answer);
    }; 

    const auto num1 = std::stod(params.at(U("num1")));
    const auto num2 = std::stod(params.at(U("num2")));
    const auto &operation = params.at(U("operation"));

    const auto &handle_success = [&](const double &result) {
        json::value answer;
        answer[U("result")] = json::value::number(result);
        request.reply(http::status_codes::OK, answer);
    };

    if (operation == U("add")) {
        handle_success(num1 + num2);
        return;
    }

    if (operation == U("subtract")) {
        handle_success(num1 - num2);
        return;
    }

    if (operation == U("multiply")) {
        handle_success(num1 * num2);
        return;
    }

    if (operation == U("divide")) {
        if (num2 == 0) {
            handle_error(U("Cannot divide by zero"));
            return;
        }

        handle_success(num1 / num2);
        return;
    }

    handle_error(U("Invalid operation"));
}

auto handle_get(const http::http_request &request) {
    const auto &http_get_vars = web::uri::split_query(request.request_uri().query());
    handle_calculation(http_get_vars, request);
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

    handle_calculation(http_post_vars, request);
}

auto main() -> int {
    listener::http_listener listener(U("http://localhost:8080/calculator"));

    listener.support(http::methods::GET, handle_get);
    listener.support(http::methods::POST, handle_post);

    try {
        listener
                .open()
                .then([]() { ucout << U("Starting to listen\n"); })
                .wait();

        std::cout << "Press Enter to exit." << std::endl;
        std::string line;
        std::getline(std::cin, line);
    } catch (std::exception const &e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
