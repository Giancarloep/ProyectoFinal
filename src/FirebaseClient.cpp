#include "FirebaseClient.h"

#include <httplib.h>

using json = nlohmann::json;

std::optional<std::string> FirebaseClient::verificarIdToken(const std::string& idToken) const {
    httplib::Client cli("identitytoolkit.googleapis.com");
    cli.set_connection_timeout(10);
    cli.set_read_timeout(10);

    json body = {{"idToken", idToken}};
    auto res = cli.Post("/v1/accounts:lookup?key=AIzaSyDkFL-rIPJP9gK9uUnSenAyYT-ZWtMRrds",
                        body.dump(), "application/json");
    if (!res || res->status != 200) return std::nullopt;

    json j = json::parse(res->body);
    if (j.contains("users") && j["users"].is_array() && !j["users"].empty()) {
        return j["users"][0].value("localId", "");
    }
    return std::nullopt;
}

std::optional<json> FirebaseClient::get(const std::string& path, const std::string& authToken) const {
    if (authToken.empty()) return std::nullopt;

    httplib::Client cli("ejercicios-11ca3-default-rtdb.firebaseio.com");
    cli.set_connection_timeout(10);
    cli.set_read_timeout(10);
    std::string url = path;
    if (url.back() != '/') url += ".json";
    else url += ".json";
    url += "?auth=" + authToken;

    auto res = cli.Get(url.c_str());
    if (!res || res->status < 200 || res->status >= 300) return std::nullopt;
    if (res->body.empty() || res->body == "null") return std::nullopt;
    return json::parse(res->body);
}

bool FirebaseClient::put(const std::string& path, const json& data, const std::string& authToken) const {
    if (authToken.empty()) return false;

    httplib::Client cli("ejercicios-11ca3-default-rtdb.firebaseio.com");
    cli.set_connection_timeout(10);
    cli.set_read_timeout(10);
    std::string url = path;
    if (url.back() != '/') url += ".json";
    else url += ".json";
    url += "?auth=" + authToken;

    httplib::Headers headers = {{"Content-Type", "application/json"}};
    auto res = cli.Put(url.c_str(), headers, data.dump(), "application/json");
    return res && res->status >= 200 && res->status < 300;
}

bool FirebaseClient::patch(const std::string& path, const json& data, const std::string& authToken) const {
    if (authToken.empty()) return false;

    httplib::Client cli("ejercicios-11ca3-default-rtdb.firebaseio.com");
    cli.set_connection_timeout(10);
    cli.set_read_timeout(10);
    std::string url = path;
    if (url.back() != '/') url += ".json";
    else url += ".json";
    url += "?auth=" + authToken;

    httplib::Headers headers = {{"Content-Type", "application/json"}};
    auto res = cli.Patch(url.c_str(), headers, data.dump(), "application/json");
    return res && res->status >= 200 && res->status < 300;
}

bool FirebaseClient::deletePath(const std::string& path, const std::string& authToken) const {
    if (authToken.empty()) return false;

    httplib::Client cli("ejercicios-11ca3-default-rtdb.firebaseio.com");
    cli.set_connection_timeout(10);
    cli.set_read_timeout(10);
    std::string url = path;
    if (url.back() != '/') url += ".json";
    else url += ".json";
    url += "?auth=" + authToken;

    httplib::Headers headers = {{"Content-Type", "application/json"}};
    auto res = cli.Delete(url.c_str(), headers, "", "application/json");
    return res && res->status >= 200 && res->status < 300;
}