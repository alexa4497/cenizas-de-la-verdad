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
#include <QPushButton>  // Añadido

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

signals:
    void regresarMenuPrincipal();  // Señal para regresar al menú principal

private slots:
    void actualizarJuego();
    void reiniciarNivel();
    void actualizarAnimacionFuego();
    void actualizarAnimacionMaya();
    void onSalirClicked();  // Nuevo slot para el botón salir

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
    //  VARIABLES PARA LA ANIMACIÓN DEL SABIO MAYA (MODIFICADO: 4x6)
    // ------------------------------------------------------------------
    QPixmap fullSpriteSheetMaya;
    QVector<QPixmap> framesMaya;
    int currentFrameMayaIndex;

    // Constantes para el nuevo sprite sheet (4x6)
    const int SPRITE_ROWS = 4;    // 4 filas
    const int SPRITE_COLUMNS = 6; // 6 columnas
    const int SPRITE_WIDTH = 60;  // Ancho de cada frame (360/6 = 60)
    const int SPRITE_HEIGHT = 60; // Alto de cada frame (240/4 = 60)

    QTimer *timerAnimacionMaya;
    int animRowOffset;
    bool isMoving;

    // Botón de salir
    QPushButton *btnSalir;  // Nuevo botón

    void moverJugador(float dx, float dy);
    void verificarColisiones();
    void verificarEstadoJuego();

    void inicializarFragmentos();
    void inicializarMuros();
    void inicializarZonasFuego();
    void inicializarUI();  // Nuevo método para inicializar UI
};

#endif // NIVELUNO_H
