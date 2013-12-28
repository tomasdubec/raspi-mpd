#include <stdlib.h>
#include <string.h>
#include "logging.h"
#include "ctrl.h"
#include "main.h"
#include "mpd.h"
#include "lcd.h"
#include "input.h"

#define MPD_QUERY_DELAY 1000
#define BUTTONS_CNT 6

typedef enum
{
    FSM_MPD,
    FSM_VOLUME
} fsm_states_t;

char pc_btn_icons[BUTTONS_CNT] = { CHAR_PREV, CHAR_PLAY, CHAR_PAUSE, CHAR_STOP, CHAR_NEXT, CHAR_SPEAKER };
int  i_btn_selected = 0;
long l_volume = 56;
fsm_states_t e_fsm_state = FSM_MPD;

extern pthread_mutex_t r_input_queue_mutex;
extern queue_t *pr_input_queue;

int ctrl_pop_input(void)
{
    queue_t *pr_tmp;
    int      i_keycode;

    LOG_DEBUG2("Locking input queue mutex");
    pthread_mutex_lock(&r_input_queue_mutex);

    if(pr_input_queue == NULL)
    {
        LOG_DEBUG2("Unlocking input queue mutex");
        pthread_mutex_unlock(&r_input_queue_mutex);
        return -1;
    }

    pr_tmp = pr_input_queue;
    pr_input_queue = pr_input_queue->pr_next;

    LOG_DEBUG("Popped keycode %d from input queue", pr_tmp->i_keycode);

    LOG_DEBUG2("Unlocking input queue mutex");
    pthread_mutex_unlock(&r_input_queue_mutex);

    i_keycode = pr_tmp->i_keycode;
    free(pr_tmp);

    return i_keycode;
}

void ctrl_set_volume(long l_vol)
{
    l_volume = l_vol;
    if(l_volume < 0)
        l_volume = 0;
    if(l_volume > 100)
        l_volume = 100;

    LOG_DEBUG("Setting volume to %ld", l_volume);

    alsa_set_volume(l_volume);
}

void ctrl_fsm(int i_keycode)
{
    switch(e_fsm_state)
    {
        case FSM_MPD:
            switch(i_keycode)
            {
                case BTN_UP:
                    i_btn_selected = (i_btn_selected + 1) % BUTTONS_CNT;
                    break;
                case BTN_DOWN:
                    i_btn_selected = (i_btn_selected - 1 + BUTTONS_CNT) % BUTTONS_CNT;
                    break;
                case BTN_OK:
                    switch(pc_btn_icons[i_btn_selected])
                    {
                        case CHAR_SPEAKER:
                            e_fsm_state = FSM_VOLUME;
                            break;
                    }
                    break;
            }
            break;
        case FSM_VOLUME:
            switch(i_keycode)
            {
                case BTN_UP:
                    ctrl_set_volume(l_volume + 5);
                    break;
                case BTN_DOWN:
                    ctrl_set_volume(l_volume - 5);
                    break;
                case BTN_OK:
                case BTN_BACK:
                    /* flush the line (song time do not refresh whole line */
                    lcd_set_line_text(2, NULL, TRUE);
                    e_fsm_state = FSM_MPD;
                    break;
            }
            break;
    }
}

void *ctrl_loop(void *data)
{
    char *pc_buf;
    char *pc_song = NULL;
    char  pc_buttons[21];
    char  pc_tmp[21];
    int   i_tmp;
    int   i_loop;
    int   i_keycode = -1;
    int   i_query_mpd = MPD_QUERY_DELAY;

    LOG_DEBUG("Control loop starting");

    memset((void*)pc_buttons, 0, 21);

    while(!b_end)
    {
        i_query_mpd--;

        if(i_query_mpd <=0)
        {
            i_query_mpd = MPD_QUERY_DELAY;

            pc_buf = mpd_get_current_song();
            if(pc_buf != NULL)
            {
                lcd_set_line_text(1, pc_buf, TRUE);
                free(pc_buf);
            }

            pc_song = mpd_get_current_song_position();
        }

        switch(e_fsm_state)
        {
            case FSM_MPD:
                if(pc_song != NULL)
                {
                    lcd_set_line_text(2, pc_song, pc_song[0] == CHAR_STOP);
                }
                break;
            case FSM_VOLUME:
                memset(pc_tmp, ' ', 21);
                pc_tmp[2] = (char)CHAR_SPEAKER;
                pc_tmp[17] = (char)CHAR_SPEAKER;
                pc_tmp[20] = '\0';
                for(i_tmp = 5; i_tmp < 15; i_tmp++)
                {
                    if(l_volume / 10 + 5 > i_tmp)
                        pc_tmp[i_tmp] = (char)CHAR_FULL;
                    else
                        pc_tmp[i_tmp] = '_';
                }
                lcd_set_line_text(2, pc_tmp, TRUE);
                break;
        }

        if(pc_song != NULL)
        {
            free(pc_song);
            pc_song = NULL;
        }

        /* handle keypresses */
        i_keycode = ctrl_pop_input();
        if(i_keycode != -1)
        {
            ctrl_fsm(i_keycode);
        }

        /* print command keys */
        for(i_tmp = 0;i_tmp < BUTTONS_CNT;i_tmp++)
        {
            pc_buttons[i_tmp * 3] = i_btn_selected == i_tmp ? (char)CHAR_FULL : ' ';
            pc_buttons[i_tmp * 3 + 1] = pc_btn_icons[i_tmp];
            pc_buttons[i_tmp * 3 + 2] = i_btn_selected == i_tmp ? (char)CHAR_FULL : ' ';
            pc_buttons[i_tmp * 3 + 3] = '\0';
        }
        lcd_set_line_text(3, pc_buttons, TRUE);

        usleep(1000);
    }

    LOG_DEBUG("Control loop quitting");

    return NULL;
}
