/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_COMMON_MATRIX_H
#define BBP_COMMON_MATRIX_H

#include <vector>
#include <cstring>
#include "BBP/Common/Types.h"
#include "BBP/Common/Exception/Exception.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Matrix (mathematics terminology, two dimensional table structure).
/*!
    \ingroup Math
*/
template <typename T, Count ROWS = 0, Count COLUMNS = 0>
class Matrix;

template <typename T, Count ROWS, Count COLUMNS>
class Matrix
{

public:
    //! Create an empty matrix.
    Matrix() {}
    //! Create a matrix of a fixed size initialized with the specified value.
    Matrix(const T & value)
    {
        fill(value);
    }

    //! Delete matrix.
    ~Matrix() {}

    //! access Matrix field by row and column index
    T &      operator() (Index row, Index column)
    {
        bbp_assert(row < ROWS && column < COLUMNS);
        return _matrix[column + row * COLUMNS];
    }

    //! access Matrix field by linear index
    T &      operator() (Index linear_access_index)
    {
        bbp_assert(linear_access_index < size());
        return _matrix[linear_access_index];
    }

    //! access Matrix field by row and column index
    const T &      operator() (Index row, Index column) const
    {
        bbp_assert(row < ROWS && column < COLUMNS);
        return _matrix[column + row * COLUMNS];
    }
    //! access Matrix field by linear index
    const T &      operator() (Index linear_access_index) const
    {
        bbp_assert(linear_access_index < size());
        return _matrix[linear_access_index];
    }

    //! Get number of columns.
    Index    columns() const
    {
        return COLUMNS;
    }

    //! Get number of rows.
    Index    rows() const
    {
        return ROWS;
    }

    //! Get number of matrix fields.
    size_t   size() const
    {
        return ROWS * COLUMNS;
    }

    //! print Matrix contents to standard output
    void     print() const
    {
        std::cout << * this;
    }

    //! fill whole matrix with one value
    void     fill(const T & value)
    {
        std::fill<T*, T>(& _matrix[0], & _matrix[ROWS * COLUMNS - 1], value);
    }

    //! Transpose matrix.
    /*!
        http://en.wikipedia.org/wiki/Transpose
    */
    Matrix<T, COLUMNS, ROWS> transpose()
    {
        Matrix<T, COLUMNS, ROWS> t;

        for (size_t y = 0; y < ROWS; ++y)
        for (size_t x = 0; x < COLUMNS; ++x)
        {
            t(y,x) = (*this)(x,y);
        }
        return t;
    }

    //! Set matrix to identity matrix.
    /*!
        If the columns != rows, std::logic_error is thrown.
        http://en.wikipedia.org/wiki/Identity_matrix
    */
    void identity()
    {
        if (COLUMNS == ROWS)
        {
            this->fill(0);
            for (size_t i = 0; i < ROWS; ++i)
                (*this)(i, i) = 1;
        }
        else
        {
            throw_exception(std::logic_error("Matrix::identity() exception: "
                "columns != rows"), SEVERE_LEVEL, __FILE__, __LINE__);
        }
    }

    //// Compute the inverse of the current matrix
    //Matrix<T, ROWS, COLUMNS> inverse()
    //{
    //    Matrix<T, ROWS, COLUMNS> inv;

    //}

protected:
    //! Array of _matrix stored in the matrix container.
    T                   _matrix[ROWS * COLUMNS];
};

// ----------------------------------------------------------------------------

template <typename T>
class Matrix<T, 0, 0>
{

public:
    //! Create an empty matrix.
    Matrix() {}
    //! Create a matrix of a fixed size initialized with the specified value.
    Matrix(const T & value)
    {
        fill(value);
    }

    //! Create an empty matrix.
    Matrix(Count rows, Count columns) 
    {
        resize(rows, columns);
    }


    //! Delete matrix.
    ~Matrix(){}

    //! access Matrix field by row and column index
    T &      operator() (Index row, Index column)
    {
        bbp_assert(row < _rows && column < _columns);
        return _matrix[column + row * _columns];
    }

    //! access Matrix field by linear index
    T &      operator() (Index linear_access_index)
    {
        bbp_assert(linear_access_index < size());
        return _matrix[linear_access_index];
    }

    //! access Matrix field by row and column index
    const T &      operator() (Index row, Index column) const
    {
        bbp_assert(row < _rows && column < _columns);
        return _matrix[column + row * _columns];
    }

    //! access Matrix field by linear index
    const T &      operator() (Index linear_access_index) const
    {
        bbp_assert(linear_access_index < size());
        return _matrix[linear_access_index];
    }


    //! Get number of columns.
    Index    columns() const
    {
        return _columns;
    }

    //! Get number of rows.
    Index    rows() const
    {
        return _rows;
    }

    //! Get number of matrix fields.
    size_t   size() const
    {
        return _columns * _rows;
    }

    //! print Matrix contents to standard output
    void     print() const
    {
        std::cout << * this;
    }

    //! fill whole matrix with one value
    void     fill(const T & value)
    {
        std::fill<T*, T>(& _matrix[0], & _matrix[_columns * _rows - 1], value);
    }

