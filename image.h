#ifndef IMAGE_H
#define IMAGE_H

#include <QString>

#include <vector>

enum class Speed
{
    VERY_SLOW,
    SLOW,
    MEDIUM,
    FAST,
    VERY_FAST,
    EXTREMLY_FAST,
};

extern Speed toSpeed(unsigned int number);
extern float fromSpeed(Speed s);
extern QString speedToText(Speed s);
extern Speed textToSpeed(QString z);

extern std::vector<QString> hardcodedDescList;

class Image
{
public:
    Image(QString p);
    Image(QString p, unsigned int c, Speed s, QString d);

    QString text() const;

    unsigned int count() const;
    Speed speed() const;
    QString path() const;
    QString desc() const;

    bool deleteOrNot();
    void deleteOnDestruction();

    bool operator==(const Image& img) const;
private:
    QString _path;

    int _count;
    Speed _speed;
    QString _description;

    int _shown;
    bool _delete;
};

#endif // IMAGE_H
