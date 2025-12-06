#ifndef INQUISIDOR_H
#define INQUISIDOR_H

#include "personajes.h"
#include <QPoint>
#include <QVector>
#include <QPair>



// Definición de la matriz de celdas para el laberinto: 0=Camino, 1=Muro
using MazeGrid = QVector<QVector<int>>;

class Inquisidor : public Personaje {
public:
    // Constructor. Posición inicial (x, y), tamaño y velocidad
    Inquisidor(float x, float y, float w, float h, float v);

    void dibujar() override;

    // Métodos del Agente Autónomo

    // a) Percepción: Obtiene la posición del objetivo (Jugador)
    QPoint percibirObjetivo(float jugadorX, float jugadorY) const;

    // b) Razonamiento (Pathfinding A*): Calcula la ruta óptima

    QVector<QPoint> calcularRuta(const MazeGrid& grid, QPoint inicio, QPoint fin);

    // c) Acción: Mueve al inquisidor siguiendo la ruta
    void ejecutarAccion(const QVector<QPoint>& ruta, float tileSize);
    const QVector<QPoint>& obtenerRutaActual() const { return rutaActual; }

    void avanzarRuta(float tileSize);
    void establecerRuta(const QVector<QPoint>& nuevaRuta) {
        rutaActual = nuevaRuta;
    }
    QPoint obtenerCelda(float x, float y) const;

private:
    // Almacena la ruta actual calculada por el Inquisidor
    QVector<QPoint> rutaActual;
};

#endif // INQUISIDOR_H
