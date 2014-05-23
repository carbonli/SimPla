/*
 * octree_forest.h
 *
 *  Created on: 2014年2月21日
 *      Author: salmon
 */

#ifndef OCTREE_FOREST_H_
#define OCTREE_FOREST_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <thread>
#include <iterator>
#include "../fetl/ntuple.h"
#include "../fetl/primitives.h"
#include "../utilities/type_utilites.h"
#include "../utilities/pretty_stream.h"
#include "../utilities/memory_pool.h"

namespace simpla
{

struct OcForest
{

	typedef OcForest this_type;
	static constexpr int MAX_NUM_NEIGHBOUR_ELEMENT = 12;
	static constexpr int MAX_NUM_VERTEX_PER_CEL = 8;
	static constexpr int NDIMS = 3;

	typedef unsigned long size_type;

	typedef size_type compact_index_type;

	struct iterator;

	struct Range;

	typedef nTuple<NDIMS, Real> coordinates_type;

	typedef std::map<iterator, nTuple<3, coordinates_type>> surface_type;

	//!< signed long is 63bit, unsigned long is 64 bit, add a sign bit
	static constexpr unsigned int FULL_DIGITS = std::numeric_limits<compact_index_type>::digits;

	static constexpr unsigned int D_FP_POS = 4; //!< default floating-point position

	static constexpr unsigned int INDEX_DIGITS = (FULL_DIGITS - CountBits<D_FP_POS>::n) / 3;

	static constexpr size_type INDEX_MASK = (1UL << INDEX_DIGITS) - 1;
	static constexpr size_type TREE_ROOT_MASK = ((1UL << (INDEX_DIGITS - D_FP_POS)) - 1) << D_FP_POS;
	static constexpr size_type ROOT_MASK = TREE_ROOT_MASK | (TREE_ROOT_MASK << INDEX_DIGITS)
			| (TREE_ROOT_MASK << (INDEX_DIGITS * 2));

	static constexpr size_type INDEX_ZERO = ((1UL << (INDEX_DIGITS - D_FP_POS - 1)) - 1) << D_FP_POS;

	static constexpr Real R_INDEX_ZERO = static_cast<Real>(INDEX_ZERO);

	static constexpr Real R_INV_DX = static_cast<Real>(1UL << D_FP_POS);
	static constexpr Real R_DX = 1.0 / R_INV_DX;
	//***************************************************************************************************

	static constexpr compact_index_type NO_HEAD_FLAG = ~((~0UL) << (INDEX_DIGITS * 3));
	/**
	 * 	Thanks my wife Dr. CHEN Xiang Lan, for her advice on bitwise operation
	 * 	    H          m  I           m    J           m K
	 * 	|--------|--------------|--------------|-------------|
	 * 	|11111111|00000000000000|00000000000000|0000000000000| <= _MH
	 * 	|00000000|11111111111111|00000000000000|0000000000000| <= _MI
	 * 	|00000000|00000000000000|11111111111111|0000000000000| <= _MJ
	 * 	|00000000|00000000000000|00000000000000|1111111111111| <= _MK
	 *
	 * 	                    I/J/K
	 *  | INDEX_DIGITS------------------------>|
	 *  |  Root------------------->| Leaf ---->|
	 *  |11111111111111111111111111|00000000000| <=_MRI
	 *  |00000000000000000000000001|00000000000| <=_DI
	 *  |00000000000000000000000000|11111111111| <=_MTI
	 *  | Page NO.->| Tree Root  ->|
	 *  |00000000000|11111111111111|11111111111| <=_MASK
	 *
	 */

	static constexpr compact_index_type _DI = 1UL << (D_FP_POS + 2 * INDEX_DIGITS);
	static constexpr compact_index_type _DJ = 1UL << (D_FP_POS + INDEX_DIGITS);
	static constexpr compact_index_type _DK = 1UL << (D_FP_POS);
	static constexpr compact_index_type _DA = _DI | _DJ | _DK;

	//mask of direction
	static constexpr compact_index_type _MI = ((1UL << (INDEX_DIGITS)) - 1) << (INDEX_DIGITS * 2);
	static constexpr compact_index_type _MJ = ((1UL << (INDEX_DIGITS)) - 1) << (INDEX_DIGITS);
	static constexpr compact_index_type _MK = ((1UL << (INDEX_DIGITS)) - 1);
	static constexpr compact_index_type _MH = ((1UL << (FULL_DIGITS - INDEX_DIGITS * 3 + 1)) - 1)
			<< (INDEX_DIGITS * 3 + 1);

	// mask of sub-tree
	static constexpr compact_index_type _MTI = ((1UL << (D_FP_POS)) - 1) << (INDEX_DIGITS * 2);
	static constexpr compact_index_type _MTJ = ((1UL << (D_FP_POS)) - 1) << (INDEX_DIGITS);
	static constexpr compact_index_type _MTK = ((1UL << (D_FP_POS)) - 1);

	// mask of root
	static constexpr compact_index_type _MRI = _MI & (~_MTI);
	static constexpr compact_index_type _MRJ = _MJ & (~_MTJ);
	static constexpr compact_index_type _MRK = _MK & (~_MTK);

//	nTuple<NDIMS, size_type> global_end_ = { 1, 1, 1 };

	unsigned long clock_ = 0;

