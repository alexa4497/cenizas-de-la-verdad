#include "niveluno.h"
#include "ui_niveluno.h"

Niveluno::Niveluno(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Niveluno)
{
    ui->setupUi(this);
}

Niveluno::~Niveluno()
{
    delete ui;
}
