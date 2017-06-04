#ifndef QDOWNLOADERTHREAD_H
#define QDOWNLOADERTHREAD_H

#include <QString>
#include <QThread>
#include <QUrl>

class QByteArray;

class qdownloaderthread : public QThread
{
public:
    qdownloaderthread(QUrl url, bool saveToFile, QByteArray& data, QObject* parent = 0);
protected:
    void run();
private:
    QString fileName() const;
private:
    QUrl _url;
    QByteArray* _data;
    bool _saveToFile;
};

#endif // QDOWNLOADERTHREAD_H
