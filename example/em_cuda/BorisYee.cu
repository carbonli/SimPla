//
// Created by salmon on 16-6-14.
//

#include "Boris.h"
#include <stdio.h>
#include "../../sp_lite/sp_def.h"
#include "../../sp_lite/spField.h"
#include "../../sp_lite/spMesh.h"
#include "../../sp_lite/spParticle.h"
#include "../../sp_lite/spPage.h"

#include "../../sp_lite/spMesh.cu"
#include "../../sp_lite/spField.cu"
#include "../../sp_lite/spParticle.cu"

/******************************************************************************************/

__global__ void spInitializeParticle_BorisYee_Kernel(spMesh *ctx, spPage** buckets, spPage * pool,
		size_type entity_size_in_byte)
{

	index_type g_x = (blockIdx.x);
	index_type g_y = (blockIdx.y);
	index_type g_z = (blockIdx.z);

	size_type g_dim_x = gridDim.x;
	size_type g_dim_y = gridDim.y;
	size_type g_dim_z = gridDim.z;

	index_type t_x = (threadIdx.x);
	index_type t_y = (threadIdx.y);
	index_type t_z = (threadIdx.z);
	size_type t_dim_x = blockDim.x;
	size_type t_dim_y = blockDim.y;
	size_type t_dim_z = blockDim.z;

	size_type g_num = g_x + (g_y + g_z * g_dim_y) * g_dim_x;
	size_type t_num = t_x + (t_y + t_z * t_dim_y) * t_dim_x;

	buckets[g_num] = &pool[g_num];
	if (t_num == 0)
	{
		buckets[g_num]->next = 0x0;
		buckets[g_num]->flag = -1;
	}

	boris_point_s*p = (boris_point_s*) (buckets[g_num]->data + (t_num * entity_size_in_byte));

	p->r[0] = 0.5;
	p->r[1] = 0.5;
	p->r[2] = 0.5;

	p->v[0] = 0.5;
	p->v[1] = 0.5;
	p->v[2] = 0.5;

	p->f = 1.0;
	p->f = 1.0;

}

void spInitializeParticle_BorisYee(spMesh *ctx, sp_particle_type *sp, size_type NUM_OF_PIC)
{

//	cudaStream_t s_shared[ctx->number_of_shared_blocks];
//
//	for (int i = 0, ie = ctx->number_of_shared_blocks; i < ie; ++i)
//	{
//		cudaStreamCreate(&s_shared[i]);
//		spInitializeParticle_BorisYee_Kernel<<<ctx->shared_blocks[i], ctx->threadsPerBlock, 0, s_shared[i]>>>(
//				(spMesh *) spObject_device_((spObject*) ctx), (sp_particle_type *) spObject_device_((spObject*) pg),
//				NUM_OF_PIC);
//	}

	cudaStream_t s_local;
	cudaStreamCreate(&s_local);
	spInitializeParticle_BorisYee_Kernel<<<ctx->private_block, ctx->threadsPerBlock, 0, s_local>>>(
			(spMesh *) spObject_device_((spObject*) ctx), sp->buckets, sp->m_pages, sp->entity_size_in_byte);

//	for (int i = 0, ie = ctx->number_of_shared_blocks; i < ie; ++i)
//	{
//		cudaStreamSynchronize(s_shared[i]); //wait for boundary
//
//	}
//
//	spSyncParticle(ctx, pg);

	cudaDeviceSynchronize(); //wait for iteration to finish
}

/******************************************************************************************/

