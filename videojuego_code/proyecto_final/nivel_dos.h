#ifndef NIVEL_DOS_H
#define NIVEL_DOS_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QKeyEvent>
#include <QVector>
#include <QPoint>
#include "inquisidor.h"
#include "personajes.h" // Asumiendo que esta es la clase base para Jugador
#include "fragmentos.h"

using MazeGrid = QVector<QVector<int>>;

class Niveldos : public QWidget
{
    Q_OBJECT

public:
    static const int TILE_SIZE = 50;
    static const int ROWS = 15; // 15 filas (750px)
    static const int COLS = 15; // 15 columnas (750px)

    Niveldos(QWidget *parent = nullptr);
    ~Niveldos() override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void gameLoop();
    void actualizarRutaInquisidor();

private:
    QTimer *timerJuego;
    QTimer *timerPathfinding;

    Personaje *jugador;
    Inquisidor *inquisidor;
    MazeGrid laberinto;
    QVector<Fragmento*> fragmentos;

    // Scroll cenital limitado (750 - 600 = 150)
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    const float MAX_SCROLL_X = COLS * TILE_SIZE - 600;
    const float MAX_SCROLL_Y = ROWS * TILE_SIZE - 600;

    void generarLaberinto();
    void actualizarScroll();
    bool esMuro(float x, float y) const;
    QPoint obtenerCelda(float x, float y) const;
    void checkColisiones();
};

#endif // NIVELDOS_H
