#ifndef MIXING_MATRIX_UTILS_H_INCLUDED
#define MIXING_MATRIX_UTILS_H_INCLUDED

#include "r8lib/matrix_exponential.h"

/** Class for FDN mixing matrices */
struct Matrix
{
    Matrix (int dim);
    Matrix (const Matrix& oldMatrix);
    ~Matrix();

    float** matrix;
    const int dim;
};

namespace MixingMatrixUtils
{
    /** Write matrix to log (useful for debugging) */
    void logMatrix (Matrix& matrix);

    // Create identity matrix
    inline void identityMatrix (Matrix& matrix)
    {
        for (int row = 0; row < matrix.dim; ++row)
            for (int col = 0; col < matrix.dim; ++col)
                matrix.matrix[row][col] = row == col ? 1.0f : 0.0f;
    }

    /** Create orthonormal matrix */
    void orthonormal (Matrix& matrix);

    /** Matrix generated for Music 424 HW7 */
    void myMatrix (Matrix& matrix);
}

#endif // MIXING_MATRIX_UTILS_H_INCLUDED
