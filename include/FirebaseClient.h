#ifndef FIREBASE_CLIENT_H
#define FIREBASE_CLIENT_H

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class FirebaseClient {
public:
    static FirebaseClient& instancia() {
        static FirebaseClient inst;
        return inst;
    }

    bool inicializado() const { return true; }

    // Verifica un ID token del cliente (Firebase Auth REST)
    std::optional<std::string> verificarIdToken(const std::string& idToken) const;

    // Lee JSON de Realtime Database en path con auth (idToken del usuario)
    std::optional<json> get(const std::string& path, const std::string& authToken) const;

    // Escribe JSON en Realtime Database en path con auth
    bool put(const std::string& path, const json& data, const std::string& authToken) const;

    // Patch (merge) JSON en Realtime Database
    bool patch(const std::string& path, const json& data, const std::string& authToken) const;

    // Borra path
    bool deletePath(const std::string& path, const std::string& authToken) const;

private:
    FirebaseClient() = default;
};

#endif