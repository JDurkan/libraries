/*
    Copyright 2015 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

/**************************************************************************************************/

#define BOOST_TEST_MODULE stlab_libraries_tests
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#if 0
#include <iostream>
#include <stlab/future.hpp>
#include <utility>
#include <exception>

using namespace stlab;
using namespace std;

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
    auto a2 = f1.recover([](auto x) { return 3; }).then([] (auto x) {
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

/*
sum is an example of an accumulating "co-routine". It will await for values, keeping an
internal sum, until the channel is closed and then it will yield the result as a string.
*/
struct sum {
    process_state _state = process_state::await;
    int _sum = 0;

    void await(int n) { _sum += n; }

    int yield() { _state = process_state::await; return _sum; }

    void close() { _state = process_state::yield; }

    auto state() const { return _state; }
};


void channelExample()
{
    /*
    Create a channel to aggregate our values.
    */
    sender<int> aggregate;
    receiver<int> receiver;
    tie(aggregate, receiver) = channel<int>();

    /*
    Create a vector to hold all the futures for each result as it is piped to channel.
    The future is of type <void> because the value is passed into the channel.
    */
    vector<stlab::future<void>> results;

    for (int n = 0; n != 10; ++n) {
        // Asynchronously generate a bunch of values.
        results.emplace_back(async(default_scheduler(), [_n = n] { return _n; })
            // Then send those values into a copy of the channel
            .then([_aggregate = aggregate](int n) {
            _aggregate(n);
        }));
    }
    // Now it is safe to close (or destruct) this channel, all the copies remain open.
    aggregate.close();

    auto pipe = receiver
        /*
        The receiver is our common end point - we attach the vector of futures to it (another)
        inefficiency here - this is a lambda whose only purpose is to hold the vector of
        futures.
        */
        | [_results = move(results)](auto x){ return x; }
    // Then we can pipe the values to our accumulator
    | sum()
        // And pipe the final value to a lambda to print it.
        // Returning void from the pipe will mark it as ready.
        | [](auto x) { cout << x << endl; };

    receiver.set_ready(); // close this end of the pipe

    // Wait for everything to execute (just for demonstration)
    //Sleep(100);
}


int main(int argc, char **argv)
{
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
    channelExample()

}

#endif