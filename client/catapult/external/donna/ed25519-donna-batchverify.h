/*
	Ed25519 batch verification
*/

#ifndef ED25519_DONNA_BATCHVERIFY_H
#define ED25519_DONNA_BATCHVERIFY_H

#include "ed25519-donna.h"
#include "ed25519.h"

#define max_batch_size 64
#define heap_batch_size ((max_batch_size * 2) + 1)

/* which limb is the 128th bit in? */
static const size_t limb128bits = (128 + bignum256modm_bits_per_limb - 1) / bignum256modm_bits_per_limb;

typedef size_t heap_index_t;

typedef struct batch_heap_t {
	unsigned char r[heap_batch_size][16]; /* 128 bit random values */
	ge25519 points[heap_batch_size];
	bignum256modm scalars[heap_batch_size];
	heap_index_t heap[heap_batch_size];
	size_t size;
} batch_heap;

/* swap two values in the heap */
static void
heap_swap(heap_index_t *heap, size_t a, size_t b) {
	heap_index_t temp;
	temp = heap[a];
	heap[a] = heap[b];
	heap[b] = temp;
}

/* add the scalar at the end of the list to the heap */
static void
heap_insert_next(batch_heap *heap) {
	size_t node = heap->size, parent;
	heap_index_t *pheap = heap->heap;
	bignum256modm *scalars = heap->scalars;

	/* insert at the bottom */
	pheap[node] = (heap_index_t)node;

	/* sift node up to its sorted spot */
	parent = (node - 1) / 2;
	while (node && lt256_modm_batch(scalars[pheap[parent]], scalars[pheap[node]], bignum256modm_limb_size - 1)) {
		heap_swap(pheap, parent, node);
		node = parent;
		parent = (node - 1) / 2;
	}
	heap->size++;
}

/* update the heap when the root element is updated */
static void
heap_updated_root(batch_heap *heap, size_t limbsize) {
	size_t node, parent, childr, childl;
	heap_index_t *pheap = heap->heap;
	bignum256modm *scalars = heap->scalars;

	/* sift root to the bottom */
	parent = 0;
	node = 1;
	childl = 1;
	childr = 2;
	while ((childr < heap->size)) {
		node = lt256_modm_batch(scalars[pheap[childl]], scalars[pheap[childr]], limbsize) ? childr : childl;
		heap_swap(pheap, parent, node);
		parent = node;
		childl = (parent * 2) + 1;
		childr = childl + 1;
	}

	/* sift root back up to its sorted spot */
	parent = (node - 1) / 2;
	while (node && lte256_modm_batch(scalars[pheap[parent]], scalars[pheap[node]], limbsize)) {
		heap_swap(pheap, parent, node);
		node = parent;
		parent = (node - 1) / 2;
	}
}

/* build the heap with count elements, count must be >= 3 */
static void
heap_build(batch_heap *heap, size_t count) {
	heap->heap[0] = 0;
	heap->size = 0;
	while (heap->size < count)
		heap_insert_next(heap);
}

/* extend the heap to contain new_count elements */
static void
heap_extend(batch_heap *heap, size_t new_count) {
	while (heap->size < new_count)
		heap_insert_next(heap);
}

/* get the top 2 elements of the heap */
static void
heap_get_top2(batch_heap *heap, heap_index_t *max1, heap_index_t *max2, size_t limbsize) {
	heap_index_t h0 = heap->heap[0], h1 = heap->heap[1], h2 = heap->heap[2];
	if (lt256_modm_batch(heap->scalars[h1], heap->scalars[h2], limbsize))
		h1 = h2;
	*max1 = h0;
	*max2 = h1;
}

/* */
static void
ge25519_multi_scalarmult_vartime_final(ge25519 *r, ge25519 *point, bignum256modm scalar) {
	const bignum256modm_element_t topbit = ((bignum256modm_element_t)1 << (bignum256modm_bits_per_limb - 1));
	size_t limb = limb128bits;
	bignum256modm_element_t flag;

	if (isone256_modm_batch(scalar)) {
		/* this will happen most of the time after bos-carter */
		*r = *point;
		return;
	} else if (iszero256_modm_batch(scalar)) {
		/* this will only happen if all scalars == 0 */
		memset(r, 0, sizeof(*r));
		r->y[0] = 1;
		r->z[0] = 1;
		return;
	}

	*r = *point;

	/* find the limb where first bit is set */
	while (!scalar[limb])
		limb--;

	/* find the first bit */
	flag = topbit;
	while ((scalar[limb] & flag) == 0)
		flag >>= 1;

	/* exponentiate */
	for (;;) {
		ge25519_double(r, r);
		if (scalar[limb] & flag)
			ge25519_add(r, r, point);

		flag >>= 1;
		if (!flag) {
			if (!limb--)
				break;
			flag = topbit;
		}
	}
}

/* count must be >= 5 */
static void
ge25519_multi_scalarmult_vartime(ge25519 *r, batch_heap *heap, size_t count) {
	heap_index_t max1, max2;

	/* start with the full limb size */
	size_t limbsize = bignum256modm_limb_size - 1;

	/* whether the heap has been extended to include the 128 bit scalars */
	int extended = 0;

	/* grab an odd number of scalars to build the heap, unknown limb sizes */
	heap_build(heap, ((count + 1) / 2) | 1);

	for (;;) {
		heap_get_top2(heap, &max1, &max2, limbsize);

		/* only one scalar remaining, we're done */
		if (iszero256_modm_batch(heap->scalars[max2]))
			break;

		/* exhausted another limb? */
		if (!heap->scalars[max1][limbsize])
			limbsize -= 1;

		/* can we extend to the 128 bit scalars? */
		if (!extended && isatmost128bits256_modm_batch(heap->scalars[max1])) {
			heap_extend(heap, count);
			heap_get_top2(heap, &max1, &max2, limbsize);
			extended = 1;
		}

		sub256_modm_batch(heap->scalars[max1], heap->scalars[max1], heap->scalars[max2], limbsize);
		ge25519_add(&heap->points[max2], &heap->points[max2], &heap->points[max1]);
		heap_updated_root(heap, limbsize);
	}

	ge25519_multi_scalarmult_vartime_final(r, &heap->points[max1], heap->scalars[max1]);
}

static int
ge25519_is_neutral_vartime(const ge25519 *p) {
	static const unsigned char zero[32] = {0};

	unsigned char point_buffer[3][32];
	curve25519_contract(point_buffer[0], p->x);
	curve25519_contract(point_buffer[1], p->y);
	curve25519_contract(point_buffer[2], p->z);
	return (memcmp(point_buffer[0], zero, 32) == 0) && (memcmp(point_buffer[1], point_buffer[2], 32) == 0);
}

#endif // ED25519_DONNA_BATCHVERIFY_H
