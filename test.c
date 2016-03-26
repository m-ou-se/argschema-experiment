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
	(int, world)
)

void program_main(const argdata_t *ad) {
	struct argdata_config config;

	if (argdata_get_argschema_config(ad, &config) != 0) {
		if (config.has_output) {
			dprintf(config.output, "Invalid configuration.\n");
		}
		exit(200);
	}

	dprintf(config.output, "Hello is %d and world is %d.\n", config.hello, config.world);

	exit(0);
}
