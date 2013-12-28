#ifndef __MPD_H__
#define __MPD_H__

int   mpd_init(void);
char *mpd_get_current_song(void);
char *mpd_get_current_song_position(void);
char *mpd_get_status(void);

#endif
