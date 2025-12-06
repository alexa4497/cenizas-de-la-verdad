#include "niveluno.h"
#include "ui_niveluno.h"
#include <QDebug>
#include <QPainter>
#include <QKeyEvent>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <QImage>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

const int NIVEL_WIDTH = 1250;
const int NIVEL_HEIGHT = 650;

Niveluno::Niveluno(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Niveluno)
    , sabioMaya(600.0f, 80.0f, 60.0f, 60.0f, 4.0f)
    , agenteFuego(2000.0f, 2000.0f)
    , FRAGMENTOS_REQUERIDOS(8)
    , TIEMPO_LIMITE_SEGUNDOS(100)
    , TIEMPO_RETENCION_MS(3000)
    , juegoEstado(0)
    , SPRITE_ROWS(4)
    , SPRITE_COLUMNS(6)
    , SPRITE_WIDTH(60)
    , SPRITE_HEIGHT(60)
{
    ui->setupUi(this);
    this->setMinimumSize(NIVEL_WIDTH, NIVEL_HEIGHT);
    setFocusPolicy(Qt::StrongFocus);

    // --- Timers Principales ---
    timerJuego = new QTimer(this);
    connect(timerJuego, &QTimer::timeout, this, &Niveluno::actualizarJuego);

    timerReinicio = new QTimer(this);
    connect(timerReinicio, &QTimer::timeout, this, &Niveluno::reiniciarNivel);

    inicializarFragmentos();
    inicializarMuros();
    inicializarZonasFuego();
    inicializarUI();

    // CARGA Y ANIMACION DEL FUEGO
    QString spriteSheetPath = ":/imagenes/multimedia/imagenes/sprites_fuego.png";
    QPixmap fullSpriteSheet(spriteSheetPath);
    framesFuego.clear();

    if (!fullSpriteSheet.isNull()) {
        int frameWidth = 100;
        int frameHeight = 100;
        framesFuego.append(fullSpriteSheet.copy(0 * frameWidth, 0, frameWidth, frameHeight));
        framesFuego.append(fullSpriteSheet.copy(1 * frameWidth, 0, frameWidth, frameHeight));
        framesFuego.append(fullSpriteSheet.copy(2 * frameWidth, 0, frameWidth, frameHeight));
        framesFuego.append(fullSpriteSheet.copy(3 * frameWidth, 0, frameWidth, frameHeight));
    }

    currentFrameFuegoIndex = 0;
    timerAnimacionFuego = new QTimer(this);
    connect(timerAnimacionFuego, &QTimer::timeout, this, &Niveluno::actualizarAnimacionFuego);
    if (!framesFuego.isEmpty()) {
        timerAnimacionFuego->start(200);
    }

    // ** B. CARGA Y ANIMACION DEL SABIO MAYA
    QString spriteSheetPathMaya = ":/imagenes/multimedia/imagenes/sprite_sabiomaya.png";
    fullSpriteSheetMaya.load(spriteSheetPathMaya);
    framesMaya.clear();
    currentFrameMayaIndex = 0;

    if (!fullSpriteSheetMaya.isNull()) {
        // Verificar que el sprite sheet tenga las dimensiones correctas (360x240)
        if (fullSpriteSheetMaya.width() == 360 && fullSpriteSheetMaya.height() == 240) {
            qDebug() << "Sprite sheet del Sabio Maya cargado correctamente: 360x240 (4x6 frames)";

            // Dividir el sprite sheet en 4 filas x 6 columnas
            for (int row = 0; row < SPRITE_ROWS; ++row) {
                for (int col = 0; col < SPRITE_COLUMNS; ++col) {
                    QPixmap frame = fullSpriteSheetMaya.copy(
                        col * SPRITE_WIDTH,
                        row * SPRITE_HEIGHT,
                        SPRITE_WIDTH,
                        SPRITE_HEIGHT
                        );
                    framesMaya.append(frame);
                }
            }
            qDebug() << "Sprite sheet dividido en" << framesMaya.size() << "frames (60x60 cada uno).";
        } else {
            qDebug() << "ADVERTENCIA: El sprite sheet no tiene las dimensiones esperadas 360x240. Tiene:"
                     << fullSpriteSheetMaya.width() << "x" << fullSpriteSheetMaya.height();

            // Fallback: dividir proporcionalmente
            int frameWidth = fullSpriteSheetMaya.width() / SPRITE_COLUMNS;
            int frameHeight = fullSpriteSheetMaya.height() / SPRITE_ROWS;

            for (int row = 0; row < SPRITE_ROWS; ++row) {
                for (int col = 0; col < SPRITE_COLUMNS; ++col) {
                    QPixmap frame = fullSpriteSheetMaya.copy(
                        col * frameWidth,
                        row * frameHeight,
                        frameWidth,
                        frameHeight
                        );
                    framesMaya.append(frame);
                }
            }
            qDebug() << "División proporcional completada. Total frames:" << framesMaya.size();
        }
    } else {
        qDebug() << "ERROR CRÍTICO: No se pudo cargar el sprite sheet del Sabio Maya.";
    }

    // Inicializar variables de estado y animacion
    animRowOffset = 0;
    isMoving = false;

    // Timer para la animacion de caminata
    timerAnimacionMaya = new QTimer(this);
    connect(timerAnimacionMaya, &QTimer::timeout, this, &Niveluno::actualizarAnimacionMaya);
    if (!framesMaya.isEmpty()) {
        timerAnimacionMaya->start(65);
    }

    // ----------------------------------------------------
    // MUSICA DE FONDO
    // ----------------------------------------------------
    audioOutput = new QAudioOutput(this);
    audioOutput->setVolume(0.5);

    musicaFondo = new QMediaPlayer(this);
    musicaFondo->setAudioOutput(audioOutput);
    musicaFondo->setSource(QUrl("qrc:/sonido/multimedia/audio/audio_nivel1.ogg"));

    connect(musicaFondo, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            musicaFondo->play();
        }
    });

    // Iniciar la reproducción inmediatamente
    musicaFondo->play();

    // --- Inicio del Juego
    timerElapsedJuego.start();
    timerJuego->start(16);
}

