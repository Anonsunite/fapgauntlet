#include "qdownloader.h"
#include "mainwindow.h"

#include <regex>

QDownloader::QDownloader(QUrl url, bool saveToFile, QObject *parent)
    : QObject(parent),
      _url(url),
      _data(),
      _saveToFile(saveToFile),
      _thread(url, saveToFile, _data, parent)
{
    connect(&_thread, SIGNAL(finished()), this, SLOT(emitDownloaded()));
    _thread.start();
}

void QDownloader::emitDownloaded()
{
    emit downloaded();
}

QByteArray QDownloader::data() const
{
    return _data;
}

QString QDownloader::fileName() const
{
    std::regex r("[0-9]+\\.(png|jpg|jpeg|gif|webm)", std::regex::ECMAScript);
    const std::string url = _url.toString().toStdString();
    auto fileName = std::sregex_iterator(url.begin(), url.end(), r);
    auto str = fileName->str();
    return QString::fromStdString(str);
}
