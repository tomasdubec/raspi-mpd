#include <pthread.h>
#include <wiringPi.h>
#include <signal.h>
#include "logging.h"
#include "input.h"
#include "main.h"
#include "lcd.h"
#include "mpd.h"

int b_end = FALSE;

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

    set_line_text(1, "             booting             ");
    set_line_text(2, "--------------------");
    set_line_text(3, VERSION);
}

int main(int argc, char **argv)
{
    pthread_t thr_input;
    pthread_t thr_lcd;
    pthread_t thr_mpd;

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
    if(pthread_create( &thr_mpd, NULL, mpd_loop, NULL) != 0)
    {
        LOG_ERROR("Failed to create MPD thread");
        return(1);
    }

    pthread_join(thr_input, NULL);
    pthread_join(thr_lcd, NULL);
    pthread_join(thr_mpd, NULL);

    return 0;
}

