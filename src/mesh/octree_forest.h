/*
 * octree_forest.h
 *
 *  Created on: 2014年2月21日
 *      Author: salmon
 */

#ifndef OCTREE_FOREST_H_
#define OCTREE_FOREST_H_

#include <algorithm>
#include <cmath>
#include <limits>

#include "../fetl/ntuple.h"
#include "../utilities/type_utilites.h"

namespace simpla
{

struct OcForest
{

	typedef OcForest this_type;
	static constexpr int MAX_NUM_VERTEX_PER_CEL = 8;
	static constexpr int NUM_OF_DIMS = 3;

	typedef unsigned long size_type;
	typedef unsigned long compact_index_type;

	/**
	 * 	Thanks my wife Dr. CHEN Xiang Lan, for her advice on  these bitwise operation
	 * 	               m            m             m
	 * 	|--------|------------|--------------|-------------|
	 * 	               I              J             K
	 */
	//!< signed long is 63bit, unsigned long is 64 bit, add a sign bit
	static constexpr unsigned int FULL_DIGITS = std::numeric_limits<compact_index_type>::digits;

	static constexpr unsigned int MAX_TREE_HEIGHT = 4;

	static constexpr unsigned int INDEX_DIGITS = (FULL_DIGITS - CountBits<MAX_TREE_HEIGHT>::n) / 3;

	static constexpr unsigned int DIGITS_HEAD = FULL_DIGITS - INDEX_DIGITS * 3;

	static constexpr size_type INDEX_MAX = static_cast<size_type>(((1L) << (INDEX_DIGITS)) - 1);

	static constexpr size_type INDEX_MIN = 0;

	static constexpr double dh = 1.0 / static_cast<double>(INDEX_MAX + 1);

	struct index_type
	{
		size_type H :DIGITS_HEAD;
		size_type I :INDEX_DIGITS;
		size_type J :INDEX_DIGITS;
		size_type K :INDEX_DIGITS;

#define DEF_OP(_OP_)                                                   \
		inline index_type operator _OP_##=(index_type const &r)        \
		{                                                              \
			H = std::max(H, r.H);                                      \
			I _OP_##= r.I;                                             \
			J _OP_##= r.J;                                             \
			K _OP_##= r.K;                                             \
			return *this;                                              \
		}                                                              \
        inline index_type operator _OP_ (index_type const &r)const     \
		{                                                              \
			index_type t;                                              \
			t.H = std::max(H, r.H);                                    \
			t.I = I _OP_ r.I;                                          \
			t.J = J _OP_ r.J;                                          \
			t.K = K _OP_ r.K;                                          \
			return t;                                                  \
		}                                                              \
		inline index_type operator _OP_##=(compact_index_type  r )     \
		{    this->operator _OP_##=(_C(r));  return *this;      }      \
		inline index_type operator _OP_ (compact_index_type  r)const   \
		{   return std::move(this->operator _OP_ (_C(r)));    }

		DEF_OP(+)
		DEF_OP(-)
		DEF_OP(^)
		DEF_OP(&)
		DEF_OP(|)
#undef DEF_OP

	};

	nTuple<3, unsigned int> index_digits_ = { INDEX_DIGITS - MAX_TREE_HEIGHT, INDEX_DIGITS - MAX_TREE_HEIGHT,
	        INDEX_DIGITS - MAX_TREE_HEIGHT };

	compact_index_type _MI = 0UL;
	compact_index_type _MJ = 0UL;
	compact_index_type _MK = 0UL;
	compact_index_type _MA = _MI | _MJ | _MK;

	//  public:
	OcForest()
	{
		Update();
	}
	OcForest(nTuple<3, unsigned int> const & d)
	{
		SetDimensions(d);
		Update();
	}

	~OcForest()
	{
	}
	this_type & operator=(const this_type&) = delete;

	static compact_index_type &_C(index_type &s)
	{
		return *reinterpret_cast<compact_index_type *>(&s);
	}

	static compact_index_type const &_C(index_type const &s)
	{
		return *reinterpret_cast<compact_index_type const*>(&s);
	}

	void SetDimensions(nTuple<3, unsigned int> const & d)
	{
		index_digits_[0] = count_bits(d[0]) - 1;
		index_digits_[1] = count_bits(d[1]) - 1;
		index_digits_[2] = count_bits(d[2]) - 1;
		Update();
	}
	nTuple<3, unsigned int> GetDimensions() const
	{
		return nTuple<3, unsigned int>( { 1U << index_digits_[0], 1U << index_digits_[1], 1U << index_digits_[2] });

	}
	void Update()
	{
		_MI = _C(index_type( { 0, 1U << (INDEX_DIGITS - index_digits_[0]), 0, 0 }));
		_MJ = _C(index_type( { 0, 0, 1U << (INDEX_DIGITS - index_digits_[1]), 0 }));
		_MK = _C(index_type( { 0, 0, 0, 1U << (INDEX_DIGITS - index_digits_[2]) }));
		_MA = _MI | _MJ | _MK;
	}

