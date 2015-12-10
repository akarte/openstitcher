#ifndef DECIMALSLIDER_H
#define DECIMALSLIDER_H

#include <QSlider>
#include <QLabel>

class DecimalSlider : public QSlider
{
    Q_OBJECT
public:
    explicit DecimalSlider(QWidget *parent = 0);
    void setParameters(double numStep, double maxValue, double minValue);
private:
    double numStep;
signals:
    void doubleValueChanged(double value);
public slots:
    void notifyValueChanged(int value);
};

#endif // DECIMALSLIDER_H