void Niveluno::inicializarUI() {
    // Crear y configurar el botón de salir
    btnSalir = new QPushButton("Salir", this);
    btnSalir->setGeometry(NIVEL_WIDTH - 100, 10, 80, 30);
    btnSalir->setStyleSheet(
        "QPushButton {"
        "    background-color: #d32f2f;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    font-weight: bold;"
        "    font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #b71c1c;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #7f0000;"
        "}"
        );


    connect(btnSalir, &QPushButton::clicked, this, &Niveluno::onSalirClicked);
}

void Niveluno::onSalirClicked() {
    // Detener timers y música
    if (timerJuego && timerJuego->isActive()) {
        timerJuego->stop();
    }
    if (timerAnimacionMaya && timerAnimacionMaya->isActive()) {
        timerAnimacionMaya->stop();
    }
    if (timerAnimacionFuego && timerAnimacionFuego->isActive()) {
        timerAnimacionFuego->stop();
    }
    if (timerReinicio && timerReinicio->isActive()) {
        timerReinicio->stop();
    }

    if (musicaFondo) {
        musicaFondo->stop();
    }

    qDebug() << "Saliendo del nivel 1...";

    // Emitir señal para regresar al menú principal
    emit regresarMenuPrincipal();

    // CERRAR LA VENTANA - AÑADE ESTA LÍNEA
    this->close();
}

Niveluno::~Niveluno() {
    if (musicaFondo) {
        musicaFondo->stop();
    }
    delete ui;
}

// IMPLEMENTACIÓN DE MÉTODOS DE INICIALIZACIÓN

void Niveluno::inicializarFragmentos() {
    fragmentos.clear();
    fragmentos.append(Fragmento(1165, 65));
    fragmentos.append(Fragmento(705, 550));
    fragmentos.append(Fragmento(1085, 340));
    fragmentos.append(Fragmento(1175, 550));
    fragmentos.append(Fragmento(160, 60));
    fragmentos.append(Fragmento(70, 140));
    fragmentos.append(Fragmento(150, 220));
    fragmentos.append(Fragmento(300, 555));
}

