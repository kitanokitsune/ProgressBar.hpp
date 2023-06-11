#pragma once
#ifndef PROGRESSBAR_HPP__
#define PROGRESSBAR_HPP__
/*****************************************************************************
ProgressBar.hpp: CUI Progress bar

******************************************************************************
ProgressBar.hpp is under MIT license
----------------------------------
Copyright (c) 2023 Kitanokitsune

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#include <stdio.h>  /* <cstdio> causes errors to BCC 5.5 */
#include <string>
#include <iomanip>


#ifdef _WIN32
#  include <windows.h>
#  ifndef COMMON_LVB_GRID_HORIZONTAL
#    define COMMON_LVB_GRID_HORIZONTAL  0x0400
#  endif
#  ifndef COMMON_LVB_UNDERSCORE
#    define COMMON_LVB_UNDERSCORE  0x8000
#  endif
#else
#  include <sys/ioctl.h>
#  include <unistd.h>
#endif

#define MAX_BAR_LENGTH 256
#define DEFAULT_CONSOLE_WIDTH 80

#define pbar_max(a,b) ((a)>(b) ? (a) : (b))

#define pbar_round(x) static_cast<int>(x + 0.5)   /* NOTE: x >= 0.0 */
#define pbar_floor(x) static_cast<int>(x)         /* NOTE: x >= 0.0 */

#define MAXDIGITS(x)   (((sizeof(x)*1233)>>9)+1)  /* NOTE: 266 >= sizeof(x) */
#define MAXDIGITS_ULONG  MAXDIGITS(unsigned long)


class ProgressBar {
  private:
    double percent;
    double step;
    double tick;
    double checkpoint;
    unsigned long total;
    unsigned long state;
    int consolewidth;
    char charBarOn;
    char charBarOff;
    char charBarArrow;
    char charBracketLeft;
    char charBracketRight;
    std::string bartitle;
    std::string strtotal;
    bool unknown_console;
    bool num_style_is_percent;
#ifdef _WIN32
    CONSOLE_CURSOR_INFO cursor_info;
    HANDLE hStdoutHandle;
    DWORD consoleattribute;
#endif


  private:

#ifdef _WIN32
    inline void invert_attribute(void) {
        SetConsoleTextAttribute(hStdoutHandle,
                                // COMMON_LVB_UNDERSCORE |
                                COMMON_LVB_GRID_HORIZONTAL |
                                ((consoleattribute>>4) & 0x0fU) |
                                ((consoleattribute<<4) & 0xf0U));
    };

    inline void restore_attribute(void) {
        SetConsoleTextAttribute(hStdoutHandle, consoleattribute);
    };

    int get_consolewidth(void) {
        BOOL bRet;
        CONSOLE_SCREEN_BUFFER_INFO info;
        bRet = GetConsoleScreenBufferInfo(
            hStdoutHandle, 
            &info);
        if (bRet) {
            return (1 + info.srWindow.Right);
        }
        return DEFAULT_CONSOLE_WIDTH;
    };
#else
    int get_consolewidth(void) {
        struct winsize ws;
        if( ioctl( STDERR_FILENO, TIOCGWINSZ, &ws ) != -1 ) {
            if( 0 < ws.ws_col && ws.ws_col == (size_t)ws.ws_col ) {
                return ws.ws_col;
            }
        }
        return DEFAULT_CONSOLE_WIDTH;
    };
#endif

