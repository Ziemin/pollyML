#pragma once

#include <isl/isl-noexceptions.h>

namespace pollyML {

template <typename T>
class NonCopyable {

protected:
  NonCopyable() = default;
  ~NonCopyable() = default;
  NonCopyable(NonCopyable&& other) = default;
  NonCopyable& operator=(NonCopyable&& other) = default;

private:
  NonCopyable(const NonCopyable& other);
  NonCopyable& operator=(const NonCopyable& other);
};


template <typename T>
T* get_id_user(isl::id id) {
    return static_cast<T*>(id.get_user());
}

} // namespace pollyML
