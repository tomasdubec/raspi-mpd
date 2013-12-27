#include <mpd/client.h>
#include "logging.h"
#include "main.h"
#include "mpd.h"
#include "lcd.h"

struct mpd_connection *pr_mpd_conn;

int mpd_init(void)
{
    pr_mpd_conn = mpd_connection_new(NULL, 0, 30000);
    if (mpd_connection_get_error(pr_mpd_conn) != MPD_ERROR_SUCCESS)
    {
        LOG_ERROR("Failed to connect to MPD: %s", mpd_connection_get_error_message(pr_mpd_conn));
        mpd_connection_free(pr_mpd_conn);
        return 0;
    }
    LOG_DEBUG("MPD client initialized");
    return 1;
}

void sec_to_time(int i_seconds, char *pc_buf, int i_max_len)
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

void *mpd_loop(void *data)
{
    struct mpd_status *pr_mpd_status;
    struct mpd_song   *pr_mpd_song;
    char               pc_buf[100];
    char               pc_ela[20];
    char               pc_tot[20];

    LOG_DEBUG("MPD thread starting");

    if(!mpd_init()) return;

    while(!b_end)
    {
        mpd_send_status(pr_mpd_conn);
        pr_mpd_status = mpd_recv_status(pr_mpd_conn);
        if(pr_mpd_status == NULL)
        {
            LOG_ERROR("MPD error: %s", mpd_connection_get_error_message(pr_mpd_conn));
            break;
        }
        if(mpd_status_get_state(pr_mpd_status) == MPD_STATE_PLAY ||
           mpd_status_get_state(pr_mpd_status) == MPD_STATE_PAUSE)
        {
            sec_to_time(mpd_status_get_elapsed_time(pr_mpd_status), pc_ela, 20);
            sec_to_time(mpd_status_get_total_time(pr_mpd_status), pc_tot, 20);
            snprintf(pc_buf, 100, "%s %s/%s", mpd_status_get_state(pr_mpd_status) == MPD_STATE_PLAY ? "|>" : mpd_status_get_state(pr_mpd_status) == MPD_STATE_PAUSE ? "||" : "[]", pc_ela, pc_tot);
        }
        else if(mpd_status_get_state(pr_mpd_status) == MPD_STATE_STOP)
        {
            snprintf(pc_buf, 100, "[]");
        }
        set_line_text(2, pc_buf);

        mpd_status_free(pr_mpd_status);

        mpd_send_current_song(pr_mpd_conn);
        while((pr_mpd_song = mpd_recv_song(pr_mpd_conn)) != NULL)
        {
            if(mpd_song_get_tag(pr_mpd_song, MPD_TAG_TITLE, 0) != NULL)
            {
                snprintf(pc_buf, 100, "%s - %s", mpd_song_get_tag(pr_mpd_song, MPD_TAG_ARTIST, 0), mpd_song_get_tag(pr_mpd_song, MPD_TAG_TITLE, 0));
                set_line_text(1, pc_buf);
            }
            else
            {
                set_line_text(1, (char *)mpd_song_get_tag(pr_mpd_song, MPD_TAG_NAME, 0));
            }

            mpd_song_free(pr_mpd_song);
        }
        sleep(1);
    }

    mpd_connection_free(pr_mpd_conn);

    LOG_DEBUG("MPD thread quitting");
}

