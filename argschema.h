#include <argdata.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "argdata-util.h"

#define ARGSCHEMA_DEF(name, def) \
	struct argdata_##name { \
		_ARGSCHEMA_ITERATE(_ARGSCHEMA_STRUCT_A def) \
	}; \
	asm ( \
		".pushsection argschema." #name ", \"a\"\n" \
		"_argschema_" #name ":\n" \
		".global _argschema_" #name "\n" \
		_ARGSCHEMA_ITERATE(_ARGSCHEMA_ASM_A def) \
		".popsection\n" \
	); \
	static inline int argdata_get_argdata_##name( \
		const argdata_t *ad, \
		struct argdata_##name *result) { \
		int error = 0; \
		argdata_iterator_t i = argdata_map_iterator(ad); \
		while (argdata_map_iterator_next(&i)) { \
			const char *key; \
			error = argdata_get_str_c(&i.key, &key); \
			if (error) { \
				error = EINVAL; \
				break; \
			} \
			const argdata_t *val = &i.value; \
			_ARGSCHEMA_ITERATE(_ARGSCHEMA_PARSE_A def) \
			{ \
				error = EINVAL; \
				break; \
			} \
		} \
		if (i.error) error = i.error; \
		return error; \
	}

// -------- STRUCT --------

#define _ARGSCHEMA_STRUCT_A(...) _ARGSCHEMA_STRUCT_ITEM(__VA_ARGS__) _ARGSCHEMA_STRUCT_B
#define _ARGSCHEMA_STRUCT_B(...) _ARGSCHEMA_STRUCT_ITEM(__VA_ARGS__) _ARGSCHEMA_STRUCT_A
#define _ARGSCHEMA_STRUCT_A_END
#define _ARGSCHEMA_STRUCT_B_END

#define _ARGSCHEMA_STRUCT_ITEM(type, ...) \
	_ARGSCHEMA_IF( \
		_ARGSCHEMA_IS_ANNOTATE_##type, \
		_ARGSCHEMA_STRUCT_ANN, \
		_ARGSCHEMA_STRUCT_MEM \
	)(_ARGSCHEMA_VA_LAST(__VA_ARGS__), type, __VA_ARGS__)

#define _ARGSCHEMA_STRUCT_ANN(...)

#define _ARGSCHEMA_STRUCT_MEM(name, ...) \
	_ARGSCHEMA_STRUCT_MEM_(name, __VA_ARGS__)

#define _ARGSCHEMA_STRUCT_MEM_(name, type, ...) \
	_ARGSCHEMA_STRUCT_##type(__VA_ARGS__) \
	bool has_##name;

#define _ARGSCHEMA_STRUCT_int(name) \
	int name;
#define _ARGSCHEMA_STRUCT_fd(name) \
	int name;
#define _ARGSCHEMA_STRUCT_str(name) \
	struct { \
		size_t size; \
		const char *str; \
	} name;
#define _ARGSCHEMA_STRUCT_struct(type, name) \
	struct argdata_##type name;
#define _ARGSCHEMA_STRUCT_seq(type, name) \
	struct { \
		size_t size; \
		_ARGSCHEMA_STRUCT_NEST(_ARGSCHEMA_UNWRAP type, *items) \
	} name;
#define _ARGSCHEMA_STRUCT_map(ktype, vtype, name) \
	struct { \
		size_t size; \
		_ARGSCHEMA_STRUCT_NEST(_ARGSCHEMA_UNWRAP ktype, *keys) \
		_ARGSCHEMA_STRUCT_NEST(_ARGSCHEMA_UNWRAP vtype, *values) \
	} name;

#define _ARGSCHEMA_STRUCT_NEST(type, ...) _ARGSCHEMA_STRUCT_NEST_(type, __VA_ARGS__)
#define _ARGSCHEMA_STRUCT_NEST_(type, ...) _ARGSCHEMA_STRUCT_##type(__VA_ARGS__)

// -------- ASM --------

