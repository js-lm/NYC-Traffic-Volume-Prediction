#pragma once

#include <csv2/reader.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <fmt/core.h>
#include <unordered_map>

#include "constants.hpp"
#include "utilities.hpp"

namespace _{

    struct WeatherRecord{
        units::Timestamp timestamp;
        std::vector<std::string> fields;
    };

} // namespace _

inline void mergeWeather(
    const std::string &weatherCsvPath,
    const std::string &trafficLocationDirectory,
    const std::string &outputDirectory
){
    fmt::println("loading weather: {}", weatherCsvPath);

    csv2::Reader<
        csv2::delimiter<','>, 
        csv2::quote_character<'"'>, 
        csv2::first_row_is_header<true>,
        csv2::trim_policy::trim_whitespace
    > weatherCsv;

    weatherCsv.mmap(weatherCsvPath);

    std::vector<std::string> weatherHeader;
    for(const auto &cell : weatherCsv.header()){
        std::string value;
        cell.read_value(value);
        weatherHeader.push_back(value);
    }

    size_t locationIdIndex{utilities::findColumn(weatherHeader, constants::column_names::LocationId)};
    size_t timeIndex{utilities::findColumn(weatherHeader, constants::column_names::Time)};

    // group weather by station ID
    std::unordered_map<int, std::vector<_::WeatherRecord>> weatherByStation;
    size_t weatherRowCount{0};

    for(const auto &row : weatherCsv){
        weatherRowCount++;
        if(weatherRowCount % constants::system::RowProgressInterval == 0){
            fmt::println("loaded {} weather records", weatherRowCount);
        }

        std::vector<std::string> fields;
        for(const auto &cell : row){
            std::string value;
            cell.read_value(value);
            fields.push_back(value);
        }

        size_t requiredSize{std::max(locationIdIndex, timeIndex) + 1};
        if(fields.size() < requiredSize){
            fmt::println(
                "[!!! row {} has only {} columns when it should have {}, skipping... !!!]", 
                weatherRowCount, fields.size(), requiredSize
            );
            continue;
        }

        int locationId{std::stoi(fields[locationIdIndex])};
        units::Timestamp timestamp{utilities::parseTimestamp(fields[timeIndex])};

        weatherByStation[locationId].push_back({timestamp, fields});
    }

    fmt::println("loaded {} weather records for {} stations", weatherRowCount, weatherByStation.size());

    // sort weather records by time
    for(auto &[stationId, records] : weatherByStation){
        std::sort(records.begin(), records.end(), [](const _::WeatherRecord &left, const _::WeatherRecord &right){
            return left.timestamp < right.timestamp;
        });
    }

    std::vector<std::filesystem::path> trafficFiles;
    for(const auto &entry : std::filesystem::directory_iterator(trafficLocationDirectory)){
        if(entry.is_regular_file() && entry.path().extension() == ".csv"){
            trafficFiles.push_back(entry.path());
        }
    }

    fmt::println("found {} traffic files", trafficFiles.size());

    std::filesystem::create_directories(outputDirectory);

    size_t fileCount{0};
    for(const auto &trafficFile : trafficFiles){
        fileCount++;

        csv2::Reader<
            csv2::delimiter<','>, 
            csv2::quote_character<'"'>, 
            csv2::first_row_is_header<true>,
            csv2::trim_policy::trim_whitespace
        > trafficCsv;

        trafficCsv.mmap(trafficFile.string());

        std::vector<std::string> trafficHeader;
        for(const auto &cell : trafficCsv.header()){
            std::string value;
            cell.read_value(value);
            trafficHeader.push_back(value);
        }

        size_t stationIdIndex   {utilities::findColumn(trafficHeader, constants::column_names::WeatherStationId)};
        size_t yearIndex        {utilities::findColumn(trafficHeader, constants::column_names::Year)};
        size_t monthIndex       {utilities::findColumn(trafficHeader, constants::column_names::Month)};
        size_t dayIndex         {utilities::findColumn(trafficHeader, constants::column_names::Day)};
        size_t hourIndex        {utilities::findColumn(trafficHeader, constants::column_names::Hour)};
        size_t minuteIndex      {utilities::findColumn(trafficHeader, constants::column_names::Minute)};

        std::vector<std::vector<std::string>> trafficRows;
        int stationId{0};

        for(const auto &row : trafficCsv){
            std::vector<std::string> fields;
            for(const auto &cell : row){
                std::string value;
                cell.read_value(value);
                fields.push_back(value);
            }

            size_t requiredSize{std::max({stationIdIndex, yearIndex, monthIndex, dayIndex, hourIndex, minuteIndex}) + 1};

            if(fields.size() < requiredSize) continue;
            if(stationId == 0) stationId = std::stoi(fields[stationIdIndex]);

            trafficRows.push_back(fields);
        }

        if(trafficRows.empty()){
            continue;
        }

        auto &weatherRecords{weatherByStation[stationId]};

        if(weatherRecords.empty()){
            fmt::println(
                "[!!! no weather data for station {}, skipping file {}... !!!]", 
                stationId, trafficFile.filename().string()
            );
            continue;
        }

        std::filesystem::path outputPath{std::filesystem::path(outputDirectory) / trafficFile.filename()};
        std::ofstream out{outputPath};

        for(size_t i{0}; i < trafficHeader.size(); i++){
            if(i > 0) out << ',';
            out << trafficHeader[i];
        }
        for(const auto &col : weatherHeader){
            out << ',' << col;
        }
        out << '\n';

        size_t weatherIndex{0};
        size_t skippedRowCount{0};
        for(const auto &trafficFields : trafficRows){
            units::Timestamp trafficTime;
            trafficTime.year    = std::stoi(trafficFields[yearIndex]);
            trafficTime.month   = std::stoi(trafficFields[monthIndex]);
            trafficTime.day     = std::stoi(trafficFields[dayIndex]);
            trafficTime.hour    = std::stoi(trafficFields[hourIndex]);
            trafficTime.minute  = std::stoi(trafficFields[minuteIndex]);

            // find matching weather record with a simple linear search since there are only 13 station
            while(weatherIndex < weatherRecords.size() && weatherRecords[weatherIndex].timestamp < trafficTime){
                weatherIndex++;
            }
            if(weatherIndex >= weatherRecords.size()){
                weatherIndex = weatherRecords.size() - 1;
            }

            int timeDifferenceMinutes{trafficTime.absoluteDifferenceInMinutes(weatherRecords[weatherIndex].timestamp)};
            if(timeDifferenceMinutes > constants::system::MaxWeatherTimeDifferenceMinutes){
                skippedRowCount++;
                if(skippedRowCount <= constants::system::MaxSkippedRowWarnings){
                    fmt::println(
                        "[!!! weather data is {} minutes away from traffic data for file {}, skipping row... !!!]",
                        timeDifferenceMinutes, trafficFile.filename().string()
                    );
                }
                continue;
            }

            for(size_t i{0}; i < trafficFields.size(); i++){
                if(i > 0) out << ',';
                out << trafficFields[i];
            }

            const auto &weatherFields{weatherRecords[weatherIndex].fields};
            for(const auto &field : weatherFields){
                out << ',' << field;
            }
            out << '\n';
        }

        if(skippedRowCount > 0){
            fmt::println(
                "[!!! skipped {} rows in file {} due to weather data being more than 2 hours away !!!]",
                skippedRowCount, trafficFile.filename().string()
            );
        }

        if(fileCount % constants::system::FileProgressInterval == 0){
            fmt::println("merged {} files", fileCount);
        }
    }

    fmt::println("done: merged {} files in {}", fileCount, outputDirectory);
}
