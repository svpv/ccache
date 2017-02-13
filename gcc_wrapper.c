#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __GLIBC__
#include <error.h>
#else
#include <libgen.h>
#define PROG basename(argv[0])
#define program_invocation_short_name PROG
#define error(status, errnum, func)	\
do {					\
	fprintf(stderr, "%s: %s: %m\n", PROG, func); \
	if (status) exit(status);	\
} while (0)
#endif

struct assoc {
	const char *name;
	const char *value;
} table[] = {
	{ "c++", "g++" },
	{ "cc", "gcc" },
	{ "f77", "gfortran" },
	{ "f95", "gfortran" },
	{ "g77", "gfortran" },
	{ "gcc_wrapper", "gcc" },
	{ "jar", "fastjar" },
	{ "rmic", "grmic" },
	{ "rmiregistry", "grmiregistry" },
	{ "tree1", "gtreelang" }
};

int
main(int ac, char **argv)
{
	char   *suffix = "", *target_name, *bin_name, *path;
	const char *call_name = program_invocation_short_name;
	const char *version = getenv("GCC_VERSION");
	const char *target = getenv("GCC_TARGET");
	const char *use_ccache = getenv("GCC_USE_CCACHE");
	size_t i;

	for (i = 0; i < sizeof(table)/sizeof(table[0]); ++i)
		if (!strcmp(call_name, table[i].name))
		{
			call_name = table[i].value;
			break;
		}

	if (version)
	{
		while (target && *target && isspace(*target))
			target++;

		while (*version && isspace(*version))
			version++;
		if (*version)
			if (asprintf(&suffix, "-%s", version) < 0)
				error(EXIT_FAILURE, errno, "asprintf");
	}

	if (!target || !*target)
		target = TARGET;

	if (asprintf(&target_name, "%s-%s%s", target, call_name, suffix) < 0)
		error(EXIT_FAILURE, errno, "asprintf");

	bin_name = target_name;
	if (use_ccache)
	{
		while (*use_ccache && isspace(*use_ccache))
			use_ccache++;
		if (*use_ccache)
			bin_name = "ccache";
	}

	if (asprintf(&path, "%s/%s", BINDIR, bin_name) < 0)
		error(EXIT_FAILURE, errno, "asprintf");

	argv[0] = target_name;
	execv(path, argv);
	perror(path);
	return EXIT_FAILURE;
}
