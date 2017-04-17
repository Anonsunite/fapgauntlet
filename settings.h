#ifndef SETTIGS_H
#define SETTIGS_H

#include <QString>

#include <vector>

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
};

#endif // SETTIGS_H
