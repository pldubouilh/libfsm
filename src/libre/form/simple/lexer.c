/* Generated by lx */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>

#include LX_HEADER

static enum lx_token z1(struct lx *lx);
static enum lx_token z2(struct lx *lx);
static enum lx_token z3(struct lx *lx);

static int
lx_getc(struct lx *lx)
{
	int c;

	assert(lx != NULL);
	assert(lx->lgetc != NULL);

	if (lx->c != EOF) {
		c = lx->c, lx->c = EOF;
	} else {
		c = lx->lgetc(lx);
		if (c == EOF) {
			return EOF;
		}
	}

	lx->end.byte++;
	lx->end.col++;

	if (c == '\n') {
		lx->end.line++;
		lx->end.col = 1;
	}

	return c;
}

static void
lx_simple_ungetc(struct lx *lx, int c)
{
	assert(lx != NULL);
	assert(lx->c == EOF);

	lx->c = c;

	if (lx->pop != NULL) {
		lx->pop(lx);
	}

	lx->end.byte--;
	lx->end.col--;

	if (c == '\n') {
		lx->end.line--;
		lx->end.col = 0; /* XXX: lost information */
	}
}

int
lx_simple_fgetc(struct lx *lx)
{
	assert(lx != NULL);
	assert(lx->opaque != NULL);

	return fgetc(lx->opaque);
}

int
lx_simple_sgetc(struct lx *lx)
{
	char *s;

	assert(lx != NULL);
	assert(lx->opaque != NULL);

	s = lx->opaque;
	if (*s == '\0') {
		return EOF;
	}

	return lx->opaque = s + 1, *s;
}

int
lx_simple_agetc(struct lx *lx)
{
	struct lx_arr *a;

	assert(lx != NULL);
	assert(lx->opaque != NULL);

	a = lx->opaque;

	assert(a != NULL);
	assert(a->p != NULL);

	if (a->len == 0) {
		return EOF;
	}

	return a->len--, *a->p++;
}

int
lx_simple_dgetc(struct lx *lx)
{
	struct lx_fd *d;

	assert(lx != NULL);
	assert(lx->opaque != NULL);

	d = lx->opaque;
	assert(d->fd != -1);
	assert(d->p != NULL);

	if (d->len == 0) {
		ssize_t r;

		assert((fcntl(d->fd, F_GETFL) & O_NONBLOCK) == 0);

		d->p = (char *) d + sizeof *d;

		r = read(d->fd, d->p, d->bufsz);
		if (r == -1) {
			assert(errno != EAGAIN);
			return EOF;
		}

		if (r == 0) {
			return EOF;
		}

		d->len = r;
	}

	return d->len--, *d->p++;
}

int
lx_simple_dynpush(struct lx *lx, char c)
{
	struct lx_dynbuf *t;

	assert(lx != NULL);
	assert(c != EOF);

	t = lx->buf;

	assert(t != NULL);

	if (t->p == t->a + t->len) {
		size_t len;
		char *tmp;

		if (t->len == 0) {
			assert(LX_DYN_LOW > 0);
			len = LX_DYN_LOW;
		} else {
			len = t->len * LX_DYN_FACTOR;
			if (len < t->len) {
				errno = ERANGE;
				return -1;
			}
		}

		tmp = realloc(t->a, len);
		if (tmp == NULL) {
			return -1;
		}

		t->p   = tmp + (t->p - t->a);
		t->a   = tmp;
		t->len = len;
	}

	assert(t->p != NULL);
	assert(t->a != NULL);

	*t->p++ = c;

	return 0;
}

void
lx_simple_dynpop(struct lx *lx)
{
	struct lx_dynbuf *t;

	assert(lx != NULL);

	t = lx->buf;

	assert(t != NULL);
	assert(t->a != NULL);
	assert(t->p >= t->a);

	if (t->p == t->a) {
		return;
	}

	t->p--;
}

int
lx_simple_dynclear(struct lx *lx)
{
	struct lx_dynbuf *t;

	assert(lx != NULL);

	t = lx->buf;

	assert(t != NULL);

	if (t->len > LX_DYN_HIGH) {
		size_t len;
		char *tmp;

		len = t->len / LX_DYN_FACTOR;

		tmp = realloc(t->a, len);
		if (tmp == NULL) {
			return -1;
		}

		t->a   = tmp;
		t->len = len;
	}

	t->p = t->a;

	return 0;
}

