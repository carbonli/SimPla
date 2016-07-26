//
// Created by salmon on 16-7-20.
//

#include "sp_lite_def.h"

#include <assert.h>

#include "spObject.h"
#include "spParallel.h"
#include "spMesh.h"
#include "spField.h"

#define MAX_NUM_OF_FIELD_ATTR 16

typedef struct spField_s
{
    SP_MESH_ATTR_HEAD

    struct spDataType_s *m_data_type_desc_;

    void *device_data;

    int is_soa;

} spField;

int spFieldCreate(spField **f, const struct spMesh_s *mesh, int iform, int type_tag)
{
    SP_CHECK_RETURN(spMeshAttrCreate((spMeshAttr **) f, sizeof(spField), mesh, iform));

    (*f)->m = mesh;
    (*f)->iform = iform;
    (*f)->is_soa = SP_TRUE;
    (*f)->device_data = NULL;


    SP_CHECK_RETURN(spDataTypeCreate(&((*f)->m_data_type_desc_), type_tag, 0));

    return SP_SUCCESS;
}

int spFieldDestroy(spField **f)
{
    if (f != NULL && *f != NULL)
    {

        spParallelDeviceFree(&((**f).device_data));

        SP_CHECK_RETURN(spDataTypeDestroy(&((*f)->m_data_type_desc_)));
    }

    SP_CHECK_RETURN(spMeshAttrDestroy((spMeshAttr **) f));

    return SP_SUCCESS;
}

int spFieldDeploy(spField *f)
{

    if (f->device_data == NULL)
    {
        size_type s = spDataTypeSizeInByte(f->m_data_type_desc_) * spMeshNumberOfEntity(f->m, SP_DOMAIN_ALL, f->iform);

        spParallelDeviceAlloc((void **) &(f->device_data), s);
    }
    return SP_SUCCESS;
}

int spFieldIsSoA(spField const *f) { return f->is_soa; }

spDataType const *spFieldDataType(spField const *f) { return f->m_data_type_desc_; };

void *spFieldData(spField *f) { return spFieldDeviceData(f); }

void *spFieldDeviceData(spField *f) { return f->device_data; }

const void *spFieldDataConst(spField const *f) { return spFieldDeviceDataConst(f); }

const void *spFieldDeviceDataConst(spField const *f) { return f->device_data; }

int spFieldNumberOfSub(spField const *f)
{
    int iform = spMeshAttrForm((spMeshAttr const *) f);

    return (iform == VERTEX || iform == VOLUME) ? 1 : 3;
}

int spFieldSubArray(spField *f, int domain_tag, void **data, size_type *stride)
{

    spMesh const *m = spMeshAttrMesh((spMeshAttr const *) f);

    int iform = spMeshAttrForm((spMeshAttr const *) f);

    size_type ele_size_in_byte = spDataTypeSizeInByte(spFieldDataType(f));

    size_type dims[4], start[4], count[4];

    spMeshLocalDomain(m, domain_tag, dims, start, count);

    void *data_root = spFieldData(f);

    size_type head = ele_size_in_byte * (start[0] + start[1] * dims[0] + start[2] * dims[0] * dims[1]);

    int num_of_sub = spFieldNumberOfSub(f);

    if (num_of_sub == 1) { *data = spFieldData(f) + head; }
    else
    {
        size_type offset = ele_size_in_byte * spMeshNumberOfEntity(m, SP_DOMAIN_ALL, VERTEX);
        if (spFieldIsSoA(f))
        {
            for (int i = 0; i < num_of_sub; ++i) { data[i] = data_root + i * offset + head; }
            if (stride != NULL) { *stride = 1; }
        }
        else
        {
            for (int i = 0; i < num_of_sub; ++i) { data[i] = data_root + i * ele_size_in_byte + head * num_of_sub; }
            if (stride != NULL) { *stride = (size_type) num_of_sub; }
        }
    }
    return SP_SUCCESS;
};

