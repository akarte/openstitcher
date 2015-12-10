#include "config.h"
#include "ui_config.h"
#include "stitching.h"

#include <QFileDialog>
#include <QSettings>

Config::Config(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Config)
{
    ui->setupUi(this);



    ui->sliderblendings->setParameters(numSteps, 100.0, 0.0);
    ui->slidercompose->setParameters(numSteps, 1.0, 0.0);
    ui->sliderconfidence->setParameters(numSteps, 1.0, 0.0);
    ui->sliderseammpx->setParameters(numSteps, 1.0, 0.0);
    ui->sliderthreshold->setParameters(numSteps, 1.0, 0.0);
    ui->sliderworkmpx->setParameters(numSteps, 1.0, 0.0);

    connect(ui->sliderconfidence, SIGNAL(doubleValueChanged(double)),
                this, SLOT(updateLConfidenceValue(double)));
    connect(ui->sliderblendings, SIGNAL(doubleValueChanged(double)),
                this, SLOT(updateLBlendingValue(double)));
    connect(ui->slidercompose, SIGNAL(doubleValueChanged(double)),
                this, SLOT(updateLComposeValue(double)));
    connect(ui->sliderseammpx, SIGNAL(doubleValueChanged(double)),
                this, SLOT(updateLSeammpxValue(double)));
    connect(ui->sliderthreshold, SIGNAL(doubleValueChanged(double)),
                this, SLOT(updateLThresholdValue(double)));
    connect(ui->sliderworkmpx, SIGNAL(doubleValueChanged(double)),
                this, SLOT(updateLWorkmpxValue(double)));

    initializeParameters();
}

Config::~Config()
{
    delete ui;
}

void Config::initializeParameters()
{
    // Quality & Performance variables
    ui->checkpreview->setChecked(Stitching::preview);
    ui->checkgpu->setChecked(Stitching::try_gpu);

    // Motion estimation variables
    ui->combofeature->setCurrentText(QString::fromStdString(Stitching::features_type));
    ui->lconfidenceval->setText(QString::number(Stitching::match_conf, 'f', 2));
    ui->sliderconfidence->setValue(Stitching::match_conf * numSteps);
    ui->lworkmpxvalue->setText(QString::number(Stitching::work_megapix, 'f', 2));
    ui->sliderworkmpx->setValue(Stitching::work_megapix * numSteps);
    ui->lthresholdvalue->setText(QString::number(Stitching::conf_thresh, 'f', 2));
    ui->sliderthreshold->setValue(Stitching::conf_thresh * numSteps);
    ui->combocost->setCurrentText(QString::fromStdString(Stitching::ba_cost_func));
    ui->checkBox->setChecked(Stitching::ba_refine_mask.substr(4, 1) == "x");
    ui->checkBox_2->setChecked(Stitching::ba_refine_mask.substr(3, 1) == "x");
    ui->checkBox_3->setChecked(Stitching::ba_refine_mask.substr(2, 1) == "x");
    ui->checkBox_4->setChecked(Stitching::ba_refine_mask.substr(1, 1) == "x");
    ui->checkBox_5->setChecked(Stitching::ba_refine_mask.substr(0, 1) == "x");
    ui->checkwave->setChecked(Stitching::do_wave_correct);
    if(Stitching::wave_correct == detail::WAVE_CORRECT_HORIZ)
        ui->combowave->setCurrentText("horizontal");
    else if (Stitching::wave_correct == detail::WAVE_CORRECT_VERT)
        ui->combowave->setCurrentText("vertical");
    ui->combowave->setEnabled(Stitching::do_wave_correct);

    // Compositing variables
    ui->combowarp->setCurrentText(QString::fromStdString(Stitching::warp_type));
    ui->lseammpxvalue->setText(QString::number(Stitching::seam_megapix, 'f', 2));
    ui->sliderseammpx->setValue(Stitching::seam_megapix * numSteps);
    ui->comboseam->setCurrentText(QString::fromStdString(Stitching::seam_find_type));
    ui->checkcompose->setChecked(Stitching::compose_megapix != -1);
    ui->lcomposevalue->setText(QString::number(Stitching::compose_megapix, 'f', 2));
    ui->slidercompose->setValue(Stitching::compose_megapix * numSteps);
    if(Stitching::compose_megapix == -1)
        ui->slidercompose->setEnabled(false);
    else
        ui->slidercompose->setEnabled(true);

    if(Stitching::expos_comp_type == ExposureCompensator::NO)
        ui->comboexposure->setCurrentText("no");
    else if(Stitching::expos_comp_type == ExposureCompensator::GAIN)
        ui->comboexposure->setCurrentText("gain");
    else if(Stitching::expos_comp_type == ExposureCompensator::GAIN_BLOCKS)
        ui->comboexposure->setCurrentText("gain_blocks");

    if(Stitching::blend_type == Blender::NO)
        ui->comboblendingt->setCurrentText("no");
    else if (Stitching::blend_type == Blender::FEATHER)
        ui->comboblendingt->setCurrentText("feather");
    else if (Stitching::blend_type == Blender::MULTI_BAND)
        ui->comboblendingt->setCurrentText("multiband");

    ui->lblendingsvalue->setText(QString::number(Stitching::blend_strength, 'f', 2));
    ui->sliderblendings->setValue(Stitching::blend_strength * numSteps);
}