	static index_type &_C(compact_index_type &s)
	{
		return *reinterpret_cast<index_type *>(&s);
	}

	static index_type const &_C(compact_index_type const &s)
	{
		return *reinterpret_cast<index_type const*>(&s);
	}

	inline size_type HashRootIndex(index_type s, size_type const strides[3]) const
	{

		return (

		(s.I >> (INDEX_DIGITS - index_digits_[0])) * strides[0] +

		(s.J >> (INDEX_DIGITS - index_digits_[1])) * strides[1] +

		(s.K >> (INDEX_DIGITS - index_digits_[2])) * strides[2]

		);

	}

	inline size_type HashIndex(index_type s, size_type const strides[3]) const
	{

		return (

		(s.I >> (INDEX_DIGITS - index_digits_[0] - s.H + 1)) * strides[0] +

		(s.J >> (INDEX_DIGITS - index_digits_[1] - s.H + 1)) * strides[1] +

		(s.K >> (INDEX_DIGITS - index_digits_[2] - s.H + 1)) * strides[2]

		);

	}

	inline index_type GetIndex(nTuple<3, Real> const & x, unsigned int H = 0) const
	{
		index_type res;

		ASSERT(0<=x[0] && x[0]<=1.0);

		ASSERT(0<=x[1] && x[1]<=1.0);

		ASSERT(0<=x[2] && x[2]<=1.0);

		res.H = H;

		res.I = static_cast<size_type>(std::floor(x[0] * static_cast<Real>(INDEX_MAX + 1)))
		        & ((~0UL) << (INDEX_DIGITS - index_digits_[0] - H));

		res.J = static_cast<size_type>(std::floor(x[1] * static_cast<Real>(INDEX_MAX + 1)))
		        & ((~0UL) << (INDEX_DIGITS - index_digits_[1] - H));

		res.K = static_cast<size_type>(std::floor(x[2] * static_cast<Real>(INDEX_MAX + 1)))
		        & ((~0UL) << (INDEX_DIGITS - index_digits_[2] - H));

		return std::move(res);
	}

	inline nTuple<3, Real> GetCoordinates(index_type const & s) const
	{

		return nTuple<3, Real>( {

		static_cast<Real>(s.I) * dh,

		static_cast<Real>(s.J) * dh,

		static_cast<Real>(s.K) * dh

		});

	}

//***************************************************************************************************

	/**
	 *  rotate vector direction  mask
	 *  (1/2,0,0) => (0,1/2,0) or   (1/2,1/2,0) => (0,1/2,1/2)
	 * @param s
	 * @return
	 */
	compact_index_type _R(compact_index_type s) const
	{
		s >>= (_C(s).H + 1);
		return (_MJ & (s & _MI)) | (_MK & (s & _MJ)) | (_MI & (s & _MK));
	}

	compact_index_type _R(index_type s) const
	{
		return std::move(_R(_C(s)));
	}

	/**
	 *  rotate vector direction  mask
	 *  (1/2,0,0) => (0,0,1/2) or   (1/2,1/2,0) => (1/2,0,1/2)
	 * @param s
	 * @return
	 */
	compact_index_type _RR(compact_index_type s) const
	{
		s >>= (_C(s).H + 1);
		return (_MK & (s & _MI)) | (_MI & (s & _MJ)) | (_MJ & (s & _MK));
	}

	compact_index_type _RR(index_type s) const
	{
		return std::move(_RR(_C(s)));
	}

	compact_index_type _I(compact_index_type s) const
	{
		return _MA & (~((s >> (_C(s).H + 1)) & _MA));
	}

	compact_index_type _I(index_type s) const
	{
		return std::move(_I(_C(s)));
	}

	//! get the direction of vector(edge) 0=>x 1=>y 2=>z
	size_type _N(index_type s) const
	{
		return ((s.J >> (INDEX_DIGITS - index_digits_[1] - s.H - 1)) & 1UL) |

		((s.K >> (INDEX_DIGITS - index_digits_[1] - s.H - 2)) & 2UL);
	}
	size_type _N(compact_index_type s) const
	{
		return std::move(_N(_C(s)));
	}
	template<int I>
	inline int GetAdjacentCells(Int2Type<I>, Int2Type<I>, index_type s, index_type *v) const
	{
		v[0] = s;
		return 1;
	}

	inline int GetAdjacentCells(Int2Type<EDGE>, Int2Type<VERTEX>, index_type s, index_type *v) const
	{
		v[0] = s + s & (_MA >> (s.H + 1));
		v[1] = s - s & (_MA >> (s.H + 1));
		return 2;
	}

