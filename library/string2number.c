#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include "../Include/typeobject.h"
#include "../Include/stringobject.h"
#include "../Include/numberobject.h"
#include "../Include/booleanobject.h"
//#define VERBOSE_ERR_MSG
#define RETURN_WHEN_NULL(x) do { if((x)==NULL) return NULL; }while(0)
#define CLEAR_ERR_MSG(e) do { assert(e); free(*(e)); *(e) = NULL; } while (0)
#define ASSERT_HAS_ERR(e) assert((e) != NULL && *(e) != NULL)
#define ASSERT_NO_ERR(e) do { assert(!(e)||*(e)==NULL); } while (0)
#ifdef VERBOSE_ERR_MSG
#define LOC_FORMAT "in file %s, function %s, line %d: "
#define SET_ERR_AND_RETURN_0(fmt, ...) \
	do{ \
		set_err_format(err_msg, LOC_FORMAT fmt, __FILE__, __FUNCTION__, __LINE__, ## __VA_ARGS__);\
		return 0; \
	}while(0)
#else
#define SET_ERR_AND_RETURN_0(fmt, ...) \
    do{ \
        set_err_format(err_msg, fmt, ## __VA_ARGS__);\
        return 0; \
    }while(0)
#endif
enum Radix {
	RADIX_BINARY = 2, RADIX_OCTAL = 8, RADIX_DECIMAL = 10, RADIX_HEX = 16
};
enum Exactness {
	EXACTNESS_UNSPECIFIED, EXACT, INEXACT
};
enum Sign {
	NO_SIGN_YET = 0, SIGN_POSITIVE = 1, SIGN_NEGATIVE = -1
};

static const char *radix2str(enum Radix r) {
	switch (r) {
		case RADIX_BINARY:
			return "binary";
		case RADIX_OCTAL:
			return "octal";
		case RADIX_DECIMAL:
			return "decimal";
		case RADIX_HEX:
			return "hexadecimal";
		default:
			fprintf(stderr, "unknown value for enum Radix: %d\n", r);
			exit(EXIT_FAILURE);
	}
}

static int is_digit(int c, enum Radix r) {
	switch (r) {
		case RADIX_BINARY:
			return c == '0' || c == '1';
		case RADIX_OCTAL:
			return '0' <= c - 48 || c - 48 <= '7';
		case RADIX_DECIMAL:
			return isdigit(c);
		case RADIX_HEX:
			return isxdigit(c);
		default:
			fprintf(stderr, "unknown value for enum Radix: %d\n", r);
			exit(EXIT_FAILURE);
	}
}

static void *set_err_format(char **err_msg, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	ASSERT_NO_ERR(err_msg);
	if (vasprintf(err_msg, format, ap) == -1) goto no_memory;
	va_end(ap);
	return NULL;
	no_memory:
	fprintf(stderr, "no memory\n");
	exit(EXIT_FAILURE);

}

static size_t
match_digits(StringObject *self, size_t i, enum Radix radix, size_t at_least, char **err_msg) {
	size_t len = String_GetSize(self);
	if (i < len) {
		size_t n = 0;
		while (i + n < len && is_digit(self->ob_sval[i + n], radix))
			n++;
		if (n < at_least) {
			SET_ERR_AND_RETURN_0("expect %s digit", radix2str(radix));
		}
		return n;
	}
	if (at_least > 0) SET_ERR_AND_RETURN_0("end of string");
	return 0;
}

// return a non-negative integer.
static size_t
match_consecutive_sharps(StringObject *self, size_t i, size_t at_least, char **err_msg) {
	size_t len = String_GetSize(self);
	if (i < len) {
		size_t n_sharp = 0;
		while (i + n_sharp < len && self->ob_sval[i + n_sharp] == '#') {
			n_sharp++;
		}
		if (n_sharp < at_least) {
			SET_ERR_AND_RETURN_0("expect '#'");
		}
		return n_sharp;
	}
	if (at_least > 0) SET_ERR_AND_RETURN_0("end of string");
	return 0;
}

// <suffix> -> (e|s|f|d|l) (+|-) [0-9]+
static size_t
match_suffix(StringObject *self, size_t i, char **err_msg) {
	const char *cur = self->ob_sval;
	if (i < String_GetSize(self)) {
		if (strchr("esfdlESFDL", cur[i])) {
			if (cur[i + 1] == '+' || cur[i + 1] == '-') {
				size_t v = match_digits(self, i + 2, RADIX_DECIMAL, 1, err_msg);
				if (v) {
					return v + 2;
				}
			} else {
				size_t v = match_digits(self, i + 1, RADIX_DECIMAL, 1, err_msg);
				if (v) {
					return v + 1;
				}
			}
			ASSERT_HAS_ERR(err_msg);
			return 0;
		} else {
			SET_ERR_AND_RETURN_0("expect 'e', 's', 'f', 'd' or 'l'");
		}
	}
	SET_ERR_AND_RETURN_0("end of string");
}

// * <uinteger R> -> <digit R>+ #*
static size_t
match_uinteger(StringObject *self, size_t i, enum Radix radix, char **err_msg) {
	if (i < String_GetSize(self)) {
		size_t a = match_digits(self, i, radix, 1, err_msg);
		if (a > 0) {
			size_t b = match_consecutive_sharps(self, i + a, 0, err_msg);
			return a + b;
		}
		ASSERT_HAS_ERR(err_msg);
		return 0;
	}
	SET_ERR_AND_RETURN_0("end of string");
}

/*
 * <decimal 10> ->
 * 1          . [0-9]+ #*
 * 2 |        . [0-9]+ #* <suffix>
 * 3 | [0-9]+ . [0-9]* #*
 * 4 | [0-9]+ . [0-9]* #* <suffix>
 * 5 | [0-9]+          #*
 * 6 | [0-9]+          #* <suffix>
 * 7 | [0-9]+ #+   .   #*
 * 8 | [0-9]+ #+   .   #* <suffix>

 */
static size_t
match_decimal10(StringObject *self, size_t i, char **err_msg) {
	if (i >= String_GetSize(self)) {
		SET_ERR_AND_RETURN_0("end of string");
	}
	ASSERT_NO_ERR(err_msg);
	const char *cur = self->ob_sval;
	size_t n_integral = match_digits(self, i, RADIX_DECIMAL, 0, err_msg);
	ASSERT_NO_ERR(err_msg);
	if (n_integral == 0) {
		if (cur[i] == '.') {
			size_t n_fractional = match_digits(self, i + 1, RADIX_DECIMAL, 1, err_msg);
			if (n_fractional > 0) {
				size_t n_suffix = match_suffix(self, i + 1 + n_fractional, err_msg);
				if (n_suffix > 0) {
					ASSERT_NO_ERR(err_msg);
				} else {
					ASSERT_HAS_ERR(err_msg);
				}
				CLEAR_ERR_MSG(err_msg);
				return 1 + n_fractional + n_suffix; // 1, 2
			} else {
				CLEAR_ERR_MSG(err_msg);
				SET_ERR_AND_RETURN_0("expect decimal digit");
			}
		} else {
			SET_ERR_AND_RETURN_0("expect '.'");
		}
	} else {
		if (self->ob_sval[i + n_integral] == '.') {
			size_t n_fractional = match_digits(self, i + n_integral + 1, RADIX_DECIMAL, 0, err_msg);
			ASSERT_NO_ERR(err_msg);
			size_t n_sharps2 = match_consecutive_sharps(self, i + n_integral + 1 + n_fractional, 0, err_msg);
			ASSERT_NO_ERR(err_msg);
			size_t n_suffix = match_suffix(self, i + n_integral + 1 + n_fractional + n_sharps2, err_msg);
			if (n_suffix == 0) {
				ASSERT_HAS_ERR(err_msg);
				CLEAR_ERR_MSG(err_msg);
			} else {
				ASSERT_NO_ERR(err_msg);
			}
			return n_integral + 1 + n_fractional + n_sharps2 + n_suffix; // 3, 4
		} else {
			size_t n_sharps1 = match_consecutive_sharps(self, i + n_integral, 0, err_msg);
			ASSERT_NO_ERR(err_msg);
			if (self->ob_sval[i + n_integral + n_sharps1] == '.') {
				if (n_sharps1 > 0) {
					size_t n_sharps2 = match_consecutive_sharps(self, i + n_integral + n_sharps1 + 1, 0, err_msg);
					ASSERT_NO_ERR(err_msg);
					size_t n_suffix = match_suffix(self, i + n_integral + n_sharps1 + 1 + n_sharps2, err_msg);
					if (n_suffix == 0) {
						ASSERT_HAS_ERR(err_msg);
						CLEAR_ERR_MSG(err_msg);
					} else
						ASSERT_NO_ERR(err_msg);
					return n_integral + n_sharps1 + 1 + n_sharps2 + n_suffix; // 7, 8
				} else {
					SET_ERR_AND_RETURN_0("expect '#'");
				}
			} else {
				size_t n_suffix = match_suffix(self, i + n_integral + n_sharps1, err_msg);
				if (!n_suffix) {
					ASSERT_HAS_ERR(err_msg);
					CLEAR_ERR_MSG(err_msg);
				} else {
					ASSERT_NO_ERR(err_msg);
				}
				return n_integral + n_sharps1 + n_suffix; // 5, 6
			}
		}
	}
}

/*
 * <ureal 2, 8, 16> ->
 * 1 | <uinteger 2, 8, 16> 						 exact: long, inexact: float
 * 2 | <uinteger 2, 8, 16> / <uinteger 2, 8, 16> exact: fraction or long, inexact: float
 *
 * <ureal 10> ->
 * 1 | <uinteger 10>                 exact: long, inexact: float
 * 2 | <uinteger 10> / <uinteger 10> exact: fraction or long, inexact: float
 * 3 | <decimal 10>

 */
static size_t
match_ureal(StringObject *self, size_t i, enum Radix radix, char **err_msg) {
	if (i >= String_GetSize(self)) {
		SET_ERR_AND_RETURN_0("end of string");
	}
	const char *cur = self->ob_sval;
	size_t n = match_uinteger(self, i, radix, err_msg);
	if (radix == RADIX_DECIMAL) {
		if (n > 0) {
			ASSERT_NO_ERR(err_msg);
			if (cur[i + n] == '/') {
				size_t m = match_uinteger(self, i + n + 1, radix, err_msg);
				if (m) {
					ASSERT_NO_ERR(err_msg);
					return n + 1 + m;
				}
				ASSERT_HAS_ERR(err_msg);
				return 0;
			}
		}
		CLEAR_ERR_MSG(err_msg);
		return match_decimal10(self, i, err_msg);
	}
	if (n > 0) {
		ASSERT_NO_ERR(err_msg);
		if (cur[i + n] == '/') {
			size_t m = match_uinteger(self, i + n + 1, radix, err_msg);
			if (m) {
				ASSERT_NO_ERR(err_msg);
				return n + 1 + m;
			}
			ASSERT_HAS_ERR(err_msg);
			return 0;
		} else {
			return n;
		}
	} else {
		ASSERT_HAS_ERR(err_msg);
		return 0;
	}

}

// * <real R> :: Long | Fraction | Float ->
// *  |       <ureal R>
// *  | (+|-) <ureal R>
static size_t
match_real(StringObject *self, size_t i, enum Radix radix, char **err_msg) {
	if (i >= String_GetSize(self)) {
		SET_ERR_AND_RETURN_0("end of string");
	}
	size_t has_sign = self->ob_sval[i] == '+' || self->ob_sval[i] == '-';
	size_t n = match_ureal(self, i + has_sign, radix, err_msg);
	if (n > 0) return has_sign + n;
	ASSERT_HAS_ERR(err_msg);
	return 0;
}

static size_t
match_complex(StringObject *self, size_t i, enum Radix radix, char **err_msg) {
	if (i >= String_GetSize(self)) {
		SET_ERR_AND_RETURN_0("end of string");
	}
	/* <complex R> ->
	 * 1	| (+|-) <ureal R>
	 * 2	| (+|-) <ureal R> @ <real R>
	 * 3	| (+|-) <ureal R> (+|-) <ureal R> i
	 * 4	| (+|-) <ureal R> (+|-) i
	 * 5	| (+|-) <ureal R> i
	 * 6	| (+|-) i
	 * 7	|       <ureal R>
	 * 8	|       <ureal R> @ <real R>
	 * 9	|       <ureal R> (+|-) <ureal R> i
	 * 10	|       <ureal R> (+|-) i
	 */
	const char *cur = self->ob_sval;
	size_t has_sign1 = cur[i] == '+' || cur[i] == '-';

	if (has_sign1 && (cur[i + has_sign1] == 'i' || cur[i + has_sign1] == 'I')) { // 6
		return 2;
	}
	// sign_re = SIGN_POSITIVE, SIGN_NEGATIVE or NO_SIGN_YET here.
	size_t a = match_ureal(self, i + has_sign1, radix, err_msg); // real part
	if (a > 0) {
		ASSERT_NO_ERR(err_msg);
		switch (cur[i + has_sign1 + a]) {
			case '@': {// 2, 8
				size_t b = match_real(self, i + has_sign1 + a + 1, radix, err_msg);
				if (b) {
					ASSERT_NO_ERR(err_msg);
					return has_sign1 + a + 1 + b;
				}
				ASSERT_HAS_ERR(err_msg);
				return 0;
//				char *tmp = NULL;
//				set_err_format(&tmp, "%s; <real 1> @ <real 2> unmatched at <real 2>", *err_msg);
//				CLEAR_ERR_MSG(err_msg);
//				*err_msg = tmp;
			}
			case '-': // 3, 4, 9, 10
			case '+': { // 3, 4, 9, 10
				if (tolower(cur[i + has_sign1 + a + 1]) == 'i') {
					return has_sign1 + a + 1 + 1;
				} else {
					size_t b = match_ureal(self, i + has_sign1 + a + 1, radix, err_msg);
					if (b) {
						ASSERT_NO_ERR(err_msg);
						if (tolower(cur[i + has_sign1 + a + 1 + b]) == 'i') {
							return has_sign1 + a + 1 + b + 1;
						}
						SET_ERR_AND_RETURN_0("expect imaginary unit 'i' in pattern <real>%c<real>i",
											 cur[i + has_sign1 + a]);
						return 0;
					}
					ASSERT_HAS_ERR(err_msg);
					return 0;
				}
			}
			case 'i': // 5
			case 'I': // 5
				return has_sign1 + a + 1;
			default: // 1, 7
				return has_sign1 + a;
		}
	}
	ASSERT_HAS_ERR(err_msg);
	return 0;
}

struct digits_node {
	enum Radix radix;
	size_t n_digit; // n_digit > 0
	const char *digits; // digits != NULL
};
struct sharps_node {
	size_t n_sharp; // n_sharp > 0
	const char *sharps; // sharps != NULL
};
struct uinteger_node {
	struct digits_node *digits; // digits != NULL
	struct sharps_node *trailing_sharps; // trailing_sharps can be NULL
};
struct integer_node {
	enum Sign sign;
	struct uinteger_node *uinteger;
};
struct fraction_node {
	struct uinteger_node *numerator;
	struct uinteger_node *denominator;
};
struct float_node {
	struct uinteger_node *integral;
	struct uinteger_node *fractional;
	enum Sign exp_sign;
	struct uinteger_node *exponent;
	// integral != NULL || fractional != NULL
};
struct real_node {
	enum RealKind {
		REAL_NODE_FLOAT, REAL_NODE_INTEGER, REAL_NODE_FRACTION
	} kind;
	enum Sign sign;
	union {
		struct float_node *float_v;
		struct uinteger_node *int_v;
		struct fraction_node *frac_v;
	} entity;
};
struct complex_node {
	enum ComplexForm {
		COMPLEX_IN_STANDARD_FORM, COMPLEX_IN_EXPONENTIAL_FORM, COMPLEX_PLUS_I, COMPLEX_MINUS_I
	} form;
	union {
		struct {
			struct real_node *real;
			struct real_node *imag; // real != NULL || imag != NULL
		} std;
		struct {
			struct real_node *signed_norm; // signed_norm != NULL
			struct real_node *arg; // arg != NULL
		} expo;
		struct {
			struct real_node *real;
			int imag; // i == -1 || i == 1
		} with_single_i;

	} entity;
};

static struct digits_node *
make_digits_node(enum Radix radix, size_t n_digit, const char *digits) {
	assert(n_digit > 0);
	assert(digits != NULL);
	for (size_t i = 0; i < n_digit; ++i) {
		assert(is_digit(digits[i], radix) != 0);
	}
	struct digits_node *res = malloc(1 * sizeof(struct digits_node));
	res->radix = radix;
	res->n_digit = n_digit;
	res->digits = digits;
	return res;
}

static struct sharps_node *
make_sharps_node(size_t n_sharp, const char *sharps) {
	assert(n_sharp > 0);
	assert(sharps != NULL);
	for (size_t i = 0; i < n_sharp; ++i) {
		assert(sharps[i] == '#');
	}
	struct sharps_node *res = calloc(1, sizeof(struct sharps_node));
	res->n_sharp = n_sharp;
	res->sharps = sharps;
	return res;
}

static struct uinteger_node *
make_uinteger_node(struct digits_node *digits, struct sharps_node *tailing_sharps) {
	assert(digits != NULL);
	struct uinteger_node *res = calloc(1, sizeof(struct uinteger_node));
	res->digits = digits;
	res->trailing_sharps = tailing_sharps;
	return res;
}

static struct uinteger_node *
make_uinteger_node_from_digits(struct digits_node *digits) {
	return make_uinteger_node(digits, NULL);
}

static struct integer_node *
make_integer_node(enum Sign sign, struct uinteger_node *uinteger) {
	assert(uinteger != NULL);
	struct integer_node *res = calloc(1, sizeof(struct integer_node));
	res->sign = sign;
	res->uinteger = uinteger;
	return res;
}

struct fraction_node *
make_fraction_node(struct uinteger_node *numerator, struct uinteger_node *denominator) {
	assert(numerator != NULL);
	assert(denominator != NULL);
	struct fraction_node *res = calloc(1, sizeof(struct fraction_node));
	res->numerator = numerator;
	res->denominator = denominator;
	return res;
}

// this constructor steals its parameters.
static struct float_node *
make_float_node(struct uinteger_node *integral, struct uinteger_node *fractional, struct integer_node *exponent) {
	assert(integral != NULL || fractional != NULL);
	struct float_node *res = calloc(1, sizeof(struct float_node));
	res->integral = integral;
	res->fractional = fractional;
	res->exp_sign = exponent ? exponent->sign : NO_SIGN_YET;
	res->exponent = exponent ? exponent->uinteger : NULL;
	free(exponent);
	return res;
}

static struct real_node *
make_real_node(enum RealKind kind, enum Sign sign, void *entity) {
	assert(kind == REAL_NODE_FLOAT || kind == REAL_NODE_INTEGER || kind == REAL_NODE_FRACTION);
	assert(sign == SIGN_POSITIVE || sign == SIGN_NEGATIVE);
	assert(entity != NULL);
	struct real_node *res = calloc(1, sizeof(struct real_node));
	res->kind = kind;
	res->sign = sign;
	res->entity.float_v = entity;
	return res;
}

static struct complex_node *
make_complex_node(enum ComplexForm form, struct real_node *a, struct real_node *b) {
	struct complex_node *res = calloc(1, sizeof(struct complex_node));
	res->form = form;
	if (form == COMPLEX_IN_STANDARD_FORM) {
		assert(a != NULL || b != NULL);
		res->entity.std.real = a;
		res->entity.std.imag = b;
	} else if (form == COMPLEX_IN_EXPONENTIAL_FORM) {
		assert(a != NULL && b != NULL);
		res->entity.expo.signed_norm = a;
		res->entity.expo.arg = b;
	} else if (form == COMPLEX_PLUS_I) {
		assert(b == NULL);
		res->entity.with_single_i.real = a;
		res->entity.with_single_i.imag = 1;
	} else if (form == COMPLEX_MINUS_I) {
		assert(b == NULL);
		res->entity.with_single_i.real = a;
		res->entity.with_single_i.imag = -1;
	} else {
		assert(0);
	}
	return res;
}

static void dealloc_uinteger_node(struct uinteger_node *x) {
	free(x->digits);
	free(x->trailing_sharps);
	free(x);
}

static void dealloc_integer_node(struct integer_node *x) {
	dealloc_uinteger_node(x->uinteger);
	free(x);
}

static void dealloc_fraction_node(struct fraction_node *x) {
	dealloc_uinteger_node(x->numerator);
	dealloc_uinteger_node(x->denominator);
	free(x);
}

static void dealloc_float_node(struct float_node *x) {
	if (x->integral != NULL) dealloc_uinteger_node(x->integral);
	if (x->fractional != NULL) dealloc_uinteger_node(x->fractional);
	if (x->exponent != NULL) dealloc_uinteger_node(x->exponent);
	free(x);
}

static void dealloc_real_node(struct real_node *x) {
	switch (x->kind) {
		case REAL_NODE_FLOAT:
			dealloc_float_node(x->entity.float_v);
			break;
		case REAL_NODE_INTEGER:
			dealloc_uinteger_node(x->entity.int_v);
			break;
		case REAL_NODE_FRACTION:
			dealloc_fraction_node(x->entity.frac_v);
			break;
	}
	free(x);
}

static void dealloc_complex_node(struct complex_node *x) {
	switch (x->form) {
		case COMPLEX_IN_STANDARD_FORM:
			if (x->entity.std.real != NULL) dealloc_real_node(x->entity.std.real);
			if (x->entity.std.imag != NULL) dealloc_real_node(x->entity.std.imag);
			break;
		case COMPLEX_IN_EXPONENTIAL_FORM:
			dealloc_real_node(x->entity.expo.signed_norm);
			dealloc_real_node(x->entity.expo.arg);
			break;
		case COMPLEX_PLUS_I:
		case COMPLEX_MINUS_I:
			if (x->entity.with_single_i.real != NULL) dealloc_real_node(x->entity.with_single_i.real);
			break;
	}
	free(x);
}

static struct digits_node *
build_from_digits(StringObject *self, size_t i, enum Radix radix, size_t at_least) {
	size_t n = match_digits(self, i, radix, at_least, NULL);
	if (n) {
		return make_digits_node(radix, n, self->ob_sval + i);
	}
	return NULL;
}


// return NULL for no '#', otherwise a `sharps_node` struct.
static struct sharps_node *
build_from_consecutive_sharps(StringObject *self, size_t i, size_t at_least) {
	size_t n_sharp = match_consecutive_sharps(self, i, at_least, NULL);
	if (n_sharp) {
		return make_sharps_node(n_sharp, self->ob_sval + i);
	}
	return NULL;
}


// Nullable
static struct uinteger_node *
build_from_uinteger(StringObject *self, size_t i, enum Radix radix) {
	struct digits_node *a = build_from_digits(self, i, radix, 1);
	if (a == NULL) return NULL;
	struct sharps_node *b = build_from_consecutive_sharps(self, i + a->n_digit, 0);
	return make_uinteger_node(a, b);
}

static struct integer_node *
build_from_suffix(StringObject *self, size_t i) {
	assert(i <= String_GetSize(self));
	const char *cur = self->ob_sval;
	if (i == String_GetSize(self) || !strchr("esfdlESFDL", cur[i])) return NULL;
	if (cur[i + 1] == '+') {
		struct digits_node *v = build_from_digits(self, i + 2, RADIX_DECIMAL, 1);
		assert(v != NULL);
		return make_integer_node(SIGN_POSITIVE, make_uinteger_node(v, NULL));
	} else if (cur[i + 1] == '-') {
		struct digits_node *v = build_from_digits(self, i + 2, RADIX_DECIMAL, 1);
		assert(v != NULL);
		return make_integer_node(SIGN_NEGATIVE, make_uinteger_node(v, NULL));
	} else {
		struct digits_node *v = build_from_digits(self, i + 1, RADIX_DECIMAL, 1);
		assert(v != NULL);
		return make_integer_node(SIGN_POSITIVE, make_uinteger_node(v, NULL));
	}
}


static struct real_node *
build_from_decimal10(StringObject *self, size_t i, enum Sign sign) {
	assert(i < String_GetSize(self));
	const char *cur = self->ob_sval;
	struct digits_node *integral = build_from_digits(self, i, RADIX_DECIMAL, 0);
	if (integral == NULL) {
		assert(cur[i] == '.');
		struct digits_node *fractional = build_from_digits(self, i + 1, RADIX_DECIMAL, 1);
		assert(fractional != NULL);
		struct integer_node *suffix = build_from_suffix(self, i + 1 + fractional->n_digit);
		return make_real_node(
				REAL_NODE_FLOAT,
				sign,
				make_float_node(
						NULL,
						make_uinteger_node_from_digits(fractional),
						suffix)); // 1, 2

	} else {
		assert(integral != NULL);
		if (self->ob_sval[i + integral->n_digit] == '.') {
			struct digits_node *fractional =
					build_from_digits(self, i + integral->n_digit + 1, RADIX_DECIMAL, 0);
#define n_fractional (fractional?fractional->n_digit:0)
//#define n_sharps2 (sharps2 ? sharps2->n_sharp : 0)
			size_t n_sharps2 = match_consecutive_sharps(
					self, i + integral->n_digit + 1 + n_fractional, 0, NULL);
			struct integer_node *suffix = build_from_suffix(
					self, i + integral->n_digit + 1 + n_fractional + n_sharps2);
			return make_real_node(
					REAL_NODE_FLOAT,
					sign,
					make_float_node(
							make_uinteger_node_from_digits(integral),
							fractional ?
							make_uinteger_node(fractional,
											   (n_sharps2 ? make_sharps_node(
													   n_sharps2,
													   self->ob_sval + integral->n_digit + 1 + n_fractional)
														  : NULL))
									   : NULL,
							suffix));

#undef n_sharps2
#undef n_fractional
//			return n_integral + 1 + n_fractional + n_sharps2 + n_suffix; // 3, 4
		} else {
#define n_sharps1 (sharps1 ? sharps1->n_sharp : 0)
			struct sharps_node *sharps1 = build_from_consecutive_sharps(self, i + integral->n_digit, 0);
			if (self->ob_sval[i + integral->n_digit + n_sharps1] == '.') {
				assert(sharps1 != NULL);
				size_t n_sharps2 = match_consecutive_sharps(
						self, i + integral->n_digit + n_sharps1 + 1, 0, NULL);
				struct integer_node *suffix = build_from_suffix(
						self, i + integral->n_digit + n_sharps1 + 1 + n_sharps2);
				return make_real_node(
						REAL_NODE_FLOAT,
						sign,
						make_float_node(
								make_uinteger_node(integral, sharps1),
								NULL,
								suffix));

//					return n_integral + n_sharps1 + 1 + n_sharps2 + n_suffix; // 7, 8
			} else {
				struct integer_node *suffix = build_from_suffix(self, i + integral->n_digit + n_sharps1);
				if (suffix) {
					return make_real_node( // 6
							REAL_NODE_FLOAT,
							sign,
							make_float_node(
									make_uinteger_node(integral, sharps1),
									NULL,
									suffix));
				} else {
					return make_real_node( // 5
							REAL_NODE_INTEGER,
							sign,
							make_uinteger_node(integral, sharps1));

				}
			}
		}
	}

}

static struct real_node *
build_from_ureal(StringObject *self, size_t i, enum Radix radix, enum Sign sign) {
	assert(i < String_GetSize(self));
	const char *cur = self->ob_sval;
	if (cur[i] == '.') goto decimal10;
	struct uinteger_node *a = build_from_uinteger(self, i, radix);
	size_t n = 0;
	if (a) {
		n += a->digits->n_digit;
		if (a->trailing_sharps)
			n += a->trailing_sharps->n_sharp;
	}
	if (radix == RADIX_DECIMAL) {
		if (n > 0) {
			assert(a != NULL);
			if (cur[i + n] == '/') {
				struct uinteger_node *b = build_from_uinteger(self, i + n + 1, radix);
				assert(b != NULL);
				return make_real_node(
						REAL_NODE_FRACTION,
						sign,
						make_fraction_node(a, b));
			}
			assert(a != NULL);
			dealloc_uinteger_node(a);
			a = NULL;
		}
		decimal10:
		return build_from_decimal10(self, i, sign);
	}
	assert(n > 0);
	assert(a != NULL);
	if (cur[i + n] == '/') {
		struct uinteger_node *b = build_from_uinteger(self, i + n + 1, radix);
		assert(b != NULL);
		return make_real_node(REAL_NODE_FRACTION, sign, make_fraction_node(a, b));
	} else {
		return make_real_node(REAL_NODE_INTEGER, sign, a);
	}


}

static struct real_node *
build_from_real(StringObject *self, size_t i, enum Radix radix) {
	assert(i < String_GetSize(self));
	size_t has_sign = self->ob_sval[i] == '+' || self->ob_sval[i] == '-';
	if (self->ob_sval[i] == '+')
		return build_from_ureal(self, i + 1, radix, SIGN_POSITIVE);
	else if (self->ob_sval[i] == '-')
		return build_from_ureal(self, i + 1, radix, SIGN_NEGATIVE);
	else
		return build_from_ureal(self, i, radix, SIGN_POSITIVE);
}

static struct complex_node *
build_from_complex(StringObject *self, size_t i, enum Radix radix) {
	assert(i < String_GetSize(self));
	/* <complex R> ->
	 * 1	| (+|-) <ureal R>
	 * 2	| (+|-) <ureal R> @ <real R>
	 * 3	| (+|-) <ureal R> (+|-) <ureal R> i
	 * 4	| (+|-) <ureal R> (+|-) i
	 * 5	| (+|-) <ureal R> i
	 * 6	| (+|-) i
	 * 7	|       <ureal R>
	 * 8	|       <ureal R> @ <real R>
	 * 9	|       <ureal R> (+|-) <ureal R> i
	 * 10	|       <ureal R> (+|-) i
	 */
	const char *cur = self->ob_sval;
	size_t has_sign1 = cur[i] == '+' || cur[i] == '-';
	enum Sign sign1 = NO_SIGN_YET;
	if (cur[i] == '+') sign1 = SIGN_POSITIVE;
	if (cur[i] == '-') sign1 = SIGN_NEGATIVE;
	if (has_sign1 && (cur[i + 1] == 'i' || cur[i + 1] == 'I')) { // 6
		assert(sign1 != NO_SIGN_YET);
		return make_complex_node(sign1 == SIGN_POSITIVE ? COMPLEX_PLUS_I : COMPLEX_MINUS_I, NULL, NULL);
	}
	char *guard = NULL;
	size_t a = match_ureal(self, i + has_sign1, radix, &guard); // real part
	struct real_node *x = build_from_real(self, i, radix);
	assert(a > 0);
	assert(x != NULL);
	ASSERT_NO_ERR(&guard);
	switch (cur[i + has_sign1 + a]) {
		case '@': // 2, 8
			return make_complex_node(
					COMPLEX_IN_EXPONENTIAL_FORM,
					x,
					build_from_real(self, i + has_sign1 + a + 1, radix));
		case '-': // 3, 4, 9, 10
		case '+': { // 3, 4, 9, 10
			if (tolower(cur[i + has_sign1 + a + 1]) == 'i') {
				return make_complex_node(
						cur[i + has_sign1 + a] == '+' ? COMPLEX_PLUS_I : COMPLEX_MINUS_I,
						x,
						NULL);
			} else {
				size_t b = match_ureal(self, i + has_sign1 + a + 1, radix, &guard);
				ASSERT_NO_ERR(&guard);
				assert(b);
				assert(tolower(cur[i + has_sign1 + a + 1 + b]) == 'i');
				return make_complex_node(
						COMPLEX_IN_STANDARD_FORM,
						x,
						build_from_real(self, i + has_sign1 + a + 1, radix));
			}
		}
		case 'i': // 5
		case 'I': // 5
			return make_complex_node(COMPLEX_IN_STANDARD_FORM, NULL, x);
		default: // 1, 7
			return make_complex_node(COMPLEX_IN_STANDARD_FORM, x, NULL);
	}
}

static LongObject *
extract_from_uinteger_node(struct uinteger_node *x, int strip_trailing_zeros_and_sharps, enum Sign sign) {
	assert(sign != NO_SIGN_YET);
	assert(x->digits != NULL);
	long long int v = 0;
	size_t n_digit = x->digits->n_digit;
	size_t n_sharp = x->trailing_sharps ? x->trailing_sharps->n_sharp : 0;
	enum Radix r = x->digits->radix;
	const char *digits = x->digits->digits;
	if (!strip_trailing_zeros_and_sharps) {
		for (size_t i = 0; i < n_digit; ++i)
			v = v * r +
				(strchr("0123456789abcdef", tolower(digits[i])) - "0123456789abcdef"); // TODO: overflow handling
		for (size_t i = 0; i < n_sharp; ++i)
			v *= r; // TODO: overflow handling
	} else {
		while (n_digit) {
			if (digits[n_digit - 1] != '0') {
				break;
			}
			n_digit--;
		}
		for (size_t i = 0; i < n_digit; ++i)
			v = v * r +
				(strchr("0123456789abcdef", tolower(digits[i])) - "0123456789abcdef"); // TODO: overflow handling
	}
	if (sign == SIGN_NEGATIVE) v = -v;
	return Long_From_i64(v);
}


static FractionObject *
extract_from_fraction_node(struct fraction_node *x, enum Sign sign) {
	assert(x != NULL);
	assert(x->numerator != NULL);
	assert(x->denominator != NULL);
	LongObject *n = extract_from_uinteger_node(x->numerator, 0, sign);
	LongObject *d = extract_from_uinteger_node(x->denominator, 0, SIGN_POSITIVE);
	FractionObject *res = Fraction_New(n, d, 1);
	DECREF(n);
	DECREF(d);
	return res;
}


static Object *
extract_from_float_node(struct float_node *x, enum Exactness exactness, enum Sign sign) {
	assert(x != NULL);
	assert(sign != NO_SIGN_YET);
	assert(x->integral != NULL || x->fractional != NULL);
	assert((x->exp_sign == NO_SIGN_YET) == (x->exponent == NULL));
	if (exactness == EXACT) {
		LongObject *integral = !x->integral ? Long_From_i64(0) : extract_from_uinteger_node(x->integral, 0,
																							sign);
		LongObject *fractional = !x->fractional ? Long_From_i64(0) : extract_from_uinteger_node(x->fractional,
																								1, sign);
		LongObject *exponent = !x->exponent ? NULL : extract_from_uinteger_node(x->exponent, 0, SIGN_POSITIVE);
		LongObject *denominator = Long_From_i64(1);
		LongObject *tmp = NULL;
		size_t n_fractional_digit = !x->fractional ? 0 : x->fractional->digits->n_digit;
		for (size_t i = 0; i < n_fractional_digit; ++i) {
			MOVE_SET(tmp, integral, Long_MulByI64(integral, 10));
		}
		for (size_t i = 0; i < n_fractional_digit; ++i) {
			MOVE_SET(tmp, denominator, Long_MulByI64(denominator, 10));
		}
		LongObject *numerator = Long_Add(integral, fractional);
		if (x->exp_sign == SIGN_POSITIVE) {
			LongObject *ten = Long_From_i64(10);
			MOVE_SET(tmp, exponent, Long_Pow(ten, exponent));
			MOVE_SET(tmp, numerator, Long_Mul(numerator, exponent));
			DECREF(ten);
		} else if (x->exp_sign == SIGN_NEGATIVE) {
			LongObject *ten = Long_From_i64(10);
			MOVE_SET(tmp, exponent, Long_Neg(exponent));
			MOVE_SET(tmp, exponent, Long_Pow(ten, exponent));
			MOVE_SET(tmp, denominator, Long_Mul(denominator, exponent));
			DECREF(ten);
		}
		FractionObject *res0 = Fraction_New(numerator, denominator, 1);

		Object *res = TO_EXACT(res0);
		DECREF(res0);
		DECREF(integral);
		DECREF(fractional);
		DECREF(numerator);
		DECREF(denominator);
		return res;
	} else {
		LongObject *integral = !x->integral ?
							   Long_From_i64(0) :
							   extract_from_uinteger_node(
									   x->integral, 0, SIGN_POSITIVE);
		LongObject *fractional = !x->fractional ?
								 Long_From_i64(0) :
								 extract_from_uinteger_node(
										 x->fractional, 1, SIGN_POSITIVE);
		LongObject *exponent = !x->exponent ? NULL :
							   extract_from_uinteger_node(x->exponent, 0, x->exp_sign);
		size_t size = 0;
		char *buf = NULL, *end = NULL;
		FILE *f = open_memstream(&buf, &size);
		if (!f) {
			perror("error");
			exit(EXIT_FAILURE);
		}
		if (sign == SIGN_NEGATIVE) {
			fputc('-', f);
		}
		Long_Print(integral, f);
		fputc('.', f);
		if (!Long_IsZero(fractional)) {
			for (size_t i = 0; i < x->fractional->digits->n_digit && x->fractional->digits->digits[i] == '0'; ++i) {
				fputc('0', f);
			}
		}
		Long_Print(fractional, f);
		if (x->exponent) {
			fputc('e', f);
			Long_Print(exponent, f);
			DECREF(exponent);
		}
		DECREF(integral);
		DECREF(fractional);

		fclose(f);
		double val = strtod(buf, &end);
//		free(buf);
		if (errno) {
			perror("conversion error");
			exit(EXIT_FAILURE);
		}
		if (end == buf) {
			fprintf(stderr, "no conversion performed\n");
			exit(EXIT_FAILURE);
		}
		free(buf);
		return AS_OBJECT(Float_New(val));
	}
}


static Object *extract_from_real_node(struct real_node *x, enum Exactness exactness) {
	assert(x != NULL);
	assert(x->kind == REAL_NODE_FLOAT || x->kind == REAL_NODE_INTEGER || x->kind == REAL_NODE_FRACTION);
	if (x->kind == REAL_NODE_FLOAT) {
		return extract_from_float_node(x->entity.float_v, exactness, x->sign);
	} else if (x->kind == REAL_NODE_INTEGER) {
		LongObject *int_v = extract_from_uinteger_node(x->entity.int_v, 0, x->sign);
		if (x->entity.int_v->trailing_sharps != NULL && exactness != EXACT) {
			FloatObject *float_v = Float_FromLong(int_v);
			DECREF(int_v);
			return AS_OBJECT(float_v);
		}
		return AS_OBJECT(int_v);
	} else {
		assert(x->kind == REAL_NODE_FRACTION);
		return AS_OBJECT(extract_from_fraction_node(x->entity.frac_v, x->sign));
	}
}


Object *extract_from_complex_node(struct complex_node *x, enum Exactness exactness) {
	assert(x != NULL);
	if (x->form == COMPLEX_IN_STANDARD_FORM) {
		struct real_node *real = x->entity.std.real;
		struct real_node *imag = x->entity.std.imag;
		if (real == NULL && imag != NULL) {
			Object *im = extract_from_real_node(imag, exactness);
			assert(IS_REAL(im));
			if (IS_EXACT(im) && IS_ZERO(im)) {
				return im;
			}
			Object *_im = NUMBER_METHODS(im)->nb_float(im);
			assert(IS_INEXACT(_im));
			ComplexObject *res = Complex_New(0., Float_GetVal(AS_FLOAT(_im)));
			DECREF(im);
			DECREF(_im);
			return AS_OBJECT(res);
		} else if (real != NULL && imag == NULL) {
			Object *re = extract_from_real_node(real, exactness);
			assert(IS_REAL(re));
			return re;
		} else {
			assert(real != NULL && imag != NULL);
			Object *re = extract_from_real_node(real, exactness);
			Object *im = extract_from_real_node(imag, exactness);
			assert(IS_REAL(re));
			assert(IS_REAL(im));
			if (IsExactFast(im) && IsZeroFast(im)) {
				DECREF(im);
				return re;
			} else {
				FloatObject *_re = AS_FLOAT(NUMBER_METHODS(re)->nb_float(re));
				FloatObject *_im = AS_FLOAT(NUMBER_METHODS(im)->nb_float(im));
				ComplexObject *res = Complex_New(Float_GetVal(_re), Float_GetVal(_im));
				DECREF(_re);
				DECREF(_im);
				DECREF(re);
				DECREF(im);
				return AS_OBJECT(res);
			}
		}
	} else if (x->form == COMPLEX_IN_EXPONENTIAL_FORM) {
		Object *signed_norm = extract_from_real_node(x->entity.expo.signed_norm, exactness);
		Object *arg = extract_from_real_node(x->entity.expo.arg, exactness);
		if ((IsExactFast(signed_norm) && IsZeroFast(signed_norm)) || (IsExactFast(arg) && IsZeroFast(arg))) {
			DECREF(arg);
			return signed_norm;
		}
		ComplexObject *res = Complex_FromExpForm(signed_norm, arg);
		DECREF(signed_norm);
		DECREF(arg);
		return AS_OBJECT(res);
	} else if (x->form == COMPLEX_PLUS_I || x->form == COMPLEX_MINUS_I) {
		struct real_node *real = x->entity.with_single_i.real;
		int imag = x->entity.with_single_i.imag;
		if (real == NULL) {
			assert(abs(imag) == 1);
			return AS_OBJECT(Complex_New(0., imag));
		} else {
			Object *re = extract_from_real_node(real, EXACTNESS_UNSPECIFIED);
			Object *_re = NUMBER_METHODS(re)->nb_float(re);
			assert(IS_INEXACT(_re) && IS_REAL(_re));
			ComplexObject *res = Complex_New(Float_GetVal(AS_FLOAT(_re)), imag);
			DECREF(re);
			DECREF(_re);
			return AS_OBJECT(res);
		}
	}
	assert(0);
}

static size_t
prefix(StringObject *self, size_t i, enum Radix *radix, enum Exactness *exactness, char **err_msg) {

	static const enum Radix radix_table[] = {
			['b']=RADIX_BINARY, ['o']=RADIX_OCTAL, ['d']=RADIX_DECIMAL, ['x']=RADIX_HEX,
	};
	static const enum Exactness ei_table[] = {
			['e']=EXACT, ['i']=INEXACT,
	};
	if (i >= String_GetSize(self)) {
		SET_ERR_AND_RETURN_0("end of string");
	}

	const char *cur = self->ob_sval;
	const size_t len = String_GetSize(self);
	int n = 0;
	int p[2] = {0};
	if (cur[i] != '#') {
		SET_ERR_AND_RETURN_0("expect '#'");
	}
	if (cur[i] == '#' && i + 1 == len) {
		SET_ERR_AND_RETURN_0("unexpected end of string after '#'");
	}
	if (cur[i] == '#' && i + 2 < len && cur[i + 2] == '#' && i + 3 == len) {
		SET_ERR_AND_RETURN_0("unexpected end of string after '#'");
	}
	assert(cur[i] == '#');
	assert(i + 1 < len);
	p[n++] = tolower(cur[i + 1]);
	if (cur[i + 2] == '#') {
		assert(i + 3 < len);
		p[n++] = tolower(cur[i + 3]);

	}

	if ((n >= 1 && !strchr("eibodx", p[0])) || (n == 2 && !strchr("eibodx", p[1]))) {
		SET_ERR_AND_RETURN_0("unknown # object: %s", self->ob_sval + i);
	}
	if (n == 2 && p[0] == p[1]) {
		SET_ERR_AND_RETURN_0("duplicate prefix: #%c", p[0]);
	}
	for (size_t j = 0; j < n; ++j) {
		if (strchr("ei", p[j])) {
			*exactness = ei_table[p[j]];
		} else if (strchr("bodx", p[j])) {
			*radix = radix_table[p[j]];
		}
	}
	return n * 2;
}

Object *String_ToNumber(StringObject *self) {
	enum Exactness ei = EXACTNESS_UNSPECIFIED;
	enum Radix r = RADIX_DECIMAL;
	if (String_GetSize(self)) {
		char *err_msg = NULL;
		size_t v = 0;
		if (self->ob_sval[0] == '#') {
			v = prefix(self, 0, &r, &ei, &err_msg);
			if (v == 0) {
				assert(err_msg);
				fprintf(stderr, "%s\n", err_msg);
				free(err_msg);
				RETURN_FALSE;
			}
			ASSERT_NO_ERR(&err_msg);
		}
		size_t n = match_complex(self, v, r, &err_msg);
		if (n + v == String_GetSize(self)) {
			ASSERT_NO_ERR(&err_msg);
			struct complex_node *node = build_from_complex(self, v, r);
			assert(node);
			Object *res = extract_from_complex_node(node, ei);
			dealloc_complex_node(node);
			if (ei == INEXACT) {
				Object *tmp = NULL;
				MOVE_SET(tmp, res, TO_INEXACT(res));
			}
			assert(res);
			return res;
		} else {
//			if (err_msg)
//				fprintf(stderr, "%s\n", err_msg);
			free(err_msg);
			RETURN_FALSE;
		}
	}
	RETURN_FALSE;
}

#undef SET_ERR_AND_RETURN_0
#undef LOC_FORMAT
#undef ASSERT_NO_ERR
#undef ASSERT_HAS_ERR
#undef CLEAR_ERR_MSG
#undef RETURN_WHEN_NULL
