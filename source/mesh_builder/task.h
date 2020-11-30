// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <stack>

constexpr static std::uint32_t PERCENT_MULTIPLIER = 1000;

struct Task
{
    std::uint32_t percent_start = 0;
    std::uint32_t percent_end = 100 * PERCENT_MULTIPLIER;
    std::uint32_t expected_steps = 10;
    std::uint32_t current_step = 0;
};

struct ProgressCalculator
{
    std::stack<Task> tasks;

    static ProgressCalculator& getInstance()
    {
        static ProgressCalculator progress_calculator;
        return progress_calculator;
    }
    std::uint32_t getProgress(std::uint32_t current_step) const
    {
        if (!tasks.empty())
        {
            const Task& top = tasks.top();
            const std::uint32_t delta = top.percent_end - top.percent_start;
            const std::uint32_t single_step = delta / top.expected_steps;
            return top.percent_start + single_step * current_step;
        }
        else
        {
            return 0;
        }
    }
    std::uint32_t getCurrentProgress() const
    {
        if (!tasks.empty())
        {
            const Task& top = tasks.top();
            return getProgress(top.current_step);
        }
        else
        {
            return 0;
        }
    }
    void startJob(std::uint32_t expected_steps)
    {
        Task new_task;
        new_task.expected_steps = expected_steps;
        if (!tasks.empty())
        {
            new_task.percent_start = getCurrentProgress();
            new_task.percent_end = getProgress(tasks.top().current_step + 1);
        }
        tasks.push(new_task);
    }
    void endJob()
    {
        assert(!tasks.empty());
        tasks.pop();
        if (!tasks.empty())
        {
            ++tasks.top().current_step;
        }
    }
    void step()
    {
        assert(!tasks.empty());
        if (!tasks.empty())
        {
            ++tasks.top().current_step;
        }
    }
};

class Job
{
    Job(std::uint32_t expected_steps)
    {
        ProgressCalculator& progress_calculator = ProgressCalculator::getInstance();
        progress_calculator.startJob(expected_steps);
    }
    ~Job()
    {
        ProgressCalculator& progress_calculator = ProgressCalculator::getInstance();
        progress_calculator.endJob();
    }
    void step()
    {
        ProgressCalculator& progress_calculator = ProgressCalculator::getInstance();
        progress_calculator.step();
    }
};