	static compact_index_type Compact(nTuple<NDIMS, size_type> const & idx)
	{
		return

		(((idx[0]) & INDEX_MASK) << (INDEX_DIGITS * 2)) |

		(((idx[1]) & INDEX_MASK) << (INDEX_DIGITS)) |

		(((idx[2]) & INDEX_MASK));
	}
	static nTuple<NDIMS, size_type> Decompact(compact_index_type s)
	{
		return nTuple<NDIMS, size_type>(
		{

		((s >> (INDEX_DIGITS * 2)) & INDEX_MASK),

		((s >> (INDEX_DIGITS)) & INDEX_MASK),

		(s & INDEX_MASK)

		});
	}
	static nTuple<NDIMS, size_type> DecompactRoot(compact_index_type s)
	{
		CHECK("");
		CHECK_BIT(s);
		return (Decompact(s) - (((1UL << (INDEX_DIGITS - D_FP_POS - 1)) - 1) << D_FP_POS)) >> D_FP_POS;

	}
	//***************************************************************************************************

	OcForest()
	{
	}

	template<typename TDict>
	OcForest(TDict const & dict)
	{
	}

	~OcForest()
	{
	}

	this_type & operator=(const this_type&) = delete;
	OcForest(const this_type&) = delete;

	void swap(OcForest & rhs)
	{
		//FIXME NOT COMPLETE!!
	}

	template<typename TV> using Container=std::shared_ptr<TV>;

	template<int iform, typename TV> inline std::shared_ptr<TV> MakeContainer() const
	{
		return (MEMPOOL.allocate_shared_ptr < TV > (GetLocalNumOfElements(iform)));
	}

	template<typename TDict, typename ...Others>
	void Load(TDict const & dict, Others const& ...)
	{
		if (dict["Dimensions"])
		{
			LOGGER << "Load OcForest ";
			SetDimensions(dict["Dimensions"].template as<nTuple<3, size_type>>());

		}

	}

	std::string Save(std::string const &path) const
	{
		std::stringstream os;

		os << "\tDimensions =  " << GetDimensions();

		return os.str();
	}

	void NextTimeStep()
	{
		++clock_;
	}
	unsigned long GetClock() const
	{
		return clock_;
	}

	//***************************************************************************************************
	// Local Data Set

	nTuple<NDIMS, size_type> global_start_, global_count_;

	nTuple<NDIMS, size_type> local_outer_start_, local_outer_count_;

	nTuple<NDIMS, size_type> local_inner_start_, local_inner_count_;

	nTuple<NDIMS, size_type> hash_stride_ =
	{	0, 0, 0};

	//
	//   |----------------|----------------|---------------|--------------|------------|
	//   ^                ^                ^               ^              ^            ^
	//   |                |                |               |              |            |
	//global          local_outer      local_inner    local_inner    local_outer     global
	// _start          _start          _start           _end           _end          _end
	//

	enum
	{
		FAST_FIRST, SLOW_FIRST
	};

	int array_order_ = SLOW_FIRST;

	template<typename TI>
	void SetDimensions(TI const &d)
	{
		for (int i = 0; i < NDIMS; ++i)
		{
			size_type length = d[i] > 0 ? d[i] : 1;

			ASSERT(length<INDEX_ZERO );

			global_start_[i] = ((INDEX_ZERO >> D_FP_POS) - length / 2);
			global_count_[i] = length;

		}

		local_outer_start_ = global_start_;
		local_outer_count_ = global_count_;

		local_inner_start_ = global_start_;
		local_inner_count_ = global_count_;
		UpdateHash();
	}

	template<typename ... Args>
	void Decompose(Args const & ... args)
	{
		Range range(local_inner_start_, local_inner_count_, 0UL);

		auto res = range.Split2(std::forward<Args const &>(args)...);

		local_outer_start_ = res.first.start_;
		local_outer_count_ = res.first.count_;

		local_inner_start_ = res.second.start_;
		local_inner_count_ = res.second.count_;
		UpdateHash();
	}

	void UpdateHash()
	{
		if (array_order_ == SLOW_FIRST)
		{
			hash_stride_[2] = 1;
			hash_stride_[1] = (local_outer_count_[2]);
			hash_stride_[0] = ((local_outer_count_[1])) * hash_stride_[1];
		}
		else
		{
			hash_stride_[0] = 1;
			hash_stride_[1] = (local_outer_count_[0]);
			hash_stride_[2] = ((local_outer_count_[1])) * hash_stride_[1];
		}
	}

	inline size_type Hash(iterator s) const
	{
		auto d =( Decompact(s.self_ ) >> D_FP_POS);

		size_type res =

		((d[0] - local_outer_start_[0])%local_outer_count_[0]) * hash_stride_[0] +

		((d[1] - local_outer_start_[1])%local_outer_count_[1]) * hash_stride_[1] +

		((d[2] - local_outer_start_[2])%local_outer_count_[2]) * hash_stride_[2];

		switch (NodeId(s.self_))
		{
			case 1:
			case 6:
			res = ((res << 1) + res);
			break;
			case 2:
			case 5:
			res = ((res << 1) + res) + 1;
			break;
			case 4:
			case 3:
			res = ((res << 1) + res) + 2;
			break;
		}

		return res;
	}

	nTuple<NDIMS, size_type> const& GetDimensions() const
	{
		return global_count_;
	}

	size_type GetNumOfElements(int IFORM = VERTEX) const
	{
		return global_count_[0] * global_count_[1] * global_count_[2] * ((IFORM == VERTEX || IFORM == VOLUME) ? 1 : 3);
	}

