#include "decimalslider.h"
#include <QLabel>
#include <QSlider>

DecimalSlider::DecimalSlider(QWidget *parent) :
    QSlider(parent)
{
    connect(this, SIGNAL(valueChanged(int)),
            this, SLOT(notifyValueChanged(int)));

    this->setOrientation(Qt::Horizontal);
    this->setRange(0, 1.0 * 1.0);
    this->numStep = 1.0;
}

void DecimalSlider::setParameters(double numStep, double maxValue, double minValue)
{
    this->numStep = numStep;
    this->setRange(minValue, numStep * maxValue);
}

void DecimalSlider::notifyValueChanged(int value) {
    double doubleValue = value / this->numStep;
    emit doubleValueChanged(doubleValue);
}
