#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <thread>

#include <httplib.h>

#include "ApiRest.h"
#include "Servicio.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

constexpr int PUERTO = 8080;
constexpr const char* URL_OLLAMA = "http://127.0.0.1:11434";

std::filesystem::path directorioWeb(const char* argv0) {
    std::error_code ec;
    std::filesystem::path exe =
        std::filesystem::absolute(argv0 ? argv0 : ".", ec);
    std::filesystem::path juntoAlExe = exe.parent_path() / "web";
    if (std::filesystem::exists(juntoAlExe / "index.html")) {
        return juntoAlExe;
    }
    return std::filesystem::path("web");
}

bool ollamaResponde() {
    httplib::Client cli(URL_OLLAMA);
    cli.set_connection_timeout(1, 0);
    cli.set_read_timeout(2, 0);
    const auto res = cli.Get("/api/tags");
    return res && res->status == 200;
}

#ifdef _WIN32
bool iniciarProcesoOllama() {
    char ruta[MAX_PATH] = {0};
    if (SearchPathA(nullptr, "ollama.exe", nullptr, MAX_PATH, ruta,
                    nullptr) == 0) {
        return false;
    }
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    std::string cmd = std::string("\"") + ruta + "\" serve";
    const BOOL ok = CreateProcessA(nullptr, cmd.data(), nullptr, nullptr,
                                   FALSE, CREATE_NO_WINDOW, nullptr, nullptr,
                                   &si, &pi);
    if (ok) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    return ok != FALSE;
}
#endif

void asegurarOllama() {
    if (ollamaResponde()) {
        std::cout << "Ollama detectado: IA generativa lista.\n";
        return;
    }
#ifdef _WIN32
    std::cout << "Ollama no esta corriendo; intentando iniciarlo...\n";
    if (!iniciarProcesoOllama()) {
        std::cout << "No se encontro ollama.exe en el PATH.\n";
        return;
    }
    for (int i = 0; i < 20; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (ollamaResponde()) {
            std::cout << "Ollama iniciado automaticamente.\n";
            return;
        }
    }
    std::cout << "Aviso: Ollama no respondio; el asistente usara el modo "
                 "experto local.\n";
#else
    std::cout << "Aviso: inicia Ollama manualmente (ollama serve) para la "
                 "IA generativa.\n";
#endif
}

} // namespace

int main(int argc, char* argv[]) {
    Servicio servicio;
    httplib::Server servidor;

    servidor.new_task_queue = [] {
        return new httplib::ThreadPool(2);
    };

    configurarApi(servidor, servicio);

    asegurarOllama();

    std::filesystem::path web = directorioWeb(argc > 0 ? argv[0] : nullptr);
    if (!servidor.set_mount_point("/", web.string())) {
        std::cerr << "No se encontro la carpeta del frontend en: " << web
                  << "\n";
        return 1;
    }

    std::cout << "Frontend:  http://localhost:" << PUERTO << "\n";

    if (!servidor.listen("0.0.0.0", PUERTO)) {
        std::cerr << "No se pudo abrir el puerto " << PUERTO
                  << " (quizas esta ocupado).\n";
        return 1;
    }
    return 0;
}
