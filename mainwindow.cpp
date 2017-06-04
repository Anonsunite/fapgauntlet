#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "json/json.h"

#include <curl/curl.h>

#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>
#include <QTime>
#include <QMenu>
#include <QDir>

#include <algorithm>
#include <cassert>
#include <sstream>
#include <regex>
#include <set>

static QRect desktopRect = QRect();

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    myMenu(nullptr),
    movie(nullptr),
    d(nullptr),
    s(nullptr),
    fullScreenAction("Fullscreen", myMenu),
    downloadFromBoards("Download from boards", myMenu),
    displayImages("Display Images", myMenu),
    displayGifs("Display GIFs", myMenu),
    displayWebms("Display WEBMs", myMenu),
    images(),
    currentUpdateRate(0),
    AnimationPulses(10),
    displayPicturesTimer(this),
    updateTextTimer(this),
    animateTextTimer(this),
    currentPixelSize(44),
    minPixelSize(44),
    maxPixelSize(54),
    increasePixelSize(true),
    paused(false),
    faps(0U),
    isGif(false),
    isWebm(false),
    p(),
    downloader(nullptr),
    threadJsonDownloader(nullptr),
    settings(),
    currentImage(""),
    save(!settings.loadLastImages.empty()),
    playVideoTimer()
{
    ui->setupUi(this);
    ui->label->setScaledContents(true);

    desktopRect = QApplication::desktop()->screenGeometry();
    this->setGeometry(0, 0, desktopRect.width(), desktopRect.height());
    ui->centralWidget->setGeometry(0, 0, desktopRect.width(), desktopRect.height());

    auto rect1 = ui->label->geometry();
    const float height = this->geometry().height();
    const float middle = rect1.height() / 2.f;

    ui->label->setGeometry(rect1.x(), static_cast<int>(4 * height / 7.f - middle), rect1.width(), rect1.height());

    ui->centralWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->centralWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowContextMenu(QPoint)));

    this->setWindowState(this->windowState() ^ Qt::WindowFullScreen);

    qsrand(static_cast<quint64>(QTime::currentTime().msec()));

    displayPicturesTimer.setSingleShot(true);
    connect(&displayPicturesTimer, SIGNAL(timeout()), SLOT(DisplayPictures()));
    updateTextTimer.setSingleShot(true);
    connect(&updateTextTimer, SIGNAL(timeout()), SLOT(UpdateText()));
    connect(&animateTextTimer, SIGNAL(timeout()), SLOT(RepositionLabel()));
    connect(&playVideoTimer, SIGNAL(timeout()), SLOT(PlayVideo()));

    this->setWindowFlags(this->windowFlags() | Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    QTimer::singleShot(50, this, SLOT(loadLastImagesSlot()));

    if(!settings.gifs && !settings.images && !settings.webms)
    {
        QMessageBox::information(this, "", "Do not try to break me, thanks. (delete or re-edit the config file to fix this)", QMessageBox::Ok);
        this->close();
    }
}

MainWindow::~MainWindow()
{
    this->hide();

    delete ui;
    delete p;
    delete d;
    delete s;
    delete myMenu;
    delete downloader;

    if(save)
    {
        std::vector<Image> all = images;
        for(auto& a : ignore)
            images.push_back(a);
        for(auto& a : all)
        {
            if(!a.deleteOrNot())
            {
                QFileInfo f(a.path());
                QString dir = f.dir().absolutePath();

                auto it = std::find(settings.loadLastImages.begin(), settings.loadLastImages.end(), dir);
                if(it == settings.loadLastImages.end())
                    settings.loadLastImages.push_back(dir);
            }
        }
    }
    else
    {
        std::vector<Image> all = images;
        for(auto& a : ignore)
            images.push_back(a);
        for(auto& a : all)
            a.deleteOrNot();
        settings.loadLastImages.clear();
    }
}


