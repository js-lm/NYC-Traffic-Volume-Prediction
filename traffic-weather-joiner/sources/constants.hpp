#pragma once

#include <vector>

namespace constants{

    constexpr double Pi{3.1415926f};

    namespace column_names{

        constexpr const char *SegmentId         {"SegmentID"};
        constexpr const char *Year              {"Yr"};
        constexpr const char *Month             {"M"};
        constexpr const char *Day               {"D"};
        constexpr const char *Hour              {"HH"};
        constexpr const char *Minute            {"MM"};
        constexpr const char *Latitude          {"latitude"};
        constexpr const char *Longitude         {"longitude"};
        constexpr const char *WeatherStationId  {"weather_station_id"};
        constexpr const char *LocationId        {"location_id"};
        constexpr const char *Time              {"time"};

    } // namespace column_names

    namespace system{

        constexpr size_t RowProgressInterval            {100000};
        constexpr size_t FileProgressInterval           {10};
        constexpr size_t SegmentProgressInterval        {100};

        constexpr int    MaxWeatherTimeDifferenceMinutes{120};
        constexpr size_t MaxSkippedRowWarnings          {5};

    } // namespace system

    namespace weather{

        struct WeatherStation{
            int id;
            double latitude;
            double longitude;
        };

        inline const std::vector<WeatherStation> &weatherStations(){
            static const std::vector<WeatherStation> stations{
                {0, 40.878735, -73.86914},
                {1, 40.808434, -73.89206},
                {2, 40.597538, -73.96039},
                {3, 40.667835, -73.93768},
                {4, 40.738136, -73.91489},
                {5, 40.738136, -74.04254},
                {6, 40.808434, -74.0199},
                {7, 40.738136, -74.17023},
                {8, 40.738136, -73.78723},
                {9, 40.667835, -73.81021},
                // 10 duplicated
                {11, 40.597538, -74.08771},
                {12, 40.52724, -74.237274},
                {13, 40.667835, -74.19266}
            };
            return stations;
        }

    } // namespace weather

    namespace paths{

        constexpr const char *TrafficInput              {"./csv/traffic_data_with_coords.csv"};
        constexpr const char *WeatherInput              {"./csv/open-meteo-no-cords.csv"};
        constexpr const char *TrafficByLocation         {"./output/traffic_by_location"};
        constexpr const char *TrafficByLocationSorted   {"./output/traffic_by_location_sorted"};
        constexpr const char *MergedTrafficWeather      {"./output/merged_traffic_weather"};
        constexpr const char *FinalOutput               {"./output/final_merged_dataset.csv"};
        constexpr const char *FinalOutputWithFeatures   {"./output/final_merged_dataset_with_features.csv"};

    } // namespace paths

    namespace flags{

        constexpr bool SplitData            {true};
        constexpr bool SortByTime           {true};
        constexpr bool MergeWeather         {true};
        constexpr bool FeatureEngineering   {true};
        constexpr bool MergeAll             {true};

    } // namespace flags

} // namespace constants