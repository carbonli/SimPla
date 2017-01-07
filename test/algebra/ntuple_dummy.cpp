//
// Created by salmon on 16-12-26.
//
#include <simpla/SIMPLA_config.h>

#include <simpla/algebra/Algebra.h>
#include <simpla/algebra/Arithmetic.h>
#include <simpla/algebra/Calculus.h>
#include <simpla/algebra/Expression.h>
#include <simpla/algebra/nTuple.h>
#include <simpla/algebra/nTupleExt.h>

#include <complex>
#include <iostream>
using namespace simpla;
using namespace simpla::algebra;

int main(int argc, char** argv) {
    double a[3] = {1, 2, 3};
    double b[3] = {1, 2, 3};
    double c[3] = {1, 2, 3};

    for (int i = 0; i < 3; ++i) { a[i] = b[i] + c[i]; }
    nTuple<Real, 3> t0 = {0, 0, 0};
    nTuple<Real, 2, 3> t1 = {{0, 1, 2}, {3, 4, 5}};
    nTuple<std::complex<Real>, 2, 3> t2 = {{0, 1, 2}, {3, 4, 5}};

    std::cout << "t1 = " << t1 << std::endl;

    std::cout << "t2 = " << t2 << std::endl;

    //    t1 *= 2;

    std::cout << "t1 = " << t1 << std::endl;

    double d[2][3] = {{10, 20, 30}, {40, 50, 60}};

    nTuple<double, 2, 3> B = {{4, 5, 6}, {7, 8, 9}};
    nTuple<double, 2, 3> A = B * 2 + 5;
    std::cout << "A=" << A << std::endl;
    std::cout << "B=" << B << std::endl;

    Real res = dot(A, B);

    std::cout << "dot(A+B, B+A) =" << res << std::endl;

    std::cout << "A=" << A << std::endl;

    nTuple<Real, 3> v = {1, 2, 3};
    nTuple<Real, 3> u = {4, 5, 6};
    nTuple<Real, 3> w = {9, 8, 7};
    std::cout << static_cast<nTuple<Real, 3> >(cross(v, u)) << std::endl;
    std::cout << static_cast<Real>(dot(cross(v, u), cross(v, u))) << std::endl;
    std::cout << static_cast<Real>(dot(cross(v, u), w)) << std::endl;
}