    //! Transpose matrix.
    /*!
        http://en.wikipedia.org/wiki/Transpose
    */
    Matrix<T> transpose()
    {
        Matrix t(_columns, _rows);

        for (size_t y = 0; y < _rows; ++y)
        for (size_t x = 0; x < _columns; ++x)
        {
            t(y,x) = (*this)(x,y);
        }
        return t;
    }

    //! Set matrix to identity matrix.
    /*!
        If the columns != rows, std::logic_error is thrown.
        http://en.wikipedia.org/wiki/Identity_matrix
    */
    void identity()
    {
        if (_columns == _rows)
        {
            this->fill(0);
            for (size_t i = 0; i < _rows; ++i)
                (*this)(i, i) = 1;
        }
        else
        {
            throw_exception(std::logic_error("Matrix::identity() exception: "
                "columns != rows"), SEVERE_LEVEL, __FILE__, __LINE__);
        }
    }
    
    //! Resize matrix dimensions (undefined state after).
    void resize(Count rows, Count columns)
    {
        _matrix.reserve(rows * columns);
        _matrix.resize(rows * columns);

        _rows = rows;
        _columns = columns;
    }

protected:
    //! Array of _matrix stored in the matrix container.
    std::vector<T>      _matrix;
    Index               _rows, 
                        _columns;
};

// ----------------------------------------------------------------------------

template <typename T, Count N>
class Matrix<T, N, N>
{

public:
    //! Create an empty matrix.
    Matrix() {}
    //! Create a matrix of a fixed size initialized with the specified value.
    Matrix(const T & value)
    {
        fill(value);
    }

    //! Delete matrix.
    ~Matrix(){}

    //! access Matrix field by row and column index
    T &      operator() (Index row, Index column)
    {
        bbp_assert(row < N && column < N);
        return _matrix[column + row * N];
    }

    //! access Matrix field by linear index
    T &      operator() (Index linear_access_index)
    {
        bbp_assert(linear_access_index < size());
        return _matrix[linear_access_index];
    }

    //! access Matrix field by row and column index
    const T &      operator() (Index row, Index column) const
    {
        bbp_assert(row < N && column < N);
        return _matrix[column + row * N];
    }

    //! access Matrix field by linear index
    const T &      operator() (Index linear_access_index) const
    {
        bbp_assert(linear_access_index < size());
        return _matrix[linear_access_index];
    }


    //! Get number of columns.
    Index    columns() const
    {
        return N;
    }

    //! Get number of rows.
    Index    rows() const
    {
        return N;
    }

    //! Get number of matrix fields.
    size_t   size() const
    {
        return N * N;
    }

    //! print Matrix contents to standard output
    void     print() const
    {
        std::cout << * this;
    }

    //! fill whole matrix with one value
    void     fill(const T & value)
    {
        std::fill<T*, T>(& _matrix[0], & _matrix[N * N - 1], value);
    }

    //! Transpose matrix.
    /*!
        http://en.wikipedia.org/wiki/Transpose
    */
    Matrix<T, N, N> transpose()
    {
        Matrix<T, N, N> t;

        for (size_t y = 0; y < N; ++y)
        for (size_t x = 0; x < N; ++x)
        {
            t(y,x) = (*this)(x,y);
        }
        return t;
    }

    //! Set matrix to identity matrix.
    /*!
        http://en.wikipedia.org/wiki/Identity_matrix
    */
    void identity()
    {
        this->fill(0);
        for (size_t i = 0; i < N; ++i)
            (*this)(i, i) = 1;
    }

    //// Compute the inverse of the current matrix
    //Matrix<T, ROWS, COLUMNS> inverse()
    //{
    //    Matrix<T, ROWS, COLUMNS> inv;

    //}

protected:
    //! Array of _matrix stored in the matrix container.
    T                   _matrix[N * N];
};

// ----------------------------------------------------------------------------

template <typename T, Count ROWS, Count COLUMNS>
std::ostream & operator << (std::ostream & lhs, 
                            const Matrix<T, ROWS, COLUMNS> & rhs)
{
    lhs << "Matrix (" << rhs.size() << ") = { " << std::endl;
    for (Index row    = 0; row    < rhs.rows()   ;  ++row)
    for (Index column = 0; column < rhs.columns();  ++column)
    {
        lhs << rhs(row, column) << " ";
    }
    lhs << "}" << std::endl;
    return lhs;
}

// ----------------------------------------------------------------------------

template <typename T, Count N>
std::ostream & operator << (std::ostream & lhs, 
                            const Matrix<T, N, N> & rhs)
{
    lhs << "Matrix (" << rhs.size() << ") = { " << std::endl;
    for (Index row    = 0; row    < rhs.rows()   ;  ++row)
    for (Index column = 0; column < rhs.columns();  ++column)
    {
        lhs << rhs(row, column) << " ";
    }
    lhs << "}" << std::endl;
    return lhs;
}

// ----------------------------------------------------------------------------

template <typename T>
std::ostream & operator << (std::ostream & lhs, 
                            const Matrix<T, 0, 0> & rhs)
{
    lhs << "Matrix (" << rhs.size() << ") = { " << std::endl;
    for (Index row    = 0; row    < rhs.rows()   ;  ++row)
    for (Index column = 0; column < rhs.columns();  ++column)
    {
        lhs << rhs(row, column) << " ";
    }
    lhs << "}" << std::endl;
    return lhs;
}

// ----------------------------------------------------------------------------

}
#endif
