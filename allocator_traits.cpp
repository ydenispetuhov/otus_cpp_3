#include <iostream>
#include <memory>
#include <vector>
#include <list>
#include <map>

template <class, class Alloc, class... Args>
struct has_construct : std::false_type
{
};

template <class Alloc, class... Args>
struct has_construct<decltype((void)std::declval<Alloc>().construct(std::declval<Args>()...)), Alloc, Args...> : std::true_type
{
};

template <class Alloc>
struct std_allocator_traits
{
    using pointer = typename Alloc::value_type *;
    using const_pointer = const typename Alloc::value_type *;
    //...other typedefs

    static pointer allocate(Alloc &a, std::size_t n)
    {
        return a.allocate(n);
    }

    static void deallocate(Alloc &a, pointer p, std::size_t n) noexcept
    {
        a.deallocate(p, n);
    }

    template <class Tp, class... Args, class = std::enable_if<has_construct<Alloc, Tp *, Args...>::value>>
    static void construct(Alloc &a, Tp *p, Args &&...args)
    {
        a.construct(p, std::forward<Args>(args)...);
    }

    template <class Tp, class... Args, class = void, class = std::enable_if<!has_construct<Alloc, Tp *, Args...>::value>>
    static void construct(Alloc &, Tp *p, Args &&...args)
    {
        ::new ((void *)p) Tp(std::forward<Args>(args)...);
    }
};

template <class T>
struct std_11_simple_allocator
{
    using value_type = T;

    std_11_simple_allocator() noexcept {}
    template <class U>
    std_11_simple_allocator(const std_11_simple_allocator<U> &) noexcept {}

    T *allocate(std::size_t n)
    {
        return static_cast<T *>(::operator new(n * sizeof(T)));
    }
    void deallocate(T *p, std::size_t n)
    {
        ::operator delete(p);
    }
};

template <class T, class U>
constexpr bool operator==(const std_11_simple_allocator<T> &a1, const std_11_simple_allocator<U> &a2) noexcept
{
    return true;
}

template <class T, class U>
constexpr bool operator!=(const std_11_simple_allocator<T> &a1, const std_11_simple_allocator<U> &a2) noexcept
{
    return false;
}

template <class T, class Allocator = std::allocator<T>>
class my_vector
{
    void push_back(const T &x)
    {
        if (size == capacity)
        {
            capacity = capacity * 2 + 1;
            T *newData = std::allocator_traits<T>::allocate(alloc, capacity);
            std::copy(data, data + capacity * sizeof(T), newData); // naive
            std::swap(newData, data);
            std::allocator_traits<T>::deallocate(alloc, newData, capacity);
        }

        std::allocator_traits<T>::construct(alloc, data + size * sizeof(T), x);
        ++size;
    }

private:
    std::size_t size = 0;
    std::size_t capacity = 0;
    T *data = nullptr;

    Allocator alloc;
};

struct deleter
{
    void operator()(void *ptr)
    {
        ::operator delete(ptr);
    }
};

// this is demo implementation of allocator
// it won't work fully with containers because of dummy allocate and deallocate methods
template <class T>
struct cpp_11_allocator
{
    using value_type = T;

    std::shared_ptr<void> pool;
    static constexpr std::size_t PoolSize = 1000;

    cpp_11_allocator() noexcept
        : pool(::operator new(sizeof(uint8_t) * PoolSize), deleter())
    {
    }

    template <class U>
    cpp_11_allocator(const cpp_11_allocator<U> &a) noexcept
    {
        pool = a.pool;
    }

    cpp_11_allocator select_on_container_copy_construction() const
    {
        std::cout << "cpp_11_allocator::select_on_container_copy_construction()" << std::endl;
        return cpp_11_allocator();
    }

    T *allocate(std::size_t n)
    {
        return static_cast<T *>(pool.get()); // dummy implementation
    }
    void deallocate(T *p, std::size_t n)
    {
        // implementation
    }

    template <class U>
    struct rebind
    {
        typedef cpp_11_allocator<U> other;
    };

    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type; // UB if std::false_type and a1 != a2;
};

template <class T, class U>
constexpr bool operator==(const cpp_11_allocator<T> &a1, const cpp_11_allocator<U> &a2) noexcept
{
    return a1.pool == a2.pool;
}

template <class T, class U>
constexpr bool operator!=(const cpp_11_allocator<T> &a1, const cpp_11_allocator<U> &a2) noexcept
{
    return a1.pool != a2.pool;
}

int main()
{

    std::vector<int, std_11_simple_allocator<int>> v = {1, 2, 3, 4, 5};
    std::list<int, std_11_simple_allocator<int>> l;
    l.push_back(3);

    std::list<int, std_11_simple_allocator<int>> l2;
    l2 = std::move(l);

    std::map<int, int, std::less<int>, std_11_simple_allocator<std::pair<const int, int>>> m;
    m[1] = 1;

    // why we need operators == & !=
    std::vector<int, cpp_11_allocator<int>> v0(10, 37);
    auto v1 = std::vector<int, cpp_11_allocator<int>>{5};
    v1 = std::move(v0); // if operator ==() = false for 2 allocators,
    // and propagate_on_container_move_assignment = std::false_type, then move is copy

    std::vector<int, cpp_11_allocator<int>> v2(v1);

    return 0;
}
