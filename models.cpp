#include "models.h"
#include "ui_models.h"

models::models(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::models)
{
    ui->setupUi(this);
}

models::~models()
{
    delete ui;
}
