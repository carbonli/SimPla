/**
 * spParticle.c
 *
 *  Created on: 2016年6月15日
 *      Author: salmon
 */
#include <stdlib.h>
#include <string.h>
#include "sp_lite_def.h"
#include "spParallel.h"

#include "spMesh.h"
#include "spPage.h"
#include "spParticle.h"

void spParticleCreate(const spMesh *mesh, spParticle **sp)
{
	*sp = (spParticle *) malloc(sizeof(spParticle));

	(*sp)->m = mesh;
	(*sp)->iform = VERTEX;
	(*sp)->number_of_attrs = 0;
	(*sp)->data = NULL;
	(*sp)->m_page_pool_ = NULL;
	(*sp)->m_pages_holder = NULL;
	(*sp)->buckets = NULL;

	ADD_PARTICLE_ATTRIBUTE((*sp), struct spParticlePoint_s, int, flag);
	ADD_PARTICLE_ATTRIBUTE((*sp), struct spParticlePoint_s, Real, rx);
	ADD_PARTICLE_ATTRIBUTE((*sp), struct spParticlePoint_s, Real, ry);
	ADD_PARTICLE_ATTRIBUTE((*sp), struct spParticlePoint_s, Real, rz);

}
MC_GLOBAL void spParticleDeployKernel(spPage *pg, void *data, size_type entity_size_in_byte, size_t NUM_PAGE_PER_CELL)
{
	for (int pos = spParallelBlockNum() * NUM_PAGE_PER_CELL;
			pos < spParallelBlockNum() * NUM_PAGE_PER_CELL + NUM_PAGE_PER_CELL; ++pos)
	{
		pg[pos].next = &(pg[pos + 1]);
		pg[pos].data = data + entity_size_in_byte * SP_NUMBER_OF_ENTITIES_IN_PAGE * pos;
	}

}
void spParticleDeploy(spParticle *sp, int PIC)
{
	size_type number_of_cell = spMeshGetNumberOfEntity(sp->m, 3/*volume*/);

	size_type num_page_per_cel = (size_type) (PIC * 3 / SP_NUMBER_OF_ENTITIES_IN_PAGE) / 2;
	sp->max_number_of_pages = number_of_cell * num_page_per_cel;

	sp->entity_size_in_byte = (size_type) (sp->attrs[sp->number_of_attrs - 1].size_in_byte
			+ sp->attrs[sp->number_of_attrs - 1].offsetof);

	sp->page_size_in_byte = sizeof(struct spPage_s) + sp->number_of_attrs * sizeof(void *);

	spParallelDeviceMalloc((void **) (&(sp->m_pages_holder)), sp->max_number_of_pages);

	spParallelDeviceMalloc((void **) (&(sp->buckets)), sizeof(spPage*) * number_of_cell);

	spParallelMemset(sp->buckets, 0x0, sizeof(spPage*) * number_of_cell);

	spParallelDeviceMalloc((&(sp->data)),
			sp->entity_size_in_byte * sp->max_number_of_pages * SP_NUMBER_OF_ENTITIES_IN_PAGE);


	spParticleDeployKernel<<< sp->m->dims,NUMBER_OF_THREADS_PER_BLOCK>>>(sp->m_pages_holder,sp->data,
			sp->entity_size_in_byte,num_page_per_cel );
	spParallelDeviceSync();        //wait for iteration to finish

	printf("======   spParticleDeployKernel Done ======\n");

	sp->m_page_pool_ = sp->m_pages_holder;
	// sp->m_pages_holder;
	// sp->m_page_pool_[sp->max_number_of_pages].next = NULL;

}

void spParticleDestroy(spParticle **sp)
{
	spParallelDeviceFree((*sp)->data);
	spParallelDeviceFree((void *) &((*sp)->m_page_pool_));
	spParallelDeviceFree((void *) ((*sp)->buckets));
	spParallelDeviceFree((void *) ((*sp)->m_pages_holder));

	free(*sp);
	*sp = NULL;
}

struct spParticleAttrEntity_s *spParticleAddAttribute(spParticle *pg, char const *name, int type_tag,
		size_type size_in_byte, int offset)
{
	struct spParticleAttrEntity_s *res = &(pg->attrs[pg->number_of_attrs]);
	strcpy(res->name, name);
	res->type_tag = type_tag;
	res->size_in_byte = size_in_byte;
	if (offset == -1)
	{
		if (pg->number_of_attrs == 0)
		{
			offset = 0;
		}
		else
		{
			offset = pg->attrs[pg->number_of_attrs - 1].offsetof + pg->attrs[pg->number_of_attrs - 1].size_in_byte;
		}
	}
	res->offsetof = offset;
	++pg->number_of_attrs;
	return res;
}

void *spParticleGetAttribute(spParticle *sp, char const *name)
{
	void *res = NULL;
	for (int i = 0; i < sp->number_of_attrs; ++i)
	{
		if (strcmp(sp->attrs[i].name, name) > 0)
		{
			res = (byte_type*) (sp->data)
					+ sp->attrs[i].offsetof * sp->max_number_of_pages * SP_NUMBER_OF_ENTITIES_IN_PAGE;
		}
	}

	return res;
}

void spParticleWrite(spParticle const *f, spIOStream *os, const char name[], int flag)
{
//	char curr_path[2048];
//
//	spIOStreamPWD(os, curr_path);
//	spIOStreamOpen(os, name);
//
//	spIOStreamOpen(os, curr_path);

}

void spParticleRead(spParticle *f, char const url[], int flag)
{

}

