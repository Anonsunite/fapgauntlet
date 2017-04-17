#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QGraphicsDropShadowEffect>
#include <QMainWindow>
#include <QUrl>

#include <memory>
#include <array>

namespace Ui
{
    class DownloadManager;
}

class QTreeWidgetItem;
class QWebEngineView;
class QDownloader;

extern const std::string boardlist[];

class DownloadManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit DownloadManager(QWidget *parent = 0);
    ~DownloadManager();

    void closeEvent(QCloseEvent *event);
    std::string currentBoard() const;
    int currentBoardInt() const;
    std::string pageHTML() const;
    bool streamFiles() const;
    QString url() const;
public slots:
    void updateAfterCheckboxClick();
private slots:
    void on_okButton_released();
    void on_verticalSlider_valueChanged(int value);
    void urlChange(QUrl url);
    void on_pushButton_clicked();
    void handleHTML(QString html);
    void pageLoadingDone(bool b);
    void jsonDownloadDone();
    void on_lineEdit_textChanged(const QString &arg1);
    void on_b_checkBox_clicked();
    void on_y_checkBox_clicked();
    void on_d_checkBox_clicked();
    void on_u_checkBox_clicked();
    void on_e_checkBox_clicked();
    void on_h_checkBox_clicked();
    void on_hm_checkBox_clicked();
    void on_hc_checkBox_clicked();
    void on_s_checkBox_clicked();
    void on_hr_checkBox_clicked();
    void on_gif_checkBox_clicked();
    void on_aco_checkBox_clicked();
    void on_comboBox_currentIndexChanged(int index);
    void on_pushButton_2_clicked();
signals:
    void closed(bool);
    void raiseHTML(QString);
private:
    void allDownloadersDone();
    bool isBoardEnabled(int i) const;
    int getNextEnabledBoard(int i);
    bool enoughBoardsEnabled() const;
    void safelyAddToCombobox(QString string);
    void setEnabledAll(bool value);
private:
    Ui::DownloadManager *ui;
    QWebEngineView* webview;
    std::string pageHtml;
    std::array<std::unique_ptr<QDownloader>, 12> downloaders;
    int downloadersDone;

    bool viewFiles;
    QGraphicsDropShadowEffect effect;
};

#endif // DOWNLOADMANAGER_H