void MainWindow::loadLastImagesSlot()
{
    if(!settings.loadLastImages.empty())
    {
        for(auto a : settings.loadLastImages)
            loadImages(a);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if(images.empty())
        return;

    if(event->key() == Qt::Key_Right)
    {
        playVideoTimer.stop();
        if(!paused)
        {
            updateTextTimer.stop();
            displayPicturesTimer.stop();
            animateTextTimer.stop();

            displayPicturesTimer.setInterval(10);
            displayPicturesTimer.start();
        }
        else
        {
            DisplayPictures();
            pause();

            if(isWebm)
            {
                p->pause();
                p->hide();
            }
        }
    }
    else if(event->key() == Qt::Key_Left)
    {
        playVideoTimer.stop();

        images.insert(images.begin(), images.back());
        images.erase(images.end() - 1);
        images.insert(images.begin(), images.back());
        images.erase(images.end() - 1);

        if(!paused)
        {
            updateTextTimer.stop();
            displayPicturesTimer.stop();
            animateTextTimer.stop();

            displayPicturesTimer.setInterval(10);
            displayPicturesTimer.start();
        }
        else
        {

            DisplayPictures();
            pause();

            if(isWebm)
            {
                p->pause();
                p->hide();
            }
        }
    }
    else if(event->key() == Qt::Key_Space)
    {
        if(paused)
        {
            unpause();

            if(isWebm)
            {
                p->show();
                p->unpause();
            }
        }
        else
        {
            playVideoTimer.stop();
            pause();

            if(isWebm)
                p->pause();
        }
    }  
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    desktopRect = QRect(QPoint(0, 0), event->size());
}

void MainWindow::PlayVideo()
{
    if(isGif)
        movie->start();

    if(isWebm)
    {
        p->playFile();
        p->show();
    }
}

bool MainWindow::eventFilter(QObject* o, QEvent* e)
{
    if(o == (QObject*) p)
    {
        if(e->type() == QEvent::Type::KeyRelease)
            keyReleaseEvent((QKeyEvent*) e);
        else if(e->type() == QEvent::Type::Resize)
            resizeEvent((QResizeEvent*) e);
        else if(e->type() == QEvent::Type::ContextMenu)
            ShowContextMenu(this->cursor().pos());
        else if(e->type() == QEvent::Type::MouseButtonRelease)
        {
            if(((QMouseEvent*) e)->button() == Qt::RightButton)
                ShowContextMenu(this->cursor().pos());
        }
        else
            e->ignore();
        return true;
    }
    return QMainWindow::eventFilter(o, e);
}

void MainWindow::DisplayPictures()
{
    currentImage = images[0];
    ui->label->setText(currentImage.text());

    if(isWebm)
    {
        isWebm = false;
        p->pause();
        p->hide();
    }

    if(currentImage.path().endsWith(".gif", Qt::CaseSensitive))
    {
        isGif = true;
        isWebm = false;
        movie = std::move(std::make_unique<QMovie>(currentImage.path()));
        movie->setScaledSize(QSize(desktopRect.width(), desktopRect.height()));
        ui->image->setMovie(&*movie);

        if(!paused)
        {
            playVideoTimer.setInterval(settings.pause);
            playVideoTimer.start();
        }
    }
    else if(currentImage.path().endsWith(".webm", Qt::CaseSensitive))
    {
        isGif = false;
        isWebm = true;
        if(!p)
            p = new VideoPlayer(ui->centralWidget);
        ui->image->setPixmap(QPixmap());
        p->installEventFilter(this);
        p->loadFile(currentImage.path());
        p->setGeometry(0, 0, desktopRect.width(), desktopRect.height());

        if(!paused)
        {
            playVideoTimer.setInterval(settings.pause);
            playVideoTimer.start();
        }
    }
    else
    {
        isGif = false;
        isWebm = false;
        //It's a normal image file
        QPixmap pm;
        pm = QPixmap(currentImage.path());
        ui->image->setPixmap(pm.scaled(desktopRect.width(), desktopRect.height(), Qt::KeepAspectRatio));
    }
    ui->image->setAlignment(Qt::AlignCenter);
    ui->image->setGeometry(0, 0, desktopRect.width(), desktopRect.height());
    ui->image->show();

    currentPixelSize = 43;
    RepositionLabel();

    images.erase(images.begin());
    images.push_back(currentImage);

    currentUpdateRate = static_cast<int>(settings.pulse() * fromSpeed(currentImage.speed()));

    updateTextTimer.setInterval(currentUpdateRate + settings.pause);
    updateTextTimer.start();
    animateTextTimer.setInterval(currentUpdateRate / AnimationPulses + settings.pause);
    animateTextTimer.start();
    displayPicturesTimer.setInterval(currentUpdateRate * currentImage.count() + settings.pause);
    displayPicturesTimer.start();

    ++faps;
}

void MainWindow::ShowContextMenu(QPoint pos)
{
    QPoint globalPos = ui->centralWidget->mapToGlobal(pos);

    if(!myMenu)
        initMenu();

    myMenu->hide();
    QAction* selectedItem = myMenu->exec(globalPos);
    if(selectedItem)
    {
        // something was chosen, do stuff
        if(selectedItem->text() == "Exit")
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "", "Do you really want to quit?",
                                          QMessageBox::Yes|QMessageBox::No);
            if(reply == QMessageBox::Yes)
                QApplication::quit();
        }
        else if(selectedItem->text() == "Fullscreen")
        {
            if(selectedItem->isChecked())
                this->setWindowState(this->windowState() | Qt::WindowFullScreen);
            else
                this->setWindowState(this->windowState() & ~Qt::WindowFullScreen);

            settings.fullscreen = selectedItem->isChecked();
        }
        else if(selectedItem->text() == "Add folder")
        {
            QFileDialog d(this);
            d.setFileMode(QFileDialog::Directory);
            d.setNameFilter(tr("Image Files (*.png *.jpg *.bmp *.gif *.jpeg *.tiff)"));
            d.setWindowModality(Qt::WindowModality::WindowModal);
            if(d.exec())
            {
                loadImages(d.selectedFiles()[0]);
            }
        }
        else if(selectedItem->text() == "Re-Shuffle Pictures")
        {
            shuffle();
        }
        else if(selectedItem->text() == "I CAME")
        {
            if(!images.empty())
                pause();
            QMessageBox::information(this, "", "FapCount: " + QString::number(faps), QMessageBox::Ok);
        }
        else if(selectedItem->text() == "Display Images")
        {
            if(!settings.gifs && !settings.webms)
            {
                QMessageBox::information(this, "", "You must look at at least one file type.", QMessageBox::Ok);
                displayImages.setChecked(true);
                return;
            }
            settings.images = selectedItem->isChecked();

            if(!settings.images)
            {
                if(images.empty())
                    return;
                std::vector<Image> ignoreImpl;
                for(auto it = images.begin(); it != images.end(); )
                {
                    Image& a = *it;
                    if(a.path().endsWith(".png") || a.path().endsWith(".jpg") || a.path().endsWith(".jpeg"))
                    {
                        ignoreImpl.push_back(a);
                        it = images.erase(it);
                    }
                    else
                        ++it;
                }
                if(images.empty())
                {
                    QMessageBox::information(this, "", "That would remove all files!");
                    settings.images = true;
                    displayImages.setChecked(true);
                    images = ignoreImpl;
                }
                else
                {
                    bool currentImageIsIgnored = false;
                    for(auto& a : ignoreImpl)
                    {
                        if(!currentImageIsIgnored)
                            currentImageIsIgnored = (a == currentImage);
                        ignore.push_back(a);
                    }
                    if(currentImageIsIgnored)
                        DisplayPictures();
                }
            }
            else
            {
                for(auto it = ignore.begin(); it != ignore.end(); )
                {
                    Image& a = *it;
                    if(a.path().endsWith(".png") || a.path().endsWith(".jpg") || a.path().endsWith(".jpeg"))
                    {
                        images.push_back(a);
                        it = ignore.erase(it);
                    }
                    else
                        ++it;
                }
                shuffle();
            }
        }
        else if(selectedItem->text() == "Display GIFs")
        {
            if(!settings.webms && !settings.images)
            {
                QMessageBox::information(this, "", "You must look at at least one file type.", QMessageBox::Ok);
                displayGifs.setChecked(true);
                return;
            }
            settings.gifs = selectedItem->isChecked();

            if(!settings.gifs)
            {
                if(images.empty())
                    return;
                std::vector<Image> ignoreImpl;
                for(auto it = images.begin(); it != images.end(); )
                {
                    Image& a = *it;
                    if(a.path().endsWith(".gif"))
                    {
                        ignoreImpl.push_back(a);
                        it = images.erase(it);
                    }
                    else
                        ++it;
                }
                if(images.empty())
                {
                    QMessageBox::information(this, "", "That would remove all files!");
                    settings.gifs = true;
                    displayGifs.setChecked(true);
                    images = ignoreImpl;
                }
                else
                {
                    bool currentImageIsIgnored = false;
                    for(auto& a : ignoreImpl)
                    {
                        if(!currentImageIsIgnored)
                            currentImageIsIgnored = (a == currentImage);
                        ignore.push_back(a);
                    }
                    if(currentImageIsIgnored)
                        DisplayPictures();
                }
            }
            else
            {
                for(auto it = ignore.begin(); it != ignore.end(); )
                {
                    Image& a = *it;
                    if(a.path().endsWith(".gif"))
                    {
                        images.push_back(a);
                        it = ignore.erase(it);
                    }
                    else
                        ++it;
                }
                shuffle();
            }
        }
        else if(selectedItem->text() == "Display WEBMs")
        {
            if(!displayGifs.isChecked() && !displayImages.isChecked())
            {
                QMessageBox::information(this, "", "You must look at at least one file type.", QMessageBox::Ok);
                displayWebms.setChecked(true);
                return;
            }
            settings.webms = selectedItem->isChecked();

            if(!settings.webms)
            {
                if(images.empty())
                    return;
                std::vector<Image> ignoreImpl;
                for(auto it = images.begin(); it != images.end(); )
                {
                    Image& a = *it;
                    if(a.path().endsWith(".webm"))
                    {
                        ignoreImpl.push_back(a);
                        it = images.erase(it);
                    }
                    else
                        ++it;
                }
                if(images.empty())
                {
                    QMessageBox::information(this, "", "That would remove all files!");
                    settings.webms = true;
                    displayWebms.setChecked(true);
                }
                else
                {
                    bool currentImageIsIgnored = false;
                    for(auto& a : ignoreImpl)
                    {
                        if(!currentImageIsIgnored)
                            currentImageIsIgnored = (a == currentImage);
                        ignore.push_back(a);
                    }
                    if(currentImageIsIgnored)
                        DisplayPictures();
                }
            }
            else
            {
                for(auto it = ignore.begin(); it != ignore.end(); )
                {
                    Image& a = *it;
                    if(a.path().endsWith(".webm"))
                    {
                        images.push_back(a);
                        it = ignore.erase(it);
                    }
                    else
                        ++it;
                }
                shuffle();
            }
        }
        else if(selectedItem->text() == "Blacklist this image")
        {
            if(images.empty())
            {
                QMessageBox::information(this, "", "TFW are you doin", QMessageBox::Ok);
                return;
            }
            updateTextTimer.stop();
            displayPicturesTimer.stop();
            animateTextTimer.stop();

            auto it = std::find(images.begin(), images.end(), currentImage);
            it->deleteOrNot();
            assert(it != images.end());
            images.erase(it);

            displayPicturesTimer.setInterval(10);
            displayPicturesTimer.start();

            if(paused)
                QTimer::singleShot(5, this, SLOT(pause()));
        }
        else if(selectedItem->text() == "Download from boards")
        {
            if(!images.empty())
            {
                if(isWebm)
                    p->pause();
                pause();
            }
            if(!d)
            {
                d = new DownloadManager(this);
                connect(d, SIGNAL(closed(bool)), this, SLOT(OnDownloadManagerClosed(bool)));
            }
            if(fullScreenAction.isChecked())
                this->setWindowState(this->windowState() & ~Qt::WindowFullScreen);
            this->setWindowState(this->windowState() | Qt::WindowMinimized);
            d->setWindowModality(Qt::WindowModal);

            d->show();
            d->activateWindow();
            d->raise();
        }
        else if(selectedItem->text() == "Settings")
        {
            if(!images.empty())
            {
                if(isWebm)
                    p->pause();
                pause();
            }
            if(!s)
            {
                s = new SettingsWindow(settings, this);
                connect(s, SIGNAL(closed()), this, SLOT(OnSettingsWindowClosed()));
            }
            if(fullScreenAction.isChecked())
                this->setWindowState(this->windowState() & ~Qt::WindowFullScreen);
            this->setWindowState(this->windowState() | Qt::WindowMinimized);
            s->setWindowModality(Qt::WindowModal);

            s->show();
            s->activateWindow();
            s->raise();
        }
        else if(selectedItem->text() == "Patreon")
        {
            QMessageBox::about(this, "", "<a href=\"http://patreon.com/anonsunite\">patreon.com/anonsunite</a>");
        }
    }
}

