#include "argschema.h"

ARGSCHEMA_DEF(libmagic,
	(int, nmagic)
	(fd, magicdir)
)

ARGSCHEMA_DEF(foobar,
	(int, nthreads)
	(fd, tmpdir)
	(array, (fd), sockets)
	(fd, logfile)
	(array, (struct, libmagic), magic_settings)
)

int main() {
	ARGSCHEMA_USE(foobar)
}

