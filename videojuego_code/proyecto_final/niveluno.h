#ifndef NIVELUNO_H
#define NIVELUNO_H

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <QPixmap>
#include <QKeyEvent>
#include <QMediaPlayer>
#include <QAudioOutput>

#include "ob.h"
#include "personajes.h"
#include "fuego.h"
#include "obstaculos.h"
#include "fragmentos.h"


namespace Ui {
class Niveluno;
}

class Niveluno : public QWidget
{
    Q_OBJECT

public:
    explicit Niveluno(QWidget *parent = nullptr);
    ~Niveluno();
    QMediaPlayer *musicaFondo;
    QAudioOutput *audioOutput;


private slots:
    void actualizarJuego();
    void reiniciarNivel();
    void actualizarAnimacionFuego();
    void actualizarAnimacionMaya();



protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    Ui::Niveluno *ui;

    // Atributos de la Lógica del Juego
    QTimer *timerJuego;
    QElapsedTimer timerElapsedJuego;
    QTimer *timerReinicio;

    // Clases de juego
    Jugador sabioMaya;
    AgenteFuego agenteFuego;

    QVector<Fragmento> fragmentos;
    QVector<Muro> muros;
    QVector<Muro> zonasFuego;

    int juegoEstado;

    // Constantes
    const int FRAGMENTOS_REQUERIDOS;
    const int TIEMPO_LIMITE_SEGUNDOS;
    const qint64 TIEMPO_RETENCION_MS;

    // ------------------------------------------------------------------
    // VARIABLES PARA LA ANIMACIÓN DEL FUEGO
    // ------------------------------------------------------------------
    QVector<QPixmap> framesFuego;
    int currentFrameFuegoIndex;
    QTimer *timerAnimacionFuego;

    // ------------------------------------------------------------------
    //  VARIABLES PARA LA ANIMACIÓN DEL SABIO MAYA
    // ------------------------------------------------------------------
    QPixmap fullSpriteSheetMaya;
    QVector<QPixmap> framesMaya;
    int currentFrameMayaIndex;

    QTimer *timerAnimacionMaya;
    int animRowOffset;
    bool isMoving;

    void moverJugador(float dx, float dy);
    void verificarColisiones();
    void verificarEstadoJuego();

    void inicializarFragmentos();
    void inicializarMuros();
    void inicializarZonasFuego();
};

#endif // NIVELUNO_H