	nTuple<NDIMS, size_type> const& GetLocalDimensions() const
	{
		return local_outer_count_;
	}
	size_type GetLocalNumOfElements(int IFORM = VERTEX) const
	{
		return local_outer_count_[0] * local_outer_count_[1] * local_outer_count_[2]
		* ((IFORM == VERTEX || IFORM == VOLUME) ? 1 : 3);
	}
	int GetDataSetShape(int IFORM, size_type * global_dims = nullptr, size_type * global_start = nullptr,
	size_type * local_dims = nullptr, size_type * local_start = nullptr, size_type * local_count = nullptr,
	size_type * local_stride = nullptr, size_type * local_block = nullptr) const
	{
		int rank = 0;

		for (int i = 0; i < NDIMS; ++i)
		{
			if ((global_count_[i]) > (global_start_[i]))
			{
				if (global_dims != nullptr)
				global_dims[rank] = (global_count_[i]);

				if (global_start != nullptr)
				global_start[rank] = (local_outer_start_[i] - global_start_[i]);

				if (local_dims != nullptr)
				local_dims[rank] = (local_outer_count_[i]);

				if (local_start != nullptr)
				local_start[rank] = (local_inner_start_[i] - local_outer_start_[i]);

				if (local_count != nullptr)
				local_count[rank] = (local_inner_count_[i]);

				++rank;
			}

		}
		if (IFORM == EDGE || IFORM == FACE)
		{
			if (global_dims != nullptr)
			global_dims[rank] = 3;

			if (global_start != nullptr)
			global_start[rank] = 0;

			if (local_dims != nullptr)
			local_dims[rank] = 3;

			if (local_start != nullptr)
			local_start[rank] = 0;

			if (local_count != nullptr)
			local_count[rank] = 3;

			++rank;
		}
		return rank;
	}

	Range GetRange(int IFORM = VERTEX) const
	{
		compact_index_type shift = 0UL;

		if (IFORM == EDGE)
		{
			shift = (_DI >> 1);

		}
		else if (IFORM == FACE)
		{
			shift = ((_DJ | _DK) >> 1);

		}
		else if (IFORM == VOLUME)
		{
			shift = ((_DI | _DJ | _DK) >> 1);

		}

		return Range(global_start_, global_count_, shift);
	}

	template<int I>
	inline int GetAdjacentCells(Int2Type<I>, Int2Type<I>, iterator s, iterator *v) const
	{
		v[0] = s;
		return 1;
	}

	inline int GetAdjacentCells(Int2Type<EDGE>, Int2Type<VERTEX>, iterator s, iterator *v) const
	{
		v[0] = s + DeltaIndex(s.self_);
		v[1] = s - DeltaIndex(s.self_);
		return 2;
	}

	inline int GetAdjacentCells(Int2Type<FACE>, Int2Type<VERTEX>, iterator s, iterator *v) const
	{
		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   2---------------*
		 *        |  /|              /|
		 *          / |             / |
		 *         /  |            /  |
		 *        3---|-----------*   |
		 *        | m |           |   |
		 *        |   1-----------|---*
		 *        |  /            |  /
		 *        | /             | /
		 *        |/              |/
		 *        0---------------*---> x
		 *
		 *
		 */

		auto di = DeltaIndex(Roate(Dual(s.self_)));
		auto dj = DeltaIndex(InverseRoate(Dual(s.self_)));

		v[0] = s - di - dj;
		v[1] = s - di - dj;
		v[2] = s + di + dj;
		v[3] = s + di + dj;

		return 4;
	}

	inline int GetAdjacentCells(Int2Type<VOLUME>, Int2Type<VERTEX>, iterator const &s, iterator *v) const
	{
		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   6---------------7
		 *        |  /|              /|
		 *          / |             / |
		 *         /  |            /  |
		 *        4---|-----------5   |
		 *        |   |           |   |
		 *        |   2-----------|---3
		 *        |  /            |  /
		 *        | /             | /
		 *        |/              |/
		 *        0---------------1   ---> x
		 *
		 *
		 */
		auto di = _DI >> (HeightOfTree(s.self_) + 1);
		auto dj = _DJ >> (HeightOfTree(s.self_) + 1);
		auto dk = _DK >> (HeightOfTree(s.self_) + 1);

		v[0] = ((s - di) - dj) - dk;
		v[1] = ((s - di) - dj) + dk;
		v[2] = ((s - di) + dj) - dk;
		v[3] = ((s - di) + dj) + dk;

		v[4] = ((s + di) - dj) - dk;
		v[5] = ((s + di) - dj) + dk;
		v[6] = ((s + di) + dj) - dk;
		v[7] = ((s + di) + dj) + dk;

		return 8;
	}

	inline int GetAdjacentCells(Int2Type<VERTEX>, Int2Type<EDGE>, iterator s, iterator *v) const
	{
		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   6---------------7
		 *        |  /|              /|
		 *          2 |             / |
		 *         /  1            /  |
		 *        4---|-----------5   |
		 *        |   |           |   |
		 *        |   2-----------|---3
		 *        3  /            |  /
		 *        | 0             | /
		 *        |/              |/
		 *        0------E0-------1   ---> x
		 *
		 *
		 */

		auto di = _DI >> (HeightOfTree(s.self_) + 1);
		auto dj = _DJ >> (HeightOfTree(s.self_) + 1);
		auto dk = _DK >> (HeightOfTree(s.self_) + 1);

		v[0] = s + di;
		v[1] = s - di;

		v[2] = s + dj;
		v[3] = s - dj;

		v[4] = s + dk;
		v[5] = s - dk;

		return 6;
	}

