#pragma once

#include "CoolD_Type.h"

namespace CoolD
{
	template <typename T>
	class DMatrix
	{
	private:
		vector< vector<T> > m_Mat;
		Duint m_Rows;
		Duint m_Cols;

	public:
		DMatrix(Duint rows, Duint cols, const T& initial);
		DMatrix(Duint rows, Duint cols, const T* pMatrix);
		DMatrix(const DMatrix<T>& rhs);
		virtual ~DMatrix();

		DMatrix<T>& operator=(const DMatrix<T>& rhs);

		//행렬 계산
		DMatrix<T> operator+(const DMatrix<T>& rhs);
		DMatrix<T>& operator+=(const DMatrix<T>& rhs);
		DMatrix<T> operator-(const DMatrix<T>& rhs);
		DMatrix<T>& operator-=(const DMatrix<T>& rhs);
		DMatrix<T> operator*(const DMatrix<T>& rhs);
		DMatrix<T>& operator*=(const DMatrix<T>& rhs);
		DMatrix<T> transpose();

		//스칼라 계산
		DMatrix<T> operator+(const T& rhs);
		DMatrix<T> operator-(const T& rhs);
		DMatrix<T> operator*(const T& rhs);
		DMatrix<T> operator/(const T& rhs);

		// 행렬 벡터 연산 
		vector<T> operator*(const vector<T>& rhs);
		vector<T> diag_vec();

		// 접근자
		T& operator()(const Duint& row, const Duint& col);
		const T& operator()(const Duint& row, const Duint& col) const;

		// 사이즈
		Duint get_rows() const;
		Duint get_cols() const;

	};

	template<typename T>
	DMatrix<T>::DMatrix(Duint rows, Duint cols, const T& initial)
	{
		m_Mat.resize(rows);
		for( Duint i = 0; i < m_Mat.size(); i++ ) {
			m_Mat[ i ].resize(cols, initial);
		}
		m_Rows = rows;
		m_Cols = cols;
	}

	template<typename T>
	DMatrix<T>::DMatrix(Duint rows, Duint cols, const T* pMatrix)
	{
		m_Rows = rows;
		m_Cols = cols;

		m_Mat.resize(rows);
		for( Duint i = 0; i < m_Mat.size(); i++ )
		{
			m_Mat[ i ].resize(cols);
			for( Duint j = 0; j < m_Mat[ i ].size(); j++ )
			{				
				m_Mat[ i ][ j ] = pMatrix[ i  * m_Cols  + j ];
			}
		}		
	}

	template<typename T>
	DMatrix<T>::DMatrix(const DMatrix<T>& rhs)
	{
		m_Mat = rhs.m_Mat;
		m_Rows = rhs.get_rows();
		m_Cols = rhs.get_cols();
	}

	template<typename T>
	DMatrix<T>::~DMatrix() {}

	template<typename T>
	DMatrix<T>& DMatrix<T>::operator=(const DMatrix<T>& rhs)
	{
		if( &rhs == this )
			return *this;

		Duint new_rows = rhs.get_rows();
		Duint new_cols = rhs.get_cols();

		m_Mat.resize(new_rows);
		for( Duint i = 0; i < m_Mat.size(); i++ ) {
			m_Mat[ i ].resize(new_cols);
		}

		for( Duint i = 0; i < new_rows; i++ ) {
			for( Duint j = 0; j < new_cols; j++ ) {
				m_Mat[ i ][ j ] = rhs(i, j);
			}
		}
		m_Rows = new_rows;
		m_Cols = new_cols;

		return *this;
	}


	template<typename T>
	DMatrix<T> DMatrix<T>::operator+(const DMatrix<T>& rhs)
	{
		DMatrix result(m_Rows, m_Cols, 0.0);

		for( Duint i = 0; i < m_Rows; i++ ) {
			for( Duint j = 0; j < m_Cols; j++ ) {
				result(i, j) = m_Mat[ i ][ j ] + rhs(i, j);
			}
		}

		return result;
	}


	template<typename T>
	DMatrix<T>& DMatrix<T>::operator+=(const DMatrix<T>& rhs)
	{
		Duint m_Rows = rhs.get_rows();
		Duint m_Cols = rhs.get_cols();

		for( Duint i = 0; i < m_Rows; i++ ) {
			for( Duint j = 0; j < m_Cols; j++ ) {
				m_Mat[ i ][ j ] += rhs(i, j);
			}
		}

		return *this;
	}

	template<typename T>
	DMatrix<T> DMatrix<T>::operator-(const DMatrix<T>& rhs)
	{
		Duint m_Rows = rhs.get_rows();
		Duint m_Cols = rhs.get_cols();
		DMatrix result(m_Rows, m_Cols, 0.0);

		for( Duint i = 0; i < m_Rows; i++ ) {
			for( Duint j = 0; j < m_Cols; j++ ) {
				result(i, j) = m_Mat[ i ][ j ] - rhs(i, j);
			}
		}

		return result;
	}

