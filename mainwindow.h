#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "image.h"
#include "settings.h"
#include "videoplayer.h"
#include "downloadmanager.h"
#include "settingswindow.h"
#include "qdownloader.h"

#include <QMainWindow>
#include <QAction>
#include <QLabel>
#include <QMovie>
#include <QTimer>

#include <vector>
#include <memory>
#include <queue>

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void keyReleaseEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent *event);
private slots:
    void ShowContextMenu(QPoint pos);
    void DisplayPictures();
    void UpdateText();
    void RepositionLabel(bool changePixelSize = true);
    void OnDownloadManagerClosed(bool ok);
    void OnSettingsWindowClosed();
    void downloadNext();
    void addImageFromDownload();
    void threadJsonDownloadDone();
    void PlayVideo();
private:
    void initMenu();
    void shuffle();
    void pause();
    void unpause();
    void stop();
    void loadImages(QString path);
private:
    Ui::MainWindow *ui;
    QMenu *myMenu;
    std::unique_ptr<QMovie> movie;
    DownloadManager* d;
    SettingsWindow* s;

    QAction fullScreenAction;
    QAction downloadFromBoards;
    QAction displayImages;
    QAction displayGifs;
    QAction displayWebms;

    std::vector<Image> images;

    float currentUpdateRate;
    QTimer displayPicturesTimer;
    QTimer updateTextTimer;
    QTimer animateTextTimer;
    int currentPixelSize;
    bool increasePixelSize;
    bool paused;

    unsigned int faps;
    bool isGif;
    bool isWebm;
    VideoPlayer* p;

    std::queue<Image> toDownload;
    QDownloader* downloader;
    std::unique_ptr<QDownloader> threadJsonDownloader;

    Settings settings;
    Image currentImage;
    bool save;

    std::vector<Image> ignore;

    QTimer playVideoTimer;
};

#endif // MAINWINDOW_H
