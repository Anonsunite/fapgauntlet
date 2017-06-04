#ifndef QDOWNLOADER_H
#define QDOWNLOADER_H

#include "qdownloaderthread.h"

#include <QObject>
#include <QUrl>

#include <thread>

class QDownloader : public QObject
{
    Q_OBJECT

public:
    explicit QDownloader(QUrl url, bool saveToFile = true, QObject *parent = 0);

    QByteArray data() const;
    QString fileName() const;
private slots:
    void emitDownloaded();
signals:
    void downloaded();
private:
    QUrl _url;
    QByteArray _data;
    bool _saveToFile;
    qdownloaderthread _thread;
};

#endif // QDOWNLOADER_H
