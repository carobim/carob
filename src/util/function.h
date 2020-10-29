/*************************************
** Tsunagari Tile Engine            **
** function.h                       **
** Copyright 2019-2020 Paul Merrill **
*************************************/

// **********
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// **********

#ifndef SRC_UTIL_FUNCTION_H_
#define SRC_UTIL_FUNCTION_H_

#include "util/align.h"
#include "util/assert.h"
#include "util/meta.h"
#include "util/move.h"
#include "util/new.h"
#include "util/noexcept.h"

#pragma warning(push)
#pragma warning(disable : 26495)  // Always initialize a member variable.

#ifdef _MSC_VER
#define NO_VTABLE __declspec(novtable)
#else
#define NO_VTABLE
#endif

template<class Class, class Ret, class This, class... Args>
auto
invoke(Ret Class::*f, This&& t, Args&&... args) noexcept
        -> decltype((forward_<This>(t).*f)(forward_<Args>(args)...)) {
    return (forward_<This>(t).*f)(forward_<Args>(args)...);
}

template<class F, class... Args>
auto
invoke(F&& f, Args&&... args) noexcept
        -> decltype(forward_<F>(f)(forward_<Args>(args)...)) {
    return forward_<F>(f)(forward_<Args>(args)...);
}

template<class F>
class Function;  // Undefined.

namespace function {
    template<class R>
    class base;

    template<class R, class... ArgTypes>
    class NO_VTABLE base<R(ArgTypes...) noexcept> {
        base(const base&) noexcept;
        base&
        operator=(const base&) noexcept;

     public:
        base() noexcept = default;
        virtual ~base() noexcept = default;
        virtual base*
        clone() const noexcept = 0;
        virtual void
        clone(base*) const noexcept = 0;
        virtual void
        destroy() noexcept = 0;
        virtual void
        destroyDeallocate() noexcept = 0;
        virtual R
        operator()(ArgTypes&&...) noexcept = 0;
    };

    template<class F, class R>
    class func;

    template<class F, class R, class... ArgTypes>
    class func<F, R(ArgTypes...) noexcept> final
            : public base<R(ArgTypes...) noexcept> {
        F f;

     public:
        explicit func(F&& f) noexcept : f(static_cast<F&&>(f)) {}
        explicit func(const F& f) noexcept : f(f) {}

        base<R(ArgTypes...) noexcept>*
        clone() const noexcept;
        void
        clone(base<R(ArgTypes...) noexcept>*) const noexcept;
        void
        destroy() noexcept;
        void
        destroyDeallocate() noexcept;
        R
        operator()(ArgTypes&&... args) noexcept;
    };

    template<class F, class R, class... ArgTypes>
    base<R(ArgTypes...)>*
    func<F, R(ArgTypes...) noexcept>::clone() const noexcept {
        // return new func(f);
        void* buf = malloc(sizeof(func));
        new (buf) func(f);
        return reinterpret_cast<base<R(ArgTypes...) noexcept>*>(buf);
    }

    template<class F, class R, class... ArgTypes>
    void
    func<F, R(ArgTypes...)>::clone(
            base<R(ArgTypes...) noexcept>* p) const noexcept {
        new (p) func(f);
    }

    template<class F, class R, class... ArgTypes>
    void
    func<F, R(ArgTypes...) noexcept>::destroy() noexcept {
        f.~F();
    }

    template<class F, class R, class... ArgTypes>
    void
    func<F, R(ArgTypes...) noexcept>::destroyDeallocate() noexcept {
        delete this;
    }

    template<class F, class R, class... ArgTypes>
    R
    func<F, R(ArgTypes...) noexcept>::operator()(ArgTypes&&... args) noexcept {
        // return invoke(f, forward_<ArgTypes>(args)...);
        return f(forward_<ArgTypes>(args)...);
    }
}  // namespace function

template<class R, class... ArgTypes>
class Function<R(ArgTypes...) noexcept> {
 private:
    typedef function::base<R(ArgTypes...) noexcept> base;

    static base*
    asBase(void* p) noexcept {
        return reinterpret_cast<base*>(p);
    }

    Align<void* [3]> buf;
    base* f;

 public:
    inline Function() noexcept : f(0) {}
    Function(const Function&) noexcept;
    Function(Function&&) noexcept;
    template<class F>
    Function(F) noexcept;
    ~Function() noexcept;

    template<class F>
    void
    set(F& something,
        EnableIf<sizeof(function::func<F, R(ArgTypes...) noexcept>) <=
                 sizeof(buf)> = True()) noexcept;
    template<class F>
    void
    set(F& something,
        EnableIf<!(sizeof(function::func<F, R(ArgTypes...) noexcept>) <=
                   sizeof(buf))> = True()) noexcept;

