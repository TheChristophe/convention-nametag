#include "server.hpp"

#include <filesystem>
#include <fstream>

#include "video/helper.hpp"

// files delivered expected to be small, so skipping async data delivery for simplicity
std::optional<std::string> AttemptLoadFile(const std::filesystem::path &file) {
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

std::string UrlDecode(std::string encoded) {
    for (size_t i = 0; i < encoded.size() - 2; i++) {
        if (encoded[i] == '%' && (i + 2) < encoded.size()) {
            char hex[3] = {encoded[i + 1], encoded[i + 2], '\0'};
            char decoded[2] = {static_cast<char>(std::strtol(hex, nullptr, 16)), '\0'};
            encoded.replace(i, 3, decoded);
        }
    }

    return encoded;
}

namespace ResponseCodes {
const auto HTTP_200_OK = uWS::HTTP_200_OK;
const auto HTTP_204_NO_CONTENT = "204 No Content";
const auto HTTP_404_NOT_FOUND = "404 Not Found";
const auto HTTP_409_CONFLICT = "409 Conflict";
} // namespace ResponseCodes

const auto RESPONSE_404 = "<!doctype html>\n"
                          "<html lang=\"en\">\n"
                          "  <head><meta charset=\"utf-8\"><title>404 Not Found</title></head>\n"
                          "  <body>Not found</body>\n"
                          "</html>";

const auto videoFolder = std::filesystem::path("videos");

template <bool SSL = false> void serveFile(uWS::HttpResponse<SSL> *res, uWS::HttpRequest *req) {
    // cut off / or c++ will interpret it as root path
    std::string url = std::string(req->getUrl()).substr(1);
    // if accessing "/"
    if (url.empty()) {
        url = "index.html";
    }
    url = std::filesystem::current_path().parent_path() / "frontend/build" / url;
    auto file = AttemptLoadFile(url);
    if (not file.has_value()) {
        res->writeStatus(ResponseCodes::HTTP_404_NOT_FOUND);
        res->writeHeader("Access-Control-Allow-Origin", "*");
        res->end(RESPONSE_404);
    } else {
        res->writeStatus(ResponseCodes::HTTP_200_OK);
        res->writeHeader("Access-Control-Allow-Origin", "*");
        res->end(file.value());
    }
}

std::vector<std::filesystem::path> listFiles() {
    if (std::filesystem::exists(videoFolder) && std::filesystem::is_directory(videoFolder)) {
        std::vector<std::filesystem::path> files;
        auto iterator = std::filesystem::directory_iterator(videoFolder);
        std::copy(std::filesystem::begin(iterator), std::filesystem::end(iterator), std::back_inserter(files));
        return files;
    }
    return {};
}

void getRoot(uWS::HttpResponse<false> *res, uWS::HttpRequest *req) { serveFile(res, req); }

void getRootFile(uWS::HttpResponse<false> *res, uWS::HttpRequest *req) { serveFile(res, req); }

void getVideos(uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
    auto files = listFiles();
    std::vector<nlohmann::json> fileNames;
    std::transform(files.begin(), files.end(), std::back_inserter(fileNames),
        [](const std::filesystem::path &path) -> nlohmann::json {
            nlohmann::json entry{
                {"filename", std::string(path.filename())},
            };
            if (const auto duration = getVideoDuration(path); duration.has_value()) {
                entry["duration"] = duration.value();
            }
            return entry;
        });
    auto json = nlohmann::json({{"videos", fileNames}});

    res->writeStatus(ResponseCodes::HTTP_200_OK);
    res->writeHeader("content-type", "application/json");
    res->writeHeader("Access-Control-Allow-Origin", "*");
    res->end(json.dump());
}

void postVideo(uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
    auto urlDecoded = UrlDecode(std::string(req->getParameter(0)));
    auto path = videoFolder / std::filesystem::path(urlDecoded).filename();

    if (!std::filesystem::exists(videoFolder)) {
        std::filesystem::create_directory(videoFolder);
    }

    // use C files because idk how to do binary with ofstream
    if (exists(path)) {
        std::cerr << "File collision, upload for " << path << " refused" << std::endl;
        res->writeStatus(ResponseCodes::HTTP_409_CONFLICT);
        res->writeHeader("Access-Control-Allow-Origin", "*");
        res->end();
        return;
    }

    FILE *out = fopen(path.c_str(), "wb");

    res->onData([&out, &res](std::string_view chunk, auto isLast) {
        /* Buffer this anywhere you want to */
        fwrite(chunk.data(), chunk.size(), 1, out);

        if (isLast) {
            fclose(out);
            res->writeStatus(ResponseCodes::HTTP_204_NO_CONTENT);
            res->writeHeader("Access-Control-Allow-Origin", "*");
            res->end();
        }
    });

    res->onAborted([&out, &path, &res]() {
        /* Request was prematurely aborted, stop reading */
        fclose(out);
        remove(path.c_str());
        std::cerr << "Upload for " << path << " aborted" << std::endl;
        res->writeStatus(ResponseCodes::HTTP_409_CONFLICT);
        res->writeHeader("Access-Control-Allow-Origin", "*");
        res->end();
        return;
    });
}

void deleteVideo(uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
    auto urlDecoded = UrlDecode(std::string(req->getParameter(0)));
    auto path = videoFolder / std::filesystem::path(urlDecoded).filename();
    std::filesystem::remove(path);

    res->writeStatus(ResponseCodes::HTTP_204_NO_CONTENT);
    res->writeHeader("Access-Control-Allow-Origin", "*");
    res->end();
}

void postPlayFile(uWS::HttpResponse<false> *res, uWS::HttpRequest *req, VideoPlayer &player) {
    auto urlDecoded = UrlDecode(std::string(req->getParameter(0)));
    auto path = videoFolder / std::filesystem::path(urlDecoded).filename();
    if (not player.PlayFile(path)) {
        std::cerr << "Could not find file " << path << std::endl;
        res->writeStatus(ResponseCodes::HTTP_404_NOT_FOUND);
        res->writeHeader("content-type", "text/html");
        res->writeHeader("Access-Control-Allow-Origin", "*");
        res->end(RESPONSE_404);
        return;
    }

    res->writeStatus(ResponseCodes::HTTP_204_NO_CONTENT);
    res->writeHeader("Access-Control-Allow-Origin", "*");
    res->end();
}

void options(uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
    res->writeStatus(ResponseCodes::HTTP_204_NO_CONTENT);
    res->writeHeader("Access-Control-Allow-Origin", "*");
    res->writeHeader("Access-Control-Allow-Methods", "*");
    // res->writeHeader("Allow", "OPTIONS, GET, HEAD, POST");
    res->end();
}

void WebServer::run(VideoPlayer &player) {
    uWS::App()
        .get("/", getRoot)
        .get("/*", getRootFile)
        .get("/videos", getVideos)
        .post("/videos/:video", postVideo)
        .del("/videos/:file", deleteVideo)
        // play specific video
        .post("/videos/:file/play",
            [&player](uWS::HttpResponse<false> *res, uWS::HttpRequest *req) { postPlayFile(res, req, player); })
        .options("/*", options)
        .listen(_port,
            [this](auto *token) {
                this->_socket = token;
                if (token) {
                    std::cout << "Listening!" << std::endl;
                    // success
                }
            })
        .run();
}

void WebServer::halt() { us_listen_socket_close(0, _socket); }
