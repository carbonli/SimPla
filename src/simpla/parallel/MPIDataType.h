/**
 * @file MPIDataType.h
 *
 *  created on: 2014-5-26
 *      Author: salmon
 */

#ifndef MPI_DATATYPE_H_
#define MPI_DATATYPE_H_

#include <mpi.h>
#include <cstdbool>
#include <cstddef>

#include "simpla/data/DataSpace.h"
#include "simpla/data/DataType.h"

namespace simpla {
namespace _impl {

bool GetMPIType(data::DataType const &data_type_desc, MPI_Datatype *new_type);

}  // namespace _impl

/**
 *  @ingroup MPI
 *  \brief MPI convert C++ m_data type and m_data space to mpi m_data type
 */
struct MPIDataType {
    MPIDataType();

    MPIDataType(MPIDataType const &);

    ~MPIDataType();

    //	static MPIDataType clone(DataType const &);

    static MPIDataType create(data::DataType const &data_type, int ndims = 0, size_t const *dims = nullptr,
                              size_t const *p_start = nullptr, size_t const *offset = nullptr,
                              size_t const *stride = nullptr, size_t const *block = nullptr, bool c_order_array = true);

    static MPIDataType create(data::DataType const &data_type, data::DataSpace const &space, bool c_order_array = true);

    void swap(MPIDataType &other);

    MPIDataType &operator=(MPIDataType const &other) {
        MPIDataType(other).swap(*this);
        return *this;
    }

    template <typename T, typename... Others>
    static MPIDataType create(Others &&... others) {
        return create(data::DataType::create<T>(), std::forward<Others>(others)...);
    }

    MPI_Datatype const &type(...) const { return m_type_; }

    size_t size() const;

   private:
    MPI_Datatype m_type_ = MPI_DATATYPE_NULL;

    bool is_commited_ = false;
};

}  // namespace simpla

#endif /* MPI_DATATYPE_H_ */