void Niveluno::inicializarMuros() {
    muros.clear();
    // Grosor del muro
    const int muro_grosor = 40;
    const int pilar_ancho = 60;

    // 1. Muros del Borde Exterior
    muros.append(Muro(0, 0, NIVEL_WIDTH, muro_grosor));
    muros.append(Muro(0, NIVEL_HEIGHT - muro_grosor, NIVEL_WIDTH, muro_grosor));
    muros.append(Muro(0, muro_grosor, muro_grosor, NIVEL_HEIGHT - 2 * muro_grosor));
    muros.append(Muro(NIVEL_WIDTH - muro_grosor, muro_grosor, muro_grosor, NIVEL_HEIGHT - 2 * muro_grosor));

    // 2. Muros Interiores
    const int pilar_altura_lateral = NIVEL_HEIGHT - 2 * muro_grosor - 100;
    const int pilar_altura_central = NIVEL_HEIGHT - 2 * muro_grosor - 200;

    int x_central = NIVEL_WIDTH / 2 - pilar_ancho / 2;
    int y_central_pegado = NIVEL_HEIGHT - muro_grosor - pilar_altura_central;
    muros.append(Muro(x_central, y_central_pegado, pilar_ancho, pilar_altura_central));
    const int muro_perpendicular_ancho = 350;
    const int muro_perpendicular_grosor = 50;
    const int y_muro_horizontal = y_central_pegado - muro_perpendicular_grosor;
    int x_inicio_horizontal = x_central + (pilar_ancho / 2) - (muro_perpendicular_ancho / 2);

    muros.append(Muro(x_inicio_horizontal, y_muro_horizontal, muro_perpendicular_ancho, muro_perpendicular_grosor));
    int x_izquierdo = NIVEL_WIDTH / 4 - pilar_ancho / 2;
    muros.append(Muro(x_izquierdo, muro_grosor, pilar_ancho, pilar_altura_lateral));

    int x_derecho = (NIVEL_WIDTH * 3) / 4 - pilar_ancho / 2;
    muros.append(Muro(x_derecho, muro_grosor, pilar_ancho, pilar_altura_lateral));
}

//ANIMACIÓN DEL SABIO MAYA

void Niveluno::actualizarAnimacionMaya() {
    if (isMoving) {
        // Calcular el índice de la columna actual (0-5)
        int currentColumnIndex = currentFrameMayaIndex % SPRITE_COLUMNS;

        // Avanzar a la siguiente columna
        currentColumnIndex = (currentColumnIndex + 1) % SPRITE_COLUMNS;

        // Establecer el índice global combinando columna + fila
        currentFrameMayaIndex = currentColumnIndex + animRowOffset;

        // Asegurarse de que el índice esté dentro del rango
        if (currentFrameMayaIndex >= framesMaya.size()) {
            currentFrameMayaIndex = animRowOffset;
        }

        update();
    }
}

