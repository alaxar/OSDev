#ifndef LOCALE_H
#define LOCALE_H

enum {
    LC_COLLATE, LC_CTYPE, LC_MONETARY, LC_NUMERIC, LC_TIME,
    _LC_NUM, LC_ALL
};

struct lconv {
    char* decimal_point;
    char* thousands_sep;
    char* grouping;
    char* int_curr_symbol;
    char* currency_symbol;
    char* mon_decimal_point;
    char* mon_thousands_group;
    char* mon_grouping;
    char* positive_sign;
    char* negative_sign;
    char frac_digits;
    char p_cs_precedes;
    char n_cs_precedes;
    char p_sep_by_space;
    char n_sep_by_space;
    char p_sign_posn;
    char n_sign_posn;
    char int_frac_digits;
    char int_p_cs_precedes;
    char int_n_cs_precedes;
    char int_p_sep_by_space;
    char int_n_sep_by_space;
    char int_p_sign_posn;
    char int_n_sign_posn;
};

#ifdef _cplusplus
extern "C" {
#endif

struct lconv* localeconv(void);
char* setlocale(int category, const char* locale);

#ifdef _cplusplus
}
#endif

#endif
