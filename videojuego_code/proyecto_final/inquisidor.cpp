#include "inquisidor.h"
#include <QDebug>
#include <algorithm>
#include <cmath>
#include <QQueue>
#include <map>

// Estructura de Comparación para QPoint
struct QPointCompare {
    bool operator()(const QPoint& a, const QPoint& b) const {
        if (a.y() != b.y()) {
            return a.y() < b.y();
        }
        return a.x() < b.x();
    }
};

// Estructura auxiliar para el algoritmo A*
struct CeldaInfo {
    QPoint padre;
    int gScore = 1000000;
    int fScore = 1000000;
};


Inquisidor::Inquisidor(float x, float y, float w, float h, float v)
    : Personaje(x, y, w, h, v) {}


void Inquisidor::dibujar() {

}

QPoint Inquisidor::percibirObjetivo(float jugadorX, float jugadorY) const {
    const float TILE_SIZE = 40.0f; // Asume que el tamaño de la celda es 50x50
    return QPoint(static_cast<int>(jugadorX / TILE_SIZE), static_cast<int>(jugadorY / TILE_SIZE));
}


void Inquisidor::ejecutarAccion(const QVector<QPoint>& ruta, float tileSize) {
    if (ruta.isEmpty() || ruta.size() == 1) {
        return; // No hay ruta o ya está en el destino
    }

    QPoint siguienteCelda = ruta[1];

    // Calcular posición objetivo (centro de la celda)
    float targetX = siguienteCelda.x() * tileSize + tileSize / 2.0f;
    float targetY = siguienteCelda.y() * tileSize + tileSize / 2.0f;

    float dx = targetX - pos_x;
    float dy = targetY - pos_y;
    float distancia = std::sqrt(dx * dx + dy * dy);

    if (distancia > 0) {
        // Movimiento suave hacia el objetivo
        if (distancia <= velocidad) {
            // LlegO al objetivo exacto
            pos_x = targetX;
            pos_y = targetY;
        } else {
            // Movimiento normal
            pos_x += (dx / distancia) * velocidad;
            pos_y += (dy / distancia) * velocidad;
        }
    }
}


// Razonamiento Implementación básica del algoritmo A*
QVector<QPoint> Inquisidor::calcularRuta(const MazeGrid& grid, QPoint inicio, QPoint fin) {

    if (grid.isEmpty() || inicio == fin) {
        return {};
    }

    int rows = grid.size();
    int cols = grid[0].size();

    // Verificar lImites de inicio y fin
    if (inicio.x() < 0 || inicio.x() >= cols || inicio.y() < 0 || inicio.y() >= rows ||
        fin.x() < 0 || fin.x() >= cols || fin.y() < 0 || fin.y() >= rows) {
        return {};
    }

    // Verificar si son muros
    if (grid[inicio.y()][inicio.x()] == 1 || grid[fin.y()][fin.x()] == 1) {
        return {};
    }

    // Estructura y Cola Abierta (sin cambios)
    std::map<QPoint, CeldaInfo, QPointCompare> celdaInfo;
    QVector<QPoint> listaAbierta;

    auto h_score = [&](QPoint p) {
        // Distancia de Manhattan (correcta)
        return std::abs(p.x() - fin.x()) + std::abs(p.y() - fin.y());
    };

    // Inicializar inicio (sin cambios)
    celdaInfo[inicio].gScore = 0;
    celdaInfo[inicio].fScore = h_score(inicio);
    listaAbierta.append(inicio);

    while (!listaAbierta.isEmpty()) {
        auto it_actual = std::min_element(listaAbierta.begin(), listaAbierta.end(),
                                          [&](const QPoint& a, const QPoint& b) {

                                              // 1. COMPARACION PRINCIPAL: Por fScore (menor es mejor)
                                              if (celdaInfo.at(a).fScore != celdaInfo.at(b).fScore) {
                                                  return celdaInfo.at(a).fScore < celdaInfo.at(b).fScore;
                                              }

                                              // 2. DESEMPATE CRITICO: Por hScore ( más cerca del objetivo)

                                              int hA = celdaInfo.at(a).fScore - celdaInfo.at(a).gScore;
                                              int hB = celdaInfo.at(b).fScore - celdaInfo.at(b).gScore;


                                              return hA < hB;
                                          });

        QPoint actual = *it_actual;
        listaAbierta.erase(it_actual);

        // Si encontramos el objetivo (sin cambios)
        if (actual == fin) {
            QVector<QPoint> ruta;
            QPoint curr = fin;
            while (curr != inicio) {
                ruta.prepend(curr);
                curr = celdaInfo.at(curr).padre;
            }

            rutaActual = ruta;
            ruta.prepend(inicio);
            return ruta;
        }

        // Definir vecinos (sin cambios)
        QVector<QPoint> vecinos = {
            QPoint(actual.x() + 1, actual.y()),
            QPoint(actual.x() - 1, actual.y()),
            QPoint(actual.x(), actual.y() + 1),
            QPoint(actual.x(), actual.y() - 1)
        };

        int rows = grid.size();
        int cols = grid[0].size();
        int nuevaGScore = celdaInfo.at(actual).gScore + 1;

        // 2. PROCESAR VECINOS
        for (const auto& vecino : vecinos) {

            // Comprobación de lImites
            if (vecino.x() < 0 || vecino.x() >= cols ||
                vecino.y() < 0 || vecino.y() >= rows)
            {
                continue;
            }

            // Comprobación de muro
            if (grid[vecino.y()][vecino.x()] == 1)
            {
                continue;
            }

            // Si encontramos un mejor camino o es la primera vez que lo visitamos
            if (celdaInfo.find(vecino) == celdaInfo.end() || nuevaGScore < celdaInfo.at(vecino).gScore) {

                CeldaInfo& info = celdaInfo[vecino];

                info.padre = actual;
                info.gScore = nuevaGScore;
                info.fScore = nuevaGScore + h_score(vecino);

                if (!listaAbierta.contains(vecino)) {
                    listaAbierta.append(vecino);
                }
            }
        }
    }

    return {};
}


void Inquisidor::avanzarRuta(float tileSize) {
    if (rutaActual.size() <= 1) {
        return;
    }

    QPoint nextCell = rutaActual[1];

    // 1. Calcular el punto objetivo (superior izquierdo del personaje centrado en la celda)
    float targetX = nextCell.x() * tileSize + tileSize / 2.0f - getAncho() / 2.0f;
    float targetY = nextCell.y() * tileSize + tileSize / 2.0f - getAlto() / 2.0f;
    const float TOLERANCIA = 2.0f;

    // 3. Verifica si la posición actual ha alcanzado el centro del nodo objetivo
    // Usamos la distancia euclidiana para ser más precisos en la cercanía:
    float dx = getPos_x() - targetX;
    float dy = getPos_y() - targetY;
    float distancia_sq = dx * dx + dy * dy;

    if (distancia_sq <= (TOLERANCIA * TOLERANCIA))
    {
        // Forzar la posición al destino EXACTO.
        pos_x = targetX;
        pos_y = targetY;
        rutaActual.removeFirst();
    }
}