void Niveluno::inicializarZonasFuego() {
    zonasFuego.clear();

    const int pilar_ancho = 60;
    const int x_central = NIVEL_WIDTH / 2 - pilar_ancho / 2;

    const int fuego_grosor_borde_ajustado = 35; // Grosor deseado (35px)
    const int fuego_inferior_grosor = 70; // Fuego inferior (70px)
    const int y_inferior = NIVEL_HEIGHT - fuego_inferior_grosor; //  (580)

    const int Y_INICIO_MURO = 175;
    const int GROSOR_MURO = 50;

    const int Y_FUEGO_SUPERIOR = Y_INICIO_MURO - fuego_grosor_borde_ajustado; // 175 - 35 = 140
    const int Y_FUEGO_INFERIOR_INICIO = Y_INICIO_MURO + GROSOR_MURO; // 175 + 50 = 225

    const int X_FUEGO_IZQ = 450 - fuego_grosor_borde_ajustado; // 415
    const int X_FUEGO_DER = 800;
    const int ANCHO_TOTAL_FUEGO = 350 + 2 * fuego_grosor_borde_ajustado; // 420
    const int ALTO_TOTAL_FUEGO = GROSOR_MURO + 2 * fuego_grosor_borde_ajustado; // 120

    // 1. ZONA INFERIOR
    zonasFuego.append(Muro(0, y_inferior, x_central, fuego_inferior_grosor));
    zonasFuego.append(Muro(x_central + pilar_ancho, y_inferior, NIVEL_WIDTH - (x_central + pilar_ancho), fuego_inferior_grosor));

    // 2. FUEGO QUE RODEA LA BARRA HORIZONTAL
    zonasFuego.append(Muro(X_FUEGO_IZQ, Y_FUEGO_SUPERIOR, ANCHO_TOTAL_FUEGO, fuego_grosor_borde_ajustado));
    zonasFuego.append(Muro(X_FUEGO_IZQ, Y_FUEGO_INFERIOR_INICIO, ANCHO_TOTAL_FUEGO, fuego_grosor_borde_ajustado));
    zonasFuego.append(Muro(X_FUEGO_IZQ, Y_FUEGO_SUPERIOR, fuego_grosor_borde_ajustado, ALTO_TOTAL_FUEGO));
    zonasFuego.append(Muro(X_FUEGO_DER, Y_FUEGO_SUPERIOR, fuego_grosor_borde_ajustado, ALTO_TOTAL_FUEGO));

    // 3. FUEGO QUE RODEA EL PILAR VERTICAL
    const int Y_INICIO_FUEGO_PILAR = Y_FUEGO_INFERIOR_INICIO + fuego_grosor_borde_ajustado; // 225 + 35 = 260
    const int Y_FINAL_FUEGO_PILAR = y_inferior; // 580
    const int altura_fuego_pilar = Y_FINAL_FUEGO_PILAR - Y_INICIO_FUEGO_PILAR; // 580 - 260 = 320
    const int X_BORDE_IZQ_PILAR = x_central - fuego_grosor_borde_ajustado; // 595 - 35 = 560
    const int X_BORDE_DER_PILAR = x_central + pilar_ancho; // 595 + 60 = 655

    zonasFuego.append(Muro(X_BORDE_IZQ_PILAR, Y_INICIO_FUEGO_PILAR, fuego_grosor_borde_ajustado, altura_fuego_pilar));
    zonasFuego.append(Muro(X_BORDE_DER_PILAR, Y_INICIO_FUEGO_PILAR, fuego_grosor_borde_ajustado, altura_fuego_pilar));

    // 4. ZONAS CUADRADAS SUPERIORES
    zonasFuego.append(Muro(120, 100, 100, 100));
    zonasFuego.append(Muro(1050, 100, 100, 100));
}

// ----------------------------------------------------
// IMPLEMENTACIÓN DE MÉTODOS DE LÓGICA Y JUEGO
// ----------------------------------------------------

void Niveluno::moverJugador(float dx, float dy) {
    sabioMaya.mover(dx, dy);

    int tamJugador = sabioMaya.ancho;
    sabioMaya.pos_x = std::min(sabioMaya.pos_x, (float)NIVEL_WIDTH - tamJugador);
    sabioMaya.pos_x = std::max(sabioMaya.pos_x, 0.0f);
    sabioMaya.pos_y = std::min(sabioMaya.pos_y, (float)NIVEL_HEIGHT - tamJugador);
    sabioMaya.pos_y = std::max(sabioMaya.pos_y, 0.0f);
}

void Niveluno::actualizarJuego() {
    if (juegoEstado != 0) return;

    verificarColisiones();
    verificarEstadoJuego();

    update();
}

