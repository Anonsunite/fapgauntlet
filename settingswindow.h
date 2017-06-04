#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QMainWindow>

namespace Ui
{
class SettingsWindow;
}

class Settings;

class SettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SettingsWindow(Settings& s, QWidget *parent = 0);
    ~SettingsWindow();

    bool restoreSession() const;
    void closeEvent(QCloseEvent *event);
signals:
    void closed();
private slots:
    void on_checkBox_clicked();
    void on_textEdit_textChanged();
    void on_pauseHorizontalSlider_sliderReleased();
    void on_speedHorizontalSlider_sliderReleased();
    void on_pushButton_clicked();
    void on_checkBox_3_clicked();

private:
    void parseDescriptions();
private:
    Ui::SettingsWindow *ui;
    Settings* settings;
};

#endif // SETTINGSWINDOW_H
