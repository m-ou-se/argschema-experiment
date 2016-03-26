#include <argdata.h>
#include <stdbool.h>
#include <stddef.h>

static inline int argdata_get_seq_size(
	const argdata_t *ad,
	size_t *size) {
	*size = 0;
	argdata_iterator_t it = argdata_seq_iterator(ad);
	while (argdata_seq_iterator_next(&it)) ++*size;
	return it.error;
}

static inline int argdata_get_map_size(
	const argdata_t *ad,
	size_t *size) {
	*size = 0;
	argdata_iterator_t it = argdata_map_iterator(ad);
	while (argdata_map_iterator_next(&it)) ++*size;
	return it.error;
}
