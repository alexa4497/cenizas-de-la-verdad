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

void MainWindow::abrirNivelUno()
{
    qDebug() << "--- Nivel 1 CONECTADO y Abriendo Niveluno ---";
    // Usamos el nombre de tu clase: Niveluno
    Niveluno *nivel_1 = new Niveluno(this);
    nivel_1->show();
    this->hide();
}

void MainWindow::abrirNivelDos()
{
    qDebug() << "--- Nivel 2 CONECTADO ---";
}

void MainWindow::abrirNivelTres()
{
    qDebug() << "--- Nivel 3 CONECTADO ---";
}
