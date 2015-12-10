#ifndef TRACKINGCONFIG_H
#define TRACKINGCONFIG_H

#include <QDialog>

namespace Ui {
class TrackingConfig;
}

class TrackingConfig : public QDialog
{
    Q_OBJECT

public:
    explicit TrackingConfig(QWidget *parent = 0);
    ~TrackingConfig();

private:
    Ui::TrackingConfig *ui;
};

#endif // TRACKINGCONFIG_H