void Config::updateLConfidenceValue(double value) {
    ui->lconfidenceval->setText(QString::number(value, 'f', 2));
    Stitching::match_conf = (float) value;
}
void Config::updateLBlendingValue(double value) {
    ui->lblendingsvalue->setText(QString::number(value, 'f', 2));
    Stitching::blend_strength = (float) value;
}
void Config::updateLComposeValue(double value) {
    ui->lcomposevalue->setText(QString::number(value, 'f', 2));
    Stitching::compose_megapix = value;
}
void Config::updateLSeammpxValue(double value) {
    ui->lseammpxvalue->setText(QString::number(value, 'f', 2));
    Stitching::seam_megapix = value;
}
void Config::updateLThresholdValue(double value) {
    ui->lthresholdvalue->setText(QString::number(value, 'f', 2));
    Stitching::conf_thresh = value;
}
void Config::updateLWorkmpxValue(double value) {
    ui->lworkmpxvalue->setText(QString::number(value, 'f', 2));
    Stitching::work_megapix = value;
}

void Config::on_combofeature_activated(const QString &arg1)
{
    Stitching::features_type = (string) arg1.toUtf8();
}

void Config::on_combocost_activated(const QString &arg1)
{
    Stitching::ba_cost_func = (string) arg1.toUtf8();
}

void Config::on_checkBox_5_clicked(bool checked)
{
    Stitching::ba_refine_mask.replace(0, 1, checked ? "x" : "_");
}

void Config::on_checkBox_4_clicked(bool checked)
{
    Stitching::ba_refine_mask.replace(1, 1, checked ? "x" : "_");
}

void Config::on_checkBox_3_clicked(bool checked)
{
    Stitching::ba_refine_mask.replace(2, 1, checked ? "x" : "_");
}

void Config::on_checkBox_2_clicked(bool checked)
{
    Stitching::ba_refine_mask.replace(3, 1, checked ? "x" : "_");
}

void Config::on_checkBox_clicked(bool checked)
{
    Stitching::ba_refine_mask.replace(4, 1, checked ? "x" : "_");
}

void Config::on_combowave_activated(const QString &arg1)
{
    string w_c = (string) arg1.toUtf8();
    if(w_c == "horizontal")
        Stitching::wave_correct = detail::WAVE_CORRECT_HORIZ;
    else if (w_c == "vertical")
        Stitching::wave_correct = detail::WAVE_CORRECT_VERT;
}

void Config::on_checkwave_clicked(bool checked)
{
    ui->combowave->setEnabled(checked);
    Stitching::do_wave_correct = checked;
}

void Config::on_combowarp_activated(const QString &arg1)
{
    Stitching::warp_type = (string) arg1.toUtf8();
}

void Config::on_comboseam_activated(const QString &arg1)
{
    Stitching::seam_find_type = (string) arg1.toUtf8();
}

void Config::on_checkcompose_clicked(bool checked)
{
    ui->slidercompose->setEnabled(checked);
    if(!checked)
    {
        Stitching::compose_megapix = -1;
        ui->lcomposevalue->setText(QString::number(Stitching::compose_megapix, 'f', 2));
    }
}

void Config::on_comboexposure_activated(const QString &arg1)
{
    if(arg1 == "no")
        Stitching::expos_comp_type = ExposureCompensator::NO;
    else if (arg1 == "gain")
        Stitching::expos_comp_type = ExposureCompensator::GAIN;
    else if (arg1 == "gain_blocks")
        Stitching::expos_comp_type = ExposureCompensator::GAIN_BLOCKS;
}

void Config::on_comboblendingt_activated(const QString &arg1)
{
    if(arg1 == "no")
        Stitching::blend_type = Blender::NO;
    else if(arg1 == "feather")
        Stitching::blend_type = Blender::FEATHER;
    else if(arg1 == "multiband")
        Stitching::blend_type = Blender::MULTI_BAND;
}
void Config::on_checkgpu_clicked(bool checked)
{
    Stitching::try_gpu = checked;
}

void Config::on_checkpreview_clicked(bool checked)
{
    Stitching::preview = checked;
}

void Config::on_btnloadpreset_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select Preset File"),QDir::currentPath(),tr("Stitching Preset (*.spt)"));
    QSettings settings(filename, QSettings::IniFormat);
    Stitching::InitParameters(&settings);
    initializeParameters();
}

void Config::on_btnsavepreset_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Set Preset Name"), QDir::currentPath(), tr("Stitching Preset (*.spt)"));
    QSettings settings(filename, QSettings::IniFormat);
    Stitching::SaveParameters(&settings);
}

void Config::on_btndefaultpreset_clicked()
{
    QString filename = QDir::currentPath() + QString("/stitching.ini");
    QSettings settings(filename, QSettings::IniFormat);
    Stitching::SaveParameters(&settings);
}
