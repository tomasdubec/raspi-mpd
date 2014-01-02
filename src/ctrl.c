#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "logging.h"
#include "ctrl.h"
#include "main.h"
#include "mpd.h"
#include "lcd.h"
#include "input.h"

#define MPD_DELAY 1000
#define BUTTONS_CNT 6

typedef enum
{
    FSM_IDLE,
    FSM_IDLE_ALARM_CHANGED,
    FSM_MPD,
    FSM_VOLUME
} fsm_states_t;

char pc_btn_icons[BUTTONS_CNT] = { CHAR_PREV, CHAR_PLAY, CHAR_PAUSE, CHAR_STOP, CHAR_NEXT, CHAR_SPEAKER };
int  i_btn_selected = 0;
int  i_idle = 0;
int  i_idle_threshold = 5;
int  b_alarm_enabled = FALSE;
long l_volume = 0;
fsm_states_t e_fsm_state = FSM_IDLE;

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
        case FSM_IDLE_ALARM_CHANGED:
            switch(i_keycode)
            {
                case BTN_BACK + LONG_PRESS_OFFSET:
                    break;
                default:
                    e_fsm_state = FSM_IDLE;
            }
            break;
        case FSM_IDLE:
            switch(i_keycode)
            {
                case BTN_IDLE:
                    break;
                case BTN_BACK + LONG_PRESS_OFFSET:
                    if(b_alarm_enabled == FALSE) b_alarm_enabled = TRUE;
                    else b_alarm_enabled = FALSE;

                    e_fsm_state = FSM_IDLE_ALARM_CHANGED;
                    break;
                default:
                    e_fsm_state = FSM_MPD;
                    break;
            }
            break;
        case FSM_MPD:
            switch(i_keycode)
            {
                case BTN_IDLE:
                    e_fsm_state = FSM_IDLE;
                    break;
                case BTN_UP:
                    i_btn_selected = (i_btn_selected + 1) % BUTTONS_CNT;
                    break;
                case BTN_DOWN:
                    i_btn_selected = (i_btn_selected - 1 + BUTTONS_CNT) % BUTTONS_CNT;
                    break;
                case BTN_OK:
                    switch(pc_btn_icons[i_btn_selected])
                    {
                        case CHAR_PLAY:
                            mpd_send_cmd(CMD_PLAY);
                            break;
                        case CHAR_PAUSE:
                            mpd_send_cmd(CMD_PAUSE);
                            break;
                        case CHAR_NEXT:
                            mpd_send_cmd(CMD_NEXT);
                            break;
                        case CHAR_PREV:
                            mpd_send_cmd(CMD_PREV);
                            break;
                        case CHAR_STOP:
                            mpd_send_cmd(CMD_STOP);
                            break;
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
                case BTN_IDLE:
                    lcd_set_line_text(2, NULL, TRUE);
                    e_fsm_state = FSM_MPD;
                    break;
                case BTN_UP:
                case BTN_UP + LONG_PRESS_OFFSET:
                    ctrl_set_volume(l_volume + 5);
                    break;
                case BTN_DOWN:
                case BTN_DOWN + LONG_PRESS_OFFSET:
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

void ctrl_get_header(char **ppc_text)
{
    char      *pc_text;
    struct tm *pr_time;
    time_t     r_time;

    *ppc_text = NULL;

    pc_text = (char *)calloc(sizeof(char), 21);
    if(pc_text == NULL)
    {
        LOG_ERROR("Failed to allocate memory");
        return;
    }

    r_time = time(NULL);
    pr_time = localtime(&r_time);
    snprintf(pc_text, 21, "  8:00         %d:%02d", pr_time->tm_hour, pr_time->tm_min);

    if(b_alarm_enabled == TRUE)
    {
        pc_text[0] = CHAR_BELL;
    }
    else
    {
        pc_text[0] = ' ';
    }

    pc_text[10] = mpd_get_status();

    *ppc_text = pc_text;
}

void *ctrl_loop(void *data)
{
    char      *pc_song_name = NULL;
    char      *pc_song = NULL;
    char      *pc_buf;
    char       pc_buttons[21];
    char       pc_tmp[21];
    int        i_tmp;
    int        i_loop;
    int        i_keycode = -1;
    int        i_delay = MPD_DELAY;

    LOG_DEBUG("Control loop starting");

    memset((void*)pc_buttons, 0, 21);

    while(!b_end)
    {
        i_delay--;
        if(i_delay < 0)
        {
            i_delay = MPD_DELAY;
            i_idle++;
        }

        switch(e_fsm_state)
        {
            case FSM_IDLE:
            case FSM_IDLE_ALARM_CHANGED:
                if(i_delay == 0)
                {
                    ctrl_get_header(&pc_buf);
                    lcd_set_line_text(0, pc_buf, FALSE);
                    if(pc_buf != NULL) free(pc_buf);
                }

                lcd_set_line_text(1, NULL, TRUE);
                lcd_set_line_text(2, NULL, TRUE);
                lcd_set_line_text(3, NULL, TRUE);
                break;

            case FSM_MPD:
                if(i_delay == 0)
                {
                    ctrl_get_header(&pc_buf);
                    lcd_set_line_text(0, pc_buf, FALSE);
                    if(pc_buf != NULL) free(pc_buf);

                    pc_song_name = mpd_get_current_song();

                    pc_song = mpd_get_current_song_position();
                }

                if(pc_song_name != NULL)
                {
                    lcd_set_line_text(1, pc_song_name, TRUE);
                    free(pc_song_name);
                    pc_song_name = NULL;
                }
                if(pc_song != NULL)
                {
                    lcd_set_line_text(2, pc_song, pc_song[0] == CHAR_STOP);
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

                break;
            case FSM_VOLUME:
                if(i_delay == 0)
                {
                    l_volume = alsa_get_volume();
                    LOG_DEBUG("Got alsa volume: %d", l_volume);
                }

                lcd_set_line_text(1, "    Music volume", TRUE);

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
            i_idle = 0;
            ctrl_fsm(i_keycode);
        }

        /* handle idle action */
        if(i_idle > i_idle_threshold)
        {
            ctrl_fsm(BTN_IDLE);
            i_idle = 0;
        }

        usleep(1000);
    }

    LOG_DEBUG("Control loop quitting");

    return NULL;
}
