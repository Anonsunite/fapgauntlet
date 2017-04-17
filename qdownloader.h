#ifndef QDOWNLOADER_H
#define QDOWNLOADER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

class QDownloader : public QObject
{
    Q_OBJECT

public:
    explicit QDownloader(QUrl url, bool saveToFile = true, QObject *parent = 0);

    QByteArray data() const;
    QString fileName() const;
signals:
    void downloaded();
private slots:
    void fileDownloaded(QNetworkReply* pReply);
private:
    QNetworkAccessManager _webCtrl;
    QUrl _url;
    QByteArray _data;
    bool _saveToFile;
};

#endif // QDOWNLOADER_H
