#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static char* get_int(va_list *ap) {
	int d = va_arg(*ap, int);
	char* str= '\0';
	if (d == 0) {
		*str = '0';
		return str;
	}
	if (d < 0) {
		*str = '-';
		str++;
		d *= -1;
	}
	char *t;
	while (d) {
		*t = (char)(d % 10 + '0');
		t++;
		d /= 10;
	}
	while (t) {
		*str = *t;
		str++;
		t--;
	}
	return str;
}

static int make_out(char *out, const char *fmt, va_list ap) {
	while (*fmt) {
		if (*fmt == '%') {
			fmt++;
			switch (*fmt) {
				case 'd': strcat(out, get_int(&ap));break;
				case 's': strcat(out, va_arg(ap, char*));break;
			}
		}
		else {
			fmt++;
		}
	}
	*out++ = '\0';
	return 0;
}

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	make_out(out, fmt, ap);
	va_end(ap);
	return 0;
  //panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
