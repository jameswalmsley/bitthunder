/**
 * BitThunder String functions, to override the cludge you get from libc!
 *
 */

void *memcpy(void *dest, const void *src, size_t size) {
	char *tmp = dest;
	const char *s = src;

	while (size--) {
		*tmp++ = *s++;
	}
	return dest;
}