void Niveluno::verificarColisiones() {
    QRect rectJugador = sabioMaya.getRectanguloColision();
    qint64 tiempoActual = timerElapsedJuego.elapsed();

    // 1. Encontrar el fragmento en contacto
    sabioMaya.fragmentoEnContactoActual = nullptr;

    for (Fragmento &f : fragmentos) {
        if (f.estaSalvado) continue;

        const int BUFFER = 10;
        QRect rectFragmentoAmpliado = f.getRectanguloColision().adjusted(-BUFFER, -BUFFER, BUFFER, BUFFER);

        if (rectJugador.intersects(rectFragmentoAmpliado)) {
            sabioMaya.fragmentoEnContactoActual = &f;
            break;
        }
    }

    // LÓGICA DE RESETEO CRÍTICO Y TIEMPO DE RETENCIÓN
    if (sabioMaya.estaIntentandoRetener && sabioMaya.fragmentoEnContactoActual == nullptr) {
        if (sabioMaya.tiempoContactoFragMs != 0) {
            sabioMaya.tiempoContactoFragMs = 0;
        }
    }

    // Lógica de conteo de retención
    bool debeContar = sabioMaya.estaIntentandoRetener && (sabioMaya.fragmentoEnContactoActual != nullptr);

    qint64 tiempoTranscurrido = 0;

    if (debeContar) {
        // 1. INICIO DEL CONTEO
        if (sabioMaya.tiempoContactoFragMs == 0) {
            sabioMaya.tiempoContactoFragMs = tiempoActual;
            qDebug() << "INICIO CONTEO OK en fragmento: " << tiempoActual << "ms";
        }

        // 2. CÁLCULO DEL TIEMPO TRANSCURRIDO
        if (sabioMaya.tiempoContactoFragMs != 0) {
            tiempoTranscurrido = tiempoActual - sabioMaya.tiempoContactoFragMs;
        }

        if (tiempoTranscurrido >= TIEMPO_RETENCION_MS) {
            if (sabioMaya.fragmentoEnContactoActual != nullptr) {
                sabioMaya.fragmentoEnContactoActual->estaSalvado = true;
                sabioMaya.fragmentosSalvados++;
                qDebug() << "C. ¡FRAGMENTO RECOGIDO CON ÉXITO! Total:" << sabioMaya.fragmentosSalvados;
            } else {
                qDebug() << "ERROR FINAL: Intento de recolección sin puntero, reseteo.";
            }

            // Resetear el estado de la clase inmediatamente
            sabioMaya.tiempoContactoFragMs = 0;
            sabioMaya.estaIntentandoRetener = false;
            sabioMaya.fragmentoEnContactoActual = nullptr;
            return;

        } else {
            // DIAGNÓSTICO DE PROGRESO
            if (sabioMaya.tiempoContactoFragMs != 0) {
                qDebug() << "B. PROGRESO: " << tiempoTranscurrido << "ms /" << TIEMPO_RETENCION_MS << "ms";
            }
        }

    } else {
        // RESETEO DEL CONTEO: Si suelta 'E' o se pierde contacto
        if (sabioMaya.tiempoContactoFragMs != 0) {
            sabioMaya.tiempoContactoFragMs = 0;
        }
    }

    // --------------------------------------------------------------------------
    // Colisiones con Muros
    // --------------------------------------------------------------------------
    if (!sabioMaya.estaIntentandoRetener) {
        for (const Muro &m : muros) {
            if (rectJugador.intersects(m.getRectanguloColision())) {
                float factorRebote = 0.5f;
                sabioMaya.pos_x -= sabioMaya.velocidad * factorRebote * (sabioMaya.pos_x < m.pos_x ? 1 : -1);
                sabioMaya.pos_y -= sabioMaya.velocidad * factorRebote * (sabioMaya.pos_y < m.pos_y ? 1 : -1);
            }
        }
    }

    // --------------------------------------------------------------------------
    // Colisiones con Zonas de Fuego Fijas
    // --------------------------------------------------------------------------
    for (const Muro &fuego : zonasFuego) {
        if (rectJugador.intersects(fuego.getRectanguloColision())) {
            if (juegoEstado == 0) { // Solo si estábamos jugando
                timerJuego->stop();
                juegoEstado = 2; // 2 = DERROTA
                timerReinicio->start(4000); // Reiniciar en 4 segundos
            }
            return;
        }
    }
}

