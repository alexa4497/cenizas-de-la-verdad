#ifndef NIVELDOS_H
#define NIVELDOS_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QKeyEvent>
#include <QVector>
#include <QPoint>
#include <QPixmap>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QPushButton>

#include "inquisidor.h"
#include "personajes.h"
#include "fragmentos.h"
#include "ob.h"

using MazeGrid = QVector<QVector<int>>;

class Niveldos : public QWidget
{
    Q_OBJECT

public:
    static const int TILE_SIZE = 50;
    static const int ROWS = 21;
    static const int COLS = 30;

    explicit Niveldos(QWidget *parent = nullptr);
    ~Niveldos() override;

signals:
    void regresarMenuPrincipal();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

public slots:
    void gameLoop();
    void actualizarRutaInquisidor();
    void actualizarAnimaciones();
    void mostrarMenuFinJuego(const QString& titulo, const QString& mensaje);
    void onSalirClicked();
    void reiniciarNivel();

private:
    QTimer *timerJuego;
    QTimer *timerPathfinding;
    QTimer *timerAnimacion;

    // Audio
    QMediaPlayer *musicaFondo;
    QAudioOutput *audioOutput;

    Jugador *jugador;  // Cambié de Personaje* a Jugador*
    Inquisidor *inquisidor;
    MazeGrid laberinto;
    QVector<Fragmento*> fragmentos;

    QPushButton *btnSalir;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    const float MAX_SCROLL_X = COLS * TILE_SIZE - 600;
    const float MAX_SCROLL_Y = ROWS * TILE_SIZE - 600;

    // Animaciones
    QPixmap spriteJugador;
    QVector<QPixmap> framesJugador;
    QPixmap spriteInquisidor;
    QVector<QPixmap> framesInquisidor;

    int frameIndexJugador = 0;
    int frameIndexInquisidor = 0;
    int direccionJugador = 2;
    int direccionInquisidor = 2;

    // Texturas
    QPixmap texturaPared;
    QPixmap texturaSuelo;
    QPixmap texturaFragmento;

    // Funciones de carga
    bool cargarTexturas();
    bool cargarSprites();
    void cargarAudio();

    // Funciones auxiliares
    void generarLaberinto();
    void actualizarScroll();
    bool esMuro(float x, float y) const;
    QPoint obtenerCelda(float x, float y) const;
    void checkColisiones();
    QPixmap obtenerFrameJugador();
    QPixmap obtenerFrameInquisidor();
    void inicializarUI();
    bool hayColision(const QRect& rect1, const QRect& rect2) const;
};

#endif // NIVELDOS_H
