#pragma once

#include "constants.hpp"
#include "utilities.hpp"

#include "time_features.hpp"

#include <string>
#include <vector>
#include <csv2/reader.hpp>
#include <filesystem>
#include <fstream>
#include <fmt/core.h>

inline void addTimeFeatures(
    const std::string &inputCsvPath, 
    const std::string &outputCsvPath
){
    fmt::println("loading {}...", inputCsvPath);

    csv2::Reader<
        csv2::delimiter<','>, 
        csv2::quote_character<'"'>, 
        csv2::first_row_is_header<true>,
        csv2::trim_policy::trim_whitespace
    > csv;

    csv.mmap(inputCsvPath);

    std::vector<std::string> header;
    for(const auto &cell : csv.header()){
        std::string value;
        cell.read_value(value);
        header.push_back(value);
    }

    size_t yearIndex    {utilities::findColumn(header, constants::column_names::Year)};
    size_t monthIndex   {utilities::findColumn(header, constants::column_names::Month)};
    size_t dayIndex     {utilities::findColumn(header, constants::column_names::Day)};
    size_t hourIndex    {utilities::findColumn(header, constants::column_names::Hour)};
    size_t minuteIndex  {utilities::findColumn(header, constants::column_names::Minute)};

    std::ofstream out{outputCsvPath};

    for(size_t i{0}; i < header.size(); i++){
        if(i > 0) out << ',';
        out << header[i];
    }
    out << ",is_holiday,is_weekend,month_cos,month_sin,hour_cos,hour_sin,minute_cos,minute_sin\n";

    size_t rowCount{0};
    for(const auto &row : csv){
        rowCount++;
        if(rowCount % constants::system::RowProgressInterval == 0){
            fmt::println("processed {} rows", rowCount);
        }

        std::vector<std::string> fields;
        for(const auto &cell : row){
            std::string value;
            cell.read_value(value);
            fields.push_back(value);
        }

        if(fields.empty()) continue;
        
        size_t requiredSize{std::max({yearIndex, monthIndex, dayIndex, hourIndex, minuteIndex}) + 1};
        if(fields.size() < requiredSize) continue;

        units::Timestamp timestamp;
        timestamp.year      = std::stoi(fields[yearIndex]);
        timestamp.month     = std::stoi(fields[monthIndex]);
        timestamp.day       = std::stoi(fields[dayIndex]);
        timestamp.hour      = std::stoi(fields[hourIndex]);
        timestamp.minute    = std::stoi(fields[minuteIndex]);

        auto timeFeatures   {feature_engineering::encodeTime(timestamp)};
        bool isHoliday      {feature_engineering::isHoliday(timestamp)};
        bool isWeekend      {feature_engineering::isWeekend(timestamp)};

        // write original fields
        for(size_t i{0}; i < fields.size(); i++){
            if(i > 0) out << ',';
            out << fields[i];
        }

        // write new features
        out << ',' << (isHoliday ? "1" : "0");
        out << ',' << (isWeekend ? "1" : "0");
        out << ',' << timeFeatures.monthCosine;
        out << ',' << timeFeatures.monthSine;
        out << ',' << timeFeatures.hourCosine;
        out << ',' << timeFeatures.hourSine;
        out << ',' << timeFeatures.minuteCosine;
        out << ',' << timeFeatures.minuteSine;
        out << '\n';
    }

    fmt::println("done: {} rows written to {}", rowCount, outputCsvPath);
}
