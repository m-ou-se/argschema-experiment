#define ARGSCHEMA_DEF(name, def) \
	asm ( \
		".pushsection argschema." #name ", \"a\"\n" \
		"_argschema_" #name ":\n" \
		".global _argschema_" #name "\n" \
		_ARGSCHEMA_ITERATE(_ARGSCHEMA_ASM_1 def) \
		".popsection\n" \
	); \
	struct argdata_##name { \
		_ARGSCHEMA_ITERATE(_ARGSCHEMA_STRUCT_1 def) \
	}; \
	static inline void argdata_parse_##name( \
		const argdata_t *data, \
		struct argdata_##name *result) { \
		_ARGSCHEMA_ITERATE(_ARGSCHEMA_PARSE_1 def) \
	}

#define _ARGSCHEMA_STRUCT_int(name) \
	int name;
#define _ARGSCHEMA_STRUCT_fd(name) \
	int name;
#define _ARGSCHEMA_STRUCT_struct(type, name) \
	struct argdata_##type name;
#define _ARGSCHEMA_STRUCT_array(type, name) \
	_ARGSCHEMA_STRUCT(_ARGSCHEMA_UNWRAP type, *name) \
	size_t name##_size;

#define _ARGSCHEMA_UNWRAP(...) __VA_ARGS__
#define _ARGSCHEMA_STRUCT(type, ...) _ARGSCHEMA_STRUCT2(type, __VA_ARGS__)
#define _ARGSCHEMA_STRUCT2(type, ...) _ARGSCHEMA_STRUCT_##type(__VA_ARGS__)

#define _ARGSCHEMA_ASM_int(name) \
	".byte 0\n" \
	".asciz \"" #name "\"\n"
#define _ARGSCHEMA_ASM_fd(name) \
	".byte 1\n" \
	".asciz \"" #name "\"\n"
#define _ARGSCHEMA_ASM_struct(type, name) \
	".byte 2\n" \
	".quad _argschema_" #type "\n" \
	".asciz \"" #name "\"\n"
#define _ARGSCHEMA_ASM_array(name, type, ...) \
	".byte 3\n" \
	_ARGSCHEMA_ASM_##type(,__VA_ARGS__)

#define _ARGSCHEMA_PARSE_int(name) \
	result->name = argdata_get_int_by_name(data, #name);
#define _ARGSCHEMA_PARSE_fd(name) \
	result->name = argdata_get_fd_by_name(data, #name);
#define _ARGSCHEMA_PARSE_struct(name, type) \
	argdata_parse_##type(argdata_get_map_by_name(data, #name), &result->name);
#define _ARGSCHEMA_PARSE_array(name, type, ...) \
	result->name##_size = argdata_get_array_size(data, #name); \
	for (size_t i = 0; i < result->name##_size; ++i) { \
		_ARGSCHEMA_PARSE_##type(name[i], __VA_ARGS__) \
	}

#define ARGSCHEMA_USE(name) \
	extern char _argschema_##name[]; \
	void *volatile __argschema = _argschema_##name;

#define _ARGSCHEMA_ITERATE(...) _ARGSCHEMA_ITERATE2(__VA_ARGS__)
#define _ARGSCHEMA_ITERATE2(...) __VA_ARGS__ ## _END

#define _ARGSCHEMA_STRUCT_1(type, ...) _ARGSCHEMA_STRUCT_##type(__VA_ARGS__) _ARGSCHEMA_STRUCT_2
#define _ARGSCHEMA_STRUCT_2(type, ...) _ARGSCHEMA_STRUCT_##type(__VA_ARGS__) _ARGSCHEMA_STRUCT_1
#define _ARGSCHEMA_STRUCT_1_END
#define _ARGSCHEMA_STRUCT_2_END

#define _ARGSCHEMA_ASM_1(type, ...) _ARGSCHEMA_ASM_##type(__VA_ARGS__) _ARGSCHEMA_ASM_2
#define _ARGSCHEMA_ASM_2(type, ...) _ARGSCHEMA_ASM_##type(__VA_ARGS__) _ARGSCHEMA_ASM_1
#define _ARGSCHEMA_ASM_1_END
#define _ARGSCHEMA_ASM_2_END

#define _ARGSCHEMA_PARSE_1(type, ...) _ARGSCHEMA_PARSE_##type(__VA_ARGS__) _ARGSCHEMA_PARSE_2
#define _ARGSCHEMA_PARSE_2(type, ...) _ARGSCHEMA_PARSE_##type(__VA_ARGS__) _ARGSCHEMA_PARSE_1
#define _ARGSCHEMA_PARSE_1_END
#define _ARGSCHEMA_PARSE_2_END