void spParticleSync(spParticle *f)
{
}
//
//MC_DEVICE int spPageInsert(spPage **dest, spPage **pool, int *d_tail, int *g_d_tail)
//{
//	while (1)
//	{
//		if ((*dest) != NULL)
//		{
//			while ((*d_tail = spAtomicAdd(g_d_tail, 1)) < SP_NUMBER_OF_ENTITIES_IN_PAGE)
//			{
//				if ((P_GET_FLAG((*dest)->data, *d_tail).v == 0))
//				{
//					break;
//				}
//			}
//		}
//
//		spParallelSyncThreads();
//		if ((*dest) == NULL)
//		{
//
//			if (spParallelThreadNum() == 0)
//			{
//				*dest = *pool;
//				*pool = (*pool)->next;
//				if (*dest != NULL)
//				{
//					(*dest)->next = NULL;
//				}
//				*g_d_tail = 0;
//			}
//
//		}
//		else if (*d_tail >= SP_NUMBER_OF_ENTITIES_IN_PAGE)
//		{
//			dest = &((*dest)->next);
//			if (*dest != NULL)
//			{
//				(*g_d_tail) = 0;
//			}
//
//		}
//		spParallelSyncThreads();
//
//		if (*dest == NULL)
//		{
//			return SP_MP_ERROR_POOL_IS_OVERFLOW;
//		}
//	}
//
//	return SP_MP_FINISHED;
//}
//
//MC_DEVICE int spPageScan(spPage **dest, int *d_tail, int *g_d_tail, int MASK, int tag)
//{
//	int THREAD_ID = spParallelThreadNum();
//
//	while ((*dest) != NULL)
//	{
//		while ((*d_tail = spAtomicAdd(g_d_tail, 1)) < SP_NUMBER_OF_ENTITIES_IN_PAGE)
//		{
//			if ((P_GET_FLAG((*dest)->data, *d_tail).v & MASK == tag & MASK))
//			{
//				return SP_MP_SUCCESS;
//			}
//		}
//
//		spParallelSyncThreads();
//
//		dest = &((*dest)->next);
//
//		if (THREAD_ID == 0)
//		{
//			g_d_tail = 0;
//		}
//
//		spParallelSyncThreads();
//
//	}
//	return SP_MP_FINISHED;
//}
//
//#define SP_MAP(_P_DEST_, _POS_DEST_, _P_SRC_, _POS_SRC_, _ENTITY_SIZE_IN_BYTE_)   spParallelMemcpy(_P_DEST_ + _POS_DEST_ * _ENTITY_SIZE_IN_BYTE_,_P_SRC_ + _POS_SRC_ * _ENTITY_SIZE_IN_BYTE_,   _ENTITY_SIZE_IN_BYTE_);
//
//MC_DEVICE void spUpdateParticleSortThreadKernel(spPage **dest, spPage const **src, spPage **pool,
//		size_type entity_size_in_byte, int MASK, MeshEntityId tag)
//{
//
//	MC_SHARED
//	int g_d_tail, g_s_tail;
//
//	spParallelSyncThreads();
//
//	if (spParallelThreadNum() == 0)
//	{
//		g_s_tail = 0;
//		g_d_tail = 0;
//	}
//	spParallelSyncThreads();
//
//	for (int d_tail = 0, s_tail = 0;
//			spPageMapAndPack(dest, src, &d_tail, &g_d_tail, &s_tail, &g_s_tail, /* pool, MASK,*/tag) != SP_MP_FINISHED;)
//	{
////		SP_MAP(byte_type*)((*dest)->data), d_tail, (byte_type*) ((*src)->data), s_tail, entity_size_in_byte);
//	}
//
//}

MC_DEVICE int spParticleMapAndPack(spPage **dest, spPage const **src, int *d_tail, int *g_d_tail, int *s_tail,
		int *g_s_tail, spPage **pool, MeshEntityId tag)
{

	while (*src != NULL)
	{
		if ((*dest) != NULL)
		{
			while ((*d_tail = spAtomicAdd(g_d_tail, 1)) < SP_NUMBER_OF_ENTITIES_IN_PAGE)
			{
				if ((P_GET_FLAG((*dest)->data, *d_tail).v == 0))
				{
					break;
				}
			}

			if (*d_tail < SP_NUMBER_OF_ENTITIES_IN_PAGE)
			{
				while (((*s_tail = spAtomicAdd(g_s_tail, 1)) < SP_NUMBER_OF_ENTITIES_IN_PAGE))
				{
					if (P_GET_FLAG((*src)->data, *s_tail).v == tag.v)
					{
						return SP_MP_SUCCESS;
					}
				}
			}
		}
		spParallelSyncThreads();
		if ((*dest) == NULL)
		{

			if (spParallelThreadNum() == 0)
			{
				*dest = *pool;
				*pool = (*pool)->next;
				if (*dest != NULL)
				{
					(*dest)->next = NULL;
				}
				*g_d_tail = 0;
			}
		}
		else if (*d_tail >= SP_NUMBER_OF_ENTITIES_IN_PAGE)
		{
			dest = &((*dest)->next);
			if (spParallelThreadNum() == 0)
			{
				*g_d_tail = 0;
			}
		}
		else if (*s_tail >= SP_NUMBER_OF_ENTITIES_IN_PAGE)
		{
			src = (spPage const **) &((*src)->next);
			if (spParallelThreadNum() == 0)
			{
				*g_s_tail = 0;
			}
		}

		spParallelSyncThreads();

		if (*dest == NULL)
		{
			return SP_MP_ERROR_POOL_IS_OVERFLOW;
		}
	}

	return SP_MP_FINISHED;

}