    Function&
    operator=(const Function&) noexcept;
    Function&
    operator=(Function&&) noexcept;
    template<class F>
    Function&
    operator=(F&&) noexcept;

    void
    swap(Function&) noexcept;

    inline explicit operator bool() const noexcept { return f != 0; }

    R
    operator()(ArgTypes...) const noexcept;
};

template<class R, class... ArgTypes>
Function<R(ArgTypes...) noexcept>::Function(const Function& other) noexcept {
    if (other.f == 0) {
        f = 0;
    }
    else if ((void*)other.f == &other.buf) {
        f = asBase(&buf);
        other.f->clone(f);
    }
    else {
        f = other.f->clone();
    }
}

template<class R, class... ArgTypes>
Function<R(ArgTypes...) noexcept>::Function(Function&& other) noexcept {
    if (other.f == 0) {
        f = 0;
    }
    else if ((void*)other.f == &other.buf) {
        f = asBase(&buf);
        other.f->clone(f);
    }
    else {
        f = other.f;
        other.f = 0;
    }
}

template<class R, class... ArgTypes>
template<class F>
Function<R(ArgTypes...) noexcept>::Function(F something) noexcept : f(0) {
    set(something);
}

template<class R, class... ArgTypes>
template<class F>
void
Function<R(ArgTypes...) noexcept>::set(
        F& something,
        EnableIf<sizeof(function::func<F, R(ArgTypes...) noexcept>) <=
                 sizeof(buf)>) noexcept {
    f = new ((void*)&buf)
            function::func<F, R(ArgTypes...) noexcept>(static_cast<F&&>(something));
}

template<class R, class... ArgTypes>
template<class F>
void
Function<R(ArgTypes...) noexcept>::set(
        F& something,
        EnableIf<!(sizeof(function::func<F, R(ArgTypes...) noexcept>) <=
                   sizeof(buf))>) noexcept {
    // f = new function::func<F, R(ArgTypes...)>(static_cast<F&&>(something));
    using T = function::func<F, R(ArgTypes...)>;

    void* buf = malloc(sizeof(T));
    new (buf) T(static_cast<F&&>(something));
    f = reinterpret_cast<T*>(buf);
}

template<class R, class... ArgTypes>
Function<R(ArgTypes...) noexcept>&
Function<R(ArgTypes...) noexcept>::operator=(const Function& other) noexcept {
    Function(other).swap(*this);
    return *this;
}

template<class R, class... ArgTypes>
Function<R(ArgTypes...) noexcept>&
Function<R(ArgTypes...) noexcept>::operator=(Function&& other) noexcept {
    this->~Function();
    if (other.f == 0) {
        f = 0;
    }
    else if ((void*)other.f == &other.buf) {
        f = asBase(&buf);
        other.f->clone(f);
    }
    else {
        f = other.f;
        other.f = 0;
    }
    return *this;
}

template<class R, class... ArgTypes>
template<class F>
Function<R(ArgTypes...) noexcept>&
Function<R(ArgTypes...) noexcept>::operator=(F&& other) noexcept {
    Function(forward_<F>(other)).swap(*this);
    return *this;
}

template<class R, class... ArgTypes>
Function<R(ArgTypes...) noexcept>::~Function() noexcept {
    if ((void*)f == &buf) {
        f->destroy();
    }
    else if (f) {
        f->destroyDeallocate();
    }
}

template<class R, class... ArgTypes>
void
Function<R(ArgTypes...) noexcept>::swap(Function& other) noexcept {
    if (&other == this) {
        return;
    }

    if ((void*)f == &buf && (void*)other.f == &other.buf) {
        alignas(alignof(void* [3])) char tempbuf[3 * sizeof(void*)];
        base* t = asBase(&tempbuf);
        f->clone(t);
        f->destroy();
        f = 0;
        other.f->clone(asBase(&buf));
        other.f->destroy();
        other.f = 0;
        f = asBase(&buf);
        t->clone(asBase(&other.buf));
        t->destroy();
        other.f = asBase(&other.buf);
    }
    else if ((void*)f == &buf) {
        f->clone(asBase(&other.buf));
        f->destroy();
        f = other.f;
        other.f = asBase(&other.buf);
    }
    else if ((void*)other.f == &other.buf) {
        other.f->clone(asBase(&buf));
        other.f->destroy();
        other.f = f;
        f = asBase(&buf);
    }
    else {
        swap_(f, other.f);
    }
}

template<class R, class... ArgTypes>
R
Function<R(ArgTypes...) noexcept>::operator()(ArgTypes... args) const noexcept {
    assert_(f != 0);
    return (*f)(forward_<ArgTypes>(args)...);
}

#pragma warning(pop)

#endif  // SRC_UTIL_FUNCTION_H_
