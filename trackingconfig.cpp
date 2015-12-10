#include "trackingconfig.h"
#include "ui_trackingconfig.h"

TrackingConfig::TrackingConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TrackingConfig)
{
    ui->setupUi(this);
}

TrackingConfig::~TrackingConfig()
{
    delete ui;
}