	inline int GetAdjacentCells(Int2Type<FACE>, Int2Type<EDGE>, iterator s, iterator *v) const
	{

		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   6---------------7
		 *        |  /|              /|
		 *          2 |             / |
		 *         /  1            /  |
		 *        4---|-----------5   |
		 *        |   |           |   |
		 *        |   2-----------|---3
		 *        3  /            |  /
		 *        | 0             | /
		 *        |/              |/
		 *        0---------------1   ---> x
		 *
		 *
		 */
		auto d1 = DeltaIndex(Roate(Dual(s.self_)));
		auto d2 = DeltaIndex(InverseRoate(Dual(s.self_)));
		v[0] = s - d1;
		v[1] = s + d1;
		v[2] = s - d2;
		v[3] = s + d2;

		return 4;
	}

	inline int GetAdjacentCells(Int2Type<VOLUME>, Int2Type<EDGE>, iterator s, iterator *v) const
	{

		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   6------10-------7
		 *        |  /|              /|
		 *         11 |             9 |
		 *         /  7            /  6
		 *        4---|---8-------5   |
		 *        |   |           |   |
		 *        |   2-------2---|---3
		 *        4  /            5  /
		 *        | 3             | 1
		 *        |/              |/
		 *        0-------0-------1   ---> x
		 *
		 *
		 */
		auto di = _DI >> (HeightOfTree(s.self_) + 1);
		auto dj = _DJ >> (HeightOfTree(s.self_) + 1);
		auto dk = _DK >> (HeightOfTree(s.self_) + 1);

		v[0] = (s + di) + dj;
		v[1] = (s + di) - dj;
		v[2] = (s - di) + dj;
		v[3] = (s - di) - dj;

		v[4] = (s + dk) + dj;
		v[5] = (s + dk) - dj;
		v[6] = (s - dk) + dj;
		v[7] = (s - dk) - dj;

		v[8] = (s + di) + dk;
		v[9] = (s + di) - dk;
		v[10] = (s - di) + dk;
		v[11] = (s - di) - dk;

		return 12;
	}

	inline int GetAdjacentCells(Int2Type<VERTEX>, Int2Type<FACE>, iterator s, iterator *v) const
	{
		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   6---------------7
		 *        |  /|              /|
		 *        | / |             / |
		 *        |/  |            /  |
		 *        4---|-----------5   |
		 *        |   |           |   |
		 *        | 0 2-----------|---3
		 *        |  /            |  /
		 *   11   | /      8      | /
		 *      3 |/              |/
		 * -------0---------------1   ---> x
		 *       /| 1
		 *10    / |     9
		 *     /  |
		 *      2 |
		 *
		 *
		 *
		 *              |
		 *          7   |   4
		 *              |
		 *      --------*---------
		 *              |
		 *          6   |   5
		 *              |
		 *
		 *
		 */
		auto di = _DI >> (HeightOfTree(s.self_) + 1);
		auto dj = _DJ >> (HeightOfTree(s.self_) + 1);
		auto dk = _DK >> (HeightOfTree(s.self_) + 1);

		v[0] = (s + di) + dj;
		v[1] = (s + di) - dj;
		v[2] = (s - di) + dj;
		v[3] = (s - di) - dj;

		v[4] = (s + dk) + dj;
		v[5] = (s + dk) - dj;
		v[6] = (s - dk) + dj;
		v[7] = (s - dk) - dj;

		v[8] = (s + di) + dk;
		v[9] = (s + di) - dk;
		v[10] = (s - di) + dk;
		v[11] = (s - di) - dk;

		return 12;
	}

	inline int GetAdjacentCells(Int2Type<EDGE>, Int2Type<FACE>, iterator s, iterator *v) const
	{

		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   6---------------7
		 *        |  /|              /|
		 *        | / |             / |
		 *        |/  |            /  |
		 *        4---|-----------5   |
		 *        |   |           |   |
		 *        |   2-----------|---3
		 *        |  /  0         |  /
		 *        | /      1      | /
		 *        |/              |/
		 * -------0---------------1   ---> x
		 *       /|
		 *      / |   3
		 *     /  |       2
		 *        |
		 *
		 *
		 *
		 *              |
		 *          7   |   4
		 *              |
		 *      --------*---------
		 *              |
		 *          6   |   5
		 *              |
		 *
		 *
		 */

		auto d1 = DeltaIndex(Roate((s.self_)));
		auto d2 = DeltaIndex(InverseRoate((s.self_)));

		v[0] = s - d1;
		v[1] = s + d1;
		v[2] = s - d2;
		v[3] = s + d2;

		return 4;
	}

	inline int GetAdjacentCells(Int2Type<VOLUME>, Int2Type<FACE>, iterator s, iterator *v) const
	{

		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^    /
		 *        |   6---------------7
		 *        |  /|              /|
		 *        | / |    5        / |
		 *        |/  |     1      /  |
		 *        4---|-----------5   |
		 *        | 0 |           | 2 |
		 *        |   2-----------|---3
		 *        |  /    3       |  /
		 *        | /       4     | /
		 *        |/              |/
		 * -------0---------------1   ---> x
		 *       /|
		 *
		 */

		auto di = _DI >> (HeightOfTree(s.self_) + 1);
		auto dj = _DJ >> (HeightOfTree(s.self_) + 1);
		auto dk = _DK >> (HeightOfTree(s.self_) + 1);

		v[0] = s - di;
		v[1] = s + di;

		v[2] = s - di;
		v[3] = s + dj;

		v[4] = s - dk;
		v[5] = s + dk;

		return 6;
	}

