#ifndef _CACHE_POOL_H_
#define _CACHE_POOL_H_

#include <iostream>
#include <cstdlib>
#include <list>

#include "LRUCache.h"

/**
 * Simple cache pool for allocating memory of fixed size
 */
class CachePool : public LRUCache
{
public:

	CachePool(int l_,long int size_) : l(l_), size(size_)
	{
		head = (head_t *)calloc(l,sizeof(head_t));  // initialized to 0
		size /= sizeof(Qfloat);
		size -= l * sizeof(head_t) / sizeof(Qfloat);
		size = std::max(size, 2 * (long int) l); // cache must be large enough for two columns
		lru_head.next = lru_head.prev = &lru_head;

		pool = (Qfloat *)malloc(size *sizeof(Qfloat)); // pre-allocate block of memory
		if (pool == NULL) {
			std::cerr << "FATAL: cache pool unable to allocate " << size * sizeof(Qfloat) << " bytes\n";
			exit(1);
		}
		std::cerr << "Cache pool allocated " << size * sizeof(Qfloat) << " bytes\n";
		next_pos = 0;
	}

    virtual ~CachePool()
	{
		free(pool); // free entire pre-allocated block
		free(head);
	}

   	/**
	 * Gets a column of height len from the cache pool 
	 * @return 0 if it is new memory that needs to be filled, or len if memory is already filled
	 */
    int get_data(const int index, Qfloat **data, int len) 
	{
		int rc = len;
		head_t *h = &head[index];
		if(h->len) lru_delete(h);

		if(h->len == 0)
		{
			rc = 0;
			if (len > size) // reuse an existing column space
			{
				head_t *old = lru_head.next;
				lru_delete(old);

				if (old->len != len) {
					std::cerr << "Fixed sized cache pool can only allocate fixed size blocks!\n";
					exit(1);
				}

				h->data = old->data;
				h->len = old->len;

				old->data = NULL;
				old->len = 0;
			} else {
				h->data = &pool[next_pos]; // allocate from pool
				h->len = len;

				next_pos += len;
				size -= len;
			}
		}

		lru_insert(h);
		*data = h->data;
		return rc;
	}

    void swap_index(int i, int j)
	{
		std::cerr << "Fixed size cache pool cannot be used with shrinking!\n";
		exit (1);
	}

private:
	int l;
	long int size;
	long int next_pos;

	Qfloat *pool;

	struct head_t
	{
		head_t *prev, *next;    // a circular list
		Qfloat *data;
		int len;        
	};
    
	head_t *head;
	head_t lru_head;

	void lru_delete(head_t *h)
	{
		// delete from current location
		h->prev->next = h->next;
		h->next->prev = h->prev;
	}

	void lru_insert(head_t *h)
	{
		// insert to last position
		h->next = &lru_head;
		h->prev = lru_head.prev;
		h->prev->next = h;
		h->next->prev = h;
	}
};

#endif
