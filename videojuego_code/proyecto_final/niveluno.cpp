#include "niveluno.h"
#include "ui_niveluno.h"
#include <QDebug>
#include <QPainter>
#include <QKeyEvent>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <QImage>   // Necesario para cargar imágenes con canal alfa
#include <QPixmap>// Necesario para std::min, std::max y sqrt

// niveluno.cpp (CONSTRUCTOR COMPLETO)

// Definición de constantes de tamaño del nivel
const int NIVEL_WIDTH = 1250;
const int NIVEL_HEIGHT = 650;

Niveluno::Niveluno(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Niveluno)

    , sabioMaya(600.0f, 80.0f, 60.0f, 60.0f,4.0f) // <--- Ajustado el tamaño del jugador a 60x60
    , agenteFuego(2000.0f, 2000.0f)
    , FRAGMENTOS_REQUERIDOS(8)
    , TIEMPO_LIMITE_SEGUNDOS(100)
    , TIEMPO_RETENCION_MS(3000)
    , juegoEstado(0)
{
    ui->setupUi(this);
    this->setMinimumSize(NIVEL_WIDTH, NIVEL_HEIGHT);
    setFocusPolicy(Qt::StrongFocus);

    // --- Timers Principales ---
    timerJuego = new QTimer(this);
    connect(timerJuego, &QTimer::timeout, this, &Niveluno::actualizarJuego);

    timerReinicio = new QTimer(this);
    connect(timerReinicio, &QTimer::timeout, this, &Niveluno::reiniciarNivel);

    // --- Inicialización de elementos ---
    inicializarFragmentos();
    inicializarMuros();
    inicializarZonasFuego();

    // ----------------------------------------------------------------------
    // ** A. CARGA Y ANIMACIÓN DEL FUEGO (100x100 frames) **
    // ----------------------------------------------------------------------

    QString spriteSheetPath = "C:/Users/alexa/Desktop/proyecto_final/videojuego_code/multimedia/imagenes/sprites_fuego.png";
    QPixmap fullSpriteSheet(spriteSheetPath);
    framesFuego.clear();

    if (!fullSpriteSheet.isNull()) {
        int frameWidth = 100;
        int frameHeight = 100;
        framesFuego.append(fullSpriteSheet.copy(0 * frameWidth, 0, frameWidth, frameHeight));
        framesFuego.append(fullSpriteSheet.copy(1 * frameWidth, 0, frameWidth, frameHeight));
        framesFuego.append(fullSpriteSheet.copy(2 * frameWidth, 0, frameWidth, frameHeight));
        framesFuego.append(fullSpriteSheet.copy(3 * frameWidth, 0, frameWidth, frameHeight));
        qDebug() << "Sprite sheet de fuego cargado y dividido en" << framesFuego.size() << "frames.";
    } else {
        qDebug() << "ERROR CRÍTICO: No se pudo cargar el sprite sheet de fuego desde:" << spriteSheetPath;
    }

    currentFrameFuegoIndex = 0;
    timerAnimacionFuego = new QTimer(this);
    connect(timerAnimacionFuego, &QTimer::timeout, this, &Niveluno::actualizarAnimacionFuego);
    if (!framesFuego.isEmpty()) {
        timerAnimacionFuego->start(200);
    } else {
        qDebug() << "Animación de fuego deshabilitada por errores de carga.";
    }

    // ----------------------------------------------------------------------
    // ** B. CARGA Y ANIMACIÓN DEL SABIO MAYA (60x60 frames - Sheet 240x240) **
    // ----------------------------------------------------------------------

    QString spriteSheetPathMaya = "C:/Users/alexa/Desktop/proyecto_final/videojuego_code/multimedia/imagenes/sprites_sabiomaya.png"; // <--- ¡AJUSTA ESTA RUTA!
    fullSpriteSheetMaya.load(spriteSheetPathMaya);
    framesMaya.clear();
    currentFrameMayaIndex = 0;

    if (!fullSpriteSheetMaya.isNull()) {
        int frameWidth = 60;  // <--- Tamaño del frame individual (240 / 4)
        int frameHeight = 60; // <--- Tamaño del frame individual

        // Extraer los 16 frames (4 filas x 4 columnas)
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                framesMaya.append(fullSpriteSheetMaya.copy(x * frameWidth, y * frameHeight, frameWidth, frameHeight));
            }
        }
        qDebug() << "Sprite sheet del Sabio Maya cargado y dividido en" << framesMaya.size() << "frames (60x60 cada uno).";
    } else {
        qDebug() << "ERROR CRÍTICO: No se pudo cargar el sprite sheet del Sabio Maya.";
    }

    // Inicializar variables de estado y animación
    animRowOffset = 0;
    isMoving = false;

    // Timer para la animación de caminata (80ms)
    timerAnimacionMaya = new QTimer(this);
    connect(timerAnimacionMaya, &QTimer::timeout, this, &Niveluno::actualizarAnimacionMaya);
    if (!framesMaya.isEmpty()) {
        timerAnimacionMaya->start(95);
    }

    // ----------------------------------------------------------------------
    // ** FIN DE CARGA DE SPRITES **
    // ----------------------------------------------------------------------


    // --- Inicio del Juego (Va al final de la inicialización) ---
    timerElapsedJuego.start();
    timerJuego->start(16); // ~60 FPS
    qDebug() << "Nivel 1 Iniciado. Tiempo límite:" << TIEMPO_LIMITE_SEGUNDOS << "segundos.";
}

