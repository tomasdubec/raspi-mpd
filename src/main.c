#include <pthread.h>
#include <wiringPi.h>
#include <signal.h>
#include "logging.h"
#include "input.h"
#include "main.h"
#include "lcd.h"
#include "mpd.h"
#include "ctrl.h"

int b_end = FALSE;

queue_t *pr_input_queue = NULL;
pthread_mutex_t r_input_queue_mutex;

void signal_handler(int i_signal)
{
    if(i_signal == SIGINT)
    {
        LOG_DEBUG("Caught SIGINT, ordering threads to quit");
        b_end = TRUE;
    }
}

void init(void)
{
    wiringPiSetup();

    lcd_set_line_text(0, "                    ", TRUE);
    lcd_set_line_text(1, "                    ", TRUE);
    lcd_set_line_text(2, "                    ", TRUE);
    lcd_set_line_text(3, VERSION, TRUE);

    if(!mpd_init())
    {
        b_end = TRUE;
        return;
    }

    if(pthread_mutex_init(&r_input_queue_mutex, NULL) != 0)
    {
        LOG_ERROR("Failed to initialize input queue mutex");
        b_end = TRUE;
        return;
    }
}

int main(int argc, char **argv)
{
    pthread_t thr_input;
    pthread_t thr_lcd;
    pthread_t thr_ctrl;

    init();

    if(signal(SIGINT, signal_handler) == SIG_ERR)
    {
        LOG_ERROR("Can't trap SIGINT");
        return(1);
    }

    if(pthread_create( &thr_input, NULL, input_loop, NULL) != 0)
    {
        LOG_ERROR("Failed to create input thread");
        return(1);
    }
    if(pthread_create( &thr_lcd, NULL, lcd_loop, NULL) != 0)
    {
        LOG_ERROR("Failed to create LCD thread");
        return(1);
    }
    if(pthread_create( &thr_ctrl, NULL, ctrl_loop, NULL) != 0)
    {
        LOG_ERROR("Failed to create MPD thread");
        return(1);
    }

    pthread_join(thr_input, NULL);
    pthread_join(thr_lcd, NULL);
    pthread_join(thr_ctrl, NULL);

    return 0;
}

