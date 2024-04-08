#include "server.hpp"

#include <filesystem>
#include <fstream>

#include "video/helper.hpp"

namespace fs = std::filesystem;

// files delivered expected to be small, so skipping async data delivery for simplicity
std::optional<std::string> AttemptLoadFile(const fs::path &file) {
    if (not fs::exists(file)) {
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
const auto HTTP_400_BAD_REQUEST = "400 Bad Request";
const auto HTTP_404_NOT_FOUND = "404 Not Found";
const auto HTTP_409_CONFLICT = "409 Conflict";
} // namespace ResponseCodes

const auto RESPONSE_404 = "<!doctype html>\n"
                          "<html lang=\"en\">\n"
                          "  <head><meta charset=\"utf-8\"><title>404 Not Found</title></head>\n"
                          "  <body>Not found</body>\n"
                          "</html>";

const auto videoFolder = fs::path("videos");

template <bool SSL = false> void serveFile(uWS::HttpResponse<SSL> *res, const fs::path &path) {
    // uWS::HttpRequest *req,
    // const fs::path &fileBasePath = fs::current_path().parent_path() / "frontend/build") {

    auto file = AttemptLoadFile(path);
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

std::vector<fs::path> listFiles() {
    if (fs::exists(videoFolder) && fs::is_directory(videoFolder)) {
        std::vector<fs::path> files;
        auto iterator = fs::directory_iterator(videoFolder);
        std::copy_if(fs::begin(iterator), fs::end(iterator), std::back_inserter(files),
            [](auto it) { return !fs::is_directory(it); });
        return files;
    }
    return {};
}

const auto frontendRoot = fs::current_path().parent_path() / "frontend/build";

void getRoot(uWS::HttpResponse<false> *res, uWS::HttpRequest *req) { serveFile(res, frontendRoot / "index.html"); }

void getFile(uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
    // cut off '/' or c++ will interpret it as root path
    const auto requestUrl = req->getUrl();
    std::string url = requestUrl[0] == '/' ? std::string(requestUrl).substr(1) : std::string(requestUrl);

    // if accessing "/"
    if (url.empty()) {
        url = "index.html";
    }

    url = frontendRoot / url;
    serveFile(res, url);
}

std::string thumbnailFilename(const std::string &filename) { return std::format("{}.thumb.webp", filename); }
std::string thumbnailPath(const std::string &filename) {
    return std::format("videos/thumbnails/{}", thumbnailFilename(filename));
}

void getVideos(uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
    auto files = listFiles();
    std::vector<nlohmann::json> fileNames;
    std::transform(
        files.begin(), files.end(), std::back_inserter(fileNames), [](const fs::path &path) -> nlohmann::json {
            nlohmann::json entry{{"filename", std::string(path.filename())},
                // TODO: return more elegantly
                {"thumbnail", thumbnailFilename(path.filename())}};
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
    auto *path = new fs::path(videoFolder / fs::path(urlDecoded).filename());

    if (!fs::exists(videoFolder)) {
        fs::create_directory(videoFolder);
    }
    if (!fs::exists(videoFolder / "thumbnails")) {
        fs::create_directory(videoFolder / "thumbnails");
    }

    if (exists(*path)) {
        res->writeStatus(ResponseCodes::HTTP_409_CONFLICT);
        res->writeHeader("Access-Control-Allow-Origin", "*");
        res->end();
        return;
    }

    FILE *out = fopen(path->c_str(), "wb");

    res->onData([res, out, path](std::string_view chunk, bool isLast) {
        fwrite(chunk.data(), chunk.size(), 1, out);

        if (isLast) {
            fclose(out);

            std::system(std::format(
                "ffmpeg -i {} -filter:v thumbnail=500 -frames:v 1 {}", path->c_str(), thumbnailPath(path->filename()))
                            .c_str());

            delete path;

            res->writeStatus(ResponseCodes::HTTP_204_NO_CONTENT);
            res->writeHeader("Access-Control-Allow-Origin", "*");
            res->end();
        }
    });

    res->onAborted([res, out, path]() {
        fclose(out);
        remove(*path);
        delete path;

        res->writeStatus(ResponseCodes::HTTP_400_BAD_REQUEST);
        res->writeHeader("Access-Control-Allow-Origin", "*");
        res->end();
    });
}

void deleteVideo(uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
    auto urlDecoded = UrlDecode(std::string(req->getParameter(0)));
    auto path = videoFolder / fs::path(urlDecoded).filename();
    fs::remove(videoFolder / fs::path(urlDecoded).filename());
    fs::remove(thumbnailPath(fs::path(urlDecoded).filename()));

    res->writeStatus(ResponseCodes::HTTP_204_NO_CONTENT);
    res->writeHeader("Access-Control-Allow-Origin", "*");
    res->end();
}

void postPlayFile(uWS::HttpResponse<false> *res, uWS::HttpRequest *req, VideoPlayer &player) {
    auto urlDecoded = UrlDecode(std::string(req->getParameter(0)));
    auto path = videoFolder / fs::path(urlDecoded).filename();
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

const auto thumbnailRoot = fs::current_path() / "videos/thumbnails";

void getThumbnail(uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
    std::cout << "th " << thumbnailRoot / UrlDecode(std::string(req->getParameter(0))) << std::endl;
    serveFile(res, thumbnailRoot / UrlDecode(std::string(req->getParameter(0))));
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
        .get("/*", getFile)
        .get("/videos", getVideos)
        .post("/videos/:video", postVideo)
        .del("/videos/:file", deleteVideo)
        // play specific video
        .post("/videos/:file/play",
            [&player](uWS::HttpResponse<false> *res, uWS::HttpRequest *req) { postPlayFile(res, req, player); })
        .get("/thumbnails/:thumbnail", getThumbnail)
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