void
lx_simple_dynfree(struct lx *lx)
{
	struct lx_dynbuf *t;

	assert(lx != NULL);

	t = lx->buf;

	assert(t != NULL);

	free(t->a);
}
int
lx_simple_fixedpush(struct lx *lx, char c)
{
	struct lx_fixedbuf *t;

	assert(lx != NULL);
	assert(c != EOF);

	t = lx->buf;

	assert(t != NULL);
	assert(t->p != NULL);
	assert(t->a != NULL);

	if (t->p == t->a + t->len) {
		errno = ENOMEM;
		return -1;
	}

	*t->p++ = c;

	return 0;
}

void
lx_simple_fixedpop(struct lx *lx)
{
	struct lx_fixedbuf *t;

	assert(lx != NULL);

	t = lx->buf;

	assert(t != NULL);
	assert(t->a != NULL);
	assert(t->p >= t->a);

	if (t->p == t->a) {
		return;
	}

	t->p--;
}

int
lx_simple_fixedclear(struct lx *lx)
{
	struct lx_fixedbuf *t;

	assert(lx != NULL);

	t = lx->buf;

	assert(t != NULL);
	assert(t->p != NULL);
	assert(t->a != NULL);

	t->p = t->a;

	return 0;
}

static enum lx_token
z1(struct lx *lx)
{
	int c;

	enum {
		S1, S2, S3, S4
	} state;

	assert(lx != NULL);

	if (lx->clear != NULL) {
		lx->clear(lx);
	}

	state = S4;

	lx->start = lx->end;

	while (c = lx_getc(lx), c != EOF) {
		switch (state) {
			break;

		default:
			if (lx->push != NULL) {
				if (-1 == lx->push(lx, c)) {
					return TOK_ERROR;
				}
			}
			break;

		}

		switch (state) {
		case S1:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return TOK_SEP;
			}

		case S2:
			switch (c) {
			case '0':	          continue;
			case '1':	          continue;
			case '2':	          continue;
			case '3':	          continue;
			case '4':	          continue;
			case '5':	          continue;
			case '6':	          continue;
			case '7':	          continue;
			case '8':	          continue;
			case '9':	          continue;
			default:  lx_simple_ungetc(lx, c); return TOK_COUNT;
			}

		case S3:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return lx->z = z3, TOK_CLOSECOUNT;
			}

		case S4:
			switch (c) {
			case ',': state = S1;      continue;
			case '0': state = S2;      continue;
			case '1': state = S2;      continue;
			case '2': state = S2;      continue;
			case '3': state = S2;      continue;
			case '4': state = S2;      continue;
			case '5': state = S2;      continue;
			case '6': state = S2;      continue;
			case '7': state = S2;      continue;
			case '8': state = S2;      continue;
			case '9': state = S2;      continue;
			case '}': state = S3;      continue;
			default:  lx->lgetc = NULL; return TOK_UNKNOWN;
			}
		}
	}

	lx->lgetc = NULL;

	switch (state) {
	case S1: return TOK_SEP;
	case S2: return TOK_COUNT;
	case S3: return TOK_CLOSECOUNT;
	default: errno = EINVAL; return TOK_ERROR;
	}
}

