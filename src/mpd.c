#include <stdlib.h>
#include <mpd/client.h>
#include "logging.h"
#include "main.h"
#include "mpd.h"
#include "lcd.h"

#define SONG_POSITION_LENGTH 30
#define SONG_NAME_LENGTH     500

struct mpd_connection *pr_mpd_conn = NULL;

struct mpd_status *mpd_get_mpd_status(void);

int mpd_init(void)
{
    if(pr_mpd_conn != NULL)
    {
        mpd_connection_free(pr_mpd_conn);
    }
    pr_mpd_conn = mpd_connection_new(NULL, 0, 30000);
    if (mpd_connection_get_error(pr_mpd_conn) != MPD_ERROR_SUCCESS)
    {
        LOG_ERROR("Failed to connect to MPD: %s (%d)", mpd_connection_get_error_message(pr_mpd_conn), mpd_connection_get_error(pr_mpd_conn));
        mpd_connection_free(pr_mpd_conn);
        return 0;
    }
    LOG_DEBUG("MPD client initialized");
    return 1;
}

void mpd_sec_to_time(int i_seconds, char *pc_buf, int i_max_len)
{
    int   i_min;
    int   i_hour;

    i_hour    = i_seconds/3600;
    i_seconds = i_seconds - i_hour * 3600;
    i_min     = i_seconds/60;
    i_seconds = i_seconds - i_min * 60;

    if(i_hour > 0)
        snprintf(pc_buf, i_max_len, "%d:%02d:%02d", i_hour, i_min, i_seconds);
    else
        snprintf(pc_buf, i_max_len, "%02d:%02d", i_min, i_seconds);
}

char *mpd_get_current_song(void)
{
    struct mpd_song   *pr_mpd_song;
    char              *pc_buf;

    LOG_DEBUG("MPD getting current song");

    if(pr_mpd_conn == NULL)
    {
        LOG_WARN("Getting song name failed, MPD not connected");
        return NULL;
    }

    pc_buf = (char *)calloc(sizeof(char), SONG_NAME_LENGTH);
    if(pc_buf == NULL)
    {
        LOG_ERROR("Failed to allocate memory for song position");
        return NULL;
    }

    mpd_send_current_song(pr_mpd_conn);
    while((pr_mpd_song = mpd_recv_song(pr_mpd_conn)) != NULL)
    {
        if(mpd_song_get_tag(pr_mpd_song, MPD_TAG_TITLE, 0) != NULL)
        {
            snprintf(pc_buf, SONG_NAME_LENGTH, "%s - %s", mpd_song_get_tag(pr_mpd_song, MPD_TAG_ARTIST, 0), mpd_song_get_tag(pr_mpd_song, MPD_TAG_TITLE, 0));
        }
        else
        {
            snprintf(pc_buf, SONG_NAME_LENGTH, "%s", (char *)mpd_song_get_tag(pr_mpd_song, MPD_TAG_NAME, 0));
        }
        mpd_song_free(pr_mpd_song);
    }

    return pc_buf;
}

struct mpd_status *mpd_get_mpd_status(void)
{
    struct mpd_status *pr_mpd_status;

    mpd_send_status(pr_mpd_conn);
    pr_mpd_status = mpd_recv_status(pr_mpd_conn);
    if(pr_mpd_status == NULL && mpd_connection_get_error(pr_mpd_conn) != MPD_ERROR_CLOSED)
    {
        if(mpd_connection_get_error(pr_mpd_conn) != MPD_ERROR_CLOSED)
        {
            LOG_ERROR("MPD error: %s", mpd_connection_get_error_message(pr_mpd_conn));
            return NULL;
        }
        else
        {
            LOG_WARN("Reconnecting to MPD");
            mpd_init();
            return mpd_get_mpd_status();
        }
    }

    return pr_mpd_status;
}

char *mpd_get_current_song_position(void)
{
    struct mpd_status *pr_mpd_status;
    char              *pc_buf;
    char               pc_ela[20];
    char               pc_tot[20];

    LOG_DEBUG("MPD getting current song position");

    pc_buf = (char *)calloc(sizeof(char), SONG_POSITION_LENGTH);
    if(pc_buf == NULL)
    {
        LOG_ERROR("Failed to allocate memory for song position");
        return NULL;
    }

    pr_mpd_status = mpd_get_mpd_status();
    if(pr_mpd_status == NULL)
    {
        free(pc_buf);
        return NULL;
    }

    if(mpd_status_get_state(pr_mpd_status) == MPD_STATE_PLAY ||
       mpd_status_get_state(pr_mpd_status) == MPD_STATE_PAUSE)
    {
        mpd_sec_to_time(mpd_status_get_elapsed_time(pr_mpd_status), pc_ela, 20);
        mpd_sec_to_time(mpd_status_get_total_time(pr_mpd_status), pc_tot, 20);
        snprintf(pc_buf, SONG_POSITION_LENGTH, "%c %s/%s", mpd_status_get_state(pr_mpd_status) == MPD_STATE_PLAY ? (char)CHAR_PLAY : mpd_status_get_state(pr_mpd_status) == MPD_STATE_PAUSE ? (char)CHAR_PAUSE : (char)CHAR_STOP, pc_ela, pc_tot);
    }
    else if(mpd_status_get_state(pr_mpd_status) == MPD_STATE_STOP)
    {
        snprintf(pc_buf, SONG_POSITION_LENGTH, "%c", (char)CHAR_STOP);
    }

    mpd_status_free(pr_mpd_status);

    return pc_buf;
}

char mpd_get_status(void)
{
    struct mpd_status *pr_mpd_status;
    char               c_status = ' ';

    LOG_DEBUG("MPD getting current status");

    pr_mpd_status = mpd_get_mpd_status();
    if(pr_mpd_status == NULL)
    {
        return ' ';
    }

    switch(mpd_status_get_state(pr_mpd_status))
    {
        case MPD_STATE_PLAY:
            c_status = CHAR_PLAY;
            break;
        case MPD_STATE_STOP:
            c_status = CHAR_STOP;
            break;
        case MPD_STATE_PAUSE:
            c_status = CHAR_PAUSE;
            break;
    }

    mpd_status_free(pr_mpd_status);

    return c_status;
}

/* setters */
int mpd_send_cmd(mpd_cmds_t e_cmd)
{
    struct mpd_status *pr_mpd_status;

    switch(e_cmd)
    {
        case CMD_PAUSE:
            mpd_send_toggle_pause(pr_mpd_conn);
            pr_mpd_status = mpd_recv_status(pr_mpd_conn);
            mpd_status_free(pr_mpd_status);
            break;
        case CMD_PLAY:
            mpd_run_play(pr_mpd_conn);
            break;
        case CMD_NEXT:
            mpd_run_next(pr_mpd_conn);
            break;
        case CMD_PREV:
            mpd_run_previous(pr_mpd_conn);
            break;
        case CMD_STOP:
            mpd_run_stop(pr_mpd_conn);
            break;
    }
}