//__device__ double atomicAddD(double* address, double val)
//{
//	unsigned long long int* address_as_ull = (unsigned long long int*) address;
//	unsigned long long int old = *address_as_ull, assumed;
//	do
//	{
//		assumed = old;
//		old = atomicCAS(address_as_ull, assumed, __double_as_longlong(val + __longlong_as_double(assumed)));
//		// Note: uses integer comparison to avoid hang in case of NaN (since NaN !=		NaN	)
//	} while (assumed != old);
//	return __longlong_as_double(old);
//}
#define ll 0
#define rr 1.0
#define CACHE_EXTENT_X 4
#define CACHE_EXTENT_Y 4
#define CACHE_EXTENT_Z 4
#define CACHE_SIZE (CACHE_EXTENT_X*CACHE_EXTENT_Y*CACHE_EXTENT_Z)
#define IX  1
#define IY  CACHE_EXTENT_X
#define IZ  CACHE_EXTENT_X*CACHE_EXTENT_Y
__device__
void cache_gather(Real *v, Real const f[], Real const *r0, const Real *r1)
{
	Real r[3] =
	{ r0[0] - r1[0], r0[1] - r1[1], r0[2] - r1[2] };
	id_type s = (int) (r[0]) * IX + (int) (r[1]) * IY + (int) (r[2]) * IZ;

	*v = f[s + IX + IY + IZ /*  */] * (r[0] - ll) * (r[1] - ll) * (r[2] - ll)
			+ f[s + IX + IY /*     */] * (r[0] - ll) * (r[1] - ll) * (rr - r[2])
			+ f[s + IX + IZ /*     */] * (r[0] - ll) * (rr - r[1]) * (r[2] - ll)
			+ f[s + IX /*          */] * (r[0] - ll) * (rr - r[1]) * (rr - r[2])
			+ f[s + IY + IZ /*     */] * (rr - r[0]) * (r[1] - ll) * (r[2] - ll)
			+ f[s + IY /*          */] * (rr - r[0]) * (r[1] - ll) * (rr - r[2])
			+ f[s + IZ /*          */] * (rr - r[0]) * (rr - r[1]) * (r[2] - ll)
			+ f[s /*               */] * (rr - r[0]) * (rr - r[1]) * (rr - r[2]);
}
__device__
void cache_scatter(Real *f, Real v, Real const *r0, Real const *r1)
{
	Real r[3] =
	{ r0[0] - r1[0], r0[1] - r1[1], r0[2] - r1[2] };

	atomicAdd(&(f[ IX + IY + IZ /*  */]), 1.0); // * (r[0] - ll) * (r[1] - ll) * (r[2] - ll));
	atomicAdd(&(f[ IX + IY /*       */]), 1.0); // * (r[0] - ll) * (r[1] - ll) * (rr - r[2]));
	atomicAdd(&(f[ IX + IZ /*       */]), 1.0); // * (r[0] - ll) * (rr - r[1]) * (r[2] - ll));
	atomicAdd(&(f[ IX /*            */]), 1.0); // * (r[0] - ll) * (rr - r[1]) * (rr - r[2]));
	atomicAdd(&(f[ IY + IZ /*       */]), 1.0); // * (rr - r[0]) * (r[1] - ll) * (r[2] - ll));
	atomicAdd(&(f[ IY /*            */]), 1.0); // * (rr - r[0]) * (r[1] - ll) * (rr - r[2]));
	atomicAdd(&(f[ IZ /*            */]), 1.0); // * (rr - r[0]) * (rr - r[1]) * (r[2] - ll));
	atomicAdd(&(f[0 /*             */]), 1.0); // * (rr - r[0]) * (rr - r[1]) * (rr - r[2]));

}

#undef ll
#undef rr
#undef IX
#undef IY
#undef IZ
#define _R 1.0
MC_CONSTANT Real id_to_shift_[][3] =
{ //
		{ 0, 0, 0 },           // 000
				{ _R, 0, 0 },           // 001
				{ 0, _R, 0 },           // 010
				{ 0, 0, _R },          // 011
				{ _R, _R, 0 },           // 100
				{ _R, 0, _R },          // 101
				{ 0, _R, _R },          // 110
				{ 0, _R, _R },          // 111
		};
MC_CONSTANT int sub_index_to_id_[4][3] =
{ //
		{ 0, 0, 0 }, /*VERTEX*/
		{ 1, 2, 4 }, /*EDGE*/
		{ 6, 5, 3 }, /*FACE*/
		{ 7, 7, 7 } /*VOLUME*/

		};
MC_CONSTANT int cache_cell_offset_tag[CACHE_SIZE] =
{ };
MC_CONSTANT size_type cache_cell_offset[CACHE_SIZE] =
{ };
#undef _R