// Implementación del slot actualizarAnimacionFuego

void Niveluno::actualizarAnimacionFuego() {
    if (framesFuego.size() < 2) {
        return;
    }
    currentFrameFuegoIndex = (currentFrameFuegoIndex + 1) % framesFuego.size();

    update();
}

// ----------------------------------------------------
// IMPLEMENTACIÓN DE EVENTOS TECLADO
// ----------------------------------------------------

void Niveluno::keyPressEvent(QKeyEvent *event) {
    if (juegoEstado != 0) return;
    float dx = 0.0f;
    float dy = 0.0f;

    // 1. Manejo del intento de retención ('E')
    if (event->key() == Qt::Key_E) {
        if (event->isAutoRepeat()) return;
        sabioMaya.estaIntentandoRetener = true;
    }

    if (!sabioMaya.estaIntentandoRetener) {
        // --- 2.1 Determinar dirección y movimiento ---
        // ORDEN ESPECIFICADO:
        // Fila 0: S (abajo) - Primera fila
        // Fila 1: W (arriba) - Segunda fila
        // Fila 2: D (derecha) - Tercera fila
        // Fila 3: A (izquierda) - Cuarta fila

        if (event->key() == Qt::Key_S) {  // PRIMERA FILA: S (abajo)
            dy = 1.0f;
            animRowOffset = SPRITE_COLUMNS * 0;  // Fila 0: Abajo (frames 0-5)
        }
        if (event->key() == Qt::Key_W) {  // SEGUNDA FILA: W (arriba)
            dy = -1.0f;
            animRowOffset = SPRITE_COLUMNS * 1;  // Fila 1: Arriba (frames 6-11)
        }
        if (event->key() == Qt::Key_D) {  // TERCERA FILA: D (derecha)
            dx = 1.0f;
            animRowOffset = SPRITE_COLUMNS * 2;  // Fila 2: Derecha (frames 12-17)
        }
        if (event->key() == Qt::Key_A) {  // CUARTA FILA: A (izquierda)
            dx = -1.0f;
            animRowOffset = SPRITE_COLUMNS * 3;  // Fila 3: Izquierda (frames 18-23)
        }

        if (dx != 0.0f || dy != 0.0f) {
            moverJugador(dx, dy);

            // --- 2.2 Lógica de Animación
            isMoving = true;

            // Establecer el primer frame de la nueva dirección inmediatamente
            currentFrameMayaIndex = animRowOffset;

            // Asegurarse de que el índice esté dentro del rango
            if (currentFrameMayaIndex >= framesMaya.size()) {
                currentFrameMayaIndex = animRowOffset;
            }
        } else {
            isMoving = false;
        }
    }

    update();
}

void Niveluno::verificarEstadoJuego() {
    int tiempoRestante = TIEMPO_LIMITE_SEGUNDOS - (timerElapsedJuego.elapsed() / 1000);

    // ----------------------
    // LÓGICA DE VICTORIA
    // ----------------------
    if (sabioMaya.fragmentosSalvados >= FRAGMENTOS_REQUERIDOS) {
        if (juegoEstado == 0) { // Solo si estábamos jugando
            timerJuego->stop();
            juegoEstado = 1; // 1 = VICTORIA
            timerReinicio->start(4000);
        }
        return;
    }

    // ----------------------
    // LÓGICA DE DERROTA POR TIEMPO
    // ----------------------
    if (tiempoRestante <= 0) {
        if (juegoEstado == 0) {
            timerJuego->stop();
            juegoEstado = 2; // 2 = DERROTA
            timerReinicio->start(4000);
        }
        return;
    }
}