void MainWindow::UpdateText()
{
    QString old = ui->label->text();
    auto splitted = old.split(",");
    const unsigned int oldNumber = splitted[0].toInt();

    ui->label->setText(QString::number(oldNumber - 1U) + "," + splitted[1] + "," + splitted[2]);

    if(oldNumber / 10U > (oldNumber - 1U) / 10U)
        RepositionLabel(false);

    if(oldNumber > 1)
    {
        updateTextTimer.setInterval(currentUpdateRate);
        updateTextTimer.start();
    }
}

void MainWindow::initMenu()
{
    myMenu = new QMenu(this);
    myMenu->addAction("I CAME");
    myMenu->addAction("Re-Shuffle Pictures");

    fullScreenAction.setCheckable(true);
    fullScreenAction.setChecked(!!(this->windowState() & Qt::WindowFullScreen));
    myMenu->addAction(&fullScreenAction);

    myMenu->addSeparator();

    myMenu->addAction(&downloadFromBoards);

    myMenu->addSeparator();

    displayImages.setCheckable(true);
    displayImages.setChecked(settings.images);
    myMenu->addAction(&displayImages);

    displayGifs.setCheckable(true);
    displayGifs.setChecked(settings.gifs);
    myMenu->addAction(&displayGifs);

    displayWebms.setCheckable(true);
    displayWebms.setChecked(settings.webms);
    myMenu->addAction(&displayWebms);

    myMenu->addSeparator();

    myMenu->addAction("Blacklist this image");
    myMenu->addAction("Add folder");
    myMenu->addAction("Settings");

    myMenu->addSeparator();
    myMenu->addAction("Patreon");

    myMenu->addAction("Exit");
}