#define _ARGSCHEMA_ASM_A(...) _ARGSCHEMA_ASM_ITEM(__VA_ARGS__) _ARGSCHEMA_ASM_B
#define _ARGSCHEMA_ASM_B(...) _ARGSCHEMA_ASM_ITEM(__VA_ARGS__) _ARGSCHEMA_ASM_A
#define _ARGSCHEMA_ASM_A_END
#define _ARGSCHEMA_ASM_B_END

#define _ARGSCHEMA_ASM_ITEM(type, ...) \
	_ARGSCHEMA_IF( \
		_ARGSCHEMA_IS_ANNOTATE_##type, \
		_ARGSCHEMA_ASM_ANN, \
		_ARGSCHEMA_ASM_MEM \
	)(type, __VA_ARGS__)

#define _ARGSCHEMA_ASM_ANN(type, ...) _ARGSCHEMA_ASM_ANN_##type(__VA_ARGS__)

#define _ARGSCHEMA_ASM_MEM(type, ...) \
	_ARGSCHEMA_ASM_MEM_(_ARGSCHEMA_VA_LAST(__VA_ARGS__), type, __VA_ARGS__)

#define _ARGSCHEMA_ASM_MEM_(name, ...) \
	_ARGSCHEMA_ASM_MEM__(name, __VA_ARGS__)

