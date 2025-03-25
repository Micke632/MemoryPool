#include <cstdint>
#include <assert.h>
#include <type_traits>

static inline constexpr int getShifts(uint32_t size)
{
   switch (size)
   {
   case 4:return 2;
   case 8:return 3;
   case 16:return 4;
   case 32:return 5;
   case 64:return 6;
   case 128:return 7;
   case 256:return 8;
   case 512:return 9;
   }

   //todo
   return 0;
}

template <uint32_t U>
struct power_of_two
{
   //probably pointless optimization :). but wanted to practice templates
   inline constexpr uint32_t IndexFromAddr(const uint8_t* p, const uint8_t* mem_beg_) const
   {
      constexpr int shift = getShifts(U);
      return static_cast<uint32_t>(p - mem_beg_) >> shift;
   }

   inline constexpr  uint8_t* AddrFromIndex(uint8_t* mem_beg_, uint32_t i) const
   {
      constexpr int shift = getShifts(U);
      return mem_beg_ + (i << shift);
   }

};

template <uint32_t U>
struct standard
{
   inline constexpr uint32_t IndexFromAddr(const uint8_t* p, const uint8_t* mem_beg_) const
   {
      return static_cast<uint32_t>(p - mem_beg_) / U;
   }
   inline constexpr  uint8_t* AddrFromIndex(uint8_t* mem_beg_, uint32_t i) const
   {
      return mem_beg_ + (i * U);
   }
};

template<bool, uint32_t U>
struct selector;

template<uint32_t U>
struct selector<false, U>
{
   using type = standard<U>;
};
template<uint32_t U>
struct selector<true, U>
{
   using type = power_of_two<U>;
};


template<typename T, uint32_t U = sizeof(T)>
class MemPool
{
public:
   MemPool();
   ~MemPool();
   void CreatePool(uint32_t num_cells);
   void DestroyPool();
   T* Allocate();
   void Deallocate(void * p);

private:

   const uint32_t cell_size_;
   uint32_t num_cells_;
   int32_t num_free_cells_;
   uint32_t num_init_;
   uint8_t* mem_beg_;
   uint8_t* next_;

   static constexpr bool IsPowerOfTwo = !(U == 0) && !(U & (U - 1));

   using policy_type = typename selector<IsPowerOfTwo, U>::type;

   policy_type policy;

};

template<typename T, uint32_t U>
MemPool<T,U>::MemPool()
   : cell_size_(U)
   , num_cells_(0)
   , num_free_cells_(0)
   , num_init_(0)
   , mem_beg_(nullptr)
   , next_(nullptr)
{
   static_assert(sizeof(uint32_t) <= sizeof(T), "sizeof( T ) must be equal or greater than sizeof( uint32_t )");
   static_assert(std::is_trivial<T>::value, "MemPool can only handle trivial types");
}


template<typename T, uint32_t U>
MemPool<T,U>::~MemPool()
{
   DestroyPool();
}


template<typename T, uint32_t U>
void MemPool<T,U>::CreatePool(uint32_t num_cells)
{
   num_cells_ = num_cells;
   num_free_cells_ = num_cells_;
   mem_beg_ = new uint8_t[num_cells_ * cell_size_];
   next_ = mem_beg_;
}


template<typename T, uint32_t U>
void MemPool<T,U>::DestroyPool()
{
   delete[] mem_beg_;
   mem_beg_ = nullptr;
}


template<typename T, uint32_t U>
T* MemPool<T,U>::Allocate()
{
   assert(mem_beg_);

   if (num_init_ < num_cells_)
   {
      uint32_t* p = reinterpret_cast<uint32_t*>(policy.AddrFromIndex(mem_beg_, num_init_));
      *p = ++num_init_;
   }

   //next_ can be nullptr here
   T* res = reinterpret_cast<T*>(next_);

   if (--num_free_cells_ > 0)
   {
      next_ = policy.AddrFromIndex(mem_beg_, *reinterpret_cast<uint32_t*>(next_));
   }
   else
   {
      num_free_cells_ = 0;
      next_ = nullptr;
   }

   return res;
}


template<typename T, uint32_t U>
void MemPool<T,U>::Deallocate(void * p)
{
   assert(static_cast<uint8_t*>(p) >= mem_beg_);
   assert(static_cast<uint8_t*>(p) <= policy.AddrFromIndex(mem_beg_, num_cells_));

   *static_cast<uint32_t*>(p) = next_ == nullptr ? num_cells_ : policy.IndexFromAddr(next_, mem_beg_);
   next_ = static_cast<uint8_t*>(p);
   ++num_free_cells_;

   assert(num_free_cells_ <= num_cells_);
}


