// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#include "rxcpp/rx.hpp"

namespace rxcpp {

namespace schedulers {

class relaxed_run_loop
{
private:
    typedef relaxed_run_loop this_type;
    // don't allow this instance to copy/move since it owns current_thread queue
    // for the thread it is constructed on.
    relaxed_run_loop(const this_type&);
    relaxed_run_loop(this_type&&);

    typedef scheduler::clock_type clock_type;
    typedef detail::schedulable_queue<detail::time_schedulable<scheduler_base::clock_type::time_point>> queue_type;

    typedef detail::run_loop_state::item_type item_type;
    typedef detail::run_loop_state::const_reference_item_type const_reference_item_type;

    std::shared_ptr<detail::run_loop_state> state;
    std::shared_ptr<run_loop_scheduler> sc;

public:
    relaxed_run_loop()
        : state(std::make_shared<detail::run_loop_state>())
        , sc(std::make_shared<run_loop_scheduler>(state))
    {
        // take ownership so that the current_thread scheduler
        // uses the same queue on this thread
//        queue_type::ensure(sc->create_worker_interface());
    }
    ~relaxed_run_loop()
    {
        state->lifetime.unsubscribe();

        std::unique_lock<std::mutex> guard(state->lock);

        // release ownership
//        queue_type::destroy();

        auto expired = std::move(state->q);
        // if (!state->q.empty()) std::terminate();
    }

    clock_type::time_point now() const {
        return clock_type::now();
    }
    
    composite_subscription get_subscription() const {
        return state->lifetime;
    }
    
    bool empty() const {
        return state->q.empty();
    }

    const_reference_item_type peek() const {
        return state->q.top();
    }

    void dispatch() const {
        std::unique_lock<std::mutex> guard(state->lock);
        if (state->q.empty()) {
            return;
        }
        auto& peek = state->q.top();
        if (!peek.what.is_subscribed()) {
            state->q.pop();
            return;
        }
        if (clock_type::now() < peek.when) {
            return;
        }
        auto what = peek.what;
        state->q.pop();
        state->r.reset(state->q.empty());
        guard.unlock();
        what(state->r.get_recurse());
    }

    scheduler get_scheduler() const {
        return make_scheduler(sc);
    }
};

inline scheduler make_relaxed_run_loop(const relaxed_run_loop& r) {
    return r.get_scheduler();
}

}

}