#define _ARGSCHEMA_ASM_MEM__(name, type, ...) \
	_ARGSCHEMA_ASM_##type(__VA_ARGS__) \
	_ARGSCHEMA_ASM_STRING(#name)

#define _ARGSCHEMA_ASM_INT(int) \
	".uleb128 " #int "\n"

#define _ARGSCHEMA_ASM_STRING(str) \
	".asciz " #str "\n" \

#define _ARGSCHEMA_ASM_POINTER(symbol) \
	".quad " #symbol "\n"

#define _ARGSCHEMA_ASM_ANN_doc(docstring) \
	_ARGSCHEMA_ASM_INT(0x40) \
	_ARGSCHEMA_ASM_STRING(docstring) \

#define _ARGSCHEMA_ASM_ANN_fd_cap(cap) \
	_ARGSCHEMA_ASM_INT(0x41) \
	_ARGSCHEMA_ASM_INT(cap) \

#define _ARGSCHEMA_ASM_int(name) \
	_ARGSCHEMA_ASM_INT(0)

#define _ARGSCHEMA_ASM_fd(name) \
	_ARGSCHEMA_ASM_INT(1)

#define _ARGSCHEMA_ASM_str(name) \
	_ARGSCHEMA_ASM_INT(2)

#define _ARGSCHEMA_ASM_struct(type, name) \
	_ARGSCHEMA_ASM_INT(3) \
	_ARGSCHEMA_ASM_POINTER(_argschema_##type)

#define _ARGSCHEMA_ASM_seq(type, name) \
	_ARGSCHEMA_ASM_INT(4) \
	_ARGSCHEMA_ASM_NEST(_ARGSCHEMA_UNWRAP type,)

#define _ARGSCHEMA_ASM_map(ktype, vtype, name) \
	_ARGSCHEMA_ASM_INT(5) \
	_ARGSCHEMA_ASM_NEST(_ARGSCHEMA_UNWRAP ktype,) \
	_ARGSCHEMA_ASM_NEST(_ARGSCHEMA_UNWRAP vtype,)

#define _ARGSCHEMA_ASM_NEST(type, ...) _ARGSCHEMA_ASM_NEST_(type, __VA_ARGS__)
#define _ARGSCHEMA_ASM_NEST_(type, ...) _ARGSCHEMA_ASM_##type(__VA_ARGS__)

// -------- PARSE --------

#define _ARGSCHEMA_PARSE_A(...) _ARGSCHEMA_PARSE_ITEM(__VA_ARGS__) _ARGSCHEMA_PARSE_B
#define _ARGSCHEMA_PARSE_B(...) _ARGSCHEMA_PARSE_ITEM(__VA_ARGS__) _ARGSCHEMA_PARSE_A
#define _ARGSCHEMA_PARSE_A_END
#define _ARGSCHEMA_PARSE_B_END

#define _ARGSCHEMA_PARSE_ITEM(type, ...) \
	_ARGSCHEMA_IF( \
		_ARGSCHEMA_IS_ANNOTATE_##type, \
		_ARGSCHEMA_PARSE_ANN, \
		_ARGSCHEMA_PARSE_MEM \
	)(_ARGSCHEMA_VA_LAST(__VA_ARGS__), type, __VA_ARGS__)

#define _ARGSCHEMA_PARSE_ANN(...)

#define _ARGSCHEMA_PARSE_MEM(name, ...) \
	_ARGSCHEMA_PARSE_MEM_(name, __VA_ARGS__)

#define _ARGSCHEMA_PARSE_MEM_(name, type, ...) \
	if (!result->has_##name && strcmp(key, #name) == 0) { \
		_ARGSCHEMA_PARSE_##type(__VA_ARGS__) \
		if (error) break; \
		result->has_##name = true; \
	} else

#define _ARGSCHEMA_PARSE_int(name) \
	error = argdata_get_int(val, &result->name);
#define _ARGSCHEMA_PARSE_fd(name) \
	error = argdata_get_fd(val, &result->name);
#define _ARGSCHEMA_PARSE_str(name) \
	error = argdata_get_str(val, &result->name.str, &result->name.size);
#define _ARGSCHEMA_PARSE_struct(type, name) \
	error = argdata_get_argdata_##type(val, &result->name);
#define _ARGSCHEMA_PARSE_seq(type, name) \
	error = argdata_get_seq_size(val, &result->name.size); \
	if (error) break; \
	result->name = calloc(result->name##_size, sizeof(result->name.items[0])); \
	argdata_iterator_t j = argdata_seq_iterator(val); \
	while (argdata_seq_iterator_next(&j)) { \
		val = &j.value; \
		_ARGSCHEMA_PARSE_NEST(_ARGSCHEMA_UNWRAP type, name.items[j.index]) \
		if (error) break; \
	} \
	if (j.error) error = j.error;
#define _ARGSCHEMA_PARSE_map(ktype, vtype, name) \
	error = argdata_get_map_size(val, &result->name.size); \
	if (error) break; \
	result->name.keys = calloc(result->name.size, sizeof(result->name.keys[0])); \
	result->name.values = calloc(result->name.size, sizeof(result->name.values[0])); \
	argdata_iterator_t k = argdata_map_iterator(val); \
	while (argdata_map_iterator_next(&k)) { \
		val = &k.key; \
		_ARGSCHEMA_PARSE_NEST(_ARGSCHEMA_UNWRAP ktype, name.keys[k.index]) \
		if (error) break; \
		val = &k.value; \
		_ARGSCHEMA_PARSE_NEST(_ARGSCHEMA_UNWRAP vtype, name.values[k.index]) \
		if (error) break; \
	} \
	if (k.error) error = k.error;

#define _ARGSCHEMA_PARSE_NEST(type, ...) _ARGSCHEMA_PARSE_NEST_(type, __VA_ARGS__)
#define _ARGSCHEMA_PARSE_NEST_(type, ...) _ARGSCHEMA_PARSE_##type(__VA_ARGS__)

// -------- USE --------

#define ARGSCHEMA_USE(name) \
	void argdata_main(const struct argdata_##name *); \
	void program_main(const argdata_t *ad) { \
		struct argdata_##name data = {}; \
		extern char _argschema_##name[]; \
		void *volatile __argschema = _argschema_##name; \
		(void)__argschema; \
		if (argdata_get_argdata_##name(ad, &data) != 0) exit(100); \
		argdata_main(&data); \
	}

// -------- ANNOTATE --------

#define _ARGSCHEMA_IS_ANNOTATE_doc ,
#define _ARGSCHEMA_IS_ANNOTATE_fd_cap ,

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

#define _ARGSCHEMA_IF(...) _ARGSCHEMA_IF_(__VA_ARGS__)
#define _ARGSCHEMA_IF_(condition, true_part, false_part, ...) false_part
