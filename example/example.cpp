#include "../ProgressBar.hpp"
#include "millisleep.h"
#include <cstdio>

#define MSEC 14

int main(void) {
    unsigned long i;
    unsigned long total = 123L;

    std::printf("\n* Progressbar examples *\n\n");

    /* (1) default style */
    ProgressBar pbar(total, "[DEFAULT]: ");
    pbar.start();  /* pbar.start() resets internal counter and disables a cursor */
    for (i=0; i<total; ++i) {
        millisleep(MSEC);
        pbar.update();
    }

    /* (2) change ratio style */
    pbar.set_title("[CHANGE RATIO STYLE]: ");
    pbar.use_ratiostyle();
    pbar.start();
    for (i=0; i<total; ++i) {
        millisleep(MSEC);
        pbar++;  /* pbar++ and ++pbar are equivalent to pbar.update() */
    }

    /* (3) change bracket style */
    pbar.set_title("[CHANGE BRACKET STYLE]: ");
    pbar.use_percentstyle();
    pbar.set_bracketstyle('<', '>');
    pbar.start();
    for (i=0; i<total; ++i) {
        millisleep(MSEC);
        ++pbar;
    }

    /* (4) change bar style 1 */
    pbar.set_title("[CHANGE BAR STYLE 1]: ");
    pbar.set_bracketstyle(); /* reset bracket style */
    pbar.set_barstyle('-', '>', ' ');
    pbar.start();
    for (i=0; i<total; ++i) {
        millisleep(MSEC);
        pbar.update();
    }

    /* (5) change bar style 2 */
    pbar.set_title("[CHANGE BAR STYLE 2]: ");
    pbar.set_bracketstyle(); /* reset bracket style */
    pbar.set_barstyle('#', ' ', ' ');
    pbar.start();
    for (i=0; i<total; ++i) {
        millisleep(MSEC);
        pbar.update();
    }

    /* (6) tricky example: growing bar */
    pbar.set_title("[TRICKY EXAMPLE]: ");
    pbar.set_barstyle('=', '>', '\0'); /* NOTE: the 3rd argument is NUL. */
    pbar.start();
    for (i=0; i<total; ++i) {
        millisleep(MSEC);
        pbar.update();
    }

    return 0;
}
