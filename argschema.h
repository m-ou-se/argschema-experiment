#include <argdata.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "argdata-util.h"

#define ARGSCHEMA_DEF(name, def) \
	asm ( \
		".pushsection argschema." #name ", \"a\"\n" \
		"_argschema_" #name ":\n" \
		".global _argschema_" #name "\n" \
		_ARGSCHEMA_ITERATE(_ARGSCHEMA_ASM_A def) \
		".popsection\n" \
	); \
	struct argdata_##name { \
		_ARGSCHEMA_ITERATE(_ARGSCHEMA_STRUCT_A def) \
	}; \
	static inline int argdata_get_argschema_##name( \
		const argdata_t *ad, \
		struct argdata_##name *result) { \
		int error = 0; \
		argdata_iterator_t i = argdata_map_iterator(ad); \
		while (argdata_map_iterator_next(&i)) { \
			const char *key; \
			error = argdata_get_str_c(&i.key, &key); \
			if (error) { \
				error = 123; \
				break; \
			} \
			const argdata_t *val = &i.value; \
			_ARGSCHEMA_ITERATE(_ARGSCHEMA_PARSE_A def) \
			{ \
				error = 456; \
				break; \
			} \
		} \
		if (i.error) error = i.error; \
		return error; \
	}

// -------- STRUCT --------

#define _ARGSCHEMA_STRUCT_MEM(type, ...) \
	_ARGSCHEMA_STRUCT_MEM_(_ARGSCHEMA_VA_LAST(__VA_ARGS__), type, __VA_ARGS__)

#define _ARGSCHEMA_STRUCT_MEM_(name, ...) \
	_ARGSCHEMA_STRUCT_MEM__(name, __VA_ARGS__)

#define _ARGSCHEMA_STRUCT_MEM__(name, type, ...) \
	_ARGSCHEMA_STRUCT_##type(__VA_ARGS__) \
	bool has_##name;

#define _ARGSCHEMA_STRUCT_int(name) \
	int name;
#define _ARGSCHEMA_STRUCT_fd(name) \
	int name;
#define _ARGSCHEMA_STRUCT_struct(type, name) \
	struct argdata_##type name;
#define _ARGSCHEMA_STRUCT_seq(type, name) \
	size_t name##_size; \
	_ARGSCHEMA_STRUCT(_ARGSCHEMA_UNWRAP type, *name)