    void redraw(void) {
        int bar_len;
        double f;
        char *ptr;
        int i;
        int count;
        char buf[MAX_BAR_LENGTH + 1];  /* bar length + NUL('\0') */
        char str[1 + MAXDIGITS_ULONG + 1]; /* sign + digits + NUL */
        char fmt[1 + MAXDIGITS_ULONG + 1];

        /* calculate bar length */
        bar_len = consolewidth - bartitle.size();
        if (num_style_is_percent) {
            bar_len -= 8;
        } else {
            bar_len -= (5 + 2*strtotal.size());
        }

        if (bar_len < 1) {
            bar_len = 1;
        } else if (bar_len > MAX_BAR_LENGTH) {
            bar_len = MAX_BAR_LENGTH;
        }

        /* print progress bar */
        f = percent * 0.01 * static_cast<double>(bar_len);
        count = pbar_round(f);
        fprintf(stderr, "\r%s%c", bartitle.c_str(), charBracketLeft);

        ptr = buf;
        if (unknown_console) {
            for (i=0; i<count; ++i) *(ptr++) = '#';
            *ptr = '\0';
            fprintf(stderr, "%s", buf);
        } else if (charBarOn == '\0') {
            for (i=0; i<count; ++i) *(ptr++) = ' ';
            *ptr = '\0';
#ifdef _WIN32
            invert_attribute();
            fprintf(stderr, "%s", buf);
            restore_attribute();
#else
            fprintf(stderr, "\x1b[7m%s\x1b[m", buf);
#endif
        } else {
            for (i=0; i<count; ++i) *(ptr++) = charBarOn;
            *ptr = '\0';
            fprintf(stderr, "%s", buf);
        }

        ptr = buf;
        if (i<bar_len) *(ptr++) = charBarArrow;
        if (charBarOff != '\0') for (i=count+1; i<bar_len; ++i) *(ptr++) = charBarOff;
        *ptr = '\0';
        if (num_style_is_percent) {
            fprintf(stderr, "%s%c %3d%%", buf, charBracketRight, pbar_floor(percent));
        } else {
            sprintf(fmt, "%%%ulu", static_cast<unsigned int>(strtotal.size()));
            sprintf(str, fmt, state);
            fprintf(stderr, "%s%c %s/%s", buf, charBracketRight, str, strtotal.c_str());
        }
    };

    void init_(void) {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO info;
        hStdoutHandle = GetStdHandle(STD_ERROR_HANDLE);
        if (GetConsoleScreenBufferInfo(hStdoutHandle, &info)) {
            consoleattribute = info.wAttributes;
            GetConsoleCursorInfo(hStdoutHandle, &cursor_info);
            unknown_console = false;
        } else {
            unknown_console = true;
        }
#else
        struct winsize ws;
        unknown_console = true;
        if( ioctl( STDERR_FILENO, TIOCGWINSZ, &ws ) != -1 ) {
            if( 0 < ws.ws_col && ws.ws_col == (size_t)ws.ws_col ) {
                unknown_console = false;
            }
        }
#endif
    };


  public:
    ProgressBar(unsigned long t=1) {
        init_();
        set_total(t);
        set_barstyle();
        set_width();
        set_title("");
        set_bracketstyle();
        use_percentstyle();
    };

    ProgressBar(unsigned long t, std::string str, char fgch='\0', char arrch='.', char bgch='.', int wid=-1) {
        init_();
        set_total(t);
        set_barstyle(fgch, arrch, bgch);
        set_width(wid);
        set_title(str);
        set_bracketstyle();
        use_percentstyle();
    };

    void set_barstyle(char fg='\0', char arrw='.', char bg='.') {
        charBarOn = fg;
        charBarArrow = arrw;
        charBarOff = bg;
    };

    void set_total(unsigned long t) {
        char str[1 + MAXDIGITS_ULONG + 1];

        total = pbar_max(t, 1);
        sprintf(str, "%lu", total);
        strtotal = str;
        step = 100.0/static_cast<double>(total);
        tick = pbar_max(1.0, step);
        checkpoint = tick;
        percent = 0.0;
    };

    void set_width(int width = -1) {
        if (width > 0) {
            consolewidth = width;
        } else {
            consolewidth = get_consolewidth();
        }
    };

    void set_title(std::string str) {
        bartitle = str;
    };

    void set_bracketstyle(char lbra='|', char rbra='|') {
        charBracketLeft  = lbra;
        charBracketRight = rbra;
    };

    void use_percentstyle(void) {
        num_style_is_percent = true;
    };

    void use_ratiostyle(void) {
        num_style_is_percent = false;
    };

    void reset(void) {
        percent = 0.0;
        state = 0;
        checkpoint = tick;
    };

    void start(void) {
        reset();
        set_cursor_visible(false);
        redraw();
    };

    void set_cursor_visible(bool onoff) {
#ifdef _WIN32
        cursor_info.bVisible = onoff;
        SetConsoleCursorInfo(hStdoutHandle, &cursor_info);
#else
        if (onoff) {
            fprintf(stderr, "\e[?25h");
        } else {
            fprintf(stderr, "\e[?25l");
        }
#endif
    };

    void update(void) {
        if (state >= total) return;

        ++state;
        percent += step;
        if (state >= total) {
            percent = 100.03125;
            redraw();
            fprintf(stderr, "\n");
            set_cursor_visible(true);
            fflush(stderr);
        } else if (percent >= checkpoint) {
            redraw();
            checkpoint = percent + tick;
        }
    };

    inline void operator ++() {
        update();
    };

    inline void operator ++(int n) {
        update();
    };

};

#endif /* PROGRESSBAR_HPP__ */