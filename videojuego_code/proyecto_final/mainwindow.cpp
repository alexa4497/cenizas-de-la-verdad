#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPushButton>
#include <QDebug>
#include "niveluno.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // 1. Inicialización y Diseño
    ui->setupUi(this);
    this->resize(400, 300);


    QString imagePath = "C:/Users/alexa/Desktop/proyecto_final/videojuego_code/multimedia/imagenes/img_inicio.png";

    this->setStyleSheet(QString(
                            "QMainWindow {"
                            "background-image: url(%1);"
                            "background-repeat: no-repeat;"
                            "background-position: center;"
                            "}"
                            ).arg(imagePath));




    connect(ui->btnNivel1, &QPushButton::clicked, this, &MainWindow::abrirNivelUno);
    connect(ui->btnNivel2, &QPushButton::clicked, this, &MainWindow::abrirNivelDos);
    connect(ui->btnNivel3, &QPushButton::clicked, this, &MainWindow::abrirNivelTres); // <- CORREGIDO: Usando iniciarNivel3
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 4. Implementación de Slots (Tus funciones)

void MainWindow::abrirNivelUno() {
    // 1. Ocultar la ventana del menú principal
    this->hide();

    // 2. Crear una nueva instancia del Nivel 1
    // Es importante usar 'nullptr' para que la ventana del nivel sea independiente.
    Niveluno *nivel = new Niveluno(nullptr);

    // 3. Mostrar la ventana del Nivel 1
    nivel->show();

    // 4. Asegurar que el objeto Niveluno se destruya automáticamente cuando se cierre su ventana.
    nivel->setAttribute(Qt::WA_DeleteOnClose);
}

void MainWindow::abrirNivelDos()
{
    qDebug() << "--- Nivel 2 CONECTADO ---";
}

void MainWindow::abrirNivelTres()
{
    qDebug() << "--- Nivel 3 CONECTADO ---";
}
