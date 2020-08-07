#ifndef MIXING_MATRIX_UTILS_H_INCLUDED
#define MIXING_MATRIX_UTILS_H_INCLUDED

#include <r8lib/matrix_exponential.h>

#include <cmath>
#include <ctime>
#include <memory>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstdlib>

/** Class for FDN mixing matrices */
template<size_t N>
struct Matrix
{
    Matrix() : dim(N) {}
    Matrix(const Matrix& oldMatrix) {
        for (int row = 0; row < dim; ++row)
            for (int col = 0; col < dim; ++col)
                matrix[row][col] = oldMatrix.matrix[row][col];
    }

    float matrix[N][N];
    const int dim;
};

namespace MixingMatrixUtils
{
    /** Write matrix to log (useful for debugging) */
    template<size_t N>
    void logMatrix (Matrix<N>& matrix) {
        std::stringstream ss;
        for (int row = 0; row < matrix.dim; ++row)
        {
            for (int col = 0; col < matrix.dim; ++col)
            {
                if (matrix.matrix[row][col] >= 0.0f)
                    ss << '+';

                ss << std::fixed << std::setprecision (4) << matrix.matrix[row][col];

                if (col < matrix.dim - 1)
                    ss << ", ";
            }
            ss << '\n';
        }

        std::cout << ss.str() << std::endl;
    }

    // Create identity matrix
    template<size_t N>
    inline void identityMatrix (Matrix<N>& matrix)
    {
        for (int row = 0; row < matrix.dim; ++row)
            for (int col = 0; col < matrix.dim; ++col)
                matrix.matrix[row][col] = row == col ? 1.0f : 0.0f;
    }

    /** Create orthonormal matrix */
    template<size_t N>
    void orthonormal (Matrix<N>& matrix) {
        // random seed
        srand(time(NULL));
        auto nextFloat = []() -> float {
            return static_cast<float> (rand()) / static_cast<float> (RAND_MAX);
        };

        float dcy = 2.0f;

        // set up A, a random diagonnaly symmetric matrix
        Matrix<N> A;
        for (int ind1 = 0; ind1 < matrix.dim; ++ind1)
        {
            for (int ind2 = ind1; ind2 < matrix.dim; ++ind2)
            {
                A.matrix[ind1][ind2] = (-1.0f + 2.0f * nextFloat()) * std::pow (float (ind2 - ind1), dcy);
                A.matrix[ind2][ind1] = -A.matrix[ind1][ind2];
            }
        }

        // exponentiate matrix
        std::unique_ptr<double[]> r8InMat (new double[A.dim * A.dim]);
        for (int row = 0; row < matrix.dim; ++row)
            for (int col = 0; col < matrix.dim; ++col)
                r8InMat[row * A.dim + col] = A.matrix[row][col];

        std::unique_ptr<double[]> r8OutMat;
        r8OutMat.reset (r8mat_expm1 (A.dim, r8InMat.get()));

        for (int row = 0; row < matrix.dim; ++row)
            for (int col = 0; col < matrix.dim; ++col)
                matrix.matrix[row][col] = r8OutMat[row * A.dim + col];
    }
}

#endif // MIXING_MATRIX_UTILS_H_INCLUDED
