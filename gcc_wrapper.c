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
	int cacheable;
	const char *name;
	const char *value;
} table[] = {
	{ 1, "c++", "g++" },
	{ 1, "cc", "gcc" },
	{ 1, "f77", "gfortran" },
	{ 1, "f95", "gfortran" },
	{ 1, "g77", "gfortran" },
	{ 1, "gcc_wrapper", "gcc" },
	{ 0, "jar", "fastjar" },
	{ 0, "rmic", "grmic" },
	{ 0, "rmiregistry", "grmiregistry" },
	{ 0, "tree1", "gtreelang" }
};

int
main(int argc, char **argv)
{
	char   *suffix = "", *target_name, *bin_name, *path;
	const char *call_name = program_invocation_short_name;
	const char *version = getenv("GCC_VERSION");
	const char *target = getenv("GCC_TARGET");
	const char *use_ccache = getenv("GCC_USE_CCACHE");
	int cacheable = 0;
	size_t i;

	for (i = 0; i < sizeof(table)/sizeof(table[0]); ++i)
		if (!strcmp(call_name, table[i].name))
		{
			call_name = table[i].value;
			cacheable = table[i].cacheable;
			break;
		}
		else if (!strcmp(call_name, table[i].value))
		{
			cacheable = table[i].cacheable;
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

	argv[0] = bin_name = target_name;
	if (use_ccache && cacheable)
	{
		while (*use_ccache && isspace(*use_ccache))
			use_ccache++;
		if (*use_ccache) {
			int ccache_main(int argc, char **argv);
			return ccache_main(argc, argv);
		}
	}

	if (asprintf(&path, "%s/%s", BINDIR, bin_name) < 0)
		error(EXIT_FAILURE, errno, "asprintf");

	execv(path, argv);
	perror(path);
	return EXIT_FAILURE;
}