static enum lx_token
z2(struct lx *lx)
{
	int c;

	enum {
		S1, S2, S3, S4, S5, S6, S7
	} state;

	assert(lx != NULL);

	if (lx->clear != NULL) {
		lx->clear(lx);
	}

	state = S7;

	lx->start = lx->end;

	while (c = lx_getc(lx), c != EOF) {
		switch (state) {
			break;

		default:
			if (lx->push != NULL) {
				if (-1 == lx->push(lx, c)) {
					return TOK_ERROR;
				}
			}
			break;

		}

		switch (state) {
		case S1:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return TOK_ESC;
			}

		case S2:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return TOK_RANGE;
			}

		case S3:
			switch (c) {
			case '\\': state = S1;      continue;
			case 'f': state = S1;      continue;
			case 'n': state = S1;      continue;
			case 'r': state = S1;      continue;
			case 't': state = S1;      continue;
			case 'v': state = S1;      continue;
			default:  lx_simple_ungetc(lx, c); return TOK_CHAR;
			}

		case S4:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return lx->z = z3, TOK_CLOSEGROUP;
			}

		case S5:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return TOK_INVERT;
			}

		case S6:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return TOK_CHAR;
			}

		case S7:
			switch (c) {
			case '-': state = S2;      continue;
			case '\\': state = S3;      continue;
			case ']': state = S4;      continue;
			case '^': state = S5;      continue;
			default:  state = S6;     continue;
			}
		}
	}

	lx->lgetc = NULL;

	switch (state) {
	case S1: return TOK_ESC;
	case S2: return TOK_RANGE;
	case S3: return TOK_CHAR;
	case S4: return TOK_CLOSEGROUP;
	case S5: return TOK_INVERT;
	case S6: return TOK_CHAR;
	default: errno = EINVAL; return TOK_ERROR;
	}
}