	template<typename T>
	DMatrix<T>& DMatrix<T>::operator-=(const DMatrix<T>& rhs)
	{
		Duint m_Rows = rhs.get_rows();
		Duint m_Cols = rhs.get_cols();

		for( Duint i = 0; i < m_Rows; i++ ) {
			for( Duint j = 0; j < m_Cols; j++ ) {
				m_Mat[ i ][ j ] -= rhs(i, j);
			}
		}

		return *this;
	}

	template<typename T>
	DMatrix<T> DMatrix<T>::operator*(const DMatrix<T>& rhs)
	{
		Duint m_Rows = rhs.get_rows();
		Duint m_Cols = rhs.get_cols();
		DMatrix result(m_Rows, m_Cols, 0.0);

		for( Duint i = 0; i < m_Rows; i++ ) {
			for( Duint j = 0; j < m_Cols; j++ ) {
				for( Duint k = 0; k < m_Rows; k++ ) {
					result(i, j) += m_Mat[ i ][ k ] * rhs(k, j);
				}
			}
		}

		return result;
	}

	template<typename T>
	DMatrix<T>& DMatrix<T>::operator*=(const DMatrix<T>& rhs)
	{
		DMatrix result = (*this) * rhs;
		(*this) = result;
		return *this;
	}

	template<typename T>
	DMatrix<T> DMatrix<T>::transpose()
	{
		DMatrix result(m_Rows, m_Cols, 0.0);

		for( Duint i = 0; i < m_Rows; i++ ) {
			for( Duint j = 0; j < m_Cols; j++ ) {
				result(i, j) = m_Mat[ j ][ i ];
			}
		}

		return result;
	}


	template<typename T>
	DMatrix<T> DMatrix<T>::operator+(const T& rhs)
	{
		DMatrix result(m_Rows, m_Cols, 0.0);

		for( Duint i = 0; i < m_Rows; i++ ) {
			for( Duint j = 0; j < m_Cols; j++ ) {
				result(i, j) = m_Mat[ i ][ j ] + rhs;
			}
		}

		return result;
	}

	template<typename T>
	DMatrix<T> DMatrix<T>::operator-(const T& rhs)
	{
		DMatrix result(m_Rows, m_Cols, 0.0);

		for( Duint i = 0; i < m_Rows; i++ ) {
			for( Duint j = 0; j < m_Cols; j++ ) {
				result(i, j) = m_Mat[ i ][ j ] - rhs;
			}
		}

		return result;
	}

	template<typename T>
	DMatrix<T> DMatrix<T>::operator*(const T& rhs)
	{
		DMatrix result(m_Rows, m_Cols, 0.0);

		for( Duint i = 0; i < m_Rows; i++ ) {
			for( Duint j = 0; j < m_Cols; j++ ) {
				result(i, j) = m_Mat[ i ][ j ] * rhs;
			}
		}

		return result;
	}

	template<typename T>
	DMatrix<T> DMatrix<T>::operator/(const T& rhs)
	{
		DMatrix result(m_Rows, m_Cols, 0.0);

		for( Duint i = 0; i < m_Rows; i++ ) {
			for( Duint j = 0; j < m_Cols; j++ ) {
				result(i, j) = m_Mat[ i ][ j ] / rhs;
			}
		}

		return result;
	}

	template<typename T>
	std::vector<T> DMatrix<T>::operator*(const std::vector<T>& rhs)
	{
		std::vector<T> result(rhs.size(), 0.0);

		for( Duint i = 0; i < m_Rows; i++ ) {
			for( Duint j = 0; j < m_Cols; j++ ) {
				result[ i ] = m_Mat[ i ][ j ] * rhs[ j ];
			}
		}

		return result;
	}

	template<typename T>
	std::vector<T> DMatrix<T>::diag_vec()
	{
		std::vector<T> result(m_Rows, 0.0);

		for( Duint i = 0; i < m_Rows; i++ ) {
			result[ i ] = m_Mat[ i ][ i ];
		}

		return result;
	}

	template<typename T>
	T& DMatrix<T>::operator()(const Duint& row, const Duint& col)
	{
		return m_Mat[ row ][ col ];
	}

	template<typename T>
	const T& DMatrix<T>::operator()(const Duint& row, const Duint& col) const
	{
		return m_Mat[ row ][ col ];
	}

	template<typename T>
	Duint DMatrix<T>::get_rows() const
	{
		return m_Rows;
	}

	template<typename T>
	Duint DMatrix<T>::get_cols() const
	{
		return m_Cols;
	}
}