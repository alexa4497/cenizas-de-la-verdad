#include "niveltres.h"
#include <QPainter>
#include <QMessageBox>
#include <QDebug>
#include <algorithm>
#include <QPushButton>

// ========== LIBRO PROHIBIDO ==========
LibroProhibido::LibroProhibido(float px, float py, int tipo)
    : Fragmento(px, py), tipoMovimiento(tipo) {
    xInicial = px;
    yInicial = py;
    tiempo = 0;
    angulo = 0;
    radio = 15;
    ancho = 40;
    alto = 40;
}

void LibroProhibido::actualizar() {
    tiempo += 0.016f;

    if (tipoMovimiento == 0) {
        float t = tiempo * 1.0f;
        pos_x = xInicial + 60 * sin(t);
        pos_y = yInicial + 30 * sin(2 * t);
    }
    else if (tipoMovimiento == 1) {
        angulo += 2.0f * 0.016f;
        pos_x = xInicial + 70 * cos(angulo);
        pos_y = yInicial + 70 * sin(angulo);
    }
    else if (tipoMovimiento == 2) {
        angulo += 1.5f * 0.016f;
        radio = 15 + 0.8f * tiempo;
        pos_x = xInicial + radio * cos(angulo);
        pos_y = yInicial + radio * sin(angulo);

        if (radio > 120) {
            tiempo = 0;
            radio = 15;
            angulo = 0;
        }
    }
}

// ========== NIVEL 3 ==========
Niveltres::Niveltres(QWidget *parent) : QWidget(parent) {
    resize(1200, 700);
    setWindowTitle("Nivel 3: Biblioteca Prohibida");

    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    MUNDO_ALTO = 1000;
    offsetY = 0;

    jugador = new Jugador(500, 730, 48, 48, 2.5f);

    // Cargar todas las imágenes
    cargarImagenes();

    // Cargar sonidos
    cargarSonidos();

    // 15 LIBROS EN TOTAL
    libros[0] = new LibroProhibido(150, 800, 0);
    libros[1] = new LibroProhibido(600, 800, 0);
    libros[2] = new LibroProhibido(1050, 800, 0);
    libros[3] = new LibroProhibido(400, 500, 0);
    libros[4] = new LibroProhibido(800, 500, 0);

    libros[5] = new LibroProhibido(950, 700, 1);
    libros[6] = new LibroProhibido(950, 450, 1);
    libros[7] = new LibroProhibido(250, 600, 1);
    libros[8] = new LibroProhibido(250, 300, 1);
    libros[9] = new LibroProhibido(1100, 200, 1);

    libros[10] = new LibroProhibido(400, 650, 2);
    libros[10]->tiempo = 5.0f;
    libros[10]->radio = 15 + 0.8f * 5.0f;

    libros[11] = new LibroProhibido(700, 650, 2);
    libros[11]->tiempo = 10.0f;
    libros[11]->radio = 15 + 0.8f * 10.0f;

    libros[12] = new LibroProhibido(500, 350, 2);
    libros[12]->tiempo = 15.0f;
    libros[12]->radio = 15 + 0.8f * 15.0f;

    libros[13] = new LibroProhibido(600, 150, 2);
    libros[13]->tiempo = 20.0f;
    libros[13]->radio = 15 + 0.8f * 20.0f;

    libros[14] = new LibroProhibido(1000, 100, 2);
    libros[14]->tiempo = 25.0f;
    libros[14]->radio = 15 + 0.8f * 25.0f;

    timerJuego = new QTimer(this);
    connect(timerJuego, &QTimer::timeout, this, &Niveltres::gameLoop);
    timerJuego->start(16);

    // Timer para animación del jugador
    timerAnimacion = new QTimer(this);
    connect(timerAnimacion, &QTimer::timeout, this, &Niveltres::actualizarAnimacionJugador);
    timerAnimacion->start(120);

    // Variables de animación
    frameActualJugador = 0;
    direccionJugador = 1;
    estaSaltando = false;
    frameIndex = 0;

    inicializarUI();
}

