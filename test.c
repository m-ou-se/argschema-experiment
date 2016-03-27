#include <argdata.h>
#include <stdio.h>
#include <sys/capsicum.h>

#include "argschema.h"

ARGSCHEMA_DEF(point2d,
	(int, x)
	(int, y)
)

ARGSCHEMA_DEF(config,
	(fd, output)
	(fd_cap, CAP_WRITE)
	(doc, "The file descriptor used to write all log messages to.")

	(str, name)

	(struct, point2d, origin)

	(map, (str), (struct, point2d), points)
)

ARGSCHEMA_USE(config)

void argdata_main(const struct argdata_config *config) {

	dprintf(config->output, "Hello %s!\n", config->name.str);

	dprintf(
		config->output, "Origin at (%d, %d).\n",
		config->origin.x, config->origin.y);

	for (size_t i = 0; i < config->points.size; ++i) {
		dprintf(
			config->output, "Point %s at (%d, %d).\n",
			config->points.keys[i].str,
			config->points.values[i].x, config->points.values[i].y);
	}

	exit(0);
}
