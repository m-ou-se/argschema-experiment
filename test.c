#include <argdata.h>
#include <stdio.h>

#include "argschema.h"

//ARGSCHEMA_DEF(libmagic,
//	(int, nmagic)
//	(fd, magicdir)
//)
//
//ARGSCHEMA_DEF(foobar,
//	(int, nthreads)
//	(fd, tmpdir)
//	(seq, (fd), sockets)
//	(fd, logfile)
//	(struct, libmagic, magic_settings)
//	(map, (fd), (struct, libmagic), magic_settings_per_fd)
//)

ARGSCHEMA_DEF(config,
	(fd, output)
	(int, hello)
	(seq, (int), world)
)

void program_main(const argdata_t *ad) {
	struct argdata_config config;

	if (argdata_get_argschema_config(ad, &config) != 0) {
		if (config.has_output) {
			FILE *f = fdopen(config.output, "w");
			fputs("Invalid configuration.\n", f);
			fprintf(f, "Has world: %d\n", config.has_world);
			fprintf(f, "World size: %zu\n", config.world_size);
			fprintf(f, "World: %p\n", config.world);
			for (size_t i = 0; i < config.world_size; ++i) {
				fprintf(f, "World[%zu]: %d.\n", i, config.world[i]);
			}
			argdata_print_yaml(ad, f);
			fclose(f);
		}
		exit(200);
	}

	dprintf(config.output, "Hello: %d.\n", config.hello);
	for (size_t i = 0; i < config.world_size; ++i) {
		dprintf(config.output, "World: %d.\n", config.world[i]);
	}

	exit(0);
}