	inline int GetAdjacentCells(Int2Type<VERTEX>, Int2Type<VOLUME>, iterator s, iterator *v) const
	{
		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   6---------------7
		 *        |  /|              /|
		 *        | / |             / |
		 *        |/  |            /  |
		 *        4---|-----------5   |
		 *   3    |   |    0      |   |
		 *        |   2-----------|---3
		 *        |  /            |  /
		 *        | /             | /
		 *        |/              |/
		 * -------0---------------1   ---> x
		 *  3    /|       1
		 *      / |
		 *     /  |
		 *        |
		 *
		 *
		 *
		 *              |
		 *          7   |   4
		 *              |
		 *      --------*---------
		 *              |
		 *          6   |   5
		 *              |
		 *
		 *
		 */

		auto di = _DI >> (HeightOfTree(s.self_) + 1);
		auto dj = _DJ >> (HeightOfTree(s.self_) + 1);
		auto dk = _DK >> (HeightOfTree(s.self_) + 1);

		v[0] = ((s - di) - dj) - dk;
		v[1] = ((s - di) - dj) + dk;
		v[2] = ((s - di) + dj) - dk;
		v[3] = ((s - di) + dj) + dk;

		v[4] = ((s + di) - dj) - dk;
		v[5] = ((s + di) - dj) + dk;
		v[6] = ((s + di) + dj) - dk;
		v[7] = ((s + di) + dj) + dk;

		return 8;
	}

	inline int GetAdjacentCells(Int2Type<EDGE>, Int2Type<VOLUME>, iterator s, iterator *v) const
	{

		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^
		 *        |   6---------------7
		 *        |  /|              /|
		 *        | / |             / |
		 *        |/  |            /  |
		 *        4---|-----------5   |
		 *        |   |           |   |
		 *        |   2-----------|---3
		 *        |  /  0         |  /
		 *        | /      1      | /
		 *        |/              |/
		 * -------0---------------1   ---> x
		 *       /|
		 *      / |   3
		 *     /  |       2
		 *        |
		 *
		 *
		 *
		 *              |
		 *          7   |   4
		 *              |
		 *      --------*---------
		 *              |
		 *          6   |   5
		 *              |
		 *
		 *
		 */

		auto d1 = DeltaIndex(Roate((s.self_)));
		auto d2 = DeltaIndex(InverseRoate((s.self_)));

		v[0] = s - d1 - d2;
		v[1] = s + d1 - d2;
		v[2] = s - d1 + d2;
		v[3] = s + d1 + d2;
		return 4;
	}

	inline int GetAdjacentCells(Int2Type<FACE>, Int2Type<VOLUME>, iterator s, iterator *v) const
	{

		/**
		 *
		 *                ^y
		 *               /
		 *        z     /
		 *        ^    /
		 *        |   6---------------7
		 *        |  /|              /|
		 *        | / |             / |
		 *        |/  |            /  |
		 *        4---|-----------5   |
		 *        | 0 |           |   |
		 *        |   2-----------|---3
		 *        |  /            |  /
		 *        | /             | /
		 *        |/              |/
		 * -------0---------------1   ---> x
		 *       /|
		 *
		 */

		auto d = DeltaIndex(Dual(s.self_));
		v[0] = s + d;
		v[1] = s - d;

		return 2;
	}

	//***************************************************************************************************
	//* Auxiliary functions
	//***************************************************************************************************

	static compact_index_type Dual(compact_index_type r)
	{

		return (r & (~(_DA >> (HeightOfTree(r) + 1))))
		| ((~(r & (_DA >> (HeightOfTree(r) + 1)))) & (_DA >> (HeightOfTree(r) + 1)));

	}

	static unsigned int NodeId(compact_index_type r)
	{
		auto s = (r & (_DA >> (HeightOfTree(r) + 1))) >> (D_FP_POS - HeightOfTree(r) - 1);

		return ((s >> (INDEX_DIGITS * 2)) | (s >> (INDEX_DIGITS - 1)) | (s << 2UL)) & (7UL);
	}

	static unsigned int HeightOfTree(compact_index_type r)
	{
		return r >> (INDEX_DIGITS * 3);
	}
	static compact_index_type Roate(compact_index_type r)
	{

		compact_index_type res;

		res = r & (~(_DA >> (HeightOfTree(r) + 1)));

		res |= ((r & ((_DI | _DJ) >> (HeightOfTree(r) + 1))) >> INDEX_DIGITS) |

		((r & (_DK >> (HeightOfTree(r) + 1))) << (INDEX_DIGITS * 2))

		;
		return res;

	}

	/**
	 *  rotate vector direction  mask
	 *  (1/2,0,0) => (0,0,1/2) or   (1/2,1/2,0) => (1/2,0,1/2)
	 * @param s
	 * @return
	 */
	static compact_index_type InverseRoate(compact_index_type r)
	{
		compact_index_type res;

		res = r & ~(_DA >> (HeightOfTree(r) + 1));

		res |= ((r & (_DI >> (HeightOfTree(r) + 1))) >> (INDEX_DIGITS * 2)) |

		((r & (_DJ >> (HeightOfTree(r) + 1))) << INDEX_DIGITS) |

		((r & (_DK >> (HeightOfTree(r) + 1))) << INDEX_DIGITS);

		return res;
	}
	static compact_index_type DeltaIndex(compact_index_type r)
	{
		return (r & (_DA >> (HeightOfTree(r) + 1)));
	}

