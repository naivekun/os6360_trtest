/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
* @internal todigit function
* @endinternal
*
*/
/**
********************************************************************************
* @file mv_hws_strtol.c
*
* @brief strtol(), strtoul implementation
*
*
* @version   1
********************************************************************************
*/

static int todigit(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 10;
    return -1;
}

static unsigned long _strtol(
    const char *nptr,
    char **endptr,
    int base,
    int *neg
)
{
    const char *s = nptr;
    unsigned long acc;
    int any, c;

    *neg = 0;

    /* skip white space */
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\12')
        s++;
    /* pick up leading +/- sign if any. */
    if (*s == '-') {
        *neg=1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    /* If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    if ((base == 0 || base == 16) &&
        *s == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
        base = 16;
    } else if ((base == 0 || base == 2) &&
        *s == '0' && (s[1] == 'b' || s[1] == 'B')) {
        s += 2;
        base = 2;
    }
    if (base == 0)
        base = (*s == '0') ? 8 : 10;
    /* convert */
    for (acc = 0, any = 0; *s; s++) {
        c = todigit(*s);
        if (c < 0 || c >= base)
            break;
        any = 1;
        acc *= base;
        acc += c;
    }
    if (endptr)
        *endptr = (char *)(any ? s : nptr);
    return acc;
}

long hws_strtol(
    const char *nptr,
    char **endptr,
    int base
)
{
    int neg = 0;
    long val;
    val = _strtol(nptr, endptr, base, &neg);
    return neg ? -val : val;
}

unsigned long hws_strtoul(
    const char *nptr,
    char **endptr,
    int base
)
{
    int neg = 0;
    return _strtol(nptr, endptr, base, &neg);
}


