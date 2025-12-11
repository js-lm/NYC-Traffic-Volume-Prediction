#include "split_traffic.hpp"
#include "sort_by_time.hpp"
#include "merge_weather.hpp"
#include "add_time_features.hpp"
#include "merge_split_data.hpp"

#include "constants.hpp"

int main(){
    if(constants::flags::SplitData){
        fmt::println("---Split traffic by segment---");
        splitBySegmentId(
            constants::paths::TrafficInput,
            constants::paths::TrafficByLocation
        );
        fmt::println("");
    }

    if(constants::flags::SortByTime){
        fmt::println("---Sort split data by time---");
        sortByTime(
            constants::paths::TrafficByLocation,
            constants::paths::TrafficByLocationSorted
        );
        fmt::println("");
    }

    if(constants::flags::MergeWeather){
        fmt::println("---Merge weather data---");
        mergeWeather(
            constants::paths::WeatherInput,
            constants::paths::TrafficByLocationSorted,
            constants::paths::MergedTrafficWeather
        );
        fmt::print("");
    }

    if(constants::flags::MergeAll){
        fmt::println("---Merge all files---");
        mergeSplitData(
            constants::paths::MergedTrafficWeather,
            constants::paths::FinalOutput
        );
        fmt::println("");
    }

    if(constants::flags::FeatureEngineering){
        fmt::println("---Add time features---");
        addTimeFeatures(
            constants::paths::FinalOutput,
            constants::paths::FinalOutputWithFeatures
        );
        fmt::println("");
    }

    fmt::print("All done!");

    return 0;
}
