/*
    Copyright 2015 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

/**************************************************************************************************/

#include <iostream>
#include <stlab/future.hpp>
#include <utility>
#include <exception>
#include <stlab/channel.hpp>
#include <sstream>
using namespace stlab;
using namespace std;


#if 0

/*
Copyright 2015 Adobe Systems Incorporated
Distributed under the MIT License (see license at http://stlab.adobe.com/licenses.html)

This file is intended as example code and is not production quality.
*/

/**************************************************************************************************/

#include <boost/multiprecision/cpp_int.hpp>

/**************************************************************************************************/

using namespace boost::multiprecision;
using namespace stlab;

/**************************************************************************************************/

template <typename T, typename N, typename O>
T power(T x, N n, O op)
{
    if (n == 0) return identity_element(op);

    while ((n & 1) == 0) {
        n >>= 1;
        x = op(x, x);
    }

    T result = x;
    n >>= 1;
    while (n != 0) {
        x = op(x, x);
        if ((n & 1) != 0) result = op(result, x);
        n >>= 1;
    }
    return result;
}

/**************************************************************************************************/

template <typename N>
struct multiply_2x2 {
    array<N, 4> operator()(const array<N, 4>& x, const array<N, 4>& y)
    {
        return{ x[0] * y[0] + x[1] * y[2], x[0] * y[1] + x[1] * y[3],
            x[2] * y[0] + x[3] * y[2], x[2] * y[1] + x[3] * y[3] };
    }
};

template <typename N>
array<N, 4> identity_element(const multiply_2x2<N>&) { return{ N(1), N(0), N(0), N(1) }; }

template <typename R, typename N>
R fibonacci(N n) {
    if (n == 0) return R(0);
    return power(array<R, 4>{ 1, 1, 1, 0 }, N(n - 1), multiply_2x2<R>())[0];
}


/**************************************************************************************************/

int main() {
    auto x = async(default_scheduler(), [] { return fibonacci<cpp_int>(100); });

    auto y = x.then([](const cpp_int& x) { return cpp_int(x * 2); });
    auto z = x.then([](const cpp_int& x) { return cpp_int(x / 15); });

    while (!y.get_try()) {}
    while (!x.get_try()) {}
    cout << y.get_try().get() << endl;
    cout << z.get_try().get() << endl;
}
#endif

#if 0
void simple_continuation() {
    cout << "start tasking test" << endl;
    auto f1 = async(default_scheduler(), [] { return 42; }).then(
        [](auto answer) { cout << "The Answer to the Ultimate Question of Life, the Universe and Everything is " << answer << endl << flush; });
    f1.detach();
}

void when_all_with_multiple_arguments() {
    auto f2 = async(default_scheduler(), [] { return 10; });
    auto a1 = f2.then([](auto x) { return x + 2; });
    auto a2 = f2.then([](auto x) { return x + 3; });
    auto a4 = when_all(default_scheduler(), [](auto x, auto y) {
        cout << x << ", " << y << endl;
        return 4711;
    }, a1, a2);
    //a4.detach();
    while (!a4.get_try()) {}
    auto result = a4.get_try();
    cout << "Result " << *result << endl;
}

void when_all_with_empty_range() {
    std::vector<stlab::future<int>> emptyFutures;
    auto a5 = when_all(default_scheduler(), [](std::vector<int> v) {
        cout << "Result of no parallel tasks: " << v.size() << endl << flush;
    }, std::make_pair(emptyFutures.begin(), emptyFutures.end()));
    a5.detach();
}

void when_all_with_filled_range() {
    std::vector<stlab::future<int>> someFutures;
    someFutures.push_back(async(default_scheduler(), [] { return 1; }));
    someFutures.push_back(async(default_scheduler(), [] { return 2; }));
    someFutures.push_back(async(default_scheduler(), [] { return 3; }));
    someFutures.push_back(async(default_scheduler(), [] { return 4; }));

    auto a6 = when_all(default_scheduler(), [](std::vector<int> v) {
        cout << "Result of " << v.size() << " parallel executed tasks: ";
        for (auto i : v) {
            cout << i << " ";
        }
        cout << endl << flush;
        return 4711;
    }, std::make_pair(someFutures.begin(), someFutures.end()));

    while (!a6.get_try()) {}
    auto result = a6.get_try();
    cout << "Result " << *result << endl;
}