Niveluno::~Niveluno()
{
    delete ui;
}

// ----------------------------------------------------
// IMPLEMENTACIÓN DE MÉTODOS DE INICIALIZACIÓN
// ----------------------------------------------------

void Niveluno::inicializarFragmentos() {
    fragmentos.clear();

    // El tamaño de los fragmentos es 30x30px (asumido).

    // --------------------------------------------------------------------------
    // 1. ZONA IZQUIERDA (5 Fragmentos: Alineados con el pilar gris lateral)
    // --------------------------------------------------------------------------

    fragmentos.append(Fragmento(1165, 65));
    fragmentos.append(Fragmento(705, 550));


    fragmentos.append(Fragmento(1085, 340));
    fragmentos.append(Fragmento(1175, 550));

    fragmentos.append(Fragmento(160, 60));

    fragmentos.append(Fragmento(70, 140));

    // Flanqueando la barra horizontal de la "T"
    fragmentos.append(Fragmento(150, 220));
    fragmentos.append(Fragmento(300, 555));


}


void Niveluno::inicializarMuros() {
    muros.clear();
    // Grosor del muro
    const int muro_grosor = 40;
    const int pilar_ancho = 60;

    // ----------------------------------------------------
    // 1. Muros del Borde Exterior (Perímetro 100%)
    // ----------------------------------------------------
    muros.append(Muro(0, 0, NIVEL_WIDTH, muro_grosor));
    muros.append(Muro(0, NIVEL_HEIGHT - muro_grosor, NIVEL_WIDTH, muro_grosor));
    muros.append(Muro(0, muro_grosor, muro_grosor, NIVEL_HEIGHT - 2 * muro_grosor));
    muros.append(Muro(NIVEL_WIDTH - muro_grosor, muro_grosor, muro_grosor, NIVEL_HEIGHT - 2 * muro_grosor));


    // ----------------------------------------------------
    // 2. Muros Interiores (Estructura de Tres Pilares y T central)
    // ----------------------------------------------------

    // Alturas de los pilares
    const int pilar_altura_lateral = NIVEL_HEIGHT - 2 * muro_grosor - 100;
    const int pilar_altura_central = NIVEL_HEIGHT - 2 * muro_grosor - 200;

    // Calcular X central
    int x_central = NIVEL_WIDTH / 2 - pilar_ancho / 2;
    // Calcular Y del centro (pegado al fondo)
    int y_central_pegado = NIVEL_HEIGHT - muro_grosor - pilar_altura_central;

    // --- Pilar Central (Muro vertical pegado al fondo) ---
    muros.append(Muro(x_central, y_central_pegado, pilar_ancho, pilar_altura_central));

    // --- Muro Horizontal Perpendicular (La barra de la T) ---
    const int muro_perpendicular_ancho = 350;
    const int muro_perpendicular_grosor = 50;

    // Y: La barra horizontal debe iniciar donde termina el pilar vertical.
    const int y_muro_horizontal = y_central_pegado - muro_perpendicular_grosor;

    // X Inicio: Queremos centrar la barra (350px) sobre el pilar (60px).
    int x_inicio_horizontal = x_central + (pilar_ancho / 2) - (muro_perpendicular_ancho / 2);

    muros.append(Muro(x_inicio_horizontal, y_muro_horizontal, muro_perpendicular_ancho, muro_perpendicular_grosor));

    // --- Pilares Laterales (Flotan desde arriba) ---
    int x_izquierdo = NIVEL_WIDTH / 4 - pilar_ancho / 2;
    muros.append(Muro(x_izquierdo, muro_grosor, pilar_ancho, pilar_altura_lateral));

    int x_derecho = (NIVEL_WIDTH * 3) / 4 - pilar_ancho / 2;
    muros.append(Muro(x_derecho, muro_grosor, pilar_ancho, pilar_altura_lateral));
}

