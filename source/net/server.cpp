#include "server.hpp"

#include <filesystem>
#include <fstream>

// files delivered expected to be small, so skipping async data delivery for simplicity
std::optional<std::string> AttemptLoadFile(const std::filesystem::path &file)
{
    if (not std::filesystem::exists(file)) {
        return std::nullopt;
    }
    std::ifstream t(file);
    if (not t.good()) {
        return std::nullopt;
    }
    std::string str;

    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    return str;
}

namespace ResponseCodes {
    const auto HTTP_200_OK        = uWS::HTTP_200_OK;
    const auto HTTP_404_NOT_FOUND = "404 Not Found";
} // namespace ResponseCodes

const auto RESPONSE_404 = "<!doctype html>\n"
                          "<html lang=\"en\">\n"
                          "  <head><meta charset=\"utf-8\"><title>404 Not Found</title></head>\n"
                          "  <body>Not found</body>\n"
                          "</html>";

const auto videoFolder = std::filesystem::path("videos");

template <bool SSL = false>
void serveFile(uWS::HttpResponse<SSL> *res, uWS::HttpRequest *req)
{
    // cut off / or c++ will interpret it as root path
    std::string url = std::string(req->getUrl()).substr(1);
    // if accessing "/"
    if (url.empty()) {
        url = "index.html";
    }
    url       = std::filesystem::current_path().parent_path() / "frontend/build" / url;
    auto file = AttemptLoadFile(url);
    if (not file.has_value()) {
        res->writeStatus(ResponseCodes::HTTP_404_NOT_FOUND);
        res->end(RESPONSE_404);
    }
    else {
        res->writeStatus(ResponseCodes::HTTP_200_OK);
        res->end(file.value());
    }
}

std::vector<std::filesystem::path> listFiles()
{
    if (std::filesystem::exists(videoFolder) && std::filesystem::is_directory(videoFolder)) {
        std::vector<std::filesystem::path> files;
        auto iterator = std::filesystem::directory_iterator(videoFolder);
        std::copy(std::filesystem::begin(iterator), std::filesystem::end(iterator), std::back_inserter(files));
        return files;
    }
    return {};
}

void WebServer::run(VideoPlayer &player)
{
    uWS::App()
        .get("/", [this](uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
            serveFile(res, req);
        })
        .get("/*", [this](uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
            serveFile(res, req);
        })
        // upload
        .post("/videos", [this](uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
            res->writeStatus(ResponseCodes::HTTP_200_OK);
            res->writeHeader("content-type", "application/json");
            res->end("TODO");
        })
        // get list of videos
        .get("/videos", [this](uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
            auto files = listFiles();
            std::vector<nlohmann::json> fileNames;
            std::transform(files.begin(), files.end(), std::back_inserter(fileNames), [](const std::filesystem::path &path) -> nlohmann::json {
                return { { "filename", std::string(path.filename()) } };
            });
            auto json = nlohmann::json({ { "videos", fileNames } });

            res->writeStatus(ResponseCodes::HTTP_200_OK);
            res->writeHeader("content-type", "application/json");
            res->end(json.dump());
        })
        // delete specific video
        .del("/videos/*", [this](uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
            auto path = videoFolder / std::filesystem::path(req->getUrl()).filename();
            std::filesystem::remove(path);

            res->writeHeader("content-type", "application/json");
            res->writeStatus(ResponseCodes::HTTP_200_OK);
            res->end("");
        })
        // play specific video
        .post("/videos/*:play", [this, &player](uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
            auto url = req->getUrl();
            // trim ":play"
            auto path = videoFolder / std::filesystem::path(url.substr(0, url.size() - 5)).filename();
            if (not player.PlayFile(path)) {
                res->writeHeader("content-type", "text/html");
                res->writeStatus(ResponseCodes::HTTP_404_NOT_FOUND);
                res->end(RESPONSE_404);
                return;
            }

            res->writeHeader("content-type", "application/json");
            res->writeStatus(ResponseCodes::HTTP_200_OK);
            res->end("");
        })
        .listen(_port, [this](auto *token) {
            this->_socket = token;
            if (token) {
                std::cout << "Listening!" << std::endl;
                // success
            }
        })
        .run();
}

void WebServer::halt()
{
    us_listen_socket_close(0, _socket);
}