void Niveluno::keyReleaseEvent(QKeyEvent *event) {
    if (juegoEstado != 0) return; // No procesar si el juego terminó
    if (event->isAutoRepeat()) return;

    // 1. Lógica para soltar la tecla de Retención ('E')
    if (event->key() == Qt::Key_E) {
        sabioMaya.estaIntentandoRetener = false;
        sabioMaya.tiempoContactoFragMs = 0;
    }

    if (event->key() == Qt::Key_W || event->key() == Qt::Key_S ||
        event->key() == Qt::Key_A || event->key() == Qt::Key_D) {

        isMoving = false;
        currentFrameMayaIndex = animRowOffset;

        // Asegurarse de que el índice esté dentro del rango
        if (currentFrameMayaIndex >= framesMaya.size()) {
            currentFrameMayaIndex = animRowOffset;
        }

        update();
    }

    QWidget::keyReleaseEvent(event);
}

// ----------------------------------------------------
// IMPLEMENTACIÓN DE MÉTODOS DE REINICIO
// ----------------------------------------------------

void Niveluno::reiniciarNivel() {
    timerReinicio->stop();

    sabioMaya = Jugador(600.0f, 80.0f, 60.0f, 60.0f, 4.0f);

    juegoEstado = 0;
    timerElapsedJuego.restart();

    inicializarFragmentos();

    animRowOffset = 0;
    isMoving = false;
    currentFrameMayaIndex = 0;

    timerJuego->start(16);

    qDebug() << "Juego Reiniciado y listo para jugar.";
    update();
}

