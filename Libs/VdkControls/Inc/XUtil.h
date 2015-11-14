/***************************************************************
 * Name:      XUtil.cpp
 * Purpose:   һЩ��ѧ��������
 * Author:    Wang Xiao Ning
 * Created:   2011-04-10
 **************************************************************/
#pragma once

/// \brief ǰ�Ƶ�����
template < class T >
void adv(T& t, size_t n)
{
	for( size_t i = 0; i < n ; i++ )
		t++;
}

/// \brief ��ƽ��
template< class T >
T xsqr(T x) { return x * x; }

/// \brief ���������н�С��
template< class T >
T xmin(T x, T y) { return x < y ? x : y; }

/// \brief ���������нϴ���
template< class T >
T xmax(T x, T y) { return x > y ? x : y; }

/// \brief ����������
template< class T >
void xswap(T& x, T& y)
{
	T temp = x;
	x = y;
	y = temp;
}

/// \brief ��ƽ��ֵ
template< class T, class Retval >
Retval xmean_value(T data[], int count)
{
	Retval sum = 0;
	for( int i = 0; i < count; i++ )
		sum += data[i];

	return sum / count;
}

/// \brief �󷽲�
template< class T, class Retval >
Retval xvariance(T data[], int count)
{
	Retval mean = xmean_value< T, Retval >( data, count );
	Retval variance = 0;
	for( int i = 0; i < count; i++ )
		variance += xsqr( data[i] - mean );

	return variance;
}

/// \brief ɾ��ָ��
template< class T >
void xdelete(T*& p)
{
	if( p )
	{
		delete p;
		p = NULL;
	}
}

/// \brief ɾ������ָ��
template< class T >
void xdelete_a(T*& p)
{
	if( p )
	{
		delete [] p;
		p = NULL;
	}
}

/// \brief ɾ�������е�ָ����ָ��Ķ���
template< class Container, class Iterator >
void xdel_ptrs(Container& set, Iterator first, Iterator last)
{
	Iterator i = first;
	for( ; i != last; ++i )
		delete *i;
}

/// \brief ɾ�������е�ָ����󲢻�����Դ
template< class Container, class Iterator >
void xerase_ptrs(Container& set, Iterator first, Iterator last)
{
	xdel_ptrs( set, first, last );
	set.erase( first, last );
}

#if defined( _MSC_VER ) && _MSC_VER <= 1200
#	define countof( array ) (sizeof( array ) / sizeof( array[0] ))
#else
	/// \brief ��������Ԫ�ظ���
	template< typename T, size_t N >
	char(&_ArraySizeHelper(T(&array)[N] ))[N];
#	define countof( array ) (sizeof( _ArraySizeHelper( array ) ))
#endif
