#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <vlc/vlc.h>

#include <QWidget>
#ifdef Q_WS_X11
#include <QX11EmbedContainer>
#endif

class VideoPlayer
        :
    #ifdef Q_WS_X11
        public QX11EmbedContainer
    #else
        public QWidget
    #endif
{
    Q_OBJECT

public:
    VideoPlayer(QWidget* parent = 0);
    ~VideoPlayer();

public slots:
    void loadFile(QString file);
    void playFile();
    void stop();
    void pause();
    void unpause();
private:
    libvlc_instance_t *_vlcinstance;
    libvlc_media_player_t *_mp;
    libvlc_media_t *_m;
};

#endif // VIDEOPLAYER_H
