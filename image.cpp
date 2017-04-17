#include "image.h"

#include <QFile>

#include <iterator>
#include <cassert>

Speed toSpeed(unsigned int number)
{
    switch(number)
    {
    case 0: return Speed::VERY_SLOW;
    case 1: return Speed::SLOW;
    case 2: return Speed::MEDIUM;
    default:
    case 3: return Speed::FAST;
    case 4: return Speed::VERY_FAST;
    case 5: return Speed::EXTREMLY_FAST;
    }
}

float fromSpeed(Speed s)
{
    switch(s)
    {
    case Speed::VERY_SLOW:     return 1.25f;
    case Speed::SLOW:          return 1.00f;
    case Speed::MEDIUM:        return 0.6f;
    default:
    case Speed::FAST:          return 0.33f;
    case Speed::VERY_FAST:     return 0.25f;
    case Speed::EXTREMLY_FAST: return 0.15f;
    }
}

QString speedToText(Speed s)
{
    switch(s)
    {
    case Speed::VERY_SLOW:     return "Very Slow";
    case Speed::SLOW:          return "Slow";
    default:
    case Speed::MEDIUM:        return "Medium";
    case Speed::FAST:          return "Fast";
    case Speed::VERY_FAST:     return "Very Fast";
    case Speed::EXTREMLY_FAST: return "Extremly Fast";
    }
}

Speed textToSpeed(QString z)
{
    QString s = z.toLower();

    if(s == "very slow")
        return Speed::VERY_SLOW;
    else if(s == "slow")
        return Speed::SLOW;
    else if(s == "medium")
        return Speed::MEDIUM;
    else if(s == "fast")
        return Speed::FAST;
    else if(s == "very fast")
        return Speed::VERY_FAST;
    else if(s == "extremly fast")
        return Speed::EXTREMLY_FAST;
    else
        assert(false);

    return Speed::MEDIUM;
}

std::vector<QString> hardcodedDescList = {"with spit", "normal", "force of 10000 suns",
                                          "standup", "hard", "reverse grip", "very hard",
                                          "soft", "very soft", "1 finger", "2 fingers",
                                          "3 fingers", "massage balls", "thumb only",
                                          "medium", "nippleplay"};

Image::Image(QString p)
    : _path(p),
      _count(13 + qrand() % 100),
      _speed(toSpeed(qrand() % 7)),
      _description(hardcodedDescList[qrand() % hardcodedDescList.size()]),
      _shown(0),
      _delete(false)
{

}

Image::Image(QString p, unsigned int c, Speed s, QString d)
    : _path(p),
      _count(c),
      _speed(s),
      _description(d),
      _shown(0),
      _delete(false)
{

}

QString Image::text() const
{
    return QString::number(count()) + ", " +
           speedToText(speed()) + ", " + _description;
}

QString Image::path() const
{
    return _path;
}

Speed Image::speed() const
{
    return _speed;
}

unsigned int Image::count() const
{
    return _count;
}

QString Image::desc() const
{
    return _description;
}

bool Image::deleteOrNot()
{
    if(_delete)
    {
        QFile file(_path);
        file.remove();
    }

    return _delete;
}

void Image::deleteOnDestruction()
{
    _delete = true;
}

bool Image::operator==(const Image& img) const
{
    return img._count       == _count
        && img._delete      == _delete
        && img._description == _description
        && img._path        == _path
        && img._shown       == _shown
        && img._speed       == _speed;
}
