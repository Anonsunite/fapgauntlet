#include "settingswindow.h"
#include "ui_settingswindow.h"

#include "image.h"
#include "settings.h"

#include <QMessageBox>

#include <fstream>
#include <sstream>

SettingsWindow::SettingsWindow(Settings& s, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SettingsWindow),
    settings(&s)
{
    ui->setupUi(this);

    ui->checkBox->setChecked(settings->saveOptions);
    ui->checkBox_2->setChecked(!settings->loadLastImages.empty());
    ui->checkBox_2->setEnabled(settings->saveOptions);
    ui->pauseHorizontalSlider->setValue(settings->pause / 80);
    ui->speedHorizontalSlider->setValue(settings->speed);

    QString str;
    for(unsigned int i = 0; i < hardcodedDescList.size(); ++i)
    {
        str += hardcodedDescList[i];

        if(i + 1 < hardcodedDescList.size())
            str += ",";
    }
    ui->textEdit->setText(str);

    this->setWindowFlags(this->windowFlags() | Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

bool SettingsWindow::restoreSession() const
{
    return ui->checkBox_2->isChecked();
}

void SettingsWindow::closeEvent(QCloseEvent *ev)
{
    emit closed();

    QMainWindow::closeEvent(ev);
}

void SettingsWindow::on_checkBox_clicked()
{
    bool checked = ui->checkBox->isChecked();

    ui->checkBox_2->setEnabled(checked);
    settings->saveOptions = checked;
}

void SettingsWindow::parseDescriptions()
{
    hardcodedDescList.clear();

    std::stringstream ss(ui->textEdit->toPlainText().toStdString());
    for(std::string s; std::getline(ss, s, ','); hardcodedDescList.push_back(QString::fromStdString(s)))
    {  }
}

void SettingsWindow::on_textEdit_textChanged()
{
    if(ui->textEdit->toPlainText().size() == 0)
    {
        ui->textEdit->setText("empty");
    }

    parseDescriptions();
}

void SettingsWindow::on_pauseHorizontalSlider_sliderReleased()
{
    settings->pause = ui->pauseHorizontalSlider->value() * 80;
}

void SettingsWindow::on_speedHorizontalSlider_sliderReleased()
{
    settings->speed = ui->speedHorizontalSlider->value();
}

void SettingsWindow::on_pushButton_clicked()
{
    close();
}
