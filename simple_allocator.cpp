#ifndef __PRETTY_FUNCTION__
#include "pretty.h"
#endif

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <array>

template <class T, class Allocator = std::allocator<T>>
class my_vector
{
    void push_back(const T &x)
    {
        if (size == capacity)
        {
            capacity = capacity * 2 + 1;
            // T* newData = ::operator new(capacity * sizeof(T));
            T *newData = alloc.allocate(capacity);
            std::copy(data, data + capacity * sizeof(T), newData); // naive
            std::swap(newData, data);
            alloc.deallocate(newData, capacity);
        }

        alloc.construct(data + size * sizeof(T), x);
        ++size;
    }

private:
    size_t size = 0;
    size_t capacity = 0;
    T *data = nullptr;

    Allocator alloc;
};

my_vector<int, std::allocator<int>> m;

// stateless allocator
template <class T>
struct std_03_allocator
{
    typedef T value_type;
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T &reference;
    typedef const T &const_reference;

    std_03_allocator() noexcept {}
    template <class U>
    std_03_allocator(const std_03_allocator<U> &) noexcept {}

    T *allocate(size_t n)
    {
        return static_cast<T *>(::operator new(n * sizeof(T)));
    }
    void deallocate(T *p, size_t n)
    {
        ::operator delete(p);
    }

    template <class Up, class... Args>
    void construct(Up *p, Args &&...args)
    {
        ::new ((void *)p) Up(std::forward<Args>(args)...);
    }

    void destroy(pointer p)
    {
        p->~T();
    }

    template <class U>
    struct rebind
    {
        typedef std_03_allocator<U> other;
    };
};

template <typename T, typename Alloc>
class MyList
{
    struct Node
    {
        Node *next;
        T val;
    };

public:
    void push_back(const T &val)
    {
        // Node* newNode = allocator.allocate(1); //allocator is of type std_03_allocator<int>!
        typename Alloc::template rebind<Node>::other nodeAlloc;
        Node *newNode = nodeAlloc.allocate(1);
    }

private:
    Node *head;
    Alloc allocator;
};

template <class T, class U>
constexpr bool operator==(const std_03_allocator<T> &a1, const std_03_allocator<U> &a2) noexcept
{
    return true;
}

template <class T, class U>
constexpr bool operator!=(const std_03_allocator<T> &, const std_03_allocator<U> &) noexcept
{
    return false;
}

template <class T>
struct pool_allocator
{
    typedef T value_type;

    static size_t pos;
    static constexpr size_t size = sizeof(T) * 1000;
    static uint8_t data[size];

    pool_allocator() noexcept {}
    ~pool_allocator() {}

    template <class U>
    pool_allocator(const pool_allocator<U> &) noexcept {}

    T *allocate(size_t n)
    {
        if (pos + n > size)
            throw std::bad_alloc();

        size_t cur = pos;
        pos += n;
        return reinterpret_cast<T *>(data) + cur;
    }

    void deallocate(T *p, size_t n) {}

    template <class U>
    struct rebind
    {
        typedef pool_allocator<U> other;
    };
};

template <typename T>
uint8_t pool_allocator<T>::data[size];

template <typename T>
size_t pool_allocator<T>::pos = 0;

template <class T, class U>
constexpr bool operator==(const pool_allocator<T> &a1, const pool_allocator<U> &a2) noexcept
{
    return true;
}

template <class T, class U>
constexpr bool operator!=(const pool_allocator<T> &a1, const pool_allocator<U> &a2) noexcept
{
    return false;
}

int main(int, char *[])
{

    auto v = std::vector<int, std_03_allocator<int>>{};
    for (int i = 0; i < 6; ++i)
    {
        std::cout << "vector size = " << v.size() << std::endl;
        v.push_back(i);
    }

    // why we need rebind
    MyList<int, std_03_allocator<int>> list;
    list.push_back(1);

    // why 03 allocators stateless
    pool_allocator<int> a1;
    pool_allocator<int> a2;

    std::vector<int, pool_allocator<int>> v1(10, a1);
    std::vector<int, pool_allocator<int>> v2(10, a2);

    v1.swap(v2); // no swap support for allocators

    // why we need copy constructor
    std::allocator<int> stdAl;
    std::unique_ptr<int> smartPrt(stdAl.allocate(10));

    std::vector<int> vect(10, stdAl);

    std_03_allocator<int> al;
    std_03_allocator<float> al2(al);

    return 0;
}