void MainWindow::RepositionLabel(bool changePixelSize)
{
    ui->label->setStyleSheet("background-color: none; color: #EE1289;");

    auto font = ui->label->font();
    if(changePixelSize)
    {
        if(increasePixelSize)
        {
            if(currentPixelSize < maxPixelSize)
            {
                float percents = std::min(currentPixelSize / static_cast<float>(maxPixelSize) + (1.f / AnimationPulses), 1.f);
                currentPixelSize = static_cast<int>(maxPixelSize * percents);
                font.setPixelSize(currentPixelSize);
            }
            else
            {
                float percents = std::max(currentPixelSize / static_cast<float>(minPixelSize) - (1.f / AnimationPulses), 0.9f);
                currentPixelSize = static_cast<int>(maxPixelSize * percents);
                font.setPixelSize(currentPixelSize);

                increasePixelSize = false;
            }
        }
        else
        {
            if(currentPixelSize > minPixelSize)
            {
                float percents = std::max(currentPixelSize / static_cast<float>(minPixelSize) - (1.f / AnimationPulses), 0.9f);
                currentPixelSize = static_cast<int>(minPixelSize * percents);
                font.setPixelSize(currentPixelSize);
            }
            else
            {
                float percents = std::min(currentPixelSize / static_cast<float>(maxPixelSize) + (1.f / AnimationPulses), 1.f);
                currentPixelSize = static_cast<int>(maxPixelSize * percents);
                font.setPixelSize(currentPixelSize);

                increasePixelSize = true;
            }
        }
    }
    else
    {
        font.setPixelSize(currentPixelSize);
    }
    ui->label->setFont(font);
    const int labelWidth = ui->label->fontMetrics().boundingRect(ui->label->text()).width();
    const int labelHeight = ui->label->fontMetrics().boundingRect(ui->label->text()).height();

    ui->label->setGeometry(desktopRect.width()  / 2  - (labelWidth  / 2),
                           desktopRect.height() / 16 - (labelHeight / 2),
                           labelWidth  + 10,
                           labelHeight + 10);

    animateTextTimer.setInterval(currentUpdateRate / AnimationPulses);
}