	static compact_index_type DeltaIndex(unsigned int i,compact_index_type r =0UL)
	{
		return (1UL << (INDEX_DIGITS * (NDIMS - i - 1) + D_FP_POS - HeightOfTree(r) - 1));
	}

	//! get the direction of vector(edge) 0=>x 1=>y 2=>z
	static compact_index_type DirectionOfVector(compact_index_type r)
	{
		compact_index_type s = (r & (_DA >> (HeightOfTree(r) + 1))) >> (D_FP_POS - HeightOfTree(r) - 1);

		return ((s >> (INDEX_DIGITS * 2)) | (s >> (INDEX_DIGITS - 1)) | (s << 2UL)) & (7UL);
	}

	/**
	 * Get component number or vector direction
	 * @param s
	 * @return
	 */
	static size_type ComponentNum(compact_index_type r)
	{
		size_type res = 0;
		switch (NodeId(r))
		{
			case 1:
			case 6:
			res = 0;
			break;
			case 2:
			case 5:
			res = 1;
			break;
			case 4:
			case 3:
			res = 2;
			break;
		}
		return res;
	}

	static size_type IForm(compact_index_type r)
	{
		size_type res = 0;
		switch (NodeId(r))
		{
			case 0:
			res = VERTEX;
			break;
			case 1:
			case 2:
			case 4:
			res = EDGE;
			break;

			case 3:
			case 5:
			case 6:
			res = FACE;
			break;

			case 7:
			res = VOLUME;
		}
		return res;
	}

	//****************************************************************************************************
	//iterator
	//****************************************************************************************************

	struct iterator
	{
/// One of the @link iterator_tags tag types@endlink.
		typedef std::input_iterator_tag iterator_category;

/// The type "pointed to" by the iterator.
		typedef compact_index_type value_type;

/// Distance between iterators is represented as this type.
		typedef typename simpla::OcForest::iterator difference_type;

/// This type represents a pointer-to-value_type.
		typedef value_type* pointer;

/// This type represents a reference-to-value_type.
		typedef value_type& reference;

		compact_index_type self_;

		compact_index_type start_, end_;

		iterator(compact_index_type s = 0, compact_index_type b = 0, compact_index_type e = 0)
		: self_(s), start_(b), end_(e)
		{
		}

		~iterator()
		{
		}

		bool operator==(iterator const & rhs) const
		{
			return self_ == rhs.self_;
		}

		bool operator!=(iterator const & rhs) const
		{
			return !(this->operator==(rhs));
		}

		iterator const & operator*() const
		{
			return *this;
		}

		iterator const* operator ->() const
		{
			return this;
		}

		void NextCell()
		{
			auto D = (1UL << (D_FP_POS - HeightOfTree(self_)));

			self_ += D;

			if ((self_ & _MRK) >= (end_ & _MRK))
			{
				self_ &= ~_MRK;
				self_ |= start_ & _MRK;
				self_ += D << (INDEX_DIGITS);
			}
			if ((self_ & _MRJ) >= (end_ & _MRJ))
			{
				self_ &= ~_MRJ;
				self_ |= start_ & _MRJ;
				self_ += D << (INDEX_DIGITS * 2);
			}

		}

		void PreviousCell()
		{
			auto D = (1UL << (D_FP_POS - HeightOfTree(self_)));

			self_ -= D;

			if ((self_ & _MRK) < (start_ & _MRK))
			{
				self_ &= ~_MRK;
				self_ |= (end_ - D) & _MRK;
				self_ -= D << (INDEX_DIGITS);
			}
			if ((self_ & _MRJ) < (end_ & _MRJ))
			{
				self_ &= ~_MRJ;
				self_ |= (end_ - (D << INDEX_DIGITS)) & _MRK;
				self_ -= D << (INDEX_DIGITS * 2);
			}

		}

		iterator & operator ++()
		{
			auto n = NodeId(self_);

			if (n == 0 || n == 4 || n == 3 || n == 7)
			{
				NextCell();
			}

			self_ = OcForest::Roate(self_);

			return *this;
		}
		iterator operator ++(int)
		{
			iterator res(*this);
			++res;
			return std::move(res);
		}

		iterator & operator --()
		{

			auto n = NodeId(self_);

			if (n == 0 || n == 1 || n == 6 || n == 7)
			{
				PreviousCell();
			}

			self_ = InverseRoate(self_);

			return *this;
		}

		iterator operator --(int)
		{
			iterator res(*this);
			--res;
			return std::move(res);
		}

#define DEF_OP(_OP_)                                                                       \
				inline iterator & operator _OP_##=(compact_index_type r)                           \
				{                                                                                  \
					self_ =  ( (*this) _OP_ r).self_;                                                                \
					return *this ;                                                                  \
				}                                                                                  \
				inline iterator &operator _OP_##=(iterator r)                                   \
				{                                                                                  \
					self_ = ( (*this) _OP_ r).self_;                                                                  \
					return *this;                                                                  \
				}                                                                                  \
		                                                                                           \
				inline iterator  operator _OP_(compact_index_type const &r) const                 \
				{   iterator res(*this);                                                                               \
				   res.self_=(( ( ((self_ _OP_ (r & _MI)) & _MI) |                              \
				                     ((self_ _OP_ (r & _MJ)) & _MJ) |                               \
				                     ((self_ _OP_ (r & _MK)) & _MK)                                 \
				                        )& (NO_HEAD_FLAG) ));   \
			      return res;                                              \
				}                                                                                  \
		                                                                                           \
				inline iterator operator _OP_(iterator r) const                                \
				{                                                                                  \
					return std::move(this->operator _OP_(r.self_));                                               \
				}                                                                                  \

