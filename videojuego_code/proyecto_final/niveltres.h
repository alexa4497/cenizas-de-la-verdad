#ifndef NIVELTRES_H
#define NIVELTRES_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include <cmath>
#include <QPixmap>
#include <QVector>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QPushButton>
#include "personajes.h"
#include "fragmentos.h"

class LibroProhibido : public Fragmento {
public:
    int tipoMovimiento;
    float tiempo;
    float xInicial, yInicial;
    float angulo;
    float radio;

    LibroProhibido(float x, float y, int tipo);
    void actualizar();
    void dibujar() override {}
};

class Niveltres : public QWidget
{
    Q_OBJECT

public:
    explicit Niveltres(QWidget *parent = nullptr);
    ~Niveltres();

signals:
    void regresarMenuPrincipal();  // MOVIDO A LA CLASE CORRECTA

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void gameLoop();
    void actualizarAnimacionJugador();
    void onSalirClicked();  // CAMBIADO A PRIVATE SLOTS

private:
    QTimer *timerJuego;
    QTimer *timerAnimacion;
    Jugador *jugador;
    LibroProhibido* libros[15];

    int librosCapturados = 0;
    const int LIBROS_TOTAL = 15;
    float velocidadY = 0;

    // Para scroll
    float offsetY = 0;
    int MUNDO_ALTO = 1000;
    QPushButton *btnSalir;  // AÑADIDO

    // Para animación del jugador
    QPixmap spriteSheetJugador;
    QVector<QPixmap> framesJugador;
    int frameActualJugador;
    int direccionJugador;
    bool estaSaltando;
    int frameIndex;

    // Imágenes y texturas
    QPixmap fondo;
    QPixmap texturaMuro;
    QPixmap imgLibroInfinito;
    QPixmap imgLibroCircular;
    QPixmap imgLibroEspiral;

    // Sonidos
    QMediaPlayer *sonidoSalto;
    QMediaPlayer *musicaFondo;
    QAudioOutput *audioOutputMusica;
    QAudioOutput *audioOutputEfectos;

    void checkColisiones();
    void cargarImagenes();
    void cargarSonidos();
    void inicializarUI();  // AÑADIDO
};

#endif // NIVELTRES_H