// niveluno.cpp (IMPLEMENTACIÓN DEL NUEVO SLOT)

// ------------------------------------------------------------------
// LÓGICA DE ANIMACIÓN DEL SABIO MAYA
// ------------------------------------------------------------------
void Niveluno::actualizarAnimacionMaya() {
    // Solo animamos si el jugador se está moviendo
    if (isMoving) {
        // La animación cíclica se hace dentro de los 4 frames de la fila actual.
        // La fila (animRowOffset) se establece en keyPressEvent (0, 4, 8, o 12).

        // 1. Obtener el índice de la columna actual dentro de la fila (0, 1, 2, 3)
        int currentColumnIndex = currentFrameMayaIndex % 4;

        // 2. Calcular el nuevo índice de columna (avanza al siguiente frame, y vuelve a 0 al llegar a 4)
        currentColumnIndex = (currentColumnIndex + 1) % 4;

        // 3. Establecer el índice global combinando columna + fila
        currentFrameMayaIndex = currentColumnIndex + animRowOffset;

        // 4. Forzar el repintado para mostrar el nuevo frame
        update();
    }
}

void Niveluno::inicializarZonasFuego() {
    zonasFuego.clear();

    // Constantes de Muro (Se mantienen)
    const int pilar_ancho = 60;
    const int x_central = NIVEL_WIDTH / 2 - pilar_ancho / 2; // X del pilar vertical (595)

    // GROSORES Y COORDENADAS AJUSTADAS
    const int fuego_grosor_borde_ajustado = 35; // Grosor deseado (35px)
    const int fuego_inferior_grosor = 70; // Fuego inferior (70px)
    const int y_inferior = NIVEL_HEIGHT - fuego_inferior_grosor; // Y de inicio del fuego inferior (580)

    // COORDENADAS NUMÉRICAS BASADAS EN TU MURO (Muro en Y=175, Altura=50)
    const int Y_INICIO_MURO = 175; // Nueva Y de inicio del muro horizontal (asumida)
    const int GROSOR_MURO = 50;    // Grosor del muro horizontal (asumido)

    // Cálculos de alineación para la BARRA HORIZONTAL (35px de grosor)
    const int Y_FUEGO_SUPERIOR = Y_INICIO_MURO - fuego_grosor_borde_ajustado; // 175 - 35 = 140
    const int Y_FUEGO_INFERIOR_INICIO = Y_INICIO_MURO + GROSOR_MURO; // 175 + 50 = 225

    // X ajustados: El muro va de 450 a 800.
    const int X_FUEGO_IZQ = 450 - fuego_grosor_borde_ajustado; // 415
    const int X_FUEGO_DER = 800; // 800 (inicio del borde)
    const int ANCHO_TOTAL_FUEGO = 350 + 2 * fuego_grosor_borde_ajustado; // 420
    const int ALTO_TOTAL_FUEGO = GROSOR_MURO + 2 * fuego_grosor_borde_ajustado; // 120


    // --------------------------------------------------------------------------
    // 1. ZONA INFERIOR (La "U" Roja) - Grosor 70px
    // --------------------------------------------------------------------------
    zonasFuego.append(Muro(0, y_inferior, x_central, fuego_inferior_grosor));
    zonasFuego.append(Muro(x_central + pilar_ancho, y_inferior, NIVEL_WIDTH - (x_central + pilar_ancho), fuego_inferior_grosor));


    // --------------------------------------------------------------------------
    // 2. FUEGO QUE RODEA LA BARRA HORIZONTAL (Alineación con Y=175 y 35px)
    // --------------------------------------------------------------------------

    // 2.1. Borde Superior: (X=415, Y=140, Ancho=420, Alto=35)
    zonasFuego.append(Muro(X_FUEGO_IZQ, Y_FUEGO_SUPERIOR, ANCHO_TOTAL_FUEGO, fuego_grosor_borde_ajustado));

    // 2.2. Borde Inferior: (X=415, Y=225, Ancho=420, Alto=35)
    zonasFuego.append(Muro(X_FUEGO_IZQ, Y_FUEGO_INFERIOR_INICIO, ANCHO_TOTAL_FUEGO, fuego_grosor_borde_ajustado));

    // 2.3. Borde Izquierdo: (X=415, Y=140, Ancho=35, Alto=120)
    zonasFuego.append(Muro(X_FUEGO_IZQ, Y_FUEGO_SUPERIOR, fuego_grosor_borde_ajustado, ALTO_TOTAL_FUEGO));

    // 2.4. Borde Derecho: (X=800, Y=140, Ancho=35, Alto=120)
    zonasFuego.append(Muro(X_FUEGO_DER, Y_FUEGO_SUPERIOR, fuego_grosor_borde_ajustado, ALTO_TOTAL_FUEGO));


    // --------------------------------------------------------------------------
    // 3. FUEGO QUE RODEA EL PILAR VERTICAL (Conectado a Y=225 y con Grosor 35px)
    // --------------------------------------------------------------------------

    // Y de inicio del fuego vertical es el borde inferior de tu barra horizontal (Y=225 + 35 = 260)
    const int Y_INICIO_FUEGO_PILAR = Y_FUEGO_INFERIOR_INICIO + fuego_grosor_borde_ajustado; // 225 + 35 = 260
    const int Y_FINAL_FUEGO_PILAR = y_inferior; // 580

    const int altura_fuego_pilar = Y_FINAL_FUEGO_PILAR - Y_INICIO_FUEGO_PILAR; // 580 - 260 = 320

    // X Izquierdo y Derecho del pilar (Grosor de 35px)
    const int X_BORDE_IZQ_PILAR = x_central - fuego_grosor_borde_ajustado; // 595 - 35 = 560
    const int X_BORDE_DER_PILAR = x_central + pilar_ancho; // 595 + 60 = 655

    // Borde Izquierdo del Pilar: (X=560, Y=260, Ancho=35, Alto=320)
    zonasFuego.append(Muro(X_BORDE_IZQ_PILAR, Y_INICIO_FUEGO_PILAR, fuego_grosor_borde_ajustado, altura_fuego_pilar));

    // Borde Derecho del Pilar: (X=655, Y=260, Ancho=35, Alto=320)
    zonasFuego.append(Muro(X_BORDE_DER_PILAR, Y_INICIO_FUEGO_PILAR, fuego_grosor_borde_ajustado, altura_fuego_pilar));


    // --------------------------------------------------------------------------
    // 4. ZONAS CUADRADAS SUPERIORES (Se mantienen)
    // --------------------------------------------------------------------------
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

    // 1. Encontrar el fragmento en contacto (y actualizar el puntero de la clase)
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

    // --------------------------------------------------------------------------
    // LÓGICA DE RESETEO CRÍTICO Y TIEMPO DE RETENCIÓN (Se mantiene la tuya)
    // --------------------------------------------------------------------------
    // Si la tecla 'E' está presionada pero NO hay fragmento en contacto,
    // ¡RESETEAMOS EL TIEMPO INMEDIATAMENTE!
    if (sabioMaya.estaIntentandoRetener && sabioMaya.fragmentoEnContactoActual == nullptr) {
        if (sabioMaya.tiempoContactoFragMs != 0) {
            qDebug() << "RESETEO CRÍTICO: Perdimos contacto con el fragmento mientras 'E' estaba pulsada.";
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

        // 3. VERIFICACIÓN DE ÉXITO
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
            qDebug() << "RESETEO A 0. Causa: 'E' suelta o pérdida de contacto con fragmento.";
            sabioMaya.tiempoContactoFragMs = 0;
        }
    }

    // --------------------------------------------------------------------------
    // Colisiones con Muros (Se mantiene igual)
    // --------------------------------------------------------------------------
    if (!sabioMaya.estaIntentandoRetener) {
        for (const Muro &m : muros) {
            if (rectJugador.intersects(m.getRectanguloColision())) {
                float factorRebote = 0.5f;
                // Cálculo de rebote más simple (si se quita el else if del eje opuesto)
                sabioMaya.pos_x -= sabioMaya.velocidad * factorRebote * (sabioMaya.pos_x < m.pos_x ? 1 : -1);
                sabioMaya.pos_y -= sabioMaya.velocidad * factorRebote * (sabioMaya.pos_y < m.pos_y ? 1 : -1);
            }
        }
    }

    // --------------------------------------------------------------------------
    // Colisiones con Zonas de Fuego Fijas (¡PÉRDIDA INSTANTÁNEA!)
    // --------------------------------------------------------------------------
    for (const Muro &fuego : zonasFuego) {
        if (rectJugador.intersects(fuego.getRectanguloColision())) {
            if (juegoEstado == 0) { // Solo si estábamos jugando
                timerJuego->stop();
                juegoEstado = 2; // 2 = DERROTA ☠️
                qDebug() << "DERROTA. ¡Has entrado en la zona de fuego!";
                timerReinicio->start(4000); // Reiniciar en 4 segundos
            }
            return;
        }
    }
}

