#include "vendors.h"
#include "ui_vendors.h"

vendors::vendors(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::vendors)
{
    ui->setupUi(this);
}

vendors::~vendors()
{
    delete ui;
}
