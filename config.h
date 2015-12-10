#ifndef CONFIG_H
#define CONFIG_H

#include <QDialog>

namespace Ui {
class Config;
}

class Config : public QDialog
{
    Q_OBJECT

public:
    explicit Config(QWidget *parent = 0);
    ~Config();
private:
    void initializeParameters();
    static const double numSteps = 100;
public slots:
    void updateLConfidenceValue(double value);
    void updateLBlendingValue(double value);
    void updateLComposeValue(double value);
    void updateLSeammpxValue(double value);
    void updateLThresholdValue(double value);
    void updateLWorkmpxValue(double value);
private slots:
    void on_combofeature_activated(const QString &arg1);
    void on_combocost_activated(const QString &arg1);
    void on_checkBox_5_clicked(bool checked);
    void on_checkBox_4_clicked(bool checked);
    void on_checkBox_3_clicked(bool checked);
    void on_checkBox_2_clicked(bool checked);
    void on_checkBox_clicked(bool checked);
    void on_combowave_activated(const QString &arg1);
    void on_checkwave_clicked(bool checked);
    void on_combowarp_activated(const QString &arg1);
    void on_comboseam_activated(const QString &arg1);
    void on_checkcompose_clicked(bool checked);
    void on_comboexposure_activated(const QString &arg1);
    void on_comboblendingt_activated(const QString &arg1);
    void on_checkgpu_clicked(bool checked);
    void on_checkpreview_clicked(bool checked);
    void on_btnloadpreset_clicked();
    void on_btnsavepreset_clicked();

    void on_btndefaultpreset_clicked();

private:
    Ui::Config *ui;
};

#endif // CONFIG_H
