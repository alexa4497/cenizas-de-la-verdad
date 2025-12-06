#include "personajes.h"
#include "ob.h"
#include <cmath>
#include <algorithm>

Personaje::Personaje(float x, float y, int w, int h, float v)
    : ObjetoJuego(x, y, w, h), velocidad(v) {}

void Personaje:: mover(float dx, float dy) {
    float magnitud = std::sqrt(dx * dx + dy * dy);
    if (magnitud > 0) {
        pos_x += (dx / magnitud) * velocidad;
        pos_y += (dy / magnitud) * velocidad;
    }
}

void Personaje::dibujar() {
}


Jugador:: Jugador(float x, float y, float w, float h, float vel)
    : Personaje(x, y, w, h, vel),
    fragmentosSalvados(0),
    tiempoContactoFragMs(0),
    estaIntentandoRetener(false),
    fragmentoEnContactoActual(nullptr)
{}

Jugador::Jugador(float x, float y)
    // Llama al constructor base de Personaje con valores por defecto
    : Personaje(x, y, 40.0f, 40.0f, 5.0f)
{
    fragmentosSalvados = 0;
    tiempoContactoFragMs = 0;
    estaIntentandoRetener = false;
    fragmentoEnContactoActual = nullptr;
}

void Jugador::dibujar() {

}
