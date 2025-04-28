#include <boost/pool/pool.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <iostream>
#include <map>
#include <vector>
#include <array>
#include <utility>

using Pool = boost::pool<boost::default_user_allocator_new_delete>;

const int DEFAULT_SIZE_POOL = 10;



template <typename T, int def_size = DEFAULT_SIZE_POOL>
struct my_pool_alloc {
public:
    using value_type = T;

    ~my_pool_alloc() = default;

    template <typename U>
    struct rebind {
        using other = my_pool_alloc<U, def_size>;
    };

    my_pool_alloc() : pool_ (new Pool(sizeof(T) * def_size/*nrequested_size*/, 32/*nnext_size*/, 32/*nmax_size*/)) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    // template <typename U, typename... Args>
    // void construct(U* p, Args&&... args) {
    //     new (p) U(std::forward<Args>(args)...);
    // }

    my_pool_alloc(const size_t size) : pool_(new Pool(sizeof(T) * size)) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
    
    my_pool_alloc(T& ref_val, const size_t size) : pool_(new Pool(sizeof(T) * size)) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    my_pool_alloc(Pool& pool) : pool_(&pool) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        assert(pool_size() >= sizeof(T));
    }

    template <typename U>
    my_pool_alloc(my_pool_alloc<U> const& other) : pool_(other._pool) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        assert(pool_size() >= sizeof(U));
    }

      T *allocate(const size_t n) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        T* ret = static_cast<T*>(pool_->ordered_malloc(n));
        if (!ret && n) throw std::bad_alloc();
        return ret;
    }


    void deallocate(T* ptr, const size_t n) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        if (ptr && n) pool_->ordered_free(ptr, n);
    }

    // for comparing
    size_t pool_size() const { return pool_->get_requested_size(); }

    private:
    Pool* pool_;
};

template <class T, class U> bool operator==(const my_pool_alloc<T> &a, const my_pool_alloc<U> &b) { return a.pool_size()==b.pool_size(); }
template <class T, class U> bool operator!=(const my_pool_alloc<T> &a, const my_pool_alloc<U> &b) { return a.pool_size()!=b.pool_size(); }


int factorial(int n) {
    if (n == 0) return 1;
    return n * factorial(n - 1);
};


template <class T, class Allocator = std::allocator<T>>
class my_vector
{
public: 

    my_vector(){};

    my_vector(const Allocator other): alloc(other) {}

    my_vector(const size_t n, const Allocator other): alloc(other), capacity(n) {}

    void push_back(const T &x)
    {
        if (size == capacity)
        {
            capacity = capacity * 2 + 1;
            // T* newData = ::operator new(capacity * sizeof(T));
             T* tmp = alloc.allocate(capacity);
            for (size_t i = 0; i < size; i++) {
                tmp[i] = data[i];
            }

            alloc.deallocate(data, size);
            data = tmp;
        }

        data[size] = x;
        ++size;
    }

    T& operator[](std::size_t pos) {
    if (pos >= 0 && pos <= size) return *(this->data + pos);
    throw std::out_of_range("Out of bounds element access");
    }

    T *begin() { return data; };

    T *end() { return data + size; };

    const T *begin() const { return data; };

    const T *end() const { return data + size; };

private:
    size_t size = 0;
    size_t capacity = 0;
    T *data = nullptr;

    Allocator alloc;
};


int main() {
    //using boost::container::vector;
    using std::vector;

    Pool pool(DEFAULT_SIZE_POOL);
    auto vec_pool = my_pool_alloc<int>(pool);
    auto myvec1 = my_vector<int, my_pool_alloc<int>>(vec_pool);

    //fill myvec1
    for (int i = 0; i < 10; ++i)
	{
		myvec1.push_back(i);
	}
    // out myvec1
    for (auto const &elem: myvec1) {
            std::cout << elem << std::endl;
        }

    auto myvec2 = my_vector<int>();

    //fill myvec2
    for (int i = 0; i < 10; ++i)
	{
		myvec2.push_back(i);
	}
    // out myvec2
    for (auto const &elem: myvec2) {
            std::cout << elem << std::endl;
        }

    Pool pool2(DEFAULT_SIZE_POOL);
    auto vec_pool2 = my_pool_alloc<int>(pool2);
    auto v = std::vector<int, my_pool_alloc<int>>(vec_pool2);

    //fill v
	// v.reserve(5);
	for (int i = 0; i < 10; ++i)
	{
		std::cout << "vector size = " << v.size() << std::endl;
		v.emplace_back(i);
		std::cout << std::endl;
	}
    // out v
    for (auto const &elem: v) {
            std::cout << elem << std::endl;
        }

    auto m1 = std::map<int, int, std::less<int>, boost::pool_allocator<std::pair<const int, int>>>{};
    auto m2 = std::map<int, int, std::less<int>, my_pool_alloc<std::pair<const int, int>>>{};
    
    /*
    typedef std::map<
            int,
            int,
            std::less<int>,
            boost::fast_pool_allocator<std::pair<int, int>,boost::default_user_allocator_malloc_free, boost::details::pool::null_mutex, 1024>
        >  MyMap;
    
    MyMap m2;
    */

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
