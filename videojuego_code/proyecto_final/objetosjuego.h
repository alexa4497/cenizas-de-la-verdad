#ifndef OBJETOSJUEGO_H
#define OBJETOSJUEGO_H

#include <QRect>
#include <QPointF>
#include <QVector>
#include <cmath>


// 1. CLASE BASE ObjetoJuego

class ObjetoJuego {
public:
    float pos_x;
    float pos_y;
    int ancho;
    int alto;

    virtual ~ObjetoJuego() {}

    ObjetoJuego(float x, float y, int w, int h)
        : pos_x(x), pos_y(y), ancho(w), alto(h) {}


    virtual void dibujar() = 0;

    // Método de utilidad para colisiones
    QRect getRectanguloColision() const {
        return QRect(static_cast<int>(pos_x), static_cast<int>(pos_y), ancho, alto);
    }
};


// 2. CLASE  Personaje

class Personaje : public ObjetoJuego {
public:
    float velocidad;

    Personaje(float x, float y, int w, int h, float v)
        : ObjetoJuego(x, y, w, h), velocidad(v) {}

    void mover(float dx, float dy) {
        float magnitud = std::sqrt(dx * dx + dy * dy);
        if (magnitud > 0) {
            pos_x += (dx / magnitud) * velocidad;
            pos_y += (dy / magnitud) * velocidad;
        }
    }

};


// CLASE FRAGMENTO
class Fragmento : public ObjetoJuego {
public:
    bool estaSalvado;
    bool estaQuemado;

    Fragmento(float x, float y)
        : ObjetoJuego(x, y, 20, 20), estaSalvado(false), estaQuemado(false) {}

    void dibujar() override {} // Implementación requerida
};


// CLASE MURO
class Muro : public ObjetoJuego {
public:
    Muro(float x, float y, int w, int h)
        : ObjetoJuego(x, y, w, h) {}

    void dibujar() override {} // Implementación requerida
};


// CLASE JUGADOR (Sabio Maya)
class Jugador : public Personaje {
public:
    int fragmentosSalvados;
    qint64 tiempoContactoFragMs;
    bool estaIntentandoRetener;
    Fragmento *fragmentoEnContactoActual;

    Jugador(float x, float y, float w, float h, float vel)
        : Personaje(x, y, w, h, vel),
        fragmentosSalvados(0),
        tiempoContactoFragMs(0),
        estaIntentandoRetener(false),
        fragmentoEnContactoActual(nullptr)
    {}

    void dibujar() override {}
};



// CLASE AGENTE FUEGO
class AgenteFuego : public Personaje {
public:
    AgenteFuego(float x, float y)
        : Personaje(x, y, 60, 60, 1.5f) {}

    void decidirPropagacion(QVector<Fragmento> &fragmentos) {
        if (pos_x < 700) pos_x += velocidad;
    }

    void dibujar() override {}
};

#endif // OBJETOSJUEGO_H
