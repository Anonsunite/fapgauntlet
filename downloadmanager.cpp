#include "downloadmanager.h"
#include "ui_downloadmanager.h"
#include "qdownloader.h"

#include "json/json.h"

#include <QTreeWidgetItem>

#if (QT_VERSION_MAJOR < 5)
#include <QtWebKit/QWebView>
#include <QtWebKit/QWebFrame>
#else
#include <QWebEngineView>
#include <QWebEnginePage>
#endif

#include <QMessageBox>
#include <QTimer>

#include <cassert>
#include <regex>

const std::string boardlist[] = {"/b/",  "/y/",  "/d/", "/u/",  "/e/",
                                 "/h/", "/hm/", "/hc/", "/s/", "/hr/",
                                 "/gif/", "/aco/"};
const int boardlistSize = std::distance(std::begin(boardlist),
                                        std::end(boardlist));
DownloadManager::DownloadManager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DownloadManager),
    webview(nullptr),
    pageHtml(),
    downloaders(),
    viewFiles(false),
    effect(),
    effectTimer(this)
{
    ui->setupUi(this);
    webview = new WebViewType(ui->frame);

    connect(webview, SIGNAL(urlChanged(QUrl)), this, SLOT(urlChange(QUrl)));
    connect(this, SIGNAL(raiseHTML(QString)), this, SLOT(handleHTML(QString)));
    connect(webview, SIGNAL(loadFinished(bool)), this, SLOT(pageLoadingDone(bool)));
    connect(&effectTimer, SIGNAL(timeout()), this, SLOT(blinkCombo()));
    effectTimer.setInterval(250);

    on_pushButton_2_clicked();
    effect.setOffset(0);
    effect.setColor(QColor(255, 10, 10));
    ui->comboBox->setGraphicsEffect(&effect);

    this->setWindowFlags(this->windowFlags() | Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
}

DownloadManager::~DownloadManager()
{
    delete ui;
    delete webview;
}

void DownloadManager::closeEvent(QCloseEvent *ev)
{
    emit closed(viewFiles);

    QMainWindow::closeEvent(ev);
}

std::string DownloadManager::currentBoard() const
{
    const std::string str = boardlist[ui->verticalSlider->value()];
    return std::string(str.begin() + 1, std::prev(str.end()));
}

int DownloadManager::currentBoardInt() const
{
    auto val = currentBoard();
    for(int i = 0; i < boardlistSize; ++i)
        if(boardlist[i] == "/" + val + "/")
            return i;
    assert(false);

    return 0;
}

std::string DownloadManager::pageHTML() const
{
    return pageHtml;
}

bool DownloadManager::streamFiles() const
{
    return ui->checkBox->isChecked();
}

QString DownloadManager::url() const
{
    std::string url = ui->comboBox->currentText().toStdString();
    url = std::string(url.begin() + 19, url.end());

    return QString::fromStdString("http://a.4cdn" + url);
}

void DownloadManager::on_okButton_released()
{
    viewFiles = true;

    this->close();
}

void DownloadManager::handleHTML(QString html)
{
    pageHtml = html.toStdString();
    ui->okButton->setEnabled(true);
}

void DownloadManager::on_verticalSlider_valueChanged(int value)
{
    if(!isBoardEnabled(value))
        on_verticalSlider_valueChanged(getNextEnabledBoard(value));

    auto url = QString::fromStdString("http://boards.4chan.org" + boardlist[value] + "catalog");
    safelyAddToCombobox(url);

    webview->setUrl(url);
}

void DownloadManager::urlChange(QUrl url)
{
    bool wasSet = false;
    const int size = std::distance(std::begin(boardlist), std::end(boardlist));
    for(int i = 0; i < size; ++i)
    {
        if(!isBoardEnabled(i))
            continue;
        const std::string c(boardlist[i].begin() + 1, std::prev(boardlist[i].end()));

        std::regex r("\\/" + c + "\\/", std::regex::ECMAScript);
        if(std::regex_search(url.toString().toStdString(), r))
        {
            bool oldState = ui->verticalSlider->blockSignals(true);
            ui->verticalSlider->setValue(i);
            ui->verticalSlider->blockSignals(oldState);

            wasSet = true;
            break;
        }
    }
    if(!wasSet)
    {
        webview->back();

        QTimer::singleShot(50, this, SLOT(changeDisplayedUrl()));
    }

    QString asString = url.toString();
    safelyAddToCombobox(asString);
}

void DownloadManager::changeDisplayedUrl()
{
    int index = ui->comboBox->findText(webview->url().toString());
    bool oldState = ui->comboBox->blockSignals(true);
    ui->comboBox->setCurrentIndex(index);
    ui->comboBox->blockSignals(oldState);
}

void DownloadManager::pageLoadingDone(bool b)
{
    if(!b)
    {
        ui->okButton->setEnabled(false);
        return;
    }

    QString url = ui->comboBox->currentText();

    const std::string regex = "(http|https):\\/\\/boards\\.4chan\\.org\\/" + currentBoard() + "\\/thread\\/[0-9]+";
    if(!std::regex_search(url.toStdString(), std::regex(regex)))
    {
        ui->okButton->setEnabled(false);
        return;
    }

    QTimer::singleShot(10, this, SLOT(emitRaiseHtml()));
}

void DownloadManager::emitRaiseHtml()
{
#if (QT_VERSION_MAJOR < 5)
    emit raiseHTML(webview->page()->mainFrame()->toHtml());
#else
    webview->page()->toHtml([this](const QString& html){ emit raiseHTML(html); });
#endif
}

void DownloadManager::on_pushButton_clicked()
{
    webview->back();
}

void DownloadManager::jsonDownloadDone()
{
    ++downloadersDone;

    const bool single = !(ui->searchAllBoardsCheckbox->isChecked());
    if(downloadersDone > boardlistSize - 1 || single)
        allDownloadersDone();
}

void DownloadManager::allDownloadersDone()
{
    const bool single = !(ui->searchAllBoardsCheckbox->isChecked());
    unsigned int added = 0;
    for(int i = (single ? currentBoardInt() : 0); i < boardlistSize; ++i)
    {
        std::stringstream ss(QString(downloaders[i]->data()).toStdString());
        Json::Value json;
        ss >> json;

        for(int page = 0; page < 10; ++page)
        {
            Json::Value pageJson = json[page]["threads"];
            for(unsigned int j = 0; j < pageJson.size(); ++j)
            {
                std::string threadTitle = pageJson[j].get("sub", "").asString();
                std::string toCheck = ui->lineEdit->text().toStdString();

                if(threadTitle.find(toCheck) != std::string::npos)
                {
                    int64_t thread = pageJson[j].get("no", 0).asInt64();
                    QString threadNumber = QString::number(thread);

                    QString board = QString::fromStdString(boardlist[i]);
                    QString url = "http://boards.4chan.org" + board + "thread/" + threadNumber;
                    webview->setUrl(QUrl(url));

                    safelyAddToCombobox(url);
                    ++added;
                }
            }
        }
        downloaders[i].release();

        if(single)
        {
            ui->progressBar->setValue(11);
            break;
        }
        else
            ui->progressBar->setValue(i);
    }
    if(added > 0U)
    {
        effectTimer.start();
        QTimer::singleShot(250 * 9, this, SLOT(stopEffect()));
    }
    else
    {
        if(ui->comboBox->count() == 0)
            on_verticalSlider_valueChanged(0);
    }
    setEnabledAll(true);
}

void DownloadManager::blinkCombo()
{
    qreal radius = effect.blurRadius();

    if(radius < 19.0)
        effect.setBlurRadius(20.0);
    else
        effect.setBlurRadius(0.0);
}

void DownloadManager::stopEffect()
{
    effectTimer.stop();
}

void DownloadManager::on_lineEdit_textChanged(const QString &)
{
    if(ui->lineEdit->text().size() < 4)
        ui->lineEdit->setStyleSheet("color:rgb(200,20,20)");
    else
        ui->lineEdit->setStyleSheet("");
}

bool DownloadManager::isBoardEnabled(int i) const
{
    assert(i < boardlistSize);

    switch (i)
    {
    default:
    case 0:  return ui->b_checkBox  ->isChecked();
    case 1:  return ui->y_checkBox  ->isChecked();
    case 2:  return ui->d_checkBox  ->isChecked();
    case 3:  return ui->u_checkBox  ->isChecked();
    case 4:  return ui->e_checkBox  ->isChecked();
    case 5:  return ui->h_checkBox  ->isChecked();
    case 6:  return ui->hm_checkBox ->isChecked();
    case 7:  return ui->hc_checkBox ->isChecked();
    case 8:  return ui->s_checkBox  ->isChecked();
    case 9:  return ui->hr_checkBox ->isChecked();
    case 10: return ui->gif_checkBox->isChecked();
    case 11: return ui->aco_checkBox->isChecked();
    }
}

int DownloadManager::getNextEnabledBoard(int i)
{
    int toReturn = i;

    for(int j = toReturn; j < boardlistSize; ++j)
        if(isBoardEnabled(j))
            return j;

    for(int j = toReturn; j >= 0; --j)
        if(isBoardEnabled(j))
            return j;

    assert(false);
}

void DownloadManager::updateAfterCheckboxClick()
{
    if(!isBoardEnabled(ui->verticalSlider->value()))
        on_verticalSlider_valueChanged(getNextEnabledBoard(ui->verticalSlider->value()));
}

bool DownloadManager::enoughBoardsEnabled() const
{
    for(int i = 0; i < boardlistSize; ++i)
        if(isBoardEnabled(i))
            return true;
    return false;
}

void DownloadManager::on_b_checkBox_clicked()
{
    if(!enoughBoardsEnabled() && !ui->b_checkBox->isChecked())
        ui->b_checkBox->setChecked(true);
    updateAfterCheckboxClick();
}

void DownloadManager::on_y_checkBox_clicked()
{
    if(!enoughBoardsEnabled() && !ui->y_checkBox->isChecked())
        ui->y_checkBox->setChecked(true);
    updateAfterCheckboxClick();
}

void DownloadManager::on_d_checkBox_clicked()
{
    if(!enoughBoardsEnabled() && !ui->d_checkBox->isChecked())
        ui->d_checkBox->setChecked(true);
    updateAfterCheckboxClick();
}

void DownloadManager::on_u_checkBox_clicked()
{
    if(!enoughBoardsEnabled() && !ui->u_checkBox->isChecked())
        ui->u_checkBox->setChecked(true);
    updateAfterCheckboxClick();
}

void DownloadManager::on_e_checkBox_clicked()
{
    if(!enoughBoardsEnabled() && !ui->e_checkBox->isChecked())
        ui->e_checkBox->setChecked(true);
    updateAfterCheckboxClick();
}

void DownloadManager::on_h_checkBox_clicked()
{
    if(!enoughBoardsEnabled() && !ui->h_checkBox->isChecked())
        ui->h_checkBox->setChecked(true);
    updateAfterCheckboxClick();
}

void DownloadManager::on_hm_checkBox_clicked()
{
    if(!enoughBoardsEnabled() && !ui->hm_checkBox->isChecked())
        ui->hm_checkBox->setChecked(true);
    updateAfterCheckboxClick();
}

void DownloadManager::on_hc_checkBox_clicked()
{
    if(!enoughBoardsEnabled() && !ui->hc_checkBox->isChecked())
        ui->hc_checkBox->setChecked(true);
    updateAfterCheckboxClick();
}

void DownloadManager::on_s_checkBox_clicked()
{
    if(!enoughBoardsEnabled() && !ui->s_checkBox->isChecked())
        ui->s_checkBox->setChecked(true);
    updateAfterCheckboxClick();
}

void DownloadManager::on_hr_checkBox_clicked()
{
    if(!enoughBoardsEnabled() && !ui->hr_checkBox->isChecked())
        ui->hr_checkBox->setChecked(true);
    updateAfterCheckboxClick();
}

void DownloadManager::on_gif_checkBox_clicked()
{
    if(!enoughBoardsEnabled() && !ui->gif_checkBox->isChecked())
        ui->gif_checkBox->setChecked(true);
    updateAfterCheckboxClick();
}

void DownloadManager::on_aco_checkBox_clicked()
{
    if(!enoughBoardsEnabled() && !ui->aco_checkBox->isChecked())
        ui->aco_checkBox->setChecked(true);
    updateAfterCheckboxClick();
}

void DownloadManager::on_comboBox_currentIndexChanged(int )
{
    webview->setUrl(ui->comboBox->currentText());
}

void DownloadManager::safelyAddToCombobox(QString string)
{
    if(ui->comboBox->findText(string) == -1)
    {
        ui->comboBox->addItem(string);
        int index = ui->comboBox->findText(string);
        bool oldState = ui->comboBox->blockSignals(true);
        ui->comboBox->setCurrentIndex(index);
        ui->comboBox->blockSignals(oldState);
    }
}

void DownloadManager::on_pushButton_2_clicked()
{
    if(ui->lineEdit->text().size() < 4 && ui->progressBar->value() != 0
            && ui->progressBar->value() != 11)
        return;

    if(ui->searchAllBoardsCheckbox->isChecked())
        ui->progressBar->setValue(0);
    else
        ui->progressBar->setValue(11);

    downloadersDone = 0;
    const bool single = !(ui->searchAllBoardsCheckbox->isChecked());
    for(int i = (single ? currentBoardInt() : 0); i < boardlistSize; ++i)
    {
        if(!isBoardEnabled(i))
        {
            ++downloadersDone;
            continue;
        }
        downloaders[i] = std::make_unique<QDownloader>(QUrl("http://a.4cdn.org/" + QString::fromStdString(boardlist[i])
                                                             + "/catalog.json"), false);
        connect(downloaders[i].get(), SIGNAL(downloaded()), this, SLOT(jsonDownloadDone()));

        if(single)
            break;
    }

    setEnabledAll(false);
}

void DownloadManager::setEnabledAll(bool state)
{
    ui->b_checkBox->setEnabled(state);
    ui->aco_checkBox->setEnabled(state);
    ui->d_checkBox->setEnabled(state);
    ui->e_checkBox->setEnabled(state);
    ui->gif_checkBox->setEnabled(state);
    ui->hc_checkBox->setEnabled(state);
    ui->hm_checkBox->setEnabled(state);
    ui->hr_checkBox->setEnabled(state);
    ui->h_checkBox->setEnabled(state);
    ui->s_checkBox->setEnabled(state);
    ui->u_checkBox->setEnabled(state);
    ui->y_checkBox->setEnabled(state);

    ui->comboBox->setEnabled(state);
    ui->searchAllBoardsCheckbox->setEnabled(state);
    ui->lineEdit->setEnabled(state);
    ui->pushButton->setEnabled(state);
    ui->pushButton_2->setEnabled(state);
    ui->verticalSlider->setEnabled(state);
}
