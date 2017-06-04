#ifndef SETTIGS_H
#define SETTIGS_H

#include <QString>

#include <vector>

static const int PAUSE_LENGTH = 40;

class Settings
{
public:
    Settings();
   ~Settings();

    int pulse() const;
public:
    bool saveOptions;
    std::vector<QString> loadLastImages;
    int pause;
    int speed;

    bool images;
    bool gifs;
    bool webms;

    bool fullscreen;

    bool downloadEvenIfInputInvalid;
};

#endif // SETTIGS_H
