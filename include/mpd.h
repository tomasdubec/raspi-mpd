#ifndef __MPD_H__
#define __MPD_H__

typedef enum
{
    CMD_PLAY,
    CMD_STOP,
    CMD_NEXT,
    CMD_PREV,
    CMD_PAUSE
} mpd_cmds_t;

int   mpd_init(void);

/* getters */
char *mpd_get_current_song(void);
char *mpd_get_current_song_position(void);
char *mpd_get_status(void);

/* setters */
int mpd_send_cmd(mpd_cmds_t e_cmd);

#endif
