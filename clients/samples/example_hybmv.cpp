/* ************************************************************************
 * Copyright 2018 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#include "utility.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <rocsparse.h>

int main(int argc, char* argv[])
{
    // Parse command line
    if(argc < 2)
    {
        fprintf(stderr, "%s <ndim> [<trials> <batch_size>]\n", argv[0]);
        return -1;
    }

    int ndim       = atoi(argv[1]);
    int trials     = 200;
    int batch_size = 1;

    if(argc > 2)
    {
        trials = atoi(argv[2]);
    }
    if(argc > 3)
    {
        batch_size = atoi(argv[3]);
    }

    // rocSPARSE handle
    rocsparse_handle handle;
    rocsparse_create_handle(&handle);

    hipDeviceProp_t devProp;
    int device_id = 0;

    hipGetDevice(&device_id);
    hipGetDeviceProperties(&devProp, device_id);
    printf("Device: %s\n", devProp.name);

    // Generate problem in CSR format
    std::vector<int> hAptr;
    std::vector<int> hAcol;
    std::vector<double> hAval;
    int m   = gen_2d_laplacian(ndim, hAptr, hAcol, hAval, rocsparse_index_base_zero);
    int n   = m;
    int nnz = hAptr[m];

    // Sample some random data
    srand(12345ULL);

    double halpha = static_cast<double>(rand()) / RAND_MAX;
    double hbeta  = 0.0;

    std::vector<double> hx(n);
    rocsparse_init(hx, 1, n);

    // Matrix descriptor
    rocsparse_mat_descr descrA;
    rocsparse_create_mat_descr(&descrA);

    // Offload data to device
    int* dAptr    = NULL;
    int* dAcol    = NULL;
    double* dAval = NULL;
    double* dx    = NULL;
    double* dy    = NULL;

    hipMalloc((void**)&dAptr, sizeof(int) * (m + 1));
    hipMalloc((void**)&dAcol, sizeof(int) * nnz);
    hipMalloc((void**)&dAval, sizeof(double) * nnz);
    hipMalloc((void**)&dx, sizeof(double) * n);
    hipMalloc((void**)&dy, sizeof(double) * m);

    hipMemcpy(dAptr, hAptr.data(), sizeof(int) * (m + 1), hipMemcpyHostToDevice);
    hipMemcpy(dAcol, hAcol.data(), sizeof(int) * nnz, hipMemcpyHostToDevice);
    hipMemcpy(dAval, hAval.data(), sizeof(double) * nnz, hipMemcpyHostToDevice);
    hipMemcpy(dx, hx.data(), sizeof(double) * n, hipMemcpyHostToDevice);

    // Convert CSR matrix to HYB format
    rocsparse_hyb_mat hybA;
    rocsparse_create_hyb_mat(&hybA);

    rocsparse_dcsr2hyb(
        handle, m, n, descrA, dAval, dAptr, dAcol, hybA, 0, rocsparse_hyb_partition_auto);

    // Clean up CSR structures
    hipFree(dAptr);
    hipFree(dAcol);
    hipFree(dAval);

    // Warm up
    for(int i = 0; i < 10; ++i)
    {
        // Call rocsparse hybmv
        rocsparse_dhybmv(handle, rocsparse_operation_none, &halpha, descrA, hybA, dx, &hbeta, dy);
    }

    // Device synchronization
    hipDeviceSynchronize();

    // Start time measurement
    double time = get_time_us();

    // HYB matrix vector multiplication
    for(int i = 0; i < trials; ++i)
    {
        for(int i = 0; i < batch_size; ++i)
        {
            // Call rocsparse hybmv
            rocsparse_dhybmv(
                handle, rocsparse_operation_none, &halpha, descrA, hybA, dx, &hbeta, dy);
        }

        // Device synchronization
        hipDeviceSynchronize();
    }

    time = (get_time_us() - time) / (trials * batch_size * 1e3);
    double bandwidth =
        static_cast<double>(sizeof(double) * (2 * m + nnz) + sizeof(rocsparse_int) * (nnz)) / time /
        1e6;
    double gflops = static_cast<double>(2 * nnz) / time / 1e6;
    printf("m\t\tn\t\tnnz\t\talpha\tbeta\tGFlops\tGB/s\tusec\n");
    printf("%8d\t%8d\t%9d\t%0.2lf\t%0.2lf\t%0.2lf\t%0.2lf\t%0.2lf\n",
           m,
           n,
           nnz,
           halpha,
           hbeta,
           gflops,
           bandwidth,
           time);

    // Clear up on device
    rocsparse_destroy_hyb_mat(hybA);
    rocsparse_destroy_mat_descr(descrA);
    rocsparse_destroy_handle(handle);

    return 0;
}