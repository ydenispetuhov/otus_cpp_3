#include <memory>
#include <vector>
#include <iostream>
#include <memory_resource>
#include <array>
#include <list>

namespace motivation
{

    template <typename T>
    struct MyAllocator
    {
        using value_type = T;

        MyAllocator() = default;

        template <typename U>
        MyAllocator(const MyAllocator<U> &) {}

        T *allocate(std::size_t n)
        {
            auto p = std::malloc(n * sizeof(T));
            if (!p)
                throw std::bad_alloc();
            return reinterpret_cast<T *>(p);
        }

        void deallocate(T *p, std::size_t)
        {
            std::free(p);
        }
    };

    void calculations(const std::vector<int> &)
    {
        // some code here
    }

    void someFunction()
    {
        std::vector<int> values1{0, 1, 2, 3, 4, 5};
        calculations(values1); // Ok

        std::vector<int, MyAllocator<int>> values2{5, 4, 3, 2, 1};
        // calculations(values2); // Error

        std::vector<int> values3{10, 9, 8, 7, 6, 5};
        values1 = values3; // Ok

        // values1 = values2; // Error
    }
}

namespace solution
{

    // abstract base class
    class memory_resource
    {
    public:
        void *allocate(size_t bytes)
        {
            return do_allocate(bytes);
        }

    public:
        virtual void *do_allocate(size_t) = 0;
        virtual void do_deallocate(void *, size_t, size_t) = 0;
        virtual bool do_is_equal(memory_resource const &) const noexcept = 0;
    };

    template <class T>
    class polymorphic_allocator
    {
    public:
        using value_type = T;

        polymorphic_allocator() noexcept : res(std::pmr::get_default_resource()) {}

        // not explicit!
        polymorphic_allocator(std::pmr::memory_resource *__r) noexcept : res(__r) {}

        void *allocate(size_t n)
        {
            return static_cast<T *>(res->allocate(n * sizeof(T)));
        }

        std::pmr::memory_resource *resource() const
        {
            return res;
        }

        polymorphic_allocator select_on_container_copy_construction() const
        {
            return polymorphic_allocator();
        }

        // other implementation

    private:
        std::pmr::memory_resource *res;
    };

    template <class _ValueT>
    using vector = std::vector<_ValueT, std::pmr::polymorphic_allocator<_ValueT>>;

    template <typename T>
    struct MyResource : public std::pmr::memory_resource
    {
        using value_type = T;

        MyResource() = default;

        void *do_allocate(size_t n, size_t) override
        {
            std::cout << "allocate from my resource" << std::endl;
            auto p = std::malloc(n * sizeof(T));
            if (!p)
                throw std::bad_alloc();
            return p;
        }

        void do_deallocate(void *p, size_t, size_t) override
        {
            std::free(p);
        }

        bool do_is_equal(const std::pmr::memory_resource &) const noexcept override
        {
            return true;
        }
    };

    // pmr::vector??

    void calculations(const std::pmr::vector<int> &)
    {
        // some code here
    }

    void someFunction()
    {

        std::pmr::vector<int> values1{0, 1, 2, 3, 4, 5};
        calculations(values1); // Ok

        for (auto v : values1)
        {
            // std::cout << v;
        }

        MyResource<int> myResource;
        std::pmr::vector<int> values2(&myResource);
        calculations(values2); // Ok now!
        values2.push_back(5);

        std::pmr::vector<int> values4(values2);
        values4.push_back(10);
        values4.push_back(10);

        std::pmr::vector<int> values3{10, 9, 8, 7, 6, 5};
        values1 = values3; // Ok
        values1 = values2; // Ok
    }
}

template <typename Func>
auto benchmark(Func test_func, int iterations)
{
    const auto start = std::chrono::system_clock::now();
    while (iterations-- > 0)
    {
        test_func();
    }
    const auto stop = std::chrono::system_clock::now();
    const auto secs = std::chrono::duration<double>(stop - start);
    return secs.count();
}

void test_allocs()
{
    constexpr int iterations{100};
    constexpr int total_nodes{2'00'000};

    auto default_std_alloc = [total_nodes]
    {
        std::list<int> list;
        for (int i{}; i != total_nodes; ++i)
        {
            list.push_back(i);
        }
    };

    auto default_pmr_alloc = [total_nodes]
    {
        std::pmr::list<int> list;
        for (int i{}; i != total_nodes; ++i)
        {
            list.push_back(i);
        }
    };

    constexpr int total_nodes_size = total_nodes * 32;
    std::array<std::byte, total_nodes_size> buffer; // enough to fit in all nodes

    auto pmr_alloc_and_buf = [total_nodes, &buffer]
    {
        std::pmr::monotonic_buffer_resource mbr{buffer.data(), buffer.size()};
        std::pmr::polymorphic_allocator<int> pa{&mbr};
        std::pmr::list<int> list{pa};
        for (int i{}; i != total_nodes; ++i)
        {
            list.push_back(i);
        }
    };

    const double t1 = benchmark(default_std_alloc, iterations);
    const double t2 = benchmark(default_pmr_alloc, iterations);
    const double t3 = benchmark(pmr_alloc_and_buf, iterations);

    std::cout << std::fixed
              << "t1 (default std alloc): " << t1 << " sec;" << '\n'
              << "t2 (default pmr alloc): " << t2 << " sec;" << '\n'
              << "t3 (pmr alloc buf): " << t3 << " sec;" << '\n';
}

int main()
{
    // return global default memory resource
    std::pmr::memory_resource *default_resource = std::pmr::new_delete_resource();

    // throws bad_alloc when trying to allocate
    std::pmr::memory_resource *null_resource = std::pmr::null_memory_resource();

    std::array<unsigned char, 100000> memory;
    // not thread safe
    //  It is intended for very fast memory allocations in situations
    // where memory is used to build up a few objects and then is released all at once.
    std::pmr::monotonic_buffer_resource pool{memory.data(), memory.size(), null_resource};
    std::pmr::vector<std::pmr::string> container{&pool};
    // the same pool can be shared with other containers
    std::pmr::list<std::pmr::string> lst{&pool};

    // It consists of a collection of pools that serves requests for different block sizes.
    // Each pool manages a collection of chunks that are then divided into blocks of uniform size.
    // Calls to do_allocate are dispatched to the pool serving the smallest blocks accommodating the requested size.
    std::pmr::synchronized_pool_resource sync_pool;
    std::pmr::vector<int> v1(5, 10, &sync_pool);

    std::pmr::unsynchronized_pool_resource unsync_pool;
    std::pmr::vector<int> v2(5, 10, &unsync_pool);

    // resource is not copied by default via copy constructor
    std::pmr::vector<int> v3(v2);

    std::pmr::vector<int> v4(v2, &unsync_pool);

    test_allocs();
    solution::someFunction();

    return 0;
}