void MainWindow::OnDownloadManagerClosed(bool ok)
{
    if(fullScreenAction.isChecked())
        this->setWindowState(this->windowState() | Qt::WindowFullScreen);
    this->setWindowState(this->windowState() & ~Qt::WindowMinimized);

    if(!images.empty())
    {
        if(isWebm)
            p->unpause();
        unpause();
    }
    if(!ok)
    {
        d->hide();
        return;
    }
    QString url = d->url() + ".json";
    threadJsonDownloader = std::make_unique<QDownloader>(QUrl(url), false);
    connect(threadJsonDownloader.get(), SIGNAL(downloaded()), this, SLOT(threadJsonDownloadDone()));
}

void MainWindow::OnSettingsWindowClosed()
{
    if(fullScreenAction.isChecked())
        this->setWindowState(this->windowState() | Qt::WindowFullScreen);
    this->setWindowState(this->windowState() & ~Qt::WindowMinimized);

    if(!images.empty())
    {
        if(isWebm)
            p->unpause();
        unpause();

        QString old = ui->label->text();
        auto splitted = old.split(",");
        const unsigned int oldNumber = splitted[0].toInt();

        currentUpdateRate = static_cast<int>(settings.pulse() * fromSpeed(currentImage.speed()));

        updateTextTimer.setInterval(currentUpdateRate);
        updateTextTimer.start();
        animateTextTimer.setInterval(currentUpdateRate / AnimationPulses);
        animateTextTimer.start();
        displayPicturesTimer.setInterval(currentUpdateRate * oldNumber);
        displayPicturesTimer.start();
    }

    save = s->restoreSession();
}