static enum lx_token
z3(struct lx *lx)
{
	int c;

	enum {
		S1, S2, S3, S4, S5, S6, S7, S8, S9, S10, 
		S11, S12, S13, S14, S15
	} state;

	assert(lx != NULL);

	if (lx->clear != NULL) {
		lx->clear(lx);
	}

	state = S15;

	lx->start = lx->end;

	while (c = lx_getc(lx), c != EOF) {
		switch (state) {
			break;

		default:
			if (lx->push != NULL) {
				if (-1 == lx->push(lx, c)) {
					return TOK_ERROR;
				}
			}
			break;

		}

		switch (state) {
		case S1:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return TOK_ESC;
			}

		case S2:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return TOK_EOL;
			}

		case S3:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return TOK_OPENSUB;
			}

		case S4:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return TOK_CLOSESUB;
			}

		case S5:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return TOK_STAR;
			}

		case S6:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return TOK_PLUS;
			}

		case S7:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return TOK_DOT;
			}

		case S8:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return TOK_QMARK;
			}

		case S9:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return lx->z = z2, TOK_OPENGROUP;
			}

		case S10:
			switch (c) {
			case '\\': state = S1;      continue;
			case 'f': state = S1;      continue;
			case 'n': state = S1;      continue;
			case 'r': state = S1;      continue;
			case 't': state = S1;      continue;
			case 'v': state = S1;      continue;
			default:  lx->lgetc = NULL; return TOK_UNKNOWN;
			}

		case S11:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return TOK_SOL;
			}

		case S12:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return lx->z = z1, TOK_OPENCOUNT;
			}

		case S13:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return TOK_ALT;
			}

		case S14:
			switch (c) {
			default:  lx_simple_ungetc(lx, c); return TOK_CHAR;
			}

		case S15:
			switch (c) {
			case '\x0': state = S14;      continue;
			case '\x1': state = S14;      continue;
			case '\x2': state = S14;      continue;
			case '\x3': state = S14;      continue;
			case '\x4': state = S14;      continue;
			case '\x5': state = S14;      continue;
			case '\x6': state = S14;      continue;
			case '\x7': state = S14;      continue;
			case '\x8': state = S14;      continue;
			case '\t': state = S14;      continue;
			case '\n': state = S14;      continue;
			case '\v': state = S14;      continue;
			case '\f': state = S14;      continue;
			case '\r': state = S14;      continue;
			case '\xe': state = S14;      continue;
			case '\xf': state = S14;      continue;
			case '\x10': state = S14;      continue;
			case '\x11': state = S14;      continue;
			case '\x12': state = S14;      continue;
			case '\x13': state = S14;      continue;
			case '\x14': state = S14;      continue;
			case '\x15': state = S14;      continue;
			case '\x16': state = S14;      continue;
			case '\x17': state = S14;      continue;
			case '\x18': state = S14;      continue;
			case '\x19': state = S14;      continue;
			case '\x1a': state = S14;      continue;
			case '\x1b': state = S14;      continue;
			case '\x1c': state = S14;      continue;
			case '\x1d': state = S14;      continue;
			case '\x1e': state = S14;      continue;
			case '\x1f': state = S14;      continue;
			case ' ': state = S14;      continue;
			case '!': state = S14;      continue;
			case '\"': state = S14;      continue;
			case '#': state = S14;      continue;
			case '$': state = S2;      continue;
			case '%': state = S14;      continue;
			case '&': state = S14;      continue;
			case '\'': state = S14;      continue;
			case '(': state = S3;      continue;
			case ')': state = S4;      continue;
			case '*': state = S5;      continue;
			case '+': state = S6;      continue;
			case ',': state = S14;      continue;
			case '-': state = S14;      continue;
			case '.': state = S7;      continue;
			case '/': state = S14;      continue;
			case '0': state = S14;      continue;
			case '1': state = S14;      continue;
			case '2': state = S14;      continue;
			case '3': state = S14;      continue;
			case '4': state = S14;      continue;
			case '5': state = S14;      continue;
			case '6': state = S14;      continue;
			case '7': state = S14;      continue;
			case '8': state = S14;      continue;
			case '9': state = S14;      continue;
			case ':': state = S14;      continue;
			case ';': state = S14;      continue;
			case '<': state = S14;      continue;
			case '=': state = S14;      continue;
			case '>': state = S14;      continue;
			case '?': state = S8;      continue;
			case '@': state = S14;      continue;
			case 'A': state = S14;      continue;
			case 'B': state = S14;      continue;
			case 'C': state = S14;      continue;
			case 'D': state = S14;      continue;
			case 'E': state = S14;      continue;
			case 'F': state = S14;      continue;
			case 'G': state = S14;      continue;
			case 'H': state = S14;      continue;
			case 'I': state = S14;      continue;
			case 'J': state = S14;      continue;
			case 'K': state = S14;      continue;
			case 'L': state = S14;      continue;
			case 'M': state = S14;      continue;
			case 'N': state = S14;      continue;
			case 'O': state = S14;      continue;
			case 'P': state = S14;      continue;
			case 'Q': state = S14;      continue;
			case 'R': state = S14;      continue;
			case 'S': state = S14;      continue;
			case 'T': state = S14;      continue;
			case 'U': state = S14;      continue;
			case 'V': state = S14;      continue;
			case 'W': state = S14;      continue;
			case 'X': state = S14;      continue;
			case 'Y': state = S14;      continue;
			case 'Z': state = S14;      continue;
			case '[': state = S9;      continue;
			case '\\': state = S10;      continue;
			case '^': state = S11;      continue;
			case '_': state = S14;      continue;
			case '`': state = S14;      continue;
			case 'a': state = S14;      continue;
			case 'b': state = S14;      continue;
			case 'c': state = S14;      continue;
			case 'd': state = S14;      continue;
			case 'e': state = S14;      continue;
			case 'f': state = S14;      continue;
			case 'g': state = S14;      continue;
			case 'h': state = S14;      continue;
			case 'i': state = S14;      continue;
			case 'j': state = S14;      continue;
			case 'k': state = S14;      continue;
			case 'l': state = S14;      continue;
			case 'm': state = S14;      continue;
			case 'n': state = S14;      continue;
			case 'o': state = S14;      continue;
			case 'p': state = S14;      continue;
			case 'q': state = S14;      continue;
			case 'r': state = S14;      continue;
			case 's': state = S14;      continue;
			case 't': state = S14;      continue;
			case 'u': state = S14;      continue;
			case 'v': state = S14;      continue;
			case 'w': state = S14;      continue;
			case 'x': state = S14;      continue;
			case 'y': state = S14;      continue;
			case 'z': state = S14;      continue;
			case '{': state = S12;      continue;
			case '|': state = S13;      continue;
			case '~': state = S14;      continue;
			case '\x7f': state = S14;      continue;
			case '\x80': state = S14;      continue;
			case '\x81': state = S14;      continue;
			case '\x82': state = S14;      continue;
			case '\x83': state = S14;      continue;
			case '\x84': state = S14;      continue;
			case '\x85': state = S14;      continue;
			case '\x86': state = S14;      continue;
			case '\x87': state = S14;      continue;
			case '\x88': state = S14;      continue;
			case '\x89': state = S14;      continue;
			case '\x8a': state = S14;      continue;
			case '\x8b': state = S14;      continue;
			case '\x8c': state = S14;      continue;
			case '\x8d': state = S14;      continue;
			case '\x8e': state = S14;      continue;
			case '\x8f': state = S14;      continue;
			case '\x90': state = S14;      continue;
			case '\x91': state = S14;      continue;
			case '\x92': state = S14;      continue;
			case '\x93': state = S14;      continue;
			case '\x94': state = S14;      continue;
			case '\x95': state = S14;      continue;
			case '\x96': state = S14;      continue;
			case '\x97': state = S14;      continue;
			case '\x98': state = S14;      continue;
			case '\x99': state = S14;      continue;
			case '\x9a': state = S14;      continue;
			case '\x9b': state = S14;      continue;
			case '\x9c': state = S14;      continue;
			case '\x9d': state = S14;      continue;
			case '\x9e': state = S14;      continue;
			case '\x9f': state = S14;      continue;
			case '\xa0': state = S14;      continue;
			case '\xa1': state = S14;      continue;
			case '\xa2': state = S14;      continue;
			case '\xa3': state = S14;      continue;
			case '\xa4': state = S14;      continue;
			case '\xa5': state = S14;      continue;
			case '\xa6': state = S14;      continue;
			case '\xa7': state = S14;      continue;
			case '\xa8': state = S14;      continue;
			case '\xa9': state = S14;      continue;
			case '\xaa': state = S14;      continue;
			case '\xab': state = S14;      continue;
			case '\xac': state = S14;      continue;
			case '\xad': state = S14;      continue;
			case '\xae': state = S14;      continue;
			case '\xaf': state = S14;      continue;
			case '\xb0': state = S14;      continue;
			case '\xb1': state = S14;      continue;
			case '\xb2': state = S14;      continue;
			case '\xb3': state = S14;      continue;
			case '\xb4': state = S14;      continue;
			case '\xb5': state = S14;      continue;
			case '\xb6': state = S14;      continue;
			case '\xb7': state = S14;      continue;
			case '\xb8': state = S14;      continue;
			case '\xb9': state = S14;      continue;
			case '\xba': state = S14;      continue;
			case '\xbb': state = S14;      continue;
			case '\xbc': state = S14;      continue;
			case '\xbd': state = S14;      continue;
			case '\xbe': state = S14;      continue;
			case '\xbf': state = S14;      continue;
			case '\xc0': state = S14;      continue;
			case '\xc1': state = S14;      continue;
			case '\xc2': state = S14;      continue;
			case '\xc3': state = S14;      continue;
			case '\xc4': state = S14;      continue;
			case '\xc5': state = S14;      continue;
			case '\xc6': state = S14;      continue;
			case '\xc7': state = S14;      continue;
			case '\xc8': state = S14;      continue;
			case '\xc9': state = S14;      continue;
			case '\xca': state = S14;      continue;
			case '\xcb': state = S14;      continue;
			case '\xcc': state = S14;      continue;
			case '\xcd': state = S14;      continue;
			case '\xce': state = S14;      continue;
			case '\xcf': state = S14;      continue;
			case '\xd0': state = S14;      continue;
			case '\xd1': state = S14;      continue;
			case '\xd2': state = S14;      continue;
			case '\xd3': state = S14;      continue;
			case '\xd4': state = S14;      continue;
			case '\xd5': state = S14;      continue;
			case '\xd6': state = S14;      continue;
			case '\xd7': state = S14;      continue;
			case '\xd8': state = S14;      continue;
			case '\xd9': state = S14;      continue;
			case '\xda': state = S14;      continue;
			case '\xdb': state = S14;      continue;
			case '\xdc': state = S14;      continue;
			case '\xdd': state = S14;      continue;
			case '\xde': state = S14;      continue;
			case '\xdf': state = S14;      continue;
			case '\xe0': state = S14;      continue;
			case '\xe1': state = S14;      continue;
			case '\xe2': state = S14;      continue;
			case '\xe3': state = S14;      continue;
			case '\xe4': state = S14;      continue;
			case '\xe5': state = S14;      continue;
			case '\xe6': state = S14;      continue;
			case '\xe7': state = S14;      continue;
			case '\xe8': state = S14;      continue;
			case '\xe9': state = S14;      continue;
			case '\xea': state = S14;      continue;
			case '\xeb': state = S14;      continue;
			case '\xec': state = S14;      continue;
			case '\xed': state = S14;      continue;
			case '\xee': state = S14;      continue;
			case '\xef': state = S14;      continue;
			case '\xf0': state = S14;      continue;
			case '\xf1': state = S14;      continue;
			case '\xf2': state = S14;      continue;
			case '\xf3': state = S14;      continue;
			case '\xf4': state = S14;      continue;
			case '\xf5': state = S14;      continue;
			case '\xf6': state = S14;      continue;
			case '\xf7': state = S14;      continue;
			case '\xf8': state = S14;      continue;
			case '\xf9': state = S14;      continue;
			case '\xfa': state = S14;      continue;
			case '\xfb': state = S14;      continue;
			case '\xfc': state = S14;      continue;
			case '\xfd': state = S14;      continue;
			case '\xfe': state = S14;      continue;
			case '\xff': state = S14;      continue;
			default:  lx->lgetc = NULL; return TOK_UNKNOWN;
			}
		}
	}

	lx->lgetc = NULL;

	switch (state) {
	case S1: return TOK_ESC;
	case S2: return TOK_EOL;
	case S3: return TOK_OPENSUB;
	case S4: return TOK_CLOSESUB;
	case S5: return TOK_STAR;
	case S6: return TOK_PLUS;
	case S7: return TOK_DOT;
	case S8: return TOK_QMARK;
	case S9: return TOK_OPENGROUP;
	case S11: return TOK_SOL;
	case S12: return TOK_OPENCOUNT;
	case S13: return TOK_ALT;
	case S14: return TOK_CHAR;
	default: errno = EINVAL; return TOK_ERROR;
	}
}