void Niveluno::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    // 1. CÁLCULO Y APLICACIÓN DEL ESCALADO
    qreal scaleX = width() / (qreal)NIVEL_WIDTH;
    qreal scaleY = height() / (qreal)NIVEL_HEIGHT;
    painter.scale(scaleX, scaleY);

    // Fondo
    QPixmap background_pixmap(":/imagenes/multimedia/imagenes/fondo_nivel1.jpg");
    QBrush background_brush(background_pixmap);
    background_brush.setStyle(Qt::TexturePattern);
    painter.fillRect(0, 0, NIVEL_WIDTH, NIVEL_HEIGHT, background_brush);

    // dibujo Muros
    QPixmap muro_pixmap(":/imagenes/multimedia/imagenes/paredes_nivel1.jpg");
    QBrush muro_brush;

    if (!muro_pixmap.isNull()) {
        muro_brush = QBrush(muro_pixmap);
        muro_brush.setStyle(Qt::TexturePattern);
    } else {
        muro_brush = QBrush(QColor(80, 80, 80));
    }

    painter.setBrush(muro_brush);
    painter.setPen(Qt::NoPen);

    for (const Muro &m : muros) {
        QRect rect = m.getRectanguloColision();
        painter.drawRect(rect);
    }

    // Dibujado de Zonas de Fuego Fijas
    if (!framesFuego.isEmpty() && currentFrameFuegoIndex < framesFuego.size()) {
        QPixmap currentFrame = framesFuego.at(currentFrameFuegoIndex);
        painter.setPen(Qt::NoPen);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver); // Transparencia

        for (const Muro &fuego : zonasFuego) {
            painter.drawPixmap(fuego.getRectanguloColision(), currentFrame);
        }
    } else {
        // Fallback de fuego
        painter.setBrush(Qt::red);
        painter.setPen(Qt::NoPen);
        for (const Muro &fuego : zonasFuego) {
            painter.drawRect(fuego.getRectanguloColision());
        }
    }

    // --- DIBUJADO DE FRAGMENTOS (CÍRCULOS AMARILLOS CON BORDE NEGRO) ---
    for (const Fragmento &f : fragmentos) {
        if (f.estaSalvado) {
            continue;
        }

        QPen borderPen(Qt::black);
        borderPen.setWidth(2);
        painter.setPen(borderPen);

        painter.drawPixmap(f.pos_x, f.pos_y, f.imagenFragmento);
    }

    // Dibujado del Jugador (Sabio Maya)
    if (!framesMaya.isEmpty() && currentFrameMayaIndex < framesMaya.size()) {
        QPixmap playerFrame = framesMaya.at(currentFrameMayaIndex);

        painter.setCompositionMode(QPainter::CompositionMode_SourceOver); // Para usar la transparencia

        painter.drawPixmap(sabioMaya.getRectanguloColision(), playerFrame);
    } else {
        // Fallback: Si los sprites fallan, mantiene el círculo verde
        painter.setBrush(Qt::darkGreen);
        painter.setPen(Qt::white);
        painter.drawEllipse(sabioMaya.getRectanguloColision());
    }

    // 3. DIBUJADO DE LA INTERFAZ DE USUARIO

    // --- Configuración Global de Fuente y Fondo ---
    QFont hudFont("Times New Roman", 16, QFont::Bold);
    QColor backgroundColor(50, 50, 50, 150); // Fondo gris semi-transparente
    int hudWidth = 250;
    int hudHeight = 50;

    // Dibujar el rectángulo de fondo gris para el HUD superior
    painter.setBrush(backgroundColor);
    painter.setPen(Qt::NoPen);
    painter.drawRect(10, 10, hudWidth, hudHeight);

    // Restaurar el color del texto y aplicar la fuente
    painter.setPen(Qt::white);
    painter.setFont(hudFont);

    // Texto de Fragmentos
    painter.drawText(20, 30, QString("Fragmentos: %1 / %2").arg(sabioMaya.fragmentosSalvados).arg(FRAGMENTOS_REQUERIDOS));

    // Texto de Tiempo Restante
    int tiempoRestante = TIEMPO_LIMITE_SEGUNDOS - (timerElapsedJuego.elapsed() / 1000);
    painter.drawText(20, 50, QString("Tiempo: %1 s").arg(tiempoRestante));

    // --- MENSAJE "RESCATANDO..."
    if (sabioMaya.tiempoContactoFragMs > 0) {
        qreal progreso = (timerElapsedJuego.elapsed() - sabioMaya.tiempoContactoFragMs) / (qreal)TIEMPO_RETENCION_MS;
        if (progreso > 1.0) progreso = 1.0;

        // Texto Azul Claro (Cian)
        painter.setPen(Qt::cyan);
        painter.setFont(QFont("Times New Roman", 18, QFont::Bold));

        painter.drawText(NIVEL_WIDTH / 2 - 100, NIVEL_HEIGHT - 50,
                         QString("RESCATANDO... %1%").arg(static_cast<int>(progreso * 100)));

        // Barra de progreso visual
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(30, 30, 30));
        painter.drawRect(NIVEL_WIDTH / 2 - 100, NIVEL_HEIGHT - 40, 200, 10);

        painter.setBrush(Qt::cyan);
        painter.drawRect(NIVEL_WIDTH / 2 - 100, NIVEL_HEIGHT - 40, 200 * progreso, 10);
    }

    // --- MENSAJES FINALES (VICTORIA / DERROTA)
    if (juegoEstado != 0) {
        QString mensaje = "";
        QColor colorFondo;

        if (juegoEstado == 1) {
            mensaje = "¡VICTORIA! Nivel Completado ";
            colorFondo = QColor(50, 200, 50, 180);
        } else if (juegoEstado == 2) {
            mensaje = "DERROTA - JUEGO TERMINADO ️";
            colorFondo = QColor(200, 50, 50, 180);
        }

        // Fondo semi-transparente
        painter.setBrush(colorFondo);
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, 0, NIVEL_WIDTH, NIVEL_HEIGHT);

        painter.setPen(Qt::white);
        painter.setFont(QFont("Times New Roman", 36, QFont::Bold)); // Tamaño ajustado a 36

        int centerX = NIVEL_WIDTH / 2;
        int centerY = NIVEL_HEIGHT / 2;

        // Posición X ajustada a -200 para centrado y evitar cortes
        painter.drawText(centerX - 200, centerY - 20, mensaje);

        // Mensaje de reinicio
        painter.setFont(QFont("Times New Roman", 18));
        painter.drawText(centerX - 180, centerY + 40, "Reiniciando en unos segundos...");
    }
}
