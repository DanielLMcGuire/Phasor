// Copyright 2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://phasor.pages.dev/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int vsscanf_compat(const char *str, const char *fmt, va_list ap)
{
	int assigned = 0;
	const char *s = str;
	const char *f = fmt;

	while (*f != '\0') {
		if (isspace((unsigned char)*f)) {
			while (isspace((unsigned char)*f)) f++;
			while (isspace((unsigned char)*s)) s++;
			continue;
		}

		if (*f != '%') {
			if (*s != *f) return assigned;
			s++; f++;
			continue;
		}

		f++;

		if (*f == '%') {
			if (*s != '%') return assigned;
			s++; f++;
			continue;
		}

		int suppress = 0;
		if (*f == '*') { suppress = 1; f++; }

		int width = -1;
		if (isdigit((unsigned char)*f)) {
			width = 0;
			while (isdigit((unsigned char)*f)) { width = width * 10 + (*f - '0'); f++; }
		}

		int lenmod = 0; /* 0=default, 1=long, 2=long long, -1=short, -2=char, 3=long double */
		for (;;) {
			if (*f == 'l') { lenmod = (lenmod == 1) ? 2 : 1; f++; }
			else if (*f == 'h') { lenmod = (lenmod == -1) ? -2 : -1; f++; }
			else if (*f == 'L') { lenmod = 3; f++; }
			else break;
		}

		char conv = *f;
		if (conv == '\0') return assigned;
		f++;

		if (conv != 'c' && conv != 'n') {
			while (isspace((unsigned char)*s)) s++;
		}

		switch (conv) {
			case 'd': case 'i': case 'u': case 'x': case 'X': case 'o': {
				int base = 10;
				if (conv == 'x' || conv == 'X') base = 16;
				else if (conv == 'o') base = 8;
				else if (conv == 'i') base = 0;

				char tmp[64];
				size_t maxlen = sizeof(tmp) - 1;
				if (width >= 0 && (size_t)width < maxlen) maxlen = (size_t)width;

				size_t n = 0;
				const char *p = s;
				if (*p == '+' || *p == '-') { if (n < maxlen) { tmp[n++] = *p; p++; } }
				while (*p && n < maxlen &&
				       ((base == 16 && isxdigit((unsigned char)*p)) ||
				        (base != 16 && isdigit((unsigned char)*p)))) {
					tmp[n++] = *p++;
				}
				if (n == 0 || (n == 1 && (tmp[0] == '+' || tmp[0] == '-'))) return assigned;
				tmp[n] = '\0';

				char *endp;
				const char *tokstart = s;
				if (conv == 'u') {
					unsigned long long val = strtoull(tmp, &endp, base);
					s = tokstart + (endp - tmp);
					if (!suppress) {
						if (lenmod == 1) *va_arg(ap, unsigned long *) = (unsigned long)val;
						else if (lenmod == 2) *va_arg(ap, unsigned long long *) = val;
						else if (lenmod == -1) *va_arg(ap, unsigned short *) = (unsigned short)val;
						else if (lenmod == -2) *va_arg(ap, unsigned char *) = (unsigned char)val;
						else *va_arg(ap, unsigned int *) = (unsigned int)val;
						assigned++;
					}
				} else {
					long long val = strtoll(tmp, &endp, base);
					s = tokstart + (endp - tmp);
					if (!suppress) {
						if (lenmod == 1) *va_arg(ap, long *) = (long)val;
						else if (lenmod == 2) *va_arg(ap, long long *) = val;
						else if (lenmod == -1) *va_arg(ap, short *) = (short)val;
						else if (lenmod == -2) *va_arg(ap, signed char *) = (signed char)val;
						else *va_arg(ap, int *) = (int)val;
						assigned++;
					}
				}
				break;
			}
			case 'f': case 'e': case 'g': case 'E': case 'G': case 'a': case 'A': {
				char tmp[128];
				size_t maxlen = sizeof(tmp) - 1;
				if (width >= 0 && (size_t)width < maxlen) maxlen = (size_t)width;
				size_t n = 0;
				const char *p = s;
				while (*p && n < maxlen &&
				       (isdigit((unsigned char)*p) || *p == '+' || *p == '-' || *p == '.' || *p == 'e' || *p == 'E')) {
					tmp[n++] = *p++;
				}
				if (n == 0) return assigned;
				tmp[n] = '\0';
				char *endp;
				double val = strtod(tmp, &endp);
				if (endp == tmp) return assigned;
				s += (endp - tmp);
				if (!suppress) {
					if (lenmod == 1) *va_arg(ap, double *) = val;
					else if (lenmod == 3) *va_arg(ap, long double *) = (long double)val;
					else *va_arg(ap, float *) = (float)val;
					assigned++;
				}
				break;
			}
			case 's': {
				const char *start = s;
				while (*s && !isspace((unsigned char)*s) && (width < 0 || (int)(s - start) < width)) s++;
				if (s == start) return assigned;
				if (!suppress) {
					char *dst = va_arg(ap, char *);
					memcpy(dst, start, (size_t)(s - start));
					dst[s - start] = '\0';
					assigned++;
				}
				break;
			}
			case 'c': {
				int want = (width < 0) ? 1 : width;
				int avail = 0;
				const char *p = s;
				while (avail < want && *p) { avail++; p++; }
				if (avail < want) return assigned;
				if (!suppress) {
					char *dst = va_arg(ap, char *);
					memcpy(dst, s, (size_t)want);
				}
				s += want;
				if (!suppress) assigned++;
				break;
			}
			case 'p': {
				char *endp;
				unsigned long long val = strtoull(s, &endp, 16);
				if (endp == s) return assigned;
				s = endp;
				if (!suppress) { *va_arg(ap, void **) = (void *)(uintptr_t)val; assigned++; }
				break;
			}
			default:
				return assigned;
		}
	}

	return assigned;
}

static inline int sscanf_compat(const char *str, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int r = vsscanf_compat(str, fmt, ap);
	va_end(ap);
	return r;
}

#ifdef __cplusplus
}
#endif

#ifdef _WIN32
#define sscanf sscanf_compat
#endif