	inline int GetAdjacentCells(Int2Type<FACE>, Int2Type<VERTEX>, index_type s, index_type *v) const
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

		auto di = (_R(_I(s)) >> (s.H + 1));
		auto dj = (_RR(_I(s)) >> (s.H + 1));

		v[0] = s - di - dj;
		v[1] = s - di - dj;
		v[2] = s + di + dj;
		v[3] = s + di + dj;

		return 4;
	}

	inline int GetAdjacentCells(Int2Type<VOLUME>, Int2Type<VERTEX>, index_type const &s, index_type *v) const
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
		auto di = _MI >> (s.H + 1);
		auto dj = _MJ >> (s.H + 1);
		auto dk = _MK >> (s.H + 1);

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

	inline int GetAdjacentCells(Int2Type<VERTEX>, Int2Type<EDGE>, index_type s, index_type *v) const
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

		auto di = _MI >> (s.H + 1);
		auto dj = _MJ >> (s.H + 1);
		auto dk = _MK >> (s.H + 1);

		v[0] = s + di;
		v[1] = s - di;

		v[2] = s + dj;
		v[3] = s - dj;

		v[4] = s + dk;
		v[5] = s - dk;

		return 6;
	}

	inline int GetAdjacentCells(Int2Type<FACE>, Int2Type<EDGE>, index_type s, index_type *v) const
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
		auto d1 = (_R(_I(s)) >> (s.H + 1));
		auto d2 = (_RR(_I(s)) >> (s.H + 1));
		v[0] = s - d1;
		v[1] = s + d1;
		v[2] = s - d2;
		v[3] = s + d2;

		return 4;
	}

	inline int GetAdjacentCells(Int2Type<VOLUME>, Int2Type<EDGE>, index_type s, index_type *v) const
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
		auto di = _MI >> (s.H + 1);
		auto dj = _MJ >> (s.H + 1);
		auto dk = _MK >> (s.H + 1);

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

	inline int GetAdjacentCells(Int2Type<VERTEX>, Int2Type<FACE>, index_type s, index_type *v) const
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
		auto di = _MI >> (s.H + 1);
		auto dj = _MJ >> (s.H + 1);
		auto dk = _MK >> (s.H + 1);

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

	inline int GetAdjacentCells(Int2Type<EDGE>, Int2Type<FACE>, index_type s, index_type *v) const
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

		auto d1 = (_R(s) >> (s.H + 1));
		auto d2 = (_RR(s) >> (s.H + 1));

		v[0] = s - d1;
		v[1] = s + d1;
		v[2] = s - d2;
		v[3] = s + d2;

		return 4;
	}

	inline int GetAdjacentCells(Int2Type<VOLUME>, Int2Type<FACE>, index_type s, index_type *v) const
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

		auto di = _MI >> (s.H + 1);
		auto dj = _MJ >> (s.H + 1);
		auto dk = _MK >> (s.H + 1);

		v[0] = s - di;
		v[1] = s + di;

		v[2] = s - di;
		v[3] = s + dj;

		v[4] = s - dk;
		v[5] = s + dk;

		return 6;
	}

	inline int GetAdjacentCells(Int2Type<VERTEX>, Int2Type<VOLUME>, index_type s, index_type *v) const
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

		auto di = _MI >> (s.H + 1);
		auto dj = _MJ >> (s.H + 1);
		auto dk = _MK >> (s.H + 1);

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

	inline int GetAdjacentCells(Int2Type<EDGE>, Int2Type<VOLUME>, index_type s, index_type *v) const
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

		auto d1 = (_R(s) >> (s.H + 1));
		auto d2 = (_RR(s) >> (s.H + 1));

		v[0] = s - d1 - d2;
		v[1] = s + d1 - d2;
		v[2] = s - d1 + d2;
		v[3] = s + d1 + d2;
		return 4;
	}

	inline int GetAdjacentCells(Int2Type<FACE>, Int2Type<VOLUME>, index_type s, index_type *v) const
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

		auto d = (_I(s) >> (s.H + 1));
		v[0] = s + d;
		v[1] = s - d;

		return 2;
	}

//! VERTEX -> EDGE
	template<typename TF>
	auto Grad(TF const & f,
	        index_type s) const
	                DECL_RET_TYPE(
			                ( (f[s + s & (_MA >> (s.H + 1))] - f[s - s & (_MA >> (s.H + 1))]) * static_cast<double>(1UL << s.H) )
	                )

