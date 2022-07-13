#pragma once
#include "imgui.h"
#include <array>
#include <deque>
#include <string>
namespace yorcvs::ui {
class Performance_Window {
public:
    Performance_Window()
    {
        reset();
    }
    enum update_time_item : size_t {
        collision = 0,
        health = 1,
        stamina = 2,
        velocity = 3,
        animation = 4,
        behaviour = 5,
        overall = 6,
        update_time_tracked
    };
    // samples , max , min , avg
    enum update_time_sample_tuple_element : size_t {
        samples = 0,
        max = 1,
        min = 2,
        avg = 3,
        update_time_sample_tuple_elements
    };
    enum history_plotting : int {
        line,
        histogram,
        plotting_elements
    };

    template <update_time_item item>
    void record_update_time(float value)
    {
        std::deque<float>& queue = std::get<1>(update_time_history[static_cast<size_t>(item)]);
        auto& statistics = update_time_statistics[static_cast<size_t>(item)];
        if (queue.size() == update_time_maximum_samples) {
            queue.pop_front();
        }
        queue.push_back(value);

        // compute statistics
        std::get<update_time_sample_tuple_element::avg>(statistics) *= std::get<0>(statistics);
        std::get<update_time_sample_tuple_element::samples>(statistics) += 1.0f;
        if (std::get<update_time_sample_tuple_element::max>(statistics) < value) {
            std::get<update_time_sample_tuple_element::max>(statistics) = value;
        }
        if (std::get<update_time_sample_tuple_element::min>(statistics) > value) {
            std::get<update_time_sample_tuple_element::min>(statistics) = value;
        }
        std::get<update_time_sample_tuple_element::avg>(statistics) += value;
        std::get<update_time_sample_tuple_element::avg>(statistics) /= std::get<0>(statistics);
    }
    template <update_time_item... T>
    void record_update_time(const std::array<float, update_time_item::update_time_tracked>& added_parameters)
    {
        static size_t update_counter {};
        update_counter++;
        if (update_counter >= updates_per_sample) {
            (record_update_time<T>(added_parameters[T]), ...);
            update_counter = 0;
        }
    }
    void reset()
    {
        // clear graphs
        for (auto& [parameter_name, queue] : update_time_history) {
            queue.clear();
        }
        // clear statistics
        for (auto& tup : update_time_statistics) {
            std::get<update_time_sample_tuple_element::samples>(tup) = 0.0f;
            std::get<update_time_sample_tuple_element::max>(tup) = 0.0f;
            std::get<update_time_sample_tuple_element::min>(tup) = std::numeric_limits<float>::max();
            std::get<update_time_sample_tuple_element::avg>(tup) = 0.0f;
        }
    }
    void render()
    {
        show_performance_window();
    }

private:
    static float get_update_time_sample(void* data, int index)
    {
        auto* queue = static_cast<std::deque<float>*>(data);
        return (*queue)[index];
    }
    void show_performance_parameter(size_t index)
    {
        const auto& [label, queue] = update_time_history.at(index);
        const auto [samples, max, min, avg] = update_time_statistics.at(index);
        if (ImGui::CollapsingHeader(label.c_str())) {
            if (queue.empty()) // if there's no history don't show anythingz
            {
                return;
            }
            // percent of time taken by parameter
            ImGui::Text("Current: %f ns", queue.back());
            if (!update_time_history.empty()) {
                ImGui::SameLine();
                ImGui::Text("(%f percent of total time)", queue.back() / std::get<1>(update_time_history[update_time_item::overall]).back() * 100.0f);
            }
            ImGui::Text("Max: %f ns", max);
            ImGui::Text("Avg: %f ns", avg);
            ImGui::Text("Min: %f ns", min);
            switch (plotting) {
            case history_plotting::line:
                ImGui::PlotLines("", get_update_time_sample, (void*)(&queue), static_cast<int>(queue.size()));
                break;
            case history_plotting::histogram:
                ImGui::PlotHistogram("", get_update_time_sample, (void*)(&queue), static_cast<int>(queue.size()));
                break;
            default:
                break;
            }
        }
    }
    void show_performance_window()
    {
        ImGui::Begin("Performance");
        const std::array<const char*, plotting_elements> combo_items { "line", "histogram" };
        ImGui::Combo("Plotting", &plotting, combo_items.data(), static_cast<int>(combo_items.size()));
        ImGui::Separator();
        for (size_t i = 0; i < update_time_item::update_time_tracked; i++) {
            show_performance_parameter(i);
        }
        ImGui::End();
    }
    // performance
    static constexpr size_t update_time_maximum_samples = 25;
    std::array<std::tuple<std::string, std::deque<float>>, update_time_item::update_time_tracked> update_time_history {
        { { "collision", {} },
            { "health", {} },
            { "stamina", {} },
            { "velocity", {} },
            { "animation", {} },
            { "behaviour", {} },
            { "overall", {} } }
    };
    std::array<std::tuple<float, float, float, float>, update_time_item::update_time_tracked> update_time_statistics {}; // samples , max , min , avg
    const static size_t updates_per_sample = 9; // how many samples to be skipped before another one is counted
    int plotting = history_plotting::line; // if lines or rectangles to be displayed
};
}