		DEF_OP(+)
		DEF_OP(-)
		DEF_OP(^)
		DEF_OP(&)
		DEF_OP(|)
#undef DEF_OP

		nTuple<NDIMS,size_type> Decompact()const
		{
			return (OcForest::Decompact(self_)>>D_FP_POS)- (OcForest::Decompact(start_)>>D_FP_POS);
		}

	}; // class iterator

	struct Range
	{
	public:
		typedef typename OcForest::iterator iterator;
		typedef iterator value_type;

		nTuple<NDIMS, size_type> start_ =
		{	0, 0, 0}, count_ =
		{	0, 0, 0};
		compact_index_type shift_ = 0UL;

		Range()
		{

		}
		Range(nTuple<NDIMS, size_type> const & start, nTuple<NDIMS, size_type> const& count,
		compact_index_type node_shift = 0UL)
		: start_(start), count_(count), shift_(node_shift)
		{
		}

		~Range()
		{
		}

		iterator begin() const
		{
			return iterator((Compact(start_) << D_FP_POS) | shift_, ((Compact(start_) << D_FP_POS) | shift_),
			((Compact(start_ + count_) << D_FP_POS) | shift_));
		}
		iterator end() const
		{
			iterator res(shift_, shift_, shift_);

			if (count_[0] * count_[1] * count_[2] > 0)
			{
				res = iterator(

				(Compact(start_ + count_ - 1) << D_FP_POS) | shift_,

				((Compact(start_) << D_FP_POS) | shift_),

				((Compact(start_ + count_) << D_FP_POS) | shift_)

				);
				res.NextCell();
			}
			return res;
		}

		iterator rbegin() const
		{
			iterator res(shift_, shift_, shift_);

			if (count_[0] * count_[1] * count_[2] > 0)
			{
				res = iterator(

				(Compact(start_ + count_ -1) << D_FP_POS) | shift_,

				((Compact(start_) << D_FP_POS) | shift_),

				((Compact(start_ + count_) << D_FP_POS) | shift_)

				);
				CHECK((Decompact(res.self_) >> D_FP_POS)-start_);
			}
			return res;
		}
		iterator rend() const
		{
			auto res=begin();
			res.PreviousCell();
			return res;
		}

		nTuple<NDIMS, size_type> const& Extents() const
		{
			return count_;
		}
		size_type Size() const
		{
			return size();
		}
		size_type size() const
		{
			size_type n = 1;

			for (int i = 0; i < NDIMS; ++i)
			{
				n *= count_[i];
			}
			return n;
		}
		template<typename ...Args>
		Range Split(Args const & ... args) const
		{
			return Split2(std::forward<Args const &>(args)...).first;
		}

		std::pair<Range, Range> Split2(unsigned int total, unsigned int sub, unsigned int gw = 0) const
		{
			std::pair<Range, Range> res;
			nTuple<NDIMS, size_type> num_process;
			nTuple<NDIMS, size_type> process_num;
			nTuple<NDIMS, size_type> ghost_width;

			auto extents = Extents();

			bool flag = false;
			for (int i = 0; i < NDIMS; ++i)
			{
				ghost_width[i] = gw;
				if (!flag && (extents[i] > total))
				{
					num_process[i] = total;
					process_num[i] = sub;
					flag = true;
				}
				else
				{
					num_process[i] = 1;
					process_num[i] = 0;
				}
			}
			if (!flag)
			{
				if (sub == 0)
				{
					WARNING << "I'm the master!";
					res =std::pair<Range, Range>(*this,*this);
				}
				else
				{
					WARNING << "Range is too small to split!  ";
				}
			}
			else
			{
				res = Split2(num_process, process_num, ghost_width);
			}

			return res;

		}

		std::pair<Range, Range> Split2(nTuple<NDIMS, size_type> const & num_process,
		nTuple<NDIMS, size_type> const & process_num, nTuple<NDIMS, size_type> const & ghost_width) const
		{

			nTuple<NDIMS, size_type>

			inner_start = start_,

			inner_count = count_,

			outer_start, outer_count;

			for (int i = 0; i < NDIMS; ++i)
			{

				if (2 * ghost_width[i] * num_process[i] > inner_count[i])
				{
					ERROR << "Mesh is too small to decompose! dims[" << i << "]=" << inner_count[i]

					<< " process[" << i << "]=" << num_process[i] << " ghost_width=" << ghost_width[i];
				}
				else
				{

					auto start = (inner_count[i] * process_num[i]) / num_process[i];

					auto end = (inner_count[i] * (process_num[i] + 1)) / num_process[i];

					inner_start[i] += start;
					inner_count[i] = end - start;

					outer_start[i] = inner_start[i];
					outer_count[i] = inner_count[i];

					if (process_num[i] > 0)
					{
						outer_start[i] -= ghost_width[i];
						outer_count[i] += ghost_width[i];

					}
					if (process_num[i] < num_process[i] - 1)
					{
						outer_count[i] += ghost_width[i];

					};

				}
			}

			return std::make_pair(Range(outer_start, outer_count, shift_), Range(inner_start, inner_count, shift_));
		}
	};
	// class Range

