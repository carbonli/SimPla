//
// Created by salmon on 17-8-15.
//
#include "DataBlock.h"
namespace simpla {
namespace data {

struct DataBlock::pimpl_s {};

DataBlock::DataBlock() : m_pimpl_(new pimpl_s) {}
DataBlock::~DataBlock() { delete m_pimpl_; }
DataBlock::DataBlock(int ndims, index_type const *lo, index_type const *hi) : DataBlock() {}

size_type DataBlock::size() const { return 0; }

int DataBlock::GetNDIMS() const { return 0; }
int DataBlock::GetIndexBox(index_type *lo, index_type *hi) const { return SP_SUCCESS; }

int DataBlock::Clear() {
    UNIMPLEMENTED;
    return SP_SUCCESS;
};
int DataBlock::Copy(DataBlock const &other) {
    UNIMPLEMENTED;
    return SP_SUCCESS;
};
int DataBlock::Copy2(DataBlock &other) const { return other.Copy(*this); };

int DataBlock::Copy(DataBlock const &other, index_box_type const &box) {
    UNIMPLEMENTED;
    return SP_SUCCESS;
};
int DataBlock::Copy2(DataBlock &other, index_box_type const &box) const { return other.Copy(*this, box); };

}  // namespace data
}  // namespace simpla