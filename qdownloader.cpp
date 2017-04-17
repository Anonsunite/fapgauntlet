#include "qdownloader.h"
#include <QFile>

#include <regex>

QDownloader::QDownloader(QUrl url, bool saveToFile, QObject *parent)
    : QObject(parent),
      _webCtrl(),
      _url(url),
      _data(),
      _saveToFile(saveToFile)
{
    connect(&_webCtrl, SIGNAL(finished(QNetworkReply*)), this, SLOT(fileDownloaded(QNetworkReply*)));
    QNetworkRequest request(url);
    _webCtrl.get(request);
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

void QDownloader::fileDownloaded(QNetworkReply *reply)
{
    if(!reply->isFinished())
        return;
    if(_saveToFile)
    {
        QFile file(fileName());
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();
    }
    else
        _data = reply->readAll();

    reply->deleteLater();
    emit downloaded();
}
