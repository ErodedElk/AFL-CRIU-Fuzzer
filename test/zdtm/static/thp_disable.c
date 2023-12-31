#include <sys/prctl.h>
#include <sys/mman.h>
#include "zdtmtst.h"
#include "get_smaps_bits.h"

#ifndef MADV_DONTDUMP
#define MADV_DONTDUMP 16
#endif

const char *test_doc = "Test prctl(THP_DISABLE) behaviour";
const char *test_author = "Mike Rapoport <rppt@linux.ibm.com>";

#define MEM_SIZE (2 << 20)

int main(int argc, char **argv)
{
	unsigned long orig_flags = 0, new_flags = 0;
	unsigned long orig_madv = 0, new_madv = 0;
	void *area;
	int ret;

	test_init(argc, argv);

	area = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (area == MAP_FAILED) {
		pr_perror("mmap failed");
		return -1;
	}

	test_msg("Fetch existing flags/adv\n");
	if (get_smaps_bits((unsigned long)area, &orig_flags, &orig_madv))
		return -1;

	if (prctl(PR_SET_THP_DISABLE, 1, 0, 0, 0)) {
		pr_perror("Disabling THP failed");
		return -1;
	}

	ret = prctl(PR_GET_THP_DISABLE, 0, 0, 0, 0);
	if (ret < 0) {
		pr_perror("Getting THP-disabled flag failed");
		return -1;
	}
	if (ret != 1) {
		errno = 0;
		fail("prctl(GET_THP_DISABLE) returned unexpected value: %d != 1", ret);
		return -1;
	}

	test_msg("Fetch pre-migration flags/adv\n");
	if (get_smaps_bits((unsigned long)area, &new_flags, &new_madv))
		return -1;

	errno = 0;
	if (orig_flags != new_flags) {
		fail("Flags changed %lx -> %lx", orig_flags, new_flags);
		return -1;
	}

	if (orig_madv != new_madv) {
		fail("Madvs changed %lx -> %lx", orig_madv, new_madv);
		return -1;
	}

	test_daemon();
	test_waitsig();

	ret = prctl(PR_GET_THP_DISABLE, 0, 0, 0, 0);
	if (ret < 0) {
		pr_perror("Getting post-migration THP-disabled flag failed");
		return -1;
	}
	if (ret != 1) {
		errno = 0;
		fail("post-migration prctl(GET_THP_DISABLE) returned unexpected value: %d != 1", ret);
		return -1;
	}

	if (prctl(PR_SET_THP_DISABLE, 0, 0, 0, 0)) {
		pr_perror("Enabling THP failed");
		return -1;
	}

	test_msg("Fetch restored flags/adv\n");
	if (get_smaps_bits((unsigned long)area, &new_flags, &new_madv))
		return -1;

	errno = 0;
	if (orig_flags != new_flags) {
		fail("Flags changed %lx -> %lx", orig_flags, new_flags);
		return -1;
	}

	if (orig_madv != new_madv) {
		fail("Madvs changed %lx -> %lx", orig_madv, new_madv);
		return -1;
	}

	pass();

	return 0;
}
