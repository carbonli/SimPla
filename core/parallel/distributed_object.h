/**
 * @file distributed_object.h
 * @author salmon
 * @date 2015-10-17.
 */

#ifndef SIMPLA_DISTRIBUTED_OBJECT_H
#define SIMPLA_DISTRIBUTED_OBJECT_H

#include <bits/unique_ptr.h>
#include "mpi_comm.h"
#include "mpi_aux_functions.h"
#include "mpi_update.h"
#include "../dataset/dataset.h"

namespace simpla { namespace parallel
{


struct DistributedObject
{
    DistributedObject();

    DistributedObject(DistributedObject const &) = delete;

    virtual ~DistributedObject();

    virtual void sync();

    virtual void wait();

    virtual bool is_ready() const;


    template<typename T, typename ...Others>
    void add(T const &args, Others &&...others)
    {
        add(traits::get_dataset(args));
        add(std::forward<Others>(others)...);
    }

    template<typename T>
    void add(T const &args)
    {
        add(traits::get_dataset(args));
    }

    template<typename T>
    void add(T *args)
    {
        add(traits::get_dataset(*args));
    }

    void add(DataSet ds);

    void add_link(bool is_send, int const coord_offset[], int size,
                  DataType const &d_type, std::shared_ptr<void> *p);

    void add_link(bool is_send, int const coord_offset[], DataSpace const &space,
                  DataType const &d_type, std::shared_ptr<void> *p);

    template<typename ...Args>
    void add_link_send(Args &&...args) { add_link(true, std::forward<Args>(args)...); };

    template<typename ...Args>
    void add_link_recv(Args &&...args) { add_link(false, std::forward<Args>(args)...); };

private:

    struct pimpl_s;
    std::unique_ptr<pimpl_s> pimpl_;

};
}}//namespace simpla
#endif //SIMPLA_DISTRIBUTED_OBJECT_H