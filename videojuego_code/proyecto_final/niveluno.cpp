#include "niveluno.h"
#include "ui_niveluno.h"
#include <QDebug>
#include <QPainter>
#include <QKeyEvent>
#include <algorithm>
#include <iostream>
#include <cmath> // Necesario para std::min, std::max y sqrt

// Definición de constantes de tamaño del nivel
const int NIVEL_WIDTH = 1250;
const int NIVEL_HEIGHT = 650;

// ----------------------------------------------------
// CONSTRUCTOR Y DESTRUCTOR
// ----------------------------------------------------

// NOTA: Asumiendo que QList<Muro> zonasFuego está declarado en niveluno.h
Niveluno::Niveluno(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Niveluno)
    // 1. Posición del jugador: SIN CAMBIOS (600.0f, 100.0f)
    , sabioMaya(600.0f, 100.0f)
    // 2. Agente de Fuego: Fuera del mapa (pos 2000, 2000) ya que es estático.
    , agenteFuego(2000.0f, 2000.0f)
    // Inicialización de Constantes
    , FRAGMENTOS_REQUERIDOS(8)
    , TIEMPO_LIMITE_SEGUNDOS(80)
    , TIEMPO_RETENCION_MS(3000)
    ,juegoEstado(0)
{
    ui->setupUi(this);
    this->setMinimumSize(NIVEL_WIDTH, NIVEL_HEIGHT);
    setFocusPolicy(Qt::StrongFocus);

    timerJuego = new QTimer(this);
    connect(timerJuego, &QTimer::timeout, this, &Niveluno::actualizarJuego);

    // ** CORRECCIÓN: INICIALIZACIÓN Y CONEXIÓN DE timerReinicio **
    timerReinicio = new QTimer(this);
    connect(timerReinicio, &QTimer::timeout, this, &Niveluno::reiniciarNivel);
    // -------------------------------------------------------------

    inicializarFragmentos();
    inicializarMuros();
    inicializarZonasFuego(); // ¡Añadido!

    // Inicio del Juego
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
        // ***************************************************************
        // AÑADIR: Imprimir la posición exacta en la consola al presionar 'E'
        // ***************************************************************
        int pos_x = static_cast<int>(sabioMaya.pos_x);
        int pos_y = static_cast<int>(sabioMaya.pos_y);

        qDebug() << "-----------------------------------------------";
        qDebug() << "POSICIÓN FRAGMENTO:";
        qDebug() << "X:" << pos_x << " | Y:" << pos_y;
        qDebug() << "-----------------------------------------------";
        // ***************************************************************
    }

    // 2. Manejo del movimiento
    if (!sabioMaya.estaIntentandoRetener) {
        if (event->key() == Qt::Key_W) dy = -1.0f;
        if (event->key() == Qt::Key_S) dy = 1.0f;
        if (event->key() == Qt::Key_A) dx = -1.0f;
        if (event->key() == Qt::Key_D) dx = 1.0f;

        if (dx != 0.0f || dy != 0.0f) {
            moverJugador(dx, dy);
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
    if (event->isAutoRepeat()) return;

    if (event->key() == Qt::Key_E) {
        sabioMaya.estaIntentandoRetener = false;
        qDebug() << "TECLA 'E' LIBERADA. Bandera de retención desactivada.";
    }

    QWidget::keyReleaseEvent(event);
}

// ----------------------------------------------------
// IMPLEMENTACIÓN DE MÉTODOS DE REINICIO (Slot conectado a timerReinicio)
// ----------------------------------------------------

void Niveluno::reiniciarNivel() {
    timerReinicio->stop();

    // 1. Reiniciar el estado del Jugador (sabioMaya)
    // Se crea una nueva instancia de SabioMaya en la posición inicial (600.0f, 100.0f)
    sabioMaya = Jugador(600.0f, 100.0f);

    // 2. Resetear contadores y banderas del jugador
    sabioMaya.fragmentosSalvados = 0;
    sabioMaya.tiempoContactoFragMs = 0;
    sabioMaya.estaIntentandoRetener = false;

    // 3. Reiniciar el estado del Nivel
    juegoEstado = 0; // Vuelve a 0 (Jugando)
    timerElapsedJuego.restart(); // Reiniciar el contador de tiempo de juego

    // 4. Reiniciar los elementos dinámicos
    inicializarFragmentos(); // Vuelve a colocar todos los fragmentos
    // Los muros y el fuego no necesitan reiniciarse a menos que cambien.

    // 5. Reanudar el juego principal
    timerJuego->start(16); // Inicia el ciclo de juego (~60 FPS)

    // Forzar la actualización visual
    update();

    qDebug() << "Juego Reiniciado y listo para jugar.";
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
    // --- MODIFICACIÓN PARA FONDO TIPO MOSAICO ---
    // 1. Cargar la imagen: Asegúrate de que la ruta ':/recursos/fondo_mosaico.png'
    //    es correcta y está en tu archivo de recursos (.qrc).
    QPixmap background_pixmap("C:/Users/alexa/Desktop/proyecto_final/videojuego_code/multimedia/imagenes/fondo_nivel1.jpg");

    // 2. Crear un QBrush con la imagen y establecer el patrón de textura (mosaico)
    QBrush background_brush(background_pixmap);
    background_brush.setStyle(Qt::TexturePattern); // Esto hace el efecto mosaico

    // 3. Dibujar el fondo usando el QBrush (cubre todo el nivel)
    painter.fillRect(0, 0, NIVEL_WIDTH, NIVEL_HEIGHT, background_brush);

    // 1. **CORRECCIÓN CRÍTICA DE RUTA:** // Usa la ruta del sistema de archivos *si es absolutamente necesario* (no recomendado),
    // o la ruta de recursos si la imagen está en el archivo .qrc (RECOMENDADO).
    // Intenta la ruta de recursos (asumiendo que corregiste el .qrc y es .jpg)
    QPixmap muro_pixmap("C:/Users/alexa/Desktop/proyecto_final/videojuego_code/multimedia/imagenes/paredes_nivel1.jpg"); // <--- UTILIZA ESTA RUTA SI EL ARCHIVO ESTÁ EN EL .QRC

    // 2. Configuración del pincel y Fallback de color
    QBrush muro_brush;

    if (!muro_pixmap.isNull()) {
        // Si la carga fue exitosa, usa la textura
        muro_brush = QBrush(muro_pixmap);
        muro_brush.setStyle(Qt::TexturePattern);
    } else {
        // Si la carga falló, usa un color gris sólido y avisa
        qDebug() << "ADVERTENCIA CRÍTICA: ¡No se pudo cargar la imagen del muro! Usando gris sólido.";
        muro_brush = QBrush(QColor(80, 80, 80));
    }

    // 3. Aplicar el pincel y quitar el borde
    painter.setBrush(muro_brush);
    painter.setPen(Qt::NoPen);

    for (const Muro &m : muros) {
        QRect rect = m.getRectanguloColision();

        // Rellena el muro. Si la imagen falló, el muro será GRIS SÓLIDO.
        // Si la imagen cargó, el muro será la TEXTURA.
        painter.drawRect(rect);
    }

    // Dibujado de Zonas de Fuego Fijas
    painter.setBrush(Qt::red); // ¡Rojo para el peligro!
    painter.setPen(Qt::NoPen);
    for (const Muro &fuego : zonasFuego) {
        painter.drawRect(fuego.getRectanguloColision());
    }

    // Dibujado de Fragmentos
    for (const Fragmento &f : fragmentos) {
        if (!f.estaSalvado && !f.estaQuemado) {
            painter.setBrush(Qt::yellow);
            painter.setPen(Qt::NoPen);
            painter.drawRect(f.getRectanguloColision());
        }
    }

    // Dibujado del Jugador (Sabio Maya)
    painter.setBrush(Qt::darkGreen);
    painter.setPen(Qt::white);
    painter.drawEllipse(sabioMaya.getRectanguloColision());

    // ----------------------------------------------------
    // 3. DIBUJADO DE LA INTERFAZ DE USUARIO (HUD)
    // ----------------------------------------------------

    // Dibujado del Agente de Fuego (invisible, fuera de pantalla)
    // El Agente de Fuego ya no es relevante, pero se mantiene su dibujado si insiste.
    painter.setBrush(Qt::red);
    painter.drawRect(agenteFuego.getRectanguloColision());

    // Texto de Fragmentos
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10));
    painter.drawText(20, 20, QString("Fragmentos: %1 / %2").arg(sabioMaya.fragmentosSalvados).arg(FRAGMENTOS_REQUERIDOS));

    // Texto de Tiempo Restante
    int tiempoRestante = TIEMPO_LIMITE_SEGUNDOS - (timerElapsedJuego.elapsed() / 1000);
    painter.drawText(20, 40, QString("Tiempo: %1 s").arg(tiempoRestante));

    // Mensaje "RESCATANDO..." (Barra de Progreso)
    if (sabioMaya.tiempoContactoFragMs > 0) {
        qreal progreso = (timerElapsedJuego.elapsed() - sabioMaya.tiempoContactoFragMs) / (qreal)TIEMPO_RETENCION_MS;
        if (progreso > 1.0) progreso = 1.0;

        painter.setPen(Qt::green);
        painter.setFont(QFont("Arial", 12, QFont::Bold));

        painter.drawText(NIVEL_WIDTH / 2 - 100, NIVEL_HEIGHT - 50,
                         QString("RESCATANDO... %1%").arg(static_cast<int>(progreso * 100)));

        // Barra de progreso visual
        painter.setBrush(QColor(30, 30, 30));
        painter.drawRect(NIVEL_WIDTH / 2 - 100, NIVEL_HEIGHT - 40, 200, 10);
        painter.setBrush(Qt::green);
        painter.drawRect(NIVEL_WIDTH / 2 - 100, NIVEL_HEIGHT - 40, 200 * progreso, 10);
    }

    if (juegoEstado != 0) {
        QString mensaje = "";
        QColor colorFondo;

        if (juegoEstado == 1) { // VICTORIA
            mensaje = "🏆 ¡VICTORIA! Nivel Completado 🏆";
            colorFondo = QColor(50, 200, 50, 180); // Verde semi-transparente
        } else if (juegoEstado == 2) { // DERROTA
            mensaje = "☠️ DERROTA - JUEGO TERMINADO ☠️";
            colorFondo = QColor(200, 50, 50, 180); // Rojo semi-transparente
        }

        // Fondo semi-transparente (cubre toda la pantalla)
        painter.setBrush(colorFondo);
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, 0, NIVEL_WIDTH, NIVEL_HEIGHT);

        // Mensaje Principal
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 40, QFont::Bold));

        // Cálculo del centro de la pantalla
        int centerX = NIVEL_WIDTH / 2;
        int centerY = NIVEL_HEIGHT / 2;

        // Dibuja el mensaje (posición aproximada para centrar)
        painter.drawText(centerX - 250, centerY - 20, mensaje);

        // Mensaje de reinicio
        painter.setFont(QFont("Arial", 18));
        painter.drawText(centerX - 200, centerY + 40, "Reiniciando en 4 segundos...");
    }
}