void continuation_with_error() {
    cout << "start tasking test" << endl;
    auto f1 = async(default_scheduler(), []() -> int { throw std::exception("Error in continuation"); }).then(
        [](auto answer) { cout << "The Answer to the Ultimate Question of Life, the Universe and Everything is " << answer << endl << flush; });

    try {
        while (!f1.get_try()) {}
    }
    catch (const std::exception& e) {
        cout << __FUNCTION__ << " " << e.what() << endl;
    }
}

void when_all_with_multiple_failing_arguments() {
    auto f2 = async(default_scheduler(), [] { return 10; });
    auto a1 = f2.then([](auto x) -> int { throw std::exception("Error in first argument"); });
    auto a2 = f2.then([](auto x) { return x + 3; });
    auto a4 = when_all(default_scheduler(), [](auto x, auto y) {
        cout << x << ", " << y << endl;
    }, a1, a2);

    try {
        while (!a4.get_try()) {}
    }
    catch (const std::exception& e) {
        cout << __FUNCTION__ << " " << e.what() << endl;
    }
}

void when_all_with_failing_range() {
    std::vector<stlab::future<int>> someFutures;
    someFutures.push_back(async(default_scheduler(), [] { return 1; }));
    someFutures.push_back(async(default_scheduler(), []() ->int { throw std::exception("Error in 2nd task"); }));
    someFutures.push_back(async(default_scheduler(), [] { return 3; }));
    someFutures.push_back(async(default_scheduler(), [] { return 4; }));

    auto a6 = when_all(default_scheduler(), [](std::vector<int> v) {
        cout << "Result of " << v.size() << " parallel executed tasks: ";
        for (auto i : v) {
            cout << i << " ";
        }
        cout << endl << flush;
    },
        std::make_pair(someFutures.begin(), someFutures.end()));

    try {
        while (!a6.get_try()) {}
    }
    catch (const std::exception& e) {
        cout << __FUNCTION__ << " " << e.what() << endl;
    }
}

void recover_with_a_continuation() {
    auto f1 = async(default_scheduler(), []() -> int { throw std::exception("My fault"); });
    f1.then([](auto x) { return 2; }).then([](auto x) { cout << x << endl; });
    auto a2 = f1.recover([](auto x) { return 3; }).then([](auto x) {
        cout << "Recovered from error and got: " << x << endl;
    });
    //a4.detach();
    while (!a2.get_try()) {}
}

void passivProgressExample()
{
    stlab::progress_tracker pt1;

    auto f1 = async(custom_scheduler<0>(), pt1([&_pt = pt1] { std::cout << _pt.completed() << std::endl; return 42; }))
        .then(pt1([&_pt = pt1](int x) { std::cout << _pt.completed() << std::endl; return x + 42; }));

    while (!f1.get_try());
}

void activeProgressExample()
{
    auto progress_callback = [](size_t task_number, size_t done_tasks) {
        std::cout << done_tasks << "/" << task_number << " tasks performed." << std::endl;
    };
    stlab::progress_tracker pt2(progress_callback);

    auto f2 = async(custom_scheduler<0>(), pt2([] { return 42; }))
        .then(pt2([](int x) { return x + 42; }));

    while (!f2.get_try());

    std::cout << "Result: " << *f2.get_try() << std::endl;
}
#endif

#include <tuple>
#include <iostream>

#include <stlab/future.hpp>
#include <stlab/channel.hpp>

using namespace stlab;
using namespace std;
using namespace std::chrono;

struct sum {
    process_state_scheduled _state = await_forever;
    int _sum = 0;

    void await(int n) { _sum += n; }

    int yield() { _state = await_forever; return _sum; }

    void close() { _state = yield_immediate; }

    const auto& state() const { return _state; }
};

void channelExample() {

    sender<int> send;
    receiver<int> receive;

    tie(send, receive) = channel<int>(default_scheduler());

    std::atomic_bool all_done{ false };
    auto hold = receive
        | sum()
        | [&_all_done = all_done](int x) { cout << x << '\n'; if (x == 6) _all_done = true; };

    receive.set_ready();

    send(1);
    send(2);
    send(3);
    send.close();

    while (!all_done.load()) {
        this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main(int argc, char **argv)
{
#if 0
    simple_continuation();
    when_all_with_multiple_arguments();
    when_all_with_empty_range();
    when_all_with_filled_range();
    continuation_with_error();
    when_all_with_multiple_failing_arguments();
    when_all_with_failing_range();
    recover_with_a_continuation();
    passivProgressExample();
    activeProgressExample();

#endif // 0    
    channelExample();
    int i;
    cin >> i;
}