void Niveltres::cargarImagenes() {
    // Cargar fondo
    fondo.load(":/imagenes/multimedia/imagenes/fondonivel3.png");
    if (fondo.isNull()) {
        qDebug() << "No se pudo cargar el fondo";
    } else {
        fondo = fondo.scaled(1200, MUNDO_ALTO, Qt::KeepAspectRatioByExpanding);
    }

    // Cargar textura de muros
    texturaMuro.load(":/imagenes/multimedia/imagenes/textura_madera.png");
    if (texturaMuro.isNull()) {
        qDebug() << "No se pudo cargar textura de muro";
        texturaMuro = QPixmap(64, 64);
        texturaMuro.fill(QColor(90, 70, 50));
    } else {
        texturaMuro = texturaMuro.scaled(64, 64, Qt::KeepAspectRatioByExpanding);
    }

    // Cargar imágenes de libros
    imgLibroInfinito.load(":/imagenes/multimedia/imagenes/libro_rojo.png");
    if (!imgLibroInfinito.isNull()) {
        imgLibroInfinito = imgLibroInfinito.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    imgLibroCircular.load(":/imagenes/multimedia/imagenes/libro_azul.png");
    if (!imgLibroCircular.isNull()) {
        imgLibroCircular = imgLibroCircular.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    imgLibroEspiral.load(":/imagenes/multimedia/imagenes/libro_verde.png");
    if (!imgLibroEspiral.isNull()) {
        imgLibroEspiral = imgLibroEspiral.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // Cargar sprite sheet del jugador (3x6 = 18 frames, 48x48 cada uno)
    spriteSheetJugador.load(":/imagenes/multimedia/imagenes/perosnajeFinalnivel3xx.png");
    if (!spriteSheetJugador.isNull()) {
        int frameWidth = 48;
        int frameHeight = 48;
        int columnas = 6;
        int filas = 3;

        if (spriteSheetJugador.width() != 288 || spriteSheetJugador.height() != 144) {
            qDebug() << "Sprite sheet tamaño:" << spriteSheetJugador.width() << "x" << spriteSheetJugador.height();
            frameWidth = spriteSheetJugador.width() / columnas;
            frameHeight = spriteSheetJugador.height() / filas;
        }

        framesJugador.clear();

        for (int fila = 0; fila < filas; fila++) {
            for (int col = 0; col < columnas; col++) {
                QPixmap frame = spriteSheetJugador.copy(
                    col * frameWidth,
                    fila * frameHeight,
                    frameWidth,
                    frameHeight
                    );
                framesJugador.append(frame);
            }
        }
        qDebug() << "Cargados" << framesJugador.size() << "frames del jugador (3x6)";
    } else {
        qDebug() << "No se pudo cargar sprite sheet del jugador";
    }
}

void Niveltres::cargarSonidos() {
    // 1. AUDIO OUTPUT PARA MÚSICA DE FONDO
    audioOutputMusica = new QAudioOutput(this);
    audioOutputMusica->setVolume(0.5f);  // Volumen al 50%

    // 2. AUDIO OUTPUT PARA EFECTOS DE SONIDO
    audioOutputEfectos = new QAudioOutput(this);
    audioOutputEfectos->setVolume(0.7f);  // Volumen al 70%

    // 1. MÚSICA DE FONDO
    musicaFondo = new QMediaPlayer(this);
    musicaFondo->setAudioOutput(audioOutputMusica);  // Usa audioOutputMusica
    musicaFondo->setSource(QUrl("qrc:/sonido/multimedia/audio/audio_nivel3.ogg"));

    // Verifica si se cargó el archivo
    if (musicaFondo->source().isEmpty()) {
        qDebug() << "ERROR: No se pudo cargar la música de fondo";
    } else {
        qDebug() << "Música cargada correctamente";
    }

    musicaFondo->setLoops(QMediaPlayer::Infinite);
    musicaFondo->play();

    // Verifica si está reproduciendo
    qDebug() << "Estado música:" << musicaFondo->playbackState();

    // 2. SONIDO DE SALTO
    sonidoSalto = new QMediaPlayer(this);
    sonidoSalto->setAudioOutput(audioOutputEfectos);  // Usa audioOutputEfectos

    // Carga el sonido de salto
    sonidoSalto->setSource(QUrl("qrc:/sonido/multimedia/audio/salto.ogg"));

    if (sonidoSalto->source().isEmpty()) {
        qDebug() << "ERROR: No se pudo cargar sonido de salto";
    }
}

void Niveltres::actualizarAnimacionJugador() {
    frameIndex++;

    if (estaSaltando || std::abs(velocidadY) > 0.5f) {
        frameActualJugador = frameIndex % 6;
        timerAnimacion->setInterval(80);
    }
    else if (std::abs(jugador->getVelocidad()) > 0.1f) {
        if (direccionJugador == 1) {
            frameActualJugador = 12 + (frameIndex % 6);
        } else {
            frameActualJugador = 6 + (frameIndex % 6);
        }
        timerAnimacion->setInterval(120);
    }
    else {
        if (direccionJugador == 1) {
            frameActualJugador = 12;
        } else {
            frameActualJugador = 6;
        }
        timerAnimacion->setInterval(200);
    }

    update();
}

Niveltres::~Niveltres() {
    delete jugador;
    for (int i = 0; i < 15; i++) {
        delete libros[i];
    }

    // Detener y limpiar música primero
    if (musicaFondo) {
        musicaFondo->stop();
        delete musicaFondo;
    }
    if (sonidoSalto) delete sonidoSalto;

    // Limpiar audio outputs
    if (audioOutputMusica) delete audioOutputMusica;
    if (audioOutputEfectos) delete audioOutputEfectos;

    // Asegurar que los timers se detengan
    if (timerJuego) timerJuego->stop();
    if (timerAnimacion) timerAnimacion->stop();
}


void Niveltres::gameLoop() {
    jugador->setPos_y(jugador->getPos_y() + velocidadY);
    velocidadY += 0.8f;

    if (velocidadY < -0.5f || velocidadY > 0.5f) {
        estaSaltando = true;
    } else {
        estaSaltando = false;
    }

    // Colisiones con plataformas
    if (jugador->getPos_y() >= 800) {
        jugador->setPos_y(800);
        velocidadY = 0;
        estaSaltando = false;
    }

    if (jugador->getPos_x() + jugador->getAncho() > 100 &&
        jugador->getPos_x() < 1000 &&
        jugador->getPos_y() + jugador->getAlto() >= 650 &&
        jugador->getPos_y() + jugador->getAlto() <= 670 &&
        velocidadY > 0) {
        jugador->setPos_y(650 - jugador->getAlto());
        velocidadY = 0;
        estaSaltando = false;
    }

    if (jugador->getPos_x() + jugador->getAncho() > 200 &&
        jugador->getPos_x() < 800 &&
        jugador->getPos_y() + jugador->getAlto() >= 450 &&
        jugador->getPos_y() + jugador->getAlto() <= 470 &&
        velocidadY > 0) {
        jugador->setPos_y(450 - jugador->getAlto());
        velocidadY = 0;
        estaSaltando = false;
    }

    if (jugador->getPos_x() + jugador->getAncho() > 300 &&
        jugador->getPos_x() < 700 &&
        jugador->getPos_y() + jugador->getAlto() >= 250 &&
        jugador->getPos_y() + jugador->getAlto() <= 270 &&
        velocidadY > 0) {
        jugador->setPos_y(250 - jugador->getAlto());
        velocidadY = 0;
        estaSaltando = false;
    }

    if (jugador->getPos_x() + jugador->getAncho() >= 150 && jugador->getPos_x() <= 170 &&
        jugador->getPos_y() + jugador->getAlto() >= 400 &&
        jugador->getPos_y() + jugador->getAlto() <= 420 &&
        velocidadY > 0) {
        jugador->setPos_y(400 - jugador->getAlto());
        velocidadY = 0;
        estaSaltando = false;
    }

    if (jugador->getPos_x() + jugador->getAncho() > 150 && jugador->getPos_x() < 400 &&
        jugador->getPos_y() + jugador->getAlto() >= 400 &&
        jugador->getPos_y() + jugador->getAlto() <= 420 &&
        velocidadY > 0) {
        jugador->setPos_y(400 - jugador->getAlto());
        velocidadY = 0;
        estaSaltando = false;
    }

    if (jugador->getPos_x() + jugador->getAncho() >= 900 && jugador->getPos_x() <= 920 &&
        jugador->getPos_y() + jugador->getAlto() >= 350 &&
        jugador->getPos_y() + jugador->getAlto() <= 370 &&
        velocidadY > 0) {
        jugador->setPos_y(350 - jugador->getAlto());
        velocidadY = 0;
        estaSaltando = false;
    }

    if (jugador->getPos_x() + jugador->getAncho() > 900 && jugador->getPos_x() < 1150 &&
        jugador->getPos_y() + jugador->getAlto() >= 350 &&
        jugador->getPos_y() + jugador->getAlto() <= 370 &&
        velocidadY > 0) {
        jugador->setPos_y(350 - jugador->getAlto());
        velocidadY = 0;
        estaSaltando = false;
    }

    if (jugador->getPos_x() + jugador->getAncho() > 500 && jugador->getPos_x() < 700 &&
        jugador->getPos_y() + jugador->getAlto() >= 550 &&
        jugador->getPos_y() + jugador->getAlto() <= 570 &&
        velocidadY > 0) {
        jugador->setPos_y(550 - jugador->getAlto());
        velocidadY = 0;
        estaSaltando = false;
    }

    if (jugador->getPos_x() + jugador->getAncho() > 1000 && jugador->getPos_x() < 1100 &&
        jugador->getPos_y() + jugador->getAlto() >= 550 &&
        jugador->getPos_y() + jugador->getAlto() <= 570 &&
        velocidadY > 0) {
        jugador->setPos_y(550 - jugador->getAlto());
        velocidadY = 0;
        estaSaltando = false;
    }

    if (jugador->getPos_x() + jugador->getAncho() > 1050 && jugador->getPos_x() < 1150 &&
        jugador->getPos_y() + jugador->getAlto() >= 450 &&
        jugador->getPos_y() + jugador->getAlto() <= 470 &&
        velocidadY > 0) {
        jugador->setPos_y(450 - jugador->getAlto());
        velocidadY = 0;
        estaSaltando = false;
    }

    if (jugador->getPos_x() + jugador->getAncho() > 1100 && jugador->getPos_x() < 1200 &&
        jugador->getPos_y() + jugador->getAlto() >= 350 &&
        jugador->getPos_y() + jugador->getAlto() <= 370 &&
        velocidadY > 0) {
        jugador->setPos_y(350 - jugador->getAlto());
        velocidadY = 0;
        estaSaltando = false;
    }

    // Actualizar libros
    for (int i = 0; i < 15; i++) {
        if (!libros[i]->estaSalvado) {
            libros[i]->actualizar();
        }
    }

    // SCROLL VERTICAL
    float playerY_in_window = jugador->getPos_y() - offsetY;
    const float SCROLL_ZONE = 200;

    if (playerY_in_window > height() - SCROLL_ZONE) {
        offsetY += playerY_in_window - (height() - SCROLL_ZONE);
    }
    else if (playerY_in_window < SCROLL_ZONE) {
        offsetY += playerY_in_window - SCROLL_ZONE;
    }

    offsetY = std::max(0.0f, std::min(offsetY, (float)(MUNDO_ALTO - height())));

    checkColisiones();
    update();
}

void Niveltres::checkColisiones() {
    for (int i = 0; i < 15; i++) {
        if (!libros[i]->estaSalvado) {
            QRect rectJugador = jugador->getRectanguloColision();
            QRect rectLibro = libros[i]->getRectanguloColision();

            if (rectJugador.intersects(rectLibro)) {
                libros[i]->estaSalvado = true;
                librosCapturados++;

                if (librosCapturados >= LIBROS_TOTAL) {
                    timerJuego->stop();
                    timerAnimacion->stop();

                    // SOLO mostrar mensaje y regresar
                    QMessageBox::information(this, "¡Victoria!",
                                             "¡Has recuperado todos los libros prohibidos!\n\n"
                                             "Regresando al menú principal...");

                    // Emitir señal y cerrar
                    emit regresarMenuPrincipal();
                    this->close();
                }
                break;
            }
        }
    }
}
void Niveltres::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_A) {
        jugador->setPos_x(jugador->getPos_x() - jugador->getVelocidad());
        direccionJugador = -1;
        if (jugador->getPos_x() < 0) jugador->setPos_x(0);
    } else if (event->key() == Qt::Key_D) {
        jugador->setPos_x(jugador->getPos_x() + jugador->getVelocidad());
        direccionJugador = 1;
        if (jugador->getPos_x() > 1200 - jugador->getAncho())
            jugador->setPos_x(1200 - jugador->getAncho());
    } else if (event->key() == Qt::Key_W) {
        bool enSuelo = (jugador->getPos_y() >= 800 - jugador->getAlto());

        bool enPlataformaMedia = (jugador->getPos_x() > 100 && jugador->getPos_x() < 1000 &&
                                  std::abs(jugador->getPos_y() - (650 - jugador->getAlto())) < 5);

        bool enPlataformaMediaAlta = (jugador->getPos_x() > 200 && jugador->getPos_x() < 800 &&
                                      std::abs(jugador->getPos_y() - (450 - jugador->getAlto())) < 5);

        bool enPlataformaAlta = (jugador->getPos_x() > 300 && jugador->getPos_x() < 700 &&
                                 std::abs(jugador->getPos_y() - (250 - jugador->getAlto())) < 5);

        bool enColumnaDerecha = (jugador->getPos_x() >= 900 && jugador->getPos_x() <= 920 &&
                                 std::abs(jugador->getPos_y() - (350 - jugador->getAlto())) < 5);

        bool enPlataformaDerecha = (jugador->getPos_x() > 900 && jugador->getPos_x() < 1150 &&                         
                                    std::abs(jugador->getPos_y() - (350 - jugador->getAlto())) < 5);
        bool enColumnaIzquierda = (jugador->getPos_x() >= 150 && jugador->getPos_x() <= 170 &&

                                   std::abs(jugador->getPos_y() - (400 - jugador->getAlto())) < 5);
        bool enPlataformaIzquierda = (jugador->getPos_x() > 150 && jugador->getPos_x() < 400 &&

                                      std::abs(jugador->getPos_y() - (400 - jugador->getAlto())) < 5);
        bool enPlataformaCentral = (jugador->getPos_x() > 500 && jugador->getPos_x() < 700 &&

                                    std::abs(jugador->getPos_y() - (550 - jugador->getAlto())) < 5);
        bool enEscaleraNivel1 = (jugador->getPos_x() > 1000 && jugador->getPos_x() < 1100 &&

                                 std::abs(jugador->getPos_y() - (550 - jugador->getAlto())) < 5);
        bool enEscaleraNivel2 = (jugador->getPos_x() > 1050 && jugador->getPos_x() < 1150 &&

                                 std::abs(jugador->getPos_y() - (450 - jugador->getAlto())) < 5);
        bool enEscaleraNivel3 = (jugador->getPos_x() > 1100 && jugador->getPos_x() < 1200 &&

                                 std::abs(jugador->getPos_y() - (350 - jugador->getAlto())) < 5);

        if (enSuelo || enPlataformaMedia || enPlataformaMediaAlta ||
            enPlataformaAlta || enColumnaDerecha || enPlataformaDerecha ||
            enColumnaIzquierda || enPlataformaIzquierda || enPlataformaCentral ||
            enEscaleraNivel1 || enEscaleraNivel2 || enEscaleraNivel3) {
            velocidadY = -18;


            if (sonidoSalto) {
                sonidoSalto->setPosition(0);
                sonidoSalto->play();
            }
        }
    } else if (event->key() == Qt::Key_S) {
        velocidadY += 10;
    }

    update();
}

void Niveltres::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);

    painter.translate(0, -offsetY);

    // DIBUJAR FONDO
    if (!fondo.isNull()) {
        painter.drawPixmap(0, 0, width(), MUNDO_ALTO, fondo);
    } else {
        painter.fillRect(0, 0, width(), MUNDO_ALTO, QColor(30, 25, 20));
    }

    // DIBUJAR PLATAFORMAS CON TEXTURA
    if (!texturaMuro.isNull()) {
        QBrush brushTextura(texturaMuro);
        painter.setBrush(brushTextura);
        painter.setPen(QPen(QColor(60, 50, 40), 2));
    } else {
        painter.setBrush(QColor(90, 70, 50));
        painter.setPen(QPen(QColor(60, 50, 40), 2));
    }

    // Suelo principal
    painter.drawRect(0, 800, width(), 50);

    // Plataformas horizontales principales
    painter.drawRect(100, 650, 900, 20);
    painter.drawRect(200, 450, 600, 20);
    painter.drawRect(300, 250, 400, 20);

    // PLATAFORMAS IZQUIERDAS
    painter.drawRect(150, 400, 20, 400);
    painter.drawRect(150, 400, 250, 20);

    // PLATAFORMAS DERECHAS
    painter.drawRect(900, 350, 20, 450);
    painter.drawRect(900, 350, 250, 20);

    // ESCALERA DE PLATAFORMAS DERECHA
    painter.drawRect(1000, 550, 100, 20);
    painter.drawRect(1050, 450, 100, 20);
    painter.drawRect(1100, 350, 100, 20);

    // Columnas de soporte para la escalera
    painter.drawRect(1000, 550, 20, 50);
    painter.drawRect(1050, 450, 20, 100);
    painter.drawRect(1100, 350, 20, 150);

    // PLATAFORMA CENTRAL FLOTANTE
    painter.drawRect(500, 550, 200, 20);

    // PLATAFORMA ALTA IZQUIERDA
    painter.drawRect(100, 200, 150, 20);

    // PLATAFORMA ALTA DERECHA
    painter.drawRect(950, 150, 200, 20);

    // DIBUJAR LIBROS CON IMÁGENES
    for (int i = 0; i < 15; i++) {
        if (!libros[i]->estaSalvado) {
            QPixmap* imgLibro = nullptr;

            if (libros[i]->tipoMovimiento == 0) {
                imgLibro = &imgLibroInfinito;
            } else if (libros[i]->tipoMovimiento == 1) {
                imgLibro = &imgLibroCircular;
            } else {
                imgLibro = &imgLibroEspiral;
            }

            if (imgLibro && !imgLibro->isNull()) {
                // Dibujar sombra
                painter.setBrush(QColor(0, 0, 0, 100));
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(libros[i]->pos_x + 2, libros[i]->pos_y + 2,
                                    libros[i]->ancho, libros[i]->alto);

                // Dibujar libro
                painter.drawPixmap(libros[i]->getRectanguloColision(), *imgLibro);

                // Efecto brillo si es espiral
                if (libros[i]->tipoMovimiento == 2) {
                    painter.setPen(QPen(QColor(255, 255, 200, 100), 1));
                    painter.setBrush(Qt::NoBrush);

                }
            } else {
                // Fallback a colores
                QColor color = (libros[i]->tipoMovimiento == 0) ? QColor(255, 100, 100) :
                                   (libros[i]->tipoMovimiento == 1) ? QColor(100, 100, 255) :
                                   QColor(100, 255, 100);
                painter.setBrush(color);
                painter.setPen(Qt::black);
                painter.drawRect(libros[i]->getRectanguloColision());

                // Dibujar símbolo
                painter.setPen(Qt::white);
                painter.setFont(QFont("Arial", 14, QFont::Bold));
                QString texto = (libros[i]->tipoMovimiento == 0) ? "∞" :
                                    (libros[i]->tipoMovimiento == 1) ? "O" : "S";
                painter.drawText(libros[i]->getRectanguloColision(), Qt::AlignCenter, texto);
            }

            // Dibujar trayectoria de espirales
            if (libros[i]->tipoMovimiento == 2) {
                painter.setPen(QPen(QColor(100, 255, 100, 80), 2));
                float centroX = libros[i]->xInicial;
                float centroY = libros[i]->yInicial;

                QPointF puntos[20];
                for (int j = 0; j < 20; j++) {
                    float t = libros[i]->tiempo - j * 0.5f;
                    if (t < 0) t = 0;
                    float r = 15 + 0.8f * t;
                    float ang = libros[i]->angulo - j * 0.3f;
                    puntos[j] = QPointF(centroX + r * cos(ang),
                                        centroY + r * sin(ang));
                }

                for (int j = 0; j < 19; j++) {
                    painter.drawLine(puntos[j], puntos[j + 1]);
                }
            }
        }
    }

    // DIBUJAR JUGADOR CON SPRITE
    if (!framesJugador.isEmpty() && frameActualJugador < framesJugador.size()) {
        QPixmap frame = framesJugador[frameActualJugador];

        // Dibujar sombra del jugador
        painter.setBrush(QColor(0, 0, 0, 80));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(jugador->getPos_x() + 4, jugador->getPos_y() + jugador->getAlto() - 8,
                            jugador->getAncho() - 8, 10);

        painter.drawPixmap(jugador->getRectanguloColision(), frame);
    } else {
        // Fallback a cuadrado amarillo
        painter.setBrush(Qt::yellow);
        painter.setPen(Qt::black);
        painter.drawRect(jugador->getRectanguloColision());
    }

    // HUD (sin scroll)
    painter.resetTransform();

    // Fondo semitransparente para HUD
    painter.setBrush(QColor(0, 0, 0, 180));
    painter.setPen(Qt::NoPen);
    painter.drawRect(5, 5, 180, 60);

    // Bordes decorativos
    painter.setPen(QPen(QColor(200, 180, 120), 2));
    painter.drawRect(5, 5, 180, 60);

    // Texto del HUD
    painter.setPen(Qt::white);
    painter.setFont(QFont("Times New Roman", 18, QFont::Bold));
    painter.drawText(20, 35, QString("LIBROS: %1/15").arg(librosCapturados));

    // Barra de progreso
    float progreso = (float)librosCapturados / 15.0f;
    painter.setBrush(QColor(100, 200, 100, 150));
    painter.drawRect(20, 45, 150 * progreso, 12);

    painter.setPen(QPen(QColor(255, 255, 255), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(20, 45, 150, 12);
}

// -boton de salida

void Niveltres::inicializarUI() {
    // Crear y configurar el botón de salir
    btnSalir = new QPushButton("Salir", this);
    btnSalir->setGeometry(width() - 100, 10, 80, 30);
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
    connect(btnSalir, &QPushButton::clicked, this, &Niveltres::onSalirClicked);
}

void Niveltres::onSalirClicked() {
    // Detener timers y música
    if (timerJuego && timerJuego->isActive()) {
        timerJuego->stop();
    }
    if (timerAnimacion && timerAnimacion->isActive()) {
        timerAnimacion->stop();
    }

    if (musicaFondo) {
        musicaFondo->stop();
    }

    qDebug() << "Saliendo del nivel 3...";

    // Emitir señal para regresar al menú principal
    emit regresarMenuPrincipal();

    // CERRAR LA VENTANA - AÑADE ESTA LÍNEA
    this->close();
}