	/***************************************************************************************************
	 *
	 *  Geomertry dependence
	 *
	 *  INDEX_ZERO <-> Coordinates Zero
	 *
	 */

	nTuple<NDIMS, Real> GetExtents() const
	{

		nTuple<NDIMS, Real> res;

		res = global_count_<<D_FP_POS;

		return res;
	}

	//***************************************************************************************************
	// Coordinates
	inline coordinates_type GetCoordinates(iterator const& s) const
	{
		auto d = Decompact(s.self_);

		return coordinates_type(
		{

			static_cast<Real>(d[0]-(global_start_[0] << D_FP_POS)) ,

			static_cast<Real>(d[1]-(global_start_[1] << D_FP_POS)) ,

			static_cast<Real>(d[2]-(global_start_[2] << D_FP_POS)) ,

		});
	}

	coordinates_type CoordinatesLocalToGlobal(iterator const& s, coordinates_type r) const
	{
		return GetCoordinates(s) + r * static_cast<Real>(1UL << (D_FP_POS - HeightOfTree(s.self_)));
	}

	inline iterator CoordinatesGlobalToLocalDual(coordinates_type *px, compact_index_type shift = 0UL) const
	{
		return CoordinatesGlobalToLocal(px, shift);
	}

	inline iterator CoordinatesGlobalToLocal(coordinates_type *px, compact_index_type shift = 0UL) const
	{
		auto & x = *px;

		x*=static_cast<Real>(1UL << (D_FP_POS ));

		nTuple<NDIMS, size_type> idx;idx = x;

		unsigned int h = shift >> (INDEX_DIGITS * 3);

		idx -= Decompact(shift);
		idx = idx >> (D_FP_POS - h);
		idx = idx << (D_FP_POS - h);

		x[0] = (x[0] - static_cast<Real>(idx[0]));

		x[1] = (x[1] - static_cast<Real>(idx[1]));

		x[2] = (x[2] - static_cast<Real>(idx[2]));

		return iterator(

		Compact(idx) | shift,

		Compact(local_outer_start_) | shift,

		Compact(local_outer_start_ + local_outer_count_) | shift

		);

	}

	static Real Volume(iterator s)
	{
		static constexpr double volume_[8][D_FP_POS] =
		{

			1, 1, 1, 1, // 000

			1, 1.0 / 2, 1.0 / 4, 1.0 / 8,// 001

			1, 1.0 / 2, 1.0 / 4, 1.0 / 8,// 010

			1, 1.0 / 4, 1.0 / 16, 1.0 / 64,// 011

			1, 1.0 / 2, 1.0 / 4, 1.0 / 8,// 100

			1, 1.0 / 4, 1.0 / 16, 1.0 / 64,// 101

			1, 1.0 / 4, 1.0 / 16, 1.0 / 64,// 110

			1, 1.0 / 8, 1.0 / 32, 1.0 / 128// 111

		};

		return volume_[NodeId(s.self_)][HeightOfTree(s.self_)];
	}

	static Real InvVolume(iterator s)
	{
		static constexpr double inv_volume_[8][D_FP_POS] =
		{

			1, 1, 1, 1, // 000

			1, 2, 4, 8,// 001

			1, 2, 4, 8,// 010

			1, 4, 16, 64,// 011

			1, 2, 4, 8,// 100

			1, 4, 16, 64,// 101

			1, 4, 16, 64,// 110

			1, 8, 32, 128// 111

		};

		return inv_volume_[NodeId(s.self_)][HeightOfTree(s.self_)];
	}

//	static Real Volume(iterator s)
//	{
//		static constexpr double volume_[8][D_FP_POS] =
//		{
//
//			1, 1, 1, 1, // 000
//
//			8, 4, 2, 1,// 001
//
//			8, 4, 2, 1,// 010
//
//			64, 16, 4, 1,// 011
//
//			8, 4, 2, 1,// 100
//
//			64, 16, 4, 1,// 101
//
//			64, 16, 4, 1,// 110
//
//			128, 32, 8, 1// 111
//
//		};
//
//		return volume_[NodeId(s.self_)][HeightOfTree(s.self_)];
//	}
//
//	static Real InvVolume(iterator s)
//	{
//		static constexpr double inv_volume_[8][D_FP_POS] =
//		{
//
//			1, 1, 1, 1, // 000
//
//			1.0 / 8, 1.0 / 4, 1.0 / 2, 1.0,// 001
//
//			1.0 / 8, 1.0 / 4, 1.0 / 2, 1.0,// 010
//
//			1.0 / 64, 1.0 / 16, 1.0 / 4, 1.0,// 011
//
//			1.0 / 8, 1.0 / 4, 1.0 / 2, 1.0,// 100
//
//			1.0 / 64, 1.0 / 16, 1.0 / 4, 1.0,// 101
//
//			1.0 / 64, 1.0 / 16, 1.0 / 4, 1.0,// 110
//
//			1.0 / 128, 1.0 / 32, 1.0 / 8, 1.0// 111
//
//		};
//
//		return inv_volume_[NodeId(s.self_)][HeightOfTree(s.self_)];
//	}

	static Real InvDualVolume(iterator s)
	{
		return InvVolume(Dual(s.self_));
	}
	static Real DualVolume(iterator s)
	{
		return Volume(Dual(s.self_));
	}

};
// class OcForest

}
// namespace simpla

#endif /* OCTREE_FOREST_H_ */