__device__
inline void spBorisPushOne(struct boris_point_s const *p0, struct boris_point_s *p1, Real dt, Real q, Real m,
		Real const *tE[3], Real const * tB[3], Real *tJ[4], const Real *inv_dx)
{

	Real E[3], B[3];

	cache_gather(&E[0], tE[0], p0->r, id_to_shift_[sub_index_to_id_[1/*EDGE*/][0]]);
	cache_gather(&E[1], tE[1], p0->r, id_to_shift_[sub_index_to_id_[1/*EDGE*/][1]]);
	cache_gather(&E[2], tE[2], p0->r, id_to_shift_[sub_index_to_id_[1/*EDGE*/][2]]);

	cache_gather(&B[0], tB[0], p0->r, id_to_shift_[sub_index_to_id_[2/*FACE*/][0]]);
	cache_gather(&B[1], tB[1], p0->r, id_to_shift_[sub_index_to_id_[2/*FACE*/][1]]);
	cache_gather(&B[2], tB[2], p0->r, id_to_shift_[sub_index_to_id_[2/*FACE*/][2]]);

	p1->r[0] = p0->r[0] + p0->v[0] * dt * 0.5 * inv_dx[0];
	p1->r[1] = p0->r[1] + p0->v[1] * dt * 0.5 * inv_dx[1];
	p1->r[2] = p0->r[2] + p0->v[2] * dt * 0.5 * inv_dx[2];

	Real v_[3], t[3];

	t[0] = B[0] * (q / m * dt * 0.5);
	t[1] = B[1] * (q / m * dt * 0.5);
	t[2] = B[2] * (q / m * dt * 0.5);

	p1->v[0] = p0->v[0] + E[0] * (q / m * dt * 0.5);
	p1->v[1] = p0->v[1] + E[1] * (q / m * dt * 0.5);
	p1->v[2] = p0->v[2] + E[2] * (q / m * dt * 0.5);

	v_[0] = p1->v[0] + (p1->v[1] * t[2] - p1->v[2] * t[1]);
	v_[1] = p1->v[1] + (p1->v[2] * t[0] - p1->v[0] * t[2]);
	v_[2] = p1->v[2] + (p1->v[0] * t[1] - p1->v[1] * t[0]);

	Real tt = t[0] * t[0] + t[1] * t[1] + t[2] * t[2] + 1.0;

	p1->v[0] += (v_[1] * t[2] - v_[2] * t[1]) * 2.0 / tt;
	p1->v[1] += (v_[2] * t[0] - v_[0] * t[2]) * 2.0 / tt;
	p1->v[2] += (v_[0] * t[1] - v_[1] * t[0]) * 2.0 / tt;

	p1->v[0] += E[0] * (q / m * dt * 0.5);
	p1->v[1] += E[1] * (q / m * dt * 0.5);
	p1->v[2] += E[2] * (q / m * dt * 0.5);

	p1->r[0] += p1->v[0] * dt * 0.5 * inv_dx[0];
	p1->r[1] += p1->v[1] * dt * 0.5 * inv_dx[1];
	p1->r[2] += p1->v[2] * dt * 0.5 * inv_dx[2];

	cache_scatter(tJ[0], p1->f * p1->w * q, p1->r, id_to_shift_[sub_index_to_id_[0/*VERTEX*/][0]]);
	cache_scatter(tJ[1], p1->f * p1->w * p1->v[0] * q, p1->r, id_to_shift_[sub_index_to_id_[1/*EDGE*/][0]]);
	cache_scatter(tJ[2], p1->f * p1->w * p1->v[1] * q, p1->r, id_to_shift_[sub_index_to_id_[1/*EDGE*/][1]]);
	cache_scatter(tJ[3], p1->f * p1->w * p1->v[2] * q, p1->r, id_to_shift_[sub_index_to_id_[1/*EDGE*/][2]]);

}

/******************************************************************************************/

