#include "videoplayer.h"

#include <functional>

#include <QDebug>

#ifdef Q_WS_X11
#include <QX11EmbedContainer>
#endif

static const char* const vlc_args[] = {  };


VideoPlayer::VideoPlayer(QWidget* parent)
:
#ifdef Q_WS_X11
   QX11EmbedContainer(parent),
#else
   QWidget(parent),
#endif
  _vlcinstance(libvlc_new(0, vlc_args)),
  _mp(libvlc_media_player_new(_vlcinstance)),
  _m()
{
    libvlc_video_set_key_input(_mp, false);
    libvlc_video_set_mouse_input(_mp, false);
}

//desctructor
VideoPlayer::~VideoPlayer()
{
    libvlc_media_player_stop(_mp);
    libvlc_media_release(_m);
    libvlc_media_player_release(_mp);
    libvlc_release(_vlcinstance);
}

void VideoPlayer::loadFile(QString file)
{
    _m = libvlc_media_new_path(_vlcinstance, file.toStdString().c_str());

    libvlc_media_add_option(_m, "input-repeat=-1");

    libvlc_media_player_set_media(_mp, _m);

#if defined(Q_OS_WIN)
    libvlc_media_player_set_drawable(_mp, reinterpret_cast<unsigned int>(this->winId()));
#elif defined(Q_OS_MAC)
    libvlc_media_player_set_drawable(_mp, this->winId());
#else
    const int windid = this->winId();
    libvlc_media_player_set_xwindow(_mp, windid);
#endif
}

void VideoPlayer::playFile()
{
    libvlc_media_player_play(_mp);
}

void VideoPlayer::pause()
{
    libvlc_media_player_pause(_mp);
}

void VideoPlayer::unpause()
{
    libvlc_media_player_play(_mp);
}

void VideoPlayer::stop()
{
    libvlc_media_player_stop(_mp);
}
