#pragma once

#include <cassert> // assert
#include <cstddef> // size_t, ptrdiff_t
#include <utility> // move, forward

namespace kp11::detail
{
  /// Minimal static vector implementation.
  template<typename T, std::size_t N>
  class static_vector
  {
  public: // types
    using value_type = T;
    using pointer = T *;
    using const_pointer = T const *;
    using reference = value_type &;
    using const_reference = value_type const &;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    /// Could change this, but we'll keep it simple.
    using iterator = pointer;
    /// Could change this, but we'll keep it simple.
    using const_iterator = const_pointer;

  public: // constructors
    static_vector() noexcept
    {
    }
    static_vector(static_vector const & xs)
    {
      for (auto & x : xs)
      {
        push_back(x);
      }
    }
    static_vector & operator=(static_vector const & xs)
    {
      if (this != &xs)
      {
        clear();
        for (auto & x : xs)
        {
          push_back(x);
        }
      }
      return *this;
    }
    ~static_vector()
    {
      clear();
    }

  public: // iterators
    iterator begin() noexcept
    {
      return values;
    }
    const_iterator begin() const noexcept
    {
      return values;
    }
    iterator end() noexcept
    {
      return values + size();
    }
    const_iterator end() const noexcept
    {
      return values + size();
    }
    const_iterator cbegin() const noexcept
    {
      return begin();
    }
    const_iterator cend() const noexcept
    {
      return end();
    }

  public: // capacity
    [[nodiscard]] bool empty() const noexcept
    {
      return size() == 0;
    }
    size_type size() const noexcept
    {
      return length;
    }
    static constexpr size_type max_size() noexcept
    {
      return N;
    }
    static constexpr size_type capacity() noexcept
    {
      return N;
    }

  public: // element access
    reference operator[](size_type n)
    {
      assert(n <= size());
      return values[n];
    }
    const_reference operator[](size_type n) const
    {
      assert(n <= size());
      return values[n];
    }
    reference front()
    {
      assert(size() > 0);
      return values[0];
    }
    const_reference front() const
    {
      assert(size() > 0);
      return values[0];
    }
    reference back()
    {
      assert(size() > 0);
      return values[size() - 1];
    }
    const_reference back() const
    {
      assert(size() > 0);
      return values[size() - 1];
    }

  public: // modifiers
    template<class... Args>
    reference emplace_back(Args &&... args)
    {
      assert(size() != capacity());
      new (&values[length++]) T(std::forward<Args>(args)...);
      return back();
    }
    void push_back(T const & x)
    {
      emplace_back(x);
    }
    void push_back(T && x)
    {
      emplace_back(std::move(x));
    }
    void pop_back()
    {
      assert(size() > 0);
      values[--length].~T();
    }
    void clear()
    {
      for (; length > 0; --length)
      {
        values[length - 1].~T();
      }
    }

  private: // variables
    std::size_t length = 0;
    union {
      T values[N];
    };
  };
  // relational operators
  template<typename T, std::size_t N>
  bool operator==(static_vector<T, N> const & lhs, static_vector<T, N> const & rhs)
  {
    if (lhs.size() != rhs.size())
    {
      return false;
    }
    for (auto l_first = lhs.begin(), l_last = lhs.end(), r_first = rhs.begin(); l_first != l_last;
         ++l_first, ++r_first)
    {
      if (*l_first != *r_first)
      {
        return false;
      }
    }
    return true;
  }
  template<typename T, std::size_t N>
  bool operator!=(static_vector<T, N> const & lhs, static_vector<T, N> const & rhs)
  {
    return !(lhs == rhs);
  }
}