#define RADIUS 2
#define BUFFER_SIZE 2*RADUIS*2*RADUIS*2*RADUIS
__global__ void spUpdateParticle_BorisYee_Kernel(spMesh *m, Real dt, Real charge, Real mass,
		size_type entity_size_in_byte, spPage** buckets, const Real *fE, const Real *fB, Real *fRho, Real *fJ)
{

	int g_x = (blockIdx.x);
	int g_y = (blockIdx.y);
	int g_z = (blockIdx.z);

	int g_dim_x = gridDim.x;
	int g_dim_y = gridDim.y;
	int g_dim_z = gridDim.z;

	int t_x = (threadIdx.x);
	int t_y = (threadIdx.y);
	int t_z = (threadIdx.z);
	int t_dim_x = blockDim.x;
	int t_dim_y = blockDim.y;
	int t_dim_z = blockDim.z;

	int total_g_num = g_dim_x * g_dim_y * g_dim_z;

	int g_num = g_x + (g_y + g_z * g_dim_y) * g_dim_x;
	int t_num = t_x + (t_y + t_z * t_dim_y) * t_dim_x;

	int t_max_num = 64;

	MC_SHARED Real ttE[3][CACHE_SIZE], ttB[3][CACHE_SIZE], ttJ[4][CACHE_SIZE];

	int CACHE_SHIFT = RADIUS + (RADIUS + RADIUS * g_dim_y) * g_dim_x;

	Real * tE[3], *tB[3], *tJ[4];
	tE[0] = ttE[0] + CACHE_SHIFT;
	tE[1] = ttE[1] + CACHE_SHIFT;
	tE[2] = ttE[2] + CACHE_SHIFT;

	tB[0] = ttB[0] + CACHE_SHIFT;
	tB[1] = ttB[1] + CACHE_SHIFT;
	tB[2] = ttB[2] + CACHE_SHIFT;

	tJ[0] = ttJ[0] + CACHE_SHIFT;
	tJ[1] = ttJ[1] + CACHE_SHIFT;
	tJ[2] = ttJ[2] + CACHE_SHIFT;
	tJ[3] = ttJ[3] + CACHE_SHIFT;

	int g_f_x = (g_x + t_x + g_dim_x - RADIUS) % g_dim_x;
	int g_f_y = (g_y + t_y + g_dim_y - RADIUS) % g_dim_y;
	int g_f_z = (g_z + t_z + g_dim_z - RADIUS) % g_dim_z;
	int g_f_num = g_f_x + (g_f_y + g_f_z * g_dim_y) * g_dim_x;
	{
#ifndef DEFAULT_SoA
		ttE[0][t_num] = fE[g_f_num * 3 + 0];
		ttE[1][t_num] = fE[g_f_num * 3 + 1];
		ttE[2][t_num] = fE[g_f_num * 3 + 2];

		ttB[0][t_num] = fB[g_f_num * 3 + 0];
		ttB[1][t_num] = fB[g_f_num * 3 + 1];
		ttB[2][t_num] = fB[g_f_num * 3 + 2];

#else
		ttE[0][t_num] = fE[g_f_num ];
		ttE[1][t_num] = fE[g_f_num + total_g_num];
		ttE[2][t_num] = fE[g_f_num + total_g_num*2];

		ttE[0][t_num] = fE[g_f_num ];
		ttE[1][t_num] = fE[g_f_num + total_g_num];
		ttE[2][t_num] = fE[g_f_num + total_g_num*2];

#endif
		ttJ[0][t_num] = 0;
		ttJ[1][t_num] = 0;
		ttJ[2][t_num] = 0;
		ttJ[3][t_num] = 0;

	}
	MC_SHARED int buffer_tail;
	MC_SHARED byte_type *p_buffer;

	if (t_num == 0)
	{
		p_buffer = (byte_type*) malloc(boris_buffer_s_DEPTH * entity_size_in_byte);
		buffer_tail = 0;
	}

	__syncthreads();

	spPage *src = buckets[g_f_num];

	int pos;
// FIXME DANGAURE!!! if buffer overflow then particle will be abandoned.
	while ((pos = atomicAdd(&buffer_tail, 1)) < boris_buffer_s_DEPTH)
	{
		bucket_entity_flag_t tag = 0x1;
		bucket_entity_flag_t check_flag = 0x1;
		byte_type *p0 = src->data;
		byte_type *p1 = p_buffer + pos * entity_size_in_byte;
		if ((src->flag & tag != 0) && (((spEntity_s*) p0)->tag & 0x3F) == check_flag)
		{
			spBorisPushOne((struct boris_point_s const *) p0, (struct boris_point_s *) p1, dt, charge, mass, tE, tB, tJ,
					m->inv_dx);

		}

		tag <<= 1;
		p0 += entity_size_in_byte;

	}        //	for (int n = 0; n < CACHE_SIZE; ++n)
	__syncthreads();
	if (t_num == 0)
	{
		free(p_buffer);
	}

	atomicAdd(&(fRho[g_f_num]), ttJ[0][t_num]);
	atomicAdd(&(fJ[g_f_num * 3 + 0]), ttJ[1][t_num]);
	atomicAdd(&(fJ[g_f_num * 3 + 1]), ttJ[2][t_num]);
	atomicAdd(&(fJ[g_f_num * 3 + 2]), ttJ[3][t_num]);

}

