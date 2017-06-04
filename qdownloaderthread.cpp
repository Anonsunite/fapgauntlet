#include "qdownloaderthread.h"

#include <QByteArray>
#include <QFile>

#include <curl/curl.h>

#include <sstream>
#include <regex>

std::size_t write_data(char *ptr, std::size_t size, std::size_t nmemb, void *userdata)
{
    for(std::size_t i = 0U; i < size * nmemb; ++i)
    {
        ((std::vector<char>*) userdata)->push_back(*(ptr + i));
    }
    return size * nmemb;
}


qdownloaderthread::qdownloaderthread(QUrl url, bool saveToFile, QByteArray &data, QObject* parent)
: QThread(parent),
  _url(url),
  _data(&data),
  _saveToFile(saveToFile)
{

}

void qdownloaderthread::run()
{
    std::vector<char> stream;
    CURL* curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    std::string url = _url.toString().toStdString();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);

    if(_saveToFile)
    {
        QFile file(fileName());
        file.open(QIODevice::WriteOnly);
        file.write(&stream[0], stream.size());
        file.close();
    }
    else
        (*_data) = &stream[0];
}

QString qdownloaderthread::fileName() const
{
    std::regex r("[0-9]+\\.(png|jpg|jpeg|gif|webm)", std::regex::ECMAScript);
    const std::string url = _url.toString().toStdString();
    auto fileName = std::sregex_iterator(url.begin(), url.end(), r);
    auto str = fileName->str();
    return QString::fromStdString(str);
}
