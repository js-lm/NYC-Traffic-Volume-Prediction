#pragma once

#include <csv2/reader.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <cmath>
#include <fmt/core.h>
#include <fstream>
#include <unordered_map>

#include "constants.hpp"

namespace _{

    inline int findClosestStation(double latitude, double longitude){
        const auto &stations{constants::weather::weatherStations()};
        int closestId{stations[0].id};
        
        double deltaLatitude{latitude - stations[0].latitude};
        double deltaLongitude{longitude - stations[0].longitude};
        double closestDistance{deltaLatitude * deltaLatitude + deltaLongitude * deltaLongitude};
        
        for(const auto &station : stations){
            double deltaLatitude{latitude - station.latitude};
            double deltaLongitude{longitude - station.longitude};
            double distance{deltaLatitude * deltaLatitude + deltaLongitude * deltaLongitude};
            
            if(distance < closestDistance){
                closestDistance = distance;
                closestId = station.id;
            }
        }
        
        return closestId;
    }

} // namespace _

inline void splitBySegmentId(
    const std::string &inputCsvPath, 
    const std::string &outputDirectory
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

    size_t segmentIdIndex{0};
    size_t latitudeIndex{0};
    size_t longitudeIndex{0};
    
    for(size_t i{0}; i < header.size(); i++){
        if(header[i] == constants::column_names::SegmentId) segmentIdIndex = i;
        if(header[i] == constants::column_names::Latitude)  latitudeIndex = i;
        if(header[i] == constants::column_names::Longitude) longitudeIndex = i;
    }

    struct LocationData{
        int weatherStationId;
        std::vector<std::vector<std::string>> rows;
    };

    std::unordered_map<std::string, LocationData> groups;
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

        size_t requiredSize{std::max({segmentIdIndex, latitudeIndex, longitudeIndex}) + 1};
        if(fields.size() < requiredSize) continue;

        std::string segmentId{fields[segmentIdIndex]};
        
        if(groups[segmentId].rows.empty()){
            double latitude{std::stod(fields[latitudeIndex])};
            double longitude{std::stod(fields[longitudeIndex])};
            groups[segmentId].weatherStationId = _::findClosestStation(latitude, longitude);
        }
        
        groups[segmentId].rows.push_back(fields);
    }

    fmt::println("total rows: {}", rowCount);
    fmt::println("segments: {}", groups.size());

    std::filesystem::create_directories(outputDirectory);

    size_t count{0};
    for(const auto &[segmentId, locationData] : groups){
        count++;

        std::string filename{segmentId + ".csv"};
        std::filesystem::path outputPath{std::filesystem::path(outputDirectory) / filename};

        std::ofstream out{outputPath};

        for(size_t i{0}; i < header.size(); i++){
            if(i > 0) out << ',';
            out << header[i];
        }
        out << ',' << constants::column_names::WeatherStationId << '\n';

        for(const auto &row : locationData.rows){
            for(size_t i{0}; i < row.size(); i++){
                if(i > 0) out << ',';
                out << row[i];
            }
            out << ',' << locationData.weatherStationId << '\n';
        }

        if(count % constants::system::SegmentProgressInterval == 0){
            fmt::println("written {} files", count);
        }
    }

    fmt::println("done: {} files in {}", groups.size(), outputDirectory);
}