void MainWindow::threadJsonDownloadDone()
{
    const QString str(threadJsonDownloader->data());
    const std::string data = str.toStdString();

    std::stringstream ss(data);
    Json::Value v;
    ss >> v;

    Json::Value posts = v["posts"];

    for(unsigned int i = 0; i < posts.size(); ++i)
    {
        Json::Value sub = posts[i];

        int64_t fileId = sub.get("tim", 0).asInt64();
        if(fileId == 0)
            continue;
        QString extension = QString::fromStdString(sub.get("ext", ".png").asString());
        QString path = "http://i.4cdn.org/" + QString::fromStdString(d->currentBoard()) + "/" + QString::number(fileId) + extension;

        std::string text = sub.get("com", "").asString();
        std::regex r("([1-9]|[1-9][0-9]|1[0-9][0-9]), (very slow|slow|normal|medium|fast|very fast|extreme?ly fast), [a-zA-Z0-9 ]+", std::regex::ECMAScript);
        std::smatch match;

        if(std::regex_search(text, match, r))
        {
            std::string properties = match.str();
            std::string count;
            unsigned int j = 0;
            for(;j < properties.size(); ++j)
            {
                if('0' <= properties[j] && properties[j] <= '9')
                    count.push_back(properties[j]);
                else
                {
                    ++j;++j;
                    break;
                }
            }
            properties = std::string(properties.begin() + j, properties.end());

            std::string desc;
            for(j = properties.size() - 1; j != static_cast<unsigned int>(-1); --j)
            {
                if(properties[j] != ',')
                    desc.insert(desc.begin(), properties[j]);
                else
                {
                    --j;
                    break;
                }
            }
            //remove whitespace
            desc.erase(desc.begin());
            properties = std::string(properties.begin(), properties.begin() + j + 1);

            toDownload.push(Image(path, std::stoi(count), textToSpeed(QString::fromStdString(properties)), QString::fromStdString(desc)));
        }
        else
        {
            if(settings.downloadEvenIfInputInvalid)
            {
                //Fail. Randomize the properties
                toDownload.push(Image(path));
            }
        }
    }

    if(!toDownload.empty())
        downloadNext();
}