int spFieldClear(spField *f)
{
    spFieldDeploy(f);

    size_type s = spDataTypeSizeInByte(f->m_data_type_desc_)
        * spMeshNumberOfEntity(f->m, SP_DOMAIN_ALL, f->iform);

    spParallelMemset(f->device_data, 0, s);

    return SP_SUCCESS;
}
int spFieldFill(spField *f, Real v)
{
    spFieldDeploy(f);

    return spParallelDeviceFillReal(f->device_data, v, spMeshNumberOfEntity(f->m, SP_DOMAIN_ALL, f->iform));

}
int spFieldWrite(spField *f, spIOStream *os, char const name[], int flag)
{
    spMesh const *m = spMeshAttrMesh((spMeshAttr const *) f);
    int iform = spMeshAttrForm((spMeshAttr const *) f);

    size_type size_in_byte = spMeshNumberOfEntity(m, SP_DOMAIN_ALL, iform) *
        spDataTypeSizeInByte(spFieldDataType(f));

    void *f_host;

    spParallelHostAlloc(&f_host, size_in_byte);

    spParallelMemcpy((f_host), spFieldData(f), size_in_byte);

    int ndims = spMeshNDims(m);
    int array_ndims, mesh_start_dim;

    size_type l_dims[ndims + 1];
    size_type l_start[ndims + 1];
    size_type l_count[ndims + 1];

    size_type g_dims[ndims + 1];
    size_type g_start[ndims + 1];

    size_type num_of_sub = 3;
    spMeshArrayShape(m, SP_DOMAIN_CENTER,
                     (iform == VERTEX || iform == VOLUME) ? 0 : 1,
                     &num_of_sub,
                     &array_ndims, &mesh_start_dim,
                     g_dims, g_start,
                     l_dims, l_start, l_count,
                     spFieldIsSoA(f));

    spIOStreamWriteSimple(os,
                          name,
                          spFieldDataType(f),
                          f_host,
                          array_ndims,
                          l_dims,
                          l_start,
                          NULL,
                          l_count,
                          NULL,
                          g_dims,
                          g_start,
                          flag);

    spParallelHostFree(&f_host);

    return SP_SUCCESS;
}

int spFieldRead(spField *f, spIOStream *os, char const name[], int flag)
{
    return SP_SUCCESS;
}

int spFieldSync(spField *f)
{
    spMesh const *m = spMeshAttrMesh((spMeshAttr const *) f);
    int iform = spMeshAttrForm((spMeshAttr const *) f);
    int ndims = spMeshNDims(m);
    int array_ndims, mesh_start_dim;

    size_type l_dims[ndims + 1];
    size_type l_start[ndims + 1];
    size_type l_count[ndims + 1];

    size_type num_of_sub = 3;

    spMeshArrayShape(m, SP_DOMAIN_CENTER, (iform == VERTEX || iform == VOLUME) ? 0 : 1, &num_of_sub,
                     &array_ndims, &mesh_start_dim, NULL, NULL, l_dims, l_start, l_count, spFieldIsSoA(f));


    spParallelUpdateNdArrayHalo(spFieldDeviceData(f), spFieldDataType(f),
                                array_ndims, l_dims, l_start, NULL, l_count, NULL, mesh_start_dim);
    return SP_SUCCESS;

}
int spFeildAssign(spField *f, size_type num_of_points, size_type **points, Real const **v)
{
    spMesh const *m = spMeshAttrMesh((spMeshAttr const *) f);

    if (spFieldIsSoA(f))
    {
        int num_of_sub = spFieldNumberOfSub(f);

        Real *data[num_of_sub];

        spFieldSubArray(f, SP_DOMAIN_CENTER, (void **) data, NULL);

        size_type strides[3];

        SP_CHECK_RETURN(spMeshGetStrides(m, strides));

        spParallelAssign(num_of_sub, num_of_points, points, strides, (Real **) data, v);

    }
    else
    {
        UNIMPLEMENTED;
    }
}