//! VERTEX -> EDGE
	template<typename TF>
	auto Diverge(TF const & f, index_type s, Real const a[3]) const
	DECL_RET_TYPE(
			((
							(f[s + (_MI >> (s.H + 1))] - f[s - (_MI >> (s.H + 1))]) *a[0]+
							(f[s + (_MJ >> (s.H + 1))] - f[s - (_MJ >> (s.H + 1))]) *a[1]+
							(f[s + (_MK >> (s.H + 1))] - f[s - (_MK >> (s.H + 1))]) *a[2]
					)* static_cast<double>(1UL << s.H) )
	)

	//! Curl(Field<Edge>) Edge=>FACE
	template<typename TF>
	auto Curl(TF const & f, Int2Type<EDGE>,
	        index_type s, //! this is FACE index
	        Real const a[3]) const
	                DECL_RET_TYPE(
			                ((
							                (f[ s + (_R( _I(s))>> (s.H +1) ) ] - f[s - (_R( _I(s))>> (s.H +1) )]) *a[ _N(_R( _I(s))) ]-
							                (f[ s + (_RR( _I(s))>> (s.H +1) ) ] - f[s - (_RR( _I(s))>> (s.H +1) )]) *a[ _N(_RR( _I(s))) ]
					                )* static_cast<double>(1UL << s.H) )
	                )

	//! Curl(Field<FACE>) FACE=>EDGE
	template<typename TF>
	auto Curl(TF const & f, Int2Type<FACE>,
	        index_type s, //! this is edge index
	        Real const a[3]) const
	                DECL_RET_TYPE(
			                ((
							                (f[ s + (_R( s)>> (s.H +1) ) ] - f[s - (_R( s)>> (s.H +1) )]) *a[ _N(_R( s))]-
							                (f[ s + (_RR( s)>> (s.H +1) )] - f[s - (_RR( s)>> (s.H +1) )]) *a[ _N(_RR( s))]

					                )* static_cast<double>(1UL << s.H) )
	                )
//***************************************************************************************************
//  Traversal
//
//***************************************************************************************************
//
//	template<typename ... Args>
//	void Traversal(Args const &...args) const
//	{
//		ParallelTraversal(std::forward<Args const &>(args)...);
//	}
//
//	template<typename ...Args> void ParallelTraversal(Args const &...args) const;
//
//	template<typename ...Args> void SerialTraversal(Args const &...args) const;
//
//	void _Traversal(unsigned int num_threads, unsigned int thread_id, int IFORM,
//	        std::function<void(index_type)> const &funs) const;
//
//	template<typename Fun, typename TF, typename ... Args> inline
//	void SerialForEach(Fun const &fun, TF const & l, Args const& ... args) const
//	{
//		SerialTraversal(FieldTraits<TF>::IForm, [&]( index_type s)
//		{	fun(get(l,s),get(args,s)...);});
//	}
//
//	template<typename Fun, typename TF, typename ... Args> inline
//	void SerialForEach(Fun const &fun, TF *l, Args const& ... args) const
//	{
//		if (l == nullptr)
//			ERROR << "Access value to an uninitilized container!";
//
//		SerialTraversal(FieldTraits<TF>::IForm, [&]( index_type s)
//		{	fun(get(l,s),get(args,s)...);});
//	}
//
//	template<typename Fun, typename TF, typename ... Args> inline
//	void ParallelForEach(Fun const &fun, TF const & l, Args const& ... args) const
//	{
//		ParallelTraversal(FieldTraits<TF>::IForm, [&]( index_type s)
//		{	fun(get(l,s),get(args,s)...);});
//	}
//
//	template<typename Fun, typename TF, typename ... Args> inline
//	void ParallelForEach(Fun const &fun, TF *l, Args const& ... args) const
//	{
//		if (l == nullptr)
//			ERROR << "Access value to an uninitilized container!";
//
//		ParallelTraversal(FieldTraits<TF>::IForm, [&]( index_type s)
//		{	fun(get(l,s),get(args,s)...);});
//	}
//
//	template<typename Fun, typename TF, typename ... Args> inline
//	void ForEach(Fun const &fun, TF const & l, Args const& ... args) const
//	{
//		ParallelForEach(fun, l, std::forward<Args const &>(args)...);
//	}
//	template<typename Fun, typename TF, typename ... Args> inline
//	void ForEach(Fun const &fun, TF *l, Args const& ... args) const
//	{
//		ParallelForEach(fun, l, std::forward<Args const &>(args)...);
//	}
//
////***************************************************************************************************
////* Container/Field operation
////* Field vs. Mesh
////***************************************************************************************************
//
//	template<typename TL, typename TR> void AssignContainer(int IFORM, TL * lhs, TR const &rhs) const
//	{
//		ParallelTraversal(IFORM, [&]( index_type s)
//		{	get(lhs,s)=get(rhs,s);});
//
//	}
//
//	template<typename T>
//	inline typename std::enable_if<!is_field<T>::value, T>::type get(T const &l, index_type) const
//	{
//		return std::move(l);
//	}
//

}
;

}
// namespace simpla

#endif /* OCTREE_FOREST_H_ */