#define _ARGSCHEMA_STRUCT_map(ktype, vtype, name) \
	size_t name##_size; \
	_ARGSCHEMA_STRUCT(_ARGSCHEMA_UNWRAP ktype, *name##_keys) \
	_ARGSCHEMA_STRUCT(_ARGSCHEMA_UNWRAP vtype, *name##_vals)

#define _ARGSCHEMA_STRUCT(type, ...) _ARGSCHEMA_STRUCT2(type, __VA_ARGS__)
#define _ARGSCHEMA_STRUCT2(type, ...) _ARGSCHEMA_STRUCT_##type(__VA_ARGS__)

#define _ARGSCHEMA_STRUCT_A(...) _ARGSCHEMA_STRUCT_MEM(__VA_ARGS__) _ARGSCHEMA_STRUCT_B
#define _ARGSCHEMA_STRUCT_B(...) _ARGSCHEMA_STRUCT_MEM(__VA_ARGS__) _ARGSCHEMA_STRUCT_A
#define _ARGSCHEMA_STRUCT_A_END
#define _ARGSCHEMA_STRUCT_B_END

// -------- ASM --------

#define _ARGSCHEMA_ASM_INT(int) \
	".uleb128 " #int "\n"

#define _ARGSCHEMA_ASM_STRING_IF_NONEMPTY(str) \
	"1: .ascii " #str "\n" \
	/*".if . - 1b == 0\n"*/ \
	".byte 0\n" \
	/*".endif\n" */

#define _ARGSCHEMA_ASM_POINTER(symbol) \
	".quad " #symbol "\n"

#define _ARGSCHEMA_ASM_int(name) \
	_ARGSCHEMA_ASM_INT(0) \
	_ARGSCHEMA_ASM_STRING_IF_NONEMPTY(#name)

#define _ARGSCHEMA_ASM_fd(name) \
	_ARGSCHEMA_ASM_INT(1) \
	_ARGSCHEMA_ASM_STRING_IF_NONEMPTY(#name)

#define _ARGSCHEMA_ASM_struct(type, name) \
	_ARGSCHEMA_ASM_INT(2) \
	_ARGSCHEMA_ASM_POINTER(_argschema_##type) \
	_ARGSCHEMA_ASM_STRING_IF_NONEMPTY(#name)

#define _ARGSCHEMA_ASM_seq(type, name) \
	_ARGSCHEMA_ASM_INT(3) \
	_ARGSCHEMA_ASM1(_ARGSCHEMA_UNWRAP type, name)

#define _ARGSCHEMA_ASM_map(ktype, vtype, name) \
	_ARGSCHEMA_ASM_INT(4) \
	_ARGSCHEMA_ASM1(_ARGSCHEMA_UNWRAP ktype,) \
	_ARGSCHEMA_ASM1(_ARGSCHEMA_UNWRAP vtype, name)

#define _ARGSCHEMA_ASM1(type, ...) _ARGSCHEMA_ASM2(type, __VA_ARGS__)
#define _ARGSCHEMA_ASM2(type, ...) _ARGSCHEMA_ASM_##type(__VA_ARGS__)
#define _ARGSCHEMA_ASM0(type, ...) _ARGSCHEMA_ASM_##type(__VA_ARGS__)

#define _ARGSCHEMA_ASM_A(...) _ARGSCHEMA_ASM0(__VA_ARGS__) _ARGSCHEMA_ASM_B
#define _ARGSCHEMA_ASM_B(...) _ARGSCHEMA_ASM0(__VA_ARGS__) _ARGSCHEMA_ASM_A
#define _ARGSCHEMA_ASM_A_END
#define _ARGSCHEMA_ASM_B_END

// -------- PARSE --------

#define _ARGSCHEMA_PARSE_MEM(type, ...) \
	_ARGSCHEMA_PARSE_MEM_(_ARGSCHEMA_VA_LAST(__VA_ARGS__), type, __VA_ARGS__)

#define _ARGSCHEMA_PARSE_MEM_(name, ...) \
	_ARGSCHEMA_PARSE_MEM__(name, __VA_ARGS__)

#define _ARGSCHEMA_PARSE_MEM__(name, type, ...) \
	if (!result->has_##name && strcmp(key, #name) == 0) { \
		_ARGSCHEMA_PARSE_##type(__VA_ARGS__) \
		if (error) break; \
		result->has_##name = true; \
	} else

#define _ARGSCHEMA_PARSE_int(name) \
	error = argdata_get_int(val, &result->name);
#define _ARGSCHEMA_PARSE_fd(name) \
	error = argdata_get_fd(val, &result->name);
#define _ARGSCHEMA_PARSE_struct(type, name) \
	error = argdata_get_argschema_##type(val, &result->name);
#define _ARGSCHEMA_PARSE_seq(type, name) \
	error = argdata_get_seq_size(val, &result->name##_size); \
	if (error) break; \
	result->name = calloc(result->name##_size, sizeof(result->name[0])); \
	argdata_iterator_t j = argdata_seq_iterator(val); \
	while (argdata_seq_iterator_next(&j)) { \
		val = &j.value; \
		_ARGSCHEMA_PARSE(_ARGSCHEMA_UNWRAP type, name[j.index]) \
		if (error) break; \
	} \
	if (j.error) error = j.error;
#define _ARGSCHEMA_PARSE_map(ktype, vtype, name) \
	error = argdata_get_map_size(val, &result->name##_size); \
	if (error) break; \
	result->name##_vals = calloc(result->name##_size, sizeof(result->name##_keys[0])); \
	result->name##_keys = calloc(result->name##_size, sizeof(result->name##_vals[0])); \
	argdata_iterator_t k = argdata_map_iterator(val); \
	while (argdata_map_iterator_next(&k)) { \
		val = &k.key; \
		_ARGSCHEMA_PARSE(_ARGSCHEMA_UNWRAP ktype, name##_keys[k.index]) \
		if (error) break; \
		val = &k.value; \
		_ARGSCHEMA_PARSE(_ARGSCHEMA_UNWRAP vtype, name##_vals[k.index]) \
		if (error) break; \
	} \
	if (k.error) error = k.error;

#define _ARGSCHEMA_PARSE(type, ...) _ARGSCHEMA_PARSE2(type, __VA_ARGS__)
#define _ARGSCHEMA_PARSE2(type, ...) _ARGSCHEMA_PARSE_##type(__VA_ARGS__)

#define _ARGSCHEMA_PARSE_A(...) _ARGSCHEMA_PARSE_MEM(__VA_ARGS__) _ARGSCHEMA_PARSE_B
#define _ARGSCHEMA_PARSE_B(...) _ARGSCHEMA_PARSE_MEM(__VA_ARGS__) _ARGSCHEMA_PARSE_A
#define _ARGSCHEMA_PARSE_A_END
#define _ARGSCHEMA_PARSE_B_END

// -------- USE --------

#define ARGSCHEMA_USE(name) \
	extern char _argschema_##name[]; \
	void *volatile __argschema = _argschema_##name;

// -------- UTIL --------

#define _ARGSCHEMA_ITERATE(...) _ARGSCHEMA_ITERATE_(__VA_ARGS__)
#define _ARGSCHEMA_ITERATE_(...) __VA_ARGS__ ## _END

#define _ARGSCHEMA_UNWRAP(...) __VA_ARGS__

#define _ARGSCHEMA_VA_SIZE(...) _ARGSCHEMA_VA_SIZE_(__VA_ARGS__, 6, 5, 4, 3, 2, 1, 0)
#define _ARGSCHEMA_VA_SIZE_(a,b,c,d,e,f,size,...) size

#define _ARGSCHEMA_VA_LAST(...) _ARGSCHEMA_VA_LAST_(_ARGSCHEMA_VA_SIZE(__VA_ARGS__), __VA_ARGS__)
#define _ARGSCHEMA_VA_LAST_(size, ...) _ARGSCHEMA_VA_LAST__(size, __VA_ARGS__)
#define _ARGSCHEMA_VA_LAST__(size, ...) _ARGSCHEMA_VA_LAST_##size(__VA_ARGS__)
#define _ARGSCHEMA_VA_LAST_1(x, ...) x
#define _ARGSCHEMA_VA_LAST_2(a, x ...) x
#define _ARGSCHEMA_VA_LAST_3(a, b, x ...) x
#define _ARGSCHEMA_VA_LAST_4(a, b, c, x ...) x
#define _ARGSCHEMA_VA_LAST_5(a, b, c, d, x ...) x
#define _ARGSCHEMA_VA_LAST_6(a, b, c, d, e, x ...) x