const char *
lx_simple_name(enum lx_token t)
{
	switch (t) {
	case TOK_CLOSECOUNT: return "CLOSECOUNT";
	case TOK_OPENCOUNT: return "OPENCOUNT";
	case TOK_COUNT: return "COUNT";
	case TOK_SEP: return "SEP";
	case TOK_CLOSEGROUP: return "CLOSEGROUP";
	case TOK_OPENGROUP: return "OPENGROUP";
	case TOK_RANGE: return "RANGE";
	case TOK_INVERT: return "INVERT";
	case TOK_CHAR: return "CHAR";
	case TOK_ESC: return "ESC";
	case TOK_CLOSESUB: return "CLOSESUB";
	case TOK_OPENSUB: return "OPENSUB";
	case TOK_ALT: return "ALT";
	case TOK_DOT: return "DOT";
	case TOK_PLUS: return "PLUS";
	case TOK_STAR: return "STAR";
	case TOK_QMARK: return "QMARK";
	case TOK_EOL: return "EOL";
	case TOK_SOL: return "SOL";
	case TOK_EOF:     return "EOF";
	case TOK_ERROR:   return "ERROR";
	case TOK_UNKNOWN: return "UNKNOWN";
	default: return "?";
	}
}

void
lx_simple_init(struct lx *lx)
{
	static const struct lx lx_default;

	assert(lx != NULL);

	*lx = lx_default;

	lx->c = EOF;
	lx->z = NULL;

	lx->end.byte = 0;
	lx->end.line = 1;
	lx->end.col  = 1;
}

enum lx_token
lx_simple_next(struct lx *lx)
{
	enum lx_token t;

	assert(lx != NULL);

	if (lx->lgetc == NULL) {
		return TOK_EOF;
	}

	if (lx->z == NULL) {
		lx->z = z3;
	}

	t = lx->z(lx);

	if (lx->push != NULL) {
		if (-1 == lx->push(lx, '\0')) {
			return TOK_ERROR;
		}
	}

	if (lx->lgetc == NULL && lx->free != NULL) {
		lx->free(lx);
	}

	return t;
}

