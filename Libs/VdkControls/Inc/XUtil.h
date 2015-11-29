/***************************************************************
 * Name:      XUtil.cpp
 * Purpose:   一些数学便利函数
 * Author:    Wang Xiaoning
 * Created:   2011-04-10
 **************************************************************/
#pragma once

/// \brief 前移迭代器
template <class T>
void adv(T &t, size_t n) {
    for (size_t i = 0; i < n ; i++) {
        t++;
    }
}

/// \brief 求平方
template<class T>
T xsqr(T x) {
    return x * x;
}

/// \brief 求两变量中较小者
template<class T>
T xmin(T x, T y) {
    return x < y ? x : y;
}

/// \brief 求两变量中较大者
template<class T>
T xmax(T x, T y) {
    return x > y ? x : y;
}

/// \brief 交换两变量
template<class T>
void xswap(T &x, T &y) {
    T temp = x;
    x = y;
    y = temp;
}

/// \brief 求平均值
template<class T, class Retval>
Retval xmean_value(T data[], int count) {
    Retval sum = 0;
    for (int i = 0; i < count; i++) {
        sum += data[i];
    }

    return sum / count;
}

/// \brief 求方差
template<class T, class Retval>
Retval xvariance(T data[], int count) {
    Retval mean = xmean_value<T, Retval>(data, count);
    Retval variance = 0;
    for (int i = 0; i < count; i++) {
        variance += xsqr(data[i] - mean);
    }

    return variance;
}

/// \brief 删除指针
template<class T>
void xdelete(T *&p) {
    if (p) {
        delete p;
        p = NULL;
    }
}

/// \brief 删除数组指针
template<class T>
void xdelete_a(T *&p) {
    if (p) {
        delete [] p;
        p = NULL;
    }
}

/// \brief 删除容器中的指针所指向的对象
template<class Container, class Iterator>
void xdel_ptrs(Container &set, Iterator first, Iterator last) {
    Iterator i = first;
    for (; i != last; ++i) {
        delete *i;
    }
}

/// \brief 删除容器中的指针对象并回收资源
template<class Container, class Iterator>
void xerase_ptrs(Container &set, Iterator first, Iterator last) {
    xdel_ptrs(set, first, last);
    set.erase(first, last);
}

#if defined( _MSC_VER ) && _MSC_VER <= 1200
#   define countof( array ) (sizeof( array ) / sizeof( array[0] ))
#else
/// \brief 计算数组元素个数
template<typename T, size_t N>
char(&_ArraySizeHelper(T(&array)[N]))[N];
#   define countof( array ) (sizeof( _ArraySizeHelper( array ) ))
#endif
