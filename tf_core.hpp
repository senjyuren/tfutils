#ifndef TFUTILS_TF_CORE_HPP
#define TFUTILS_TF_CORE_HPP

#include <iostream>

#include <list>

using Jchar = char;
using Jint = int;
using Jlong = long long;
using Jbyte = unsigned char;
using Juint = unsigned int;
using Julong = unsigned long;
using Jbool = bool;
using Jsize = size_t;
using Jfloat = float;

template <class T> struct RemoveType { using Type = T; };

template <class T> using SP = std::shared_ptr<T>;
template <class T> using UP = std::unique_ptr<T>;

template <class T>
constexpr static typename RemoveType<T>::Type &&move(typename RemoveType<T>::Type &&t) {
  return static_cast<typename RemoveType<T>::Type>(t);
}

template <class T, class... Args>
constexpr static SP<typename RemoveType<T>::Type> make(Args... args) {
  return std::make_shared<typename RemoveType<T>::Type>(args...);
}

#include "tf_common.hpp"

#include "tf_command.hpp"
#include "tf_command_adb.hpp"
#include "tf_command_os.hpp"
#include "tf_command_tensorflow_impl.hpp"
#include "tf_command_tensorflow_in.hpp"

#endif // TFUTILS_TF_CORE_HPP