void spUpdateParticle_BorisYee(spMesh *ctx, Real dt, sp_particle_type *pg, const sp_field_type *fE,
		const sp_field_type *fB, sp_field_type *fRho, sp_field_type *fJ)
{
//	cudaStream_t s1;
//	cudaStreamCreate(&s1);
//	cudaStream_t s2;
//	cudaStreamCreate(&s2);
//
//	spUpdateParticle_BorisYee_Kernel<<<ctx->numBlocks, ctx->threadsPerBlock, 0, s1>>>(
//			(spMesh *) spObject_device_((spObject*) ctx),
//			dt, //
//			(sp_particle_type *) spObject_device_((spObject*) pg),
//			(const sp_field_type *) spObject_device_((spObject*) fE),
//			(const sp_field_type *) spObject_device_((spObject*) fB),
//			(sp_field_type *) spObject_device_((spObject*) fRho), (sp_field_type *) spObject_device_((spObject*) fJ));
//
	spUpdateParticle_BorisYee_Kernel<<<ctx->private_block, ctx->threadsPerBlock>>>(
			(spMesh *) spObject_device_((spObject*) ctx), dt, pg->charge, pg->mass, pg->entity_size_in_byte,   //
			pg->buckets, fE->device_data, fB->device_data, fRho->device_data, fJ->device_data);
//	cudaStreamSynchronize(s1); //wait for boundary
//
//	spSyncParticle(ctx, pg);
//	spSyncField(ctx, fJ);
//	spSyncField(ctx, fRho);
//	cudaDeviceSynchronize(); //wait for iteration to finish

}
/***************************************************************************************************************/
//__global__ void spUpdateField_Yee_kernel(spMesh *ctx, Real dt, const sp_field_type *fRho, const sp_field_type *fJ,
//		sp_field_type *fE, sp_field_type *fB)
__global__ void spUpdateField_Yee_kernel(spMesh *ctx, Real dt, const Real *fRho, const Real *fJ, Real *fE, Real *fB)
{
	index_type ix = (blockIdx.x * blockDim.x + threadIdx.x);
	index_type iy = (blockIdx.y * blockDim.y + threadIdx.y);
	index_type iz = (blockIdx.z * blockDim.z + threadIdx.z);
	size_type dim_x = gridDim.x * blockDim.x;
	size_type dim_y = gridDim.y * blockDim.y;
	size_type dim_z = gridDim.z * blockDim.z;

	int n = ix + (iy + iz * dim_y) * dim_z;

	(fE)[n * 3 + 0] = ix;
	(fE)[n * 3 + 1] = iy;
	(fE)[n * 3 + 2] = iz;

}
void spUpdateField_Yee(spMesh *ctx, Real dt, const sp_field_type *fRho, const sp_field_type *fJ, sp_field_type *fE,
		sp_field_type *fB)
{

//	cudaStream_t s_shared[ctx->number_of_shared_blocks];
//
//	for (int i = 0, ie = ctx->number_of_shared_blocks; i < ie; ++i)
//	{
//		cudaStreamCreate(&s_shared[i]);
//
//		spUpdateField_Yee_kernel<<<ctx->shared_blocks[i], ctx->threadsPerBlock, 0, s_shared[i]>>>(
//				(spMesh *) spObject_device_((spObject*) ctx),
//				dt, //
//				(const sp_field_type *) spObject_device_((spObject*) fRho),
//				(const sp_field_type *) spObject_device_((spObject*) fJ),
//				(sp_field_type *) spObject_device_((spObject*) fE), (sp_field_type *) spObject_device_((spObject*) fB));
//	}
//	cudaStream_t s_local;
//	cudaStreamCreate(&s_local);
//
//	spUpdateField_Yee_kernel<<<ctx->private_block, 1>>>((spMesh *) spObject_device_((spObject*) ctx),
//			dt, //
//			(const sp_field_type *) spObject_device_((spObject*) fRho),
//			(const sp_field_type *) spObject_device_((spObject*) fJ),
//			(sp_field_type *) spObject_device_((spObject*) fE), (sp_field_type *) spObject_device_((spObject*) fB));

	dim3 grid_dim = ctx->private_block;
	grid_dim.x = ctx->private_block.x / ctx->threadsPerBlock.x;
	grid_dim.y = ctx->private_block.y / ctx->threadsPerBlock.y;
	grid_dim.z = ctx->private_block.z / ctx->threadsPerBlock.z;
	spUpdateField_Yee_kernel<<<grid_dim, ctx->threadsPerBlock>>>((spMesh *) spObject_device_((spObject*) ctx),
			dt, //
			((Real*) fRho->device_data), ((Real*) fJ->device_data), ((Real*) fE->device_data),
			((Real*) fB->device_data));
//	for (int i = 0, ie = ctx->number_of_shared_blocks; i < ie; ++i)
//	{
//		cudaStreamSynchronize(s_shared[i]); //wait for boundary
//	}
//
//	spSyncField(ctx, fE);
//	spSyncField(ctx, fB);

	cudaDeviceSynchronize(); //wait for iteration to finish

}

