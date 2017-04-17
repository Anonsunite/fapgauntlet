#include "videoplayer.h"

#include <functional>

#ifdef Q_WS_X11
#include <QX11EmbedContainer>
#endif

static const char* const vlc_args[] = {  };


VideoPlayer::VideoPlayer(QWidget* parent)
: QWidget(parent),
  _vlcinstance(libvlc_new(0, vlc_args)),
  _mp(libvlc_media_player_new(_vlcinstance)),
  _m()
{

}

//desctructor
VideoPlayer::~VideoPlayer()
{
    libvlc_media_player_stop(_mp);
    libvlc_media_release(_m);
    libvlc_media_player_release(_mp);
    libvlc_release(_vlcinstance);
}

void VideoPlayer::playFile(QString file)
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
