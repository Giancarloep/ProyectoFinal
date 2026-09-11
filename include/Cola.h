#ifndef COLA_H
#define COLA_H

#include <cstddef>
#include <stdexcept>
#include <vector>

template <typename T>
struct NodoCola {
    T dato;
    NodoCola<T>* siguiente;

    explicit NodoCola(const T& d) : dato(d), siguiente(nullptr) {}
};

template <typename T>
class Cola {
public:
    Cola() : m_frente(nullptr), m_fin(nullptr), m_tamano(0) {}

    ~Cola() { limpiar(); }

    Cola(const Cola&) = delete;
    Cola& operator=(const Cola&) = delete;

    void encolar(const T& valor) {
        NodoCola<T>* nuevo = new NodoCola<T>(valor);
        if (vacia()) {
            m_frente = nuevo;
        } else {
            m_fin->siguiente = nuevo;
        }
        m_fin = nuevo;
        ++m_tamano;
    }

    void desencolar() {
        if (vacia()) throw std::underflow_error("cola vacia");
        NodoCola<T>* aux = m_frente;
        m_frente = m_frente->siguiente;
        if (m_frente == nullptr) m_fin = nullptr;
        delete aux;
        --m_tamano;
    }

    T& frente() {
        if (vacia()) throw std::underflow_error("cola vacia");
        return m_frente->dato;
    }

    const T& frente() const {
        if (vacia()) throw std::underflow_error("cola vacia");
        return m_frente->dato;
    }

    bool vacia() const { return m_frente == nullptr; }
    std::size_t tamano() const { return m_tamano; }

    void limpiar() {
        while (!vacia()) desencolar();
    }

    // Copia
    std::vector<T> aVector() const {
        std::vector<T> v;
        v.reserve(m_tamano);
        for (NodoCola<T>* n = m_frente; n != nullptr; n = n->siguiente) {
            v.push_back(n->dato);
        }
        return v;
    }

private:
    NodoCola<T>* m_frente;
    NodoCola<T>* m_fin;
    std::size_t m_tamano;
};

#endif