// niveluno.cpp (Implementación del nuevo slot)

// niveluno.cpp (Implementación del slot actualizarAnimacionFuego)

void Niveluno::actualizarAnimacionFuego() {
    // Si la lista está vacía o solo tiene 1 frame, no animamos
    if (framesFuego.size() < 2) {
              return;
        }
               // ** CORRECCIÓN 2: Lógica de ciclo de animación **
        // Fórmula: (Índice actual + 1) % Tamaño de la lista (automáticamente 4)
        currentFrameFuegoIndex = (currentFrameFuegoIndex + 1) % framesFuego.size();

        // Forzamos el repintado para que se muestre el nuevo frame
          update();
    }

// ----------------------------------------------------
// IMPLEMENTACIÓN DE EVENTOS (TECLADO Y PINTADO)
// ----------------------------------------------------
    void Niveluno::keyPressEvent(QKeyEvent *event) {
        if (juegoEstado != 0) return;
        float dx = 0.0f;
        float dy = 0.0f;

        // 1. Manejo del intento de retención ('E')
        if (event->key() == Qt::Key_E) {
            if (event->isAutoRepeat()) return;
            sabioMaya.estaIntentandoRetener = true;
            // ... (código existente de impresión de posición) ...
        }

        // 2. Manejo del movimiento y LÓGICA DE SPRITES
        if (!sabioMaya.estaIntentandoRetener) {

            // --- 2.1 Determinar dirección y movimiento ---
            if (event->key() == Qt::Key_W) {
                dy = -1.0f;
                animRowOffset = 12; // Fila 3: Arriba
            }
            if (event->key() == Qt::Key_S) {
                dy = 1.0f;
                animRowOffset = 0;  // Fila 0: Abajo
            }
            if (event->key() == Qt::Key_A) {
                dx = -1.0f;
                animRowOffset = 4;  // Fila 1: Izquierda
            }
            if (event->key() == Qt::Key_D) {
                dx = 1.0f;
                animRowOffset = 8;  // Fila 2: Derecha
            }

            if (dx != 0.0f || dy != 0.0f) {
                moverJugador(dx, dy);

                // --- 2.2 Lógica de Animación (Solo si se está moviendo) ---
                isMoving = true;

                // Establecer el primer frame de la nueva dirección inmediatamente
                currentFrameMayaIndex = animRowOffset;
            } else {
                // Si no se presionó W, A, S, o D, pero sí otra tecla (ej: Shift, Ctrl),
                // Aseguramos que isMoving sea false (aunque keyReleaseEvent es quien lo maneja mejor).
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
            juegoEstado = 1; // 1 = VICTORIA 🏆
            qDebug() << "¡VICTORIA! Nivel completado.";
            timerReinicio->start(4000); // Reiniciar en 4 segundos
        }
        return;
    }

    // ----------------------
    // LÓGICA DE DERROTA POR TIEMPO
    // ----------------------
    if (tiempoRestante <= 0) {
        if (juegoEstado == 0) { // Solo si estábamos jugando
            timerJuego->stop();
            juegoEstado = 2; // 2 = DERROTA ☠️
            qDebug() << "DERROTA. Tiempo agotado.";
            timerReinicio->start(4000); // Reiniciar en 4 segundos
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

        // Es CRÍTICO parar el tiempo de retención aquí o en actualizarJuego
        sabioMaya.tiempoContactoFragMs = 0;

        qDebug() << "TECLA 'E' LIBERADA. Bandera de retención desactivada.";
    }

    // 2. Lógica para soltar teclas de Movimiento (W, A, S, D)
    // Esto desactiva la animación y detiene el movimiento.
    if (event->key() == Qt::Key_W || event->key() == Qt::Key_S ||
        event->key() == Qt::Key_A || event->key() == Qt::Key_D) {

        // A. Lógica para la ANIMACIÓN (detener la caminata)
        isMoving = false;

        // Fija el sprite en la pose de "idle" (primer frame de la dirección actual)
        // El valor de animRowOffset ya contiene la dirección correcta (0, 4, 8, o 12).
        currentFrameMayaIndex = animRowOffset;

        // B. Lógica para el MOVIMIENTO (detener la velocidad)
        // Esto asume que tienes propiedades de velocidad en tu clase Jugador (sabioMaya).
        // Si usas moverJugador() en keyPressEvent, debes manejar el estado aquí también.

        // (Opcional, si tu clase Jugador maneja velocidad directa. Si usas moverJugador en keyPress,
        // la velocidad se ajusta en moverJugador, pero es bueno resetear si el movimiento es continuo)
        // sabioMaya.vel_x = 0;
        // sabioMaya.vel_y = 0;

        update(); // Forzar el repintado para mostrar la pose de reposo inmediatamente
    }

    // Llama a la implementación base para mantener el comportamiento estándar.
    QWidget::keyReleaseEvent(event);
}
// ----------------------------------------------------
// IMPLEMENTACIÓN DE MÉTODOS DE REINICIO (Slot conectado a timerReinicio)
// ----------------------------------------------------

// niveluno.cpp (Fragmento de reiniciarNivel)

void Niveluno::reiniciarNivel() {
    // Detener el timer de reinicio
    timerReinicio->stop();

    // ----------------------------------------------------
    // ** CORRECCIÓN CRÍTICA: REINICIALIZAR JUGADOR A 60x60 **
    // ----------------------------------------------------
    // Usamos el constructor de 4 parámetros (x, y, ancho, alto)
    sabioMaya = Jugador(600.0f, 80.0f, 60.0f, 60.0f,4.0f);

    // Reinicializar estados de juego y timers
    juegoEstado = 0;
    timerElapsedJuego.restart();

    // Reinicializar elementos del nivel (fragmentos, muros, etc.)
    inicializarFragmentos();
    // ... (otras inicializaciones que tengas)

    // Reiniciar los estados de animación
    animRowOffset = 0;
    isMoving = false;
    currentFrameMayaIndex = 0;

    // Iniciar el timer principal del juego
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

    // ----------------------------------------------------
    // 2. DIBUJADO DEL FONDO Y MÓDULOS DEL JUEGO
    // ----------------------------------------------------

    // Fondo
    QPixmap background_pixmap("C:/Users/alexa/Desktop/proyecto_final/videojuego_code/multimedia/imagenes/fondo_nivel1.jpg");
    QBrush background_brush(background_pixmap);
    background_brush.setStyle(Qt::TexturePattern);
    painter.fillRect(0, 0, NIVEL_WIDTH, NIVEL_HEIGHT, background_brush);

    // Muros
    QPixmap muro_pixmap("C:/Users/alexa/Desktop/proyecto_final/videojuego_code/multimedia/imagenes/paredes_nivel1.jpg");
    QBrush muro_brush;

    if (!muro_pixmap.isNull()) {
        muro_brush = QBrush(muro_pixmap);
        muro_brush.setStyle(Qt::TexturePattern);
    } else {
        qDebug() << "ADVERTENCIA CRÍTICA: ¡No se pudo cargar la imagen del muro! Usando gris sólido.";
        muro_brush = QBrush(QColor(80, 80, 80));
    }

    painter.setBrush(muro_brush);
    painter.setPen(Qt::NoPen);

    for (const Muro &m : muros) {
        QRect rect = m.getRectanguloColision();
        painter.drawRect(rect);
    }

    // Dibujado de Zonas de Fuego Fijas (Animación)
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

        // 1. Configuración del Borde (Negro)
        QPen borderPen(Qt::black);
        borderPen.setWidth(2);
        painter.setPen(borderPen);

        // 2. Determinar el color de relleno
        if (f.estaQuemado) {
            painter.setBrush(Qt::darkGray);
        } else {
            painter.setBrush(Qt::yellow);
        }

        // 3. Dibujar la elipse/círculo
        painter.drawEllipse(f.getRectanguloColision());
    }

    // Dibujado del Jugador (Sabio Maya)
    // ------------------------------------------------------------------
    if (!framesMaya.isEmpty() && currentFrameMayaIndex < framesMaya.size()) {
        QPixmap playerFrame = framesMaya.at(currentFrameMayaIndex);

        painter.setCompositionMode(QPainter::CompositionMode_SourceOver); // Para usar la transparencia
        // Dibuja el frame actual (60x60) en el rectángulo de colisión del jugador.
        painter.drawPixmap(sabioMaya.getRectanguloColision(), playerFrame);
    } else {
        // Fallback: Si los sprites fallan, mantiene el círculo verde
        painter.setBrush(Qt::darkGreen);
        painter.setPen(Qt::white);
        painter.drawEllipse(sabioMaya.getRectanguloColision());
    }

    // ----------------------------------------------------
    // 3. DIBUJADO DE LA INTERFAZ DE USUARIO (HUD) - MODIFICADO
    // ----------------------------------------------------

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


    // --- MENSAJE "RESCATANDO..." (Barra de Progreso) - MODIFICADO ---
    if (sabioMaya.tiempoContactoFragMs > 0) {
        qreal progreso = (timerElapsedJuego.elapsed() - sabioMaya.tiempoContactoFragMs) / (qreal)TIEMPO_RETENCION_MS;
        if (progreso > 1.0) progreso = 1.0;

        // Texto Azul Claro (Cian)
        painter.setPen(Qt::cyan);
        painter.setFont(QFont("Times New Roman", 18, QFont::Bold));

        painter.drawText(NIVEL_WIDTH / 2 - 100, NIVEL_HEIGHT - 50,
                         QString("RESCATANDO... %1%").arg(static_cast<int>(progreso * 100)));

        // Barra de progreso visual (Relleno Azul Cian)
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(30, 30, 30));
        painter.drawRect(NIVEL_WIDTH / 2 - 100, NIVEL_HEIGHT - 40, 200, 10);

        painter.setBrush(Qt::cyan);
        painter.drawRect(NIVEL_WIDTH / 2 - 100, NIVEL_HEIGHT - 40, 200 * progreso, 10);
    }

    // --- MENSAJES FINALES (VICTORIA / DERROTA) - CORREGIDOS ---
    if (juegoEstado != 0) {
        QString mensaje = "";
        QColor colorFondo;

        if (juegoEstado == 1) { // VICTORIA
            mensaje = "🏆 ¡VICTORIA! Nivel Completado 🏆";
            colorFondo = QColor(50, 200, 50, 180);
        } else if (juegoEstado == 2) { // DERROTA
            mensaje = "DERROTA - JUEGO TERMINADO ️";
            colorFondo = QColor(200, 50, 50, 180);
        }

        // Fondo semi-transparente
        painter.setBrush(colorFondo);
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, 0, NIVEL_WIDTH, NIVEL_HEIGHT);

        // Mensaje Principal (Fuente y Posición Corregida)
        painter.setPen(Qt::white);
        painter.setFont(QFont("Times New Roman", 36, QFont::Bold)); // Tamaño ajustado a 36

        int centerX = NIVEL_WIDTH / 2;
        int centerY = NIVEL_HEIGHT / 2;

        // Posición X ajustada a -200 para centrado y evitar cortes
        painter.drawText(centerX - 200, centerY - 20, mensaje);

        // Mensaje de reinicio
        painter.setFont(QFont("Times New Roman", 18));
        painter.drawText(centerX - 180, centerY + 40, "Reiniciando en 4 segundos...");
    }
}
