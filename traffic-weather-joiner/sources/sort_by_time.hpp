#pragma once

#include <csv2/reader.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <fmt/core.h>

#include "constants.hpp"
#include "units.hpp"

inline void sortByTime(
    const std::string &inputDirectory,
    const std::string &outputDirectory
){
    std::filesystem::create_directories(outputDirectory);
    
    std::vector<std::filesystem::path> csvFiles;
    for(const auto &entry : std::filesystem::directory_iterator(inputDirectory)){
        if(entry.is_regular_file() && entry.path().extension() == ".csv"){
            csvFiles.push_back(entry.path());
        }
    }
    
    fmt::println("found {} CSV files to sort", csvFiles.size());
    
    size_t fileCount{0};
    for(const auto &inputPath : csvFiles){
        fileCount++;
        
        if(fileCount % constants::system::FileProgressInterval == 0){
            fmt::println("sorting file {}/{}", fileCount, csvFiles.size());
        }
        
        csv2::Reader<
            csv2::delimiter<','>,
            csv2::quote_character<'"'>,
            csv2::first_row_is_header<true>,
            csv2::trim_policy::trim_whitespace
        > csv;
        
        csv.mmap(inputPath.string());
        
        std::vector<std::string> header;
        for(const auto &cell : csv.header()){
            std::string value;
            cell.read_value(value);
            header.push_back(value);
        }
        
        size_t yearIndex{0};
        size_t monthIndex{0};
        size_t dayIndex{0};
        size_t hourIndex{0};
        size_t minuteIndex{0};
        
        for(size_t i{0}; i < header.size(); i++){
            if(header[i] == constants::column_names::Year)  yearIndex = i;
            if(header[i] == constants::column_names::Month) monthIndex = i;
            if(header[i] == constants::column_names::Day)   dayIndex = i;
            if(header[i] == constants::column_names::Hour)  hourIndex = i;
            if(header[i] == constants::column_names::Minute)minuteIndex = i;
        }
        
        std::vector<units::TimeRowData> rows;
        
        for(const auto &row : csv){
            std::vector<std::string> fields;
            for(const auto &cell : row){
                std::string value;
                cell.read_value(value);
                fields.push_back(value);
            }
            
            size_t requiredSize{std::max({yearIndex, monthIndex, dayIndex, hourIndex, minuteIndex}) + 1};
            if(fields.size() < requiredSize) continue;
            
            units::TimeRowData timeRow;
            timeRow.timestamp.year  = std::stoi(fields[yearIndex]);
            timeRow.timestamp.month = std::stoi(fields[monthIndex]);
            timeRow.timestamp.day   = std::stoi(fields[dayIndex]);
            timeRow.timestamp.hour  = std::stoi(fields[hourIndex]);
            timeRow.timestamp.minute= std::stoi(fields[minuteIndex]);
            timeRow.fullRow = fields;
            
            rows.push_back(timeRow);
        }
        
        std::sort(rows.begin(), rows.end());
        
        std::filesystem::path outputPath{std::filesystem::path(outputDirectory) / inputPath.filename()};
        std::ofstream out{outputPath};
        
        for(size_t i{0}; i < header.size(); i++){
            if(i > 0) out << ',';
            out << header[i];
        }
        out << '\n';
        
        for(const auto &timeRow : rows){
            for(size_t i{0}; i < timeRow.fullRow.size(); i++){
                if(i > 0) out << ',';
                out << timeRow.fullRow[i];
            }
            out << '\n';
        }
    }
    
    fmt::println("done: sorted {} files to {}", csvFiles.size(), outputDirectory);
}