void MainWindow::shuffle()
{
    if(images.empty())
        return;

    for(std::size_t i = images.size() - 1U; i > 1; --i)
    {
        const unsigned int rnd = qrand() % i;
        std::swap(images[rnd], images[i]);
    }
    auto it = std::find(images.begin(), images.end(), currentImage);
    //no assert, since currentImage might be ignored
    if(it != images.end())
    {
        std::iter_swap(images.begin(), it);
    }
}

void MainWindow::pause()
{
    paused = true;

    currentPixelSize = minPixelSize;
    RepositionLabel(false);

    QString old = ui->label->text();
    auto splitted = old.split(",");
    const unsigned int num = splitted[0].toInt();

    updateTextTimer.stop();
    displayPicturesTimer.stop();
    animateTextTimer.stop();

    updateTextTimer.setInterval(currentUpdateRate);
    displayPicturesTimer.setInterval(currentUpdateRate * num);
    animateTextTimer.setInterval(currentUpdateRate / AnimationPulses);

    if(isGif)
        ui->image->movie()->setPaused(true);
}

void MainWindow::unpause()
{
    paused = false;

    updateTextTimer.start();
    displayPicturesTimer.start();
    animateTextTimer.start();

    if(isGif)
        ui->image->movie()->setPaused(false);
}

void MainWindow::stop()
{
    updateTextTimer.stop();
    displayPicturesTimer.stop();
    animateTextTimer.stop();
}

void MainWindow::loadImages(QString path)
{
    QDir dir(path);

    QStringList buf;
    buf << "*.png" << "*.jpg" << "*.jpeg" << "*.gif" << "*.webm";
    dir.setNameFilters(buf);

    if(buf.empty())
    {
        QMessageBox::information(this, "", "No file types marked for display.", QMessageBox::Ok);
        return;
    }

    const bool wasEmpty = images.empty();
    QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Files);
    foreach(QFileInfo file, entries)
    {
        auto path = file.absoluteFilePath();

        if((!settings.images && (path.endsWith(".png")  || path.endsWith(".jpg") || path.endsWith(".jpeg")))
        || (!settings.gifs   && (path.endsWith(".gifs")))
        || (!settings.webms  && (path.endsWith(".webm"))))
        {
            ignore.push_back(Image(path));
        }
        else
        {
            images.push_back(Image(path));
        }
    }

    if(wasEmpty)
    {
        displayPicturesTimer.setInterval(10);
        displayPicturesTimer.start();
    }
}

void MainWindow::downloadNext()
{
    if(toDownload.empty())
        return;

    delete downloader;
    downloader = new QDownloader(toDownload.front().path());

    if(!toDownload.empty())
        connect(downloader, SIGNAL(downloaded()), this, SLOT(addImageFromDownload()));
}

void MainWindow::addImageFromDownload()
{
    bool wasEmpty = images.empty();
    auto path = downloader->fileName();
    if((!settings.images && (path.endsWith(".png")  || path.endsWith(".jpg") || path.endsWith(".jpeg")))
    || (!settings.gifs   && (path.endsWith(".gifs")))
    || (!settings.webms  && (path.endsWith(".webm"))))
    {
        ignore.push_back(Image(path, toDownload.front().count(), toDownload.front().speed(), toDownload.front().desc()));
    }
    else
    {
        images.push_back(Image(path, toDownload.front().count(), toDownload.front().speed(), toDownload.front().desc()));
    }

    toDownload.pop();

    if(d->streamFiles())
        std::prev(images.end())->deleteOnDestruction();

    if(wasEmpty)
    {
        if(images.empty())
        {
            assert(!ignore.empty());
            if(toDownload.empty())
            {
                QMessageBox::information(this, "", "All files in this thread are files you don\'t want to see. You should switch them back on.");
            }
        }
        else
        {
            displayPicturesTimer.setInterval(10);
            displayPicturesTimer.start();
        }
    }

    QTimer::singleShot(0, this, SLOT(downloadNext()));
}


