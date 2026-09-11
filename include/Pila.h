#ifndef PILA_H
#define PILA_H

#include <cstddef>
#include <stdexcept>

// Pila (LIFO) implementada con nodos enlazados simples.
// Uso: historial de cambios de ejercicios para "deshacer".
template <typename T>
struct NodoPila {
    T dato;
    NodoPila<T>* siguiente;

    NodoPila(const T& d, NodoPila<T>* s) : dato(d), siguiente(s) {}
};

template <typename T>
class Pila {
public:
    Pila() : m_cima(nullptr), m_tamano(0) {}

    ~Pila() { limpiar(); }

    Pila(const Pila&) = delete;
    Pila& operator=(const Pila&) = delete;

    void push(const T& valor) {
        m_cima = new NodoPila<T>(valor, m_cima);
        ++m_tamano;
    }

    void pop() {
        if (vacia()) throw std::underflow_error("pila vacia");
        NodoPila<T>* aux = m_cima;
        m_cima = m_cima->siguiente;
        delete aux;
        --m_tamano;
    }

    T& cima() {
        if (vacia()) throw std::underflow_error("pila vacia");
        return m_cima->dato;
    }

    const T& cima() const {
        if (vacia()) throw std::underflow_error("pila vacia");
        return m_cima->dato;
    }

    bool vacia() const { return m_cima == nullptr; }
    std::size_t tamano() const { return m_tamano; }

    void limpiar() {
        while (!vacia()) pop();
    }

private:
    NodoPila<T>* m_cima;
    std::size_t m_tamano;
};

#endif
