#include "types.h"
#include "ui_types.h"

typy::typy(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::typy)
{
    ui->setupUi(this);
}

typy::~typy()
{
    delete ui;
}
