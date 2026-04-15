#include "kernel/types.h"
#include "user/user.h"

#define NANO 1000000000UL
#define MIL  1000000UL

#define SEC_IN_DAY (60 * 60 * 24)
#define SEC_IN_HOUR 3600
#define SEC_IN_MINUTE 60

#define NORMAL_YEAR_DAYS 365

static int
is_leap_year(int year)
{
    if (year % 400 == 0) {
        return 1;
    }
    if (year % 100 != 0 && year % 4 == 0) {
        return 1;
    }
    return 0;
}

static int
days_in_month(int month, int year)
{
    static const int dbm[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    static const int DAYS_IN_FEB_LEAP = 29;

    if (month == 2 && is_leap_year(year)) {
        return DAYS_IN_FEB_LEAP;
    }
    return dbm[month - 1];
}

static int
days_in_year(int year)
{
    return (is_leap_year(year) ? (NORMAL_YEAR_DAYS + 1) : NORMAL_YEAR_DAYS);
}

// pads output to 2 digits
static void
print_helper_dt(int x)
{
    if (x < 10) {
        printf("0%d", x);
        return;
    }
    printf("%d", x);
}

static void
print_helper_msec(uint64 x)
{
    if (x < 10) {
        printf("00%lu", x);
        return;
    }
    if (x < 100) {
        printf("0%lu", x);
        return;
    }
    printf("%lu", x);
}

int
main(int argc, char *argv[])
{
    uint64 time = rtc();
    uint64 rtc_sec = time / NANO;
    uint64 rest_nsec = time % NANO;
    uint64 msec = rest_nsec / MIL;

    uint64 days = rtc_sec / SEC_IN_DAY;
    uint64 rest_sec = rtc_sec % SEC_IN_DAY;

    int hour = rest_sec / SEC_IN_HOUR;
    int min = (rest_sec % SEC_IN_HOUR) / SEC_IN_MINUTE;
    int sec = rest_sec % SEC_IN_MINUTE;

    int year = 1970;
    int dy = days_in_year(year);
    while (days >= dy) {
        days-= dy;
        year++;
        dy = days_in_year(year);
    }
    int month = 1;
    int dm = days_in_month(month, year);
    while (days >= dm) {
        days -= dm;
        month++;
        dm = days_in_month(month, year);
    }
    int day = (int)days + 1;
    
    print_helper_dt(day);
    printf(".");
    print_helper_dt(month);
    printf(".");
    printf("%d", year);
    printf(" ");
    print_helper_dt(hour);
    printf(":");
    print_helper_dt(min);
    printf(":");
    print_helper_dt(sec);
    printf(".");
    print_helper_msec(msec);
    printf("\n");

    exit(0);
}