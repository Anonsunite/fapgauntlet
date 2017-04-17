#include "settings.h"

#include "json/json.h"
#include "image.h"

#include <fstream>

Settings::Settings()
{
    std::fstream f("config.json", std::ios::in);
    if(f)
    {
        Json::Value v;
        f >> v;

        saveOptions = true;

        if(v.get("loaded", false).asBool())
        {
            for(unsigned int i = 0; i < v["images"].size(); ++i)
            {
                loadLastImages.push_back(QString::fromStdString(v["images"][i].asString()));
            }
        }
        std::string descs = v.get("descriptions", "with spit,normal,force of 10000 suns,standup,hard,reverse grip,very hard,soft,very soft,1 finger,2 fingers,3 fingers,massage balls,thumb only,medium,nippleplay").asString();
        hardcodedDescList.clear();

        std::stringstream ss(descs);
        for(std::string s; std::getline(ss, s, ','); hardcodedDescList.push_back(QString::fromStdString(s)))
        {  }

        pause = v.get("pause", 0).asInt() * 80;
        speed = v.get("pulse", 50).asInt();

        images = v.get("showImages", true).asBool();
        gifs = v.get("showGifs", true).asBool();
        webms = v.get("showWebms", true).asBool();

        fullscreen = v.get("fullscreen", true).asBool();
    }
    else
    {
        speed = 50;
        pause = 0;
        saveOptions = false;
        images =
        gifs   =
        webms  = true;
        fullscreen = true;
    }
}

Settings::~Settings()
{
    if(saveOptions)
    {
        Json::Value json;
        json["loaded"] = !(loadLastImages.empty());
        if(!loadLastImages.empty())
        {
            for(unsigned int i = 0; i < loadLastImages.size(); ++i)
                json["images"][i] = loadLastImages[i].toStdString();
        }
        QString str;
        for(unsigned int i = 0; i < hardcodedDescList.size(); ++i)
        {
            str += hardcodedDescList[i];

            if(i + 1 < hardcodedDescList.size())
                str += ",";
        }
        json["descriptions"] = str.toStdString();
        json["pause"] = pause / 80;
        json["pulse"] = speed;

        json["showImages"] = images;
        json["showGifs"] = gifs;
        json["showWebms"] = webms;

        json["fullscreen"] = fullscreen;

        std::fstream f("config.json", std::ios::out);
        f << json;
    }
}

int Settings::pulse() const
{
    return 37500 / speed;
}
