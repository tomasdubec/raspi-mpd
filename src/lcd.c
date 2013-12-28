#include <wiringPi.h>
#include <lcd.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logging.h"
#include "main.h"
#include "lcd.h"

char *ppc_lcd_text[4];
int   pi_dirty[4];
int   pi_offset[4];
int   pi_step[4];
int   pi_flush[4];
int   i_lcd;
int   i_lcd_initialized = FALSE;
unsigned char pc_bell[8] = { 0b00000100,
                             0b00001110,
                             0b00001110,
                             0b00001110,
                             0b00011111,
                             0b00000000,
                             0b00000100,
                             0b00000000};
unsigned char pc_play[8] = { 0b00010000,
                             0b00011000,
                             0b00011100,
                             0b00011110,
                             0b00011110,
                             0b00011100,
                             0b00011000,
                             0b00010000};
unsigned char pc_pause[8] = { 0b00000000,
                              0b00011011,
                              0b00011011,
                              0b00011011,
                              0b00011011,
                              0b00011011,
                              0b00011011,
                              0b00000000};
unsigned char pc_stop[8] = { 0b00000000,
                             0b00011111,
                             0b00011111,
                             0b00011111,
                             0b00011111,
                             0b00011111,
                             0b00011111,
                             0b00000000};
unsigned char pc_prev[8] = { 0b00010001,
                             0b00010011,
                             0b00010111,
                             0b00011111,
                             0b00011111,
                             0b00010111,
                             0b00010011,
                             0b00010001};
unsigned char pc_next[8] = { 0b00010001,
                             0b00011001,
                             0b00011101,
                             0b00011111,
                             0b00011111,
                             0b00011101,
                             0b00011001,
                             0b00010001};
unsigned char pc_speaker[8] = { 0b00000001,
                                0b00000011,
                                0b00011111,
                                0b00011111,
                                0b00011111,
                                0b00011111,
                                0b00000011,
                                0b00000001};


void lcd_init(void)
{
    int i_index;

    LOG_DEBUG("Initializing LCD GPIO");

    for(i_index = 0;i_index < 4;i_index++)
    {
        pi_dirty[i_index] = TRUE;
        pi_offset[i_index] = FALSE;
        pi_step[i_index] = TRUE;
        pi_flush[i_index] = TRUE;
    }

    i_lcd = lcdInit(4, 20, 4, 11, 10, 0, 1, 2, 3, 0, 0, 0, 0);
    lcdCharDef(i_lcd, CHAR_BELL, pc_bell);
    lcdCharDef(i_lcd, CHAR_PLAY, pc_play);
    lcdCharDef(i_lcd, CHAR_PAUSE, pc_pause);
    lcdCharDef(i_lcd, CHAR_STOP, pc_stop);
    lcdCharDef(i_lcd, CHAR_PREV, pc_prev);
    lcdCharDef(i_lcd, CHAR_NEXT, pc_next);
    lcdCharDef(i_lcd, CHAR_SPEAKER, pc_speaker);

    i_lcd_initialized = TRUE;
}

void lcd_set_line_text(int i_line, char *pc_text, int i_flush)
{
    int i_len;
    char pc_empty[1] = "";

    assert(i_line < 4);

    if(pc_text == NULL)
        pc_text = pc_empty;

    pi_flush[i_line] = i_flush;

    if(ppc_lcd_text[i_line] == NULL ||
       strlen(ppc_lcd_text[i_line]) != strlen(pc_text) ||
       strcmp(ppc_lcd_text[i_line], pc_text) != 0)
    {
        LOG_DEBUG("Setting LCD line %d to \"%s\"", i_line, pc_text)

        if(ppc_lcd_text[i_line] != NULL) free(ppc_lcd_text[i_line]);

        ppc_lcd_text[i_line] = (char*)calloc(sizeof(char), strlen(pc_text) + 1);

        strcpy(ppc_lcd_text[i_line], pc_text);

        pi_dirty[i_line] = 1;
        pi_offset[i_line] = 0;
    }
}

void lcd_set_char(int i_x, int i_y, char c_char)
{
    assert(i_lcd_initialized == TRUE);

    lcdPosition(i_lcd, i_x, i_y);
    lcdPutchar(i_lcd, c_char);
}

void *lcd_loop(void *data)
{
    int        i_index;
    struct tm *pr_time;
    time_t     r_time;
    char       pc_time[21];

    LOG_DEBUG("LCD thread starting");

    lcd_init();

    while(!b_end)
    {
        LOG_DEBUG2("   ,--------------------,");

        r_time = time(NULL);
        pr_time = localtime(&r_time);
        snprintf(pc_time, 21, "  8:00         %d:%02d", pr_time->tm_hour, pr_time->tm_min);
        pc_time[0] = 1;
        lcd_set_line_text(0, pc_time, FALSE);

        for(i_index = 0;i_index < 4;i_index++)
        {
            if(strlen(ppc_lcd_text[i_index]) > 20 ||
               pi_dirty[i_index] == 1)
            {
                if(pi_flush[i_index])
                {
                    lcdPosition(i_lcd, 0, i_index);
                    lcdPuts(i_lcd, "                    ");
                }
                lcdPosition(i_lcd, 0, i_index);
                lcdPrintf(i_lcd, "%.20s", ppc_lcd_text[i_index] + pi_offset[i_index]);

                if(pi_step[i_index] > 0 && (strlen(ppc_lcd_text[i_index]) - pi_offset[i_index]) == 20)
                {
                    pi_step[i_index] = -pi_step[i_index];
                    /* to keep the text still at start/end of scrolling */
                    pi_offset[i_index]++;
                }
                if(pi_step[i_index] < 0 && pi_offset[i_index] == 0)
                {
                    pi_step[i_index] = -pi_step[i_index];
                    /* to keep the text still at start/end of scrolling */
                    pi_offset[i_index]--;
                }
                if(strlen(ppc_lcd_text[i_index]) > 20)
                {
                    pi_offset[i_index] += pi_step[i_index];
                }

                pi_dirty[i_index] = 0;
            }
            LOG_DEBUG2("%d: \"%.20s\"", i_index, ppc_lcd_text[i_index] + pi_offset[i_index]);
        }
        LOG_DEBUG2("   '--------------------'");
        usleep(100000);
    }

    LOG_DEBUG("LCD thread quitting");

    return NULL;
}

