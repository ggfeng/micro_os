/**
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 *
 */

#include <lib/libc/yodalite_libc.h>

#ifdef CONFIG_YODALITE_STRING_ENABLE 

int8_t *yodalite_strcpy(int8_t *dest, const int8_t *src)
{
    register int8_t *ret = dest;

    if (NULL == dest || NULL == src) {
        return NULL;
    }

    while ((*dest++ = *src++))
        ;
    return ret;
}

int8_t *yodalite_strncpy(int8_t *dest, const int8_t *src, size_t n)
{
    register int8_t *ret = dest;

    if (NULL == dest || NULL == src) {
        return NULL;
    }

    while (n && (*dest++ = *src++))
        n--;
    while (n--) {
        *dest++ = '\0';
    }
    return ret;
}

int32_t yodalite_strcmp(const int8_t *s1, const int8_t *s2)
{
    register int32_t ret = 0;

    if (NULL == s1 || NULL == s2) {
        while (1)
            ; /* FIXME(junlin) should be panic? */
    }
    while (!(ret = (*s1 - *s2++)) && *s1++)
        ;
    return ret;
}

int32_t yodalite_strncmp(const int8_t *s1, const int8_t *s2, size_t n)
{
    register int32_t ret = 0;

    if (NULL == s1 || NULL == s2) {
        while (1)
            ; /* FIXME(junlin) should be panic? */
    }
    while (n-- && !(ret = (*s1 - *s2++)) && *s1++)
        ;
    return ret;
}

size_t yodalite_strlen(const int8_t *s)
{
    register int32_t ret = 0;

    if (NULL == s) {
        return 0;
    }

    while (s[ret++])
        ;
    return ret - 1;
}

int8_t *yodalite_strrchr(const int8_t *s, int32_t c)
{
    register int8_t *ret = NULL;

    if (NULL == s) {
        return NULL;
    }
    do {
        if (*s == (int8_t)c) {
            ret = (int8_t *)s;
        }
    } while (*s++);
    return ret;
}

void *yodalite_memset(void *s, int32_t c, size_t n)
{
    int8_t *xs = s;

    if (NULL == s) {
        return NULL;
    }
    while (n--) {
        *xs++ = (int8_t)c;
    }
    return s;
}

void *yodalite_memcpy(void *dest, const void *src, size_t n)
{
    register const int8_t *s = src;
    register int8_t *      d = dest;

    if (NULL == dest || NULL == src) {
        return NULL;
    }
    while (n--) {
        d[n] = s[n];
    }
    return dest;
}

int32_t yodalite_memcmp(const void *s1, const void *s2, size_t n)
{
    register const int8_t *s   = s1;
    register const int8_t *d   = s2;
    register int32_t       ret = 0;

    if (NULL == s1 || NULL == s2) {
        while (1)
            ; /* FIXME(junlin) should be panic? */
    }
    while (n-- && !(ret = (*s++ - *d++)))
        ;
    return ret;
}

void *yodalite_memmove(void *dest, const void *src, size_t count)
{
	char *tmp;
	const char *s;

	if (dest <= src) {
		tmp = dest;
		s = src;
		while (count--)
			*tmp++ = *s++;
	} else {
		tmp = dest;
		tmp += count;
		s = src;
		s += count;
		while (count--)
			*--tmp = *--s;
	}
	return dest;
}


size_t yodalite_strnlen(const char *s, size_t count)
{
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

char *yodalite_strstr(const char *s1, const char *s2)
{
	size_t l1, l2;

	l2 = yodalite_strlen(s2);
	if (!l2)
		return (char *)s1;
	l1 = yodalite_strlen(s1);
	while (l1 >= l2) {
		l1--;
		if (!yodalite_memcmp(s1, s2, l2))
			return (char *)s1;
		s1++;
	}
	return NULL;
}

char *yodalite_strcat(char *dest, const char *src)
{
	char *tmp = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;
	return tmp;
}

char *yodalite_strncat(char *dest, const char *src, size_t count)
{
	char *tmp = dest;

	if (count) {
		while (*dest)
			dest++;
		while ((*dest++ = *src++) != 0) {
			if (--count == 0) {
				*dest = '\0';
				break;
			}
		}
	}
	return tmp;
}

char *yodalite_strchr(const char *s, int c)
{
    for (; *s != (char)c; ++s)
        if (*s == '\0')
            return NULL;
    return (char *)s;
}

char *yodalite_strpbrk(const char *cs, const char *ct)
{
    const char *sc1, *sc2;

    for (sc1 = cs; *sc1 != '\0'; ++sc1) {
        for (sc2 = ct; *sc2 != '\0'; ++sc2) {
            if (*sc1 == *sc2)
                return (char *)sc1;
        }
    }
    return NULL;
}

char *yodalite_strsep(char **s, const char *ct)
{
    char *sbegin = *s;
    char *end;

    if (sbegin == NULL)
        return NULL;

    end = yodalite_strpbrk(sbegin, ct);
    if (end)
        *end++ = '\0';
    *s = end;
    return sbegin;
}

#endif
