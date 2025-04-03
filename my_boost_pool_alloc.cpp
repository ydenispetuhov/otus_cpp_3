#include <boost/pool/pool.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <iostream>
#include <map>
#include <vector>
#include <utility>

using Pool = boost::pool<boost::default_user_allocator_new_delete>;

const int DEFAULT_SIZE_POOL = 10;

template <typename T> struct my_pool_alloc_vec {

    using value_type = T;

	~my_pool_alloc_vec() = default;

    my_pool_alloc_vec(Pool& pool) : _pool(pool) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        assert(pool_size() >= sizeof(T));
    }

    template <typename U>
    my_pool_alloc_vec(my_pool_alloc_vec<U> const& other) : _pool(other._pool) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        assert(pool_size() >= sizeof(U));
    }

    T *allocate(const size_t n) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        T* ret = static_cast<T*>(_pool.ordered_malloc(n));
        if (!ret && n) throw std::bad_alloc();
        return ret;
    }


    void deallocate(T* ptr, const size_t n) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        if (ptr && n) _pool.ordered_free(ptr, n);
    }

    // for comparing
    size_t pool_size() const { return _pool.get_requested_size(); }

  private:
    Pool& _pool;
};

template <class T, class U> bool operator==(const my_pool_alloc_vec<T> &a, const my_pool_alloc_vec<U> &b) { return a.pool_size()==b.pool_size(); }
template <class T, class U> bool operator!=(const my_pool_alloc_vec<T> &a, const my_pool_alloc_vec<U> &b) { return a.pool_size()!=b.pool_size(); }

template <typename T>
class my_pool_alloc_map {
public:
    using value_type = T;

    my_pool_alloc_map() : pool_(sizeof(T) * DEFAULT_SIZE_POOL/*nrequested_size*/, 32/*nnext_size*/, 32/*nmax_size*/) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    my_pool_alloc_map(const size_t size) : pool_(sizeof(T) * size) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    template <typename U>
    my_pool_alloc_map(const my_pool_alloc_map<U>&) : pool_(sizeof(T)) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    my_pool_alloc_map(Pool& pool) : pool_(pool) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    T* allocate(std::size_t n) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        if (n > 1) {
            throw std::bad_alloc();
        }
        std::cout << "pool_.size = " << pool_.get_max_size() << std::endl;
        return static_cast<T*>(pool_.malloc());
    }

    void deallocate(T* p, std::size_t n) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        pool_.free(p);
    }

    private:
    Pool pool_;
};

template <class T, class U> bool operator==(const my_pool_alloc_map<T> &a, const my_pool_alloc_map<U> &b) { return a.pool_size()==b.pool_size(); }
template <class T, class U> bool operator!=(const my_pool_alloc_map<T> &a, const my_pool_alloc_map<U> &b) { return a.pool_size()!=b.pool_size(); }


int factorial(int n) {
    if (n == 0) return 1;
    return n * factorial(n - 1);
};

int main() {
    //using boost::container::vector;
    using std::vector;

    Pool pool(DEFAULT_SIZE_POOL);
 
    auto v = std::vector<int, my_pool_alloc_vec<int>>(10, pool);
	// v.reserve(5);
	for (int i = 0; i < 10; ++i)
	{
		std::cout << "vector size = " << v.size() << std::endl;
		v.emplace_back(i);
		std::cout << std::endl;
	}

    auto m1 = std::map<int, int, std::less<int>, boost::pool_allocator<std::pair<const int, int>>>{};
    auto m2 = std::map<int, int, std::less<int>, my_pool_alloc_map<std::pair<const int, int>>>{};

    std::cout << " fill map1 "  << std::endl;
    // fill map1
	for (int i = 0; i < 10; ++i)
	{
         m1.insert(std::pair<int, int>(i, factorial(i)));
         std::cout << "map1 size = " << m1.size() << std::endl;
	}

    std::cout << " fill map2 "  << std::endl;
    // fill map2
	for (int i = 0; i < 10; ++i)
	{
         m2.insert(std::pair<int, int>(i, factorial(i)));
         std::cout << "map2 size = " << m2.size() << std::endl;
	}

    std::cout << " output map1 "  << std::endl;
    //output map1
    for (const auto& entry1 : m1)
    {
        std::cout << "key = " << entry1.first << " -> " <<  "value = " << entry1.second << std::endl;
    }

    std::cout << " output map2 "  << std::endl;
    //output map2
    for (const auto& entry2 : m2)
    {
        std::cout << "key = " << entry2.first << " -> " <<  "value = " << entry2.second << std::endl;
    }

	return 0;
}
