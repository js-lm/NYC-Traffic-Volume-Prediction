#pragma once

#include <csv2/reader.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>

#include "constants.hpp"

inline void mergeSplitData(
    const std::string &inputDirectory, 
    const std::string &outputFilePath
){
    fmt::println("scanning {}...", inputDirectory);

    std::vector<std::filesystem::path> csvFiles;
    for(const auto &entry : std::filesystem::directory_iterator(inputDirectory)){
        if(entry.is_regular_file() && entry.path().extension() == ".csv"){
            csvFiles.push_back(entry.path());
        }
    }

    std::sort(csvFiles.begin(), csvFiles.end());

    fmt::println("found {} files", csvFiles.size());

    std::ofstream out{outputFilePath};

    bool headerWritten{false};
    size_t totalRows{0};
    size_t filesProcessed{0};

    for(const auto &csvFile : csvFiles){
        filesProcessed++;

        csv2::Reader<
            csv2::delimiter<','>, 
            csv2::quote_character<'"'>, 
            csv2::first_row_is_header<true>,
            csv2::trim_policy::trim_whitespace
        > csv;

        csv.mmap(csvFile.string());

        if(!headerWritten){
            bool firstCell{true};
            for(const auto &cell : csv.header()){
                std::string value;
                cell.read_value(value);
                if(!firstCell) out << ',';
                out << value;
                firstCell = false;
            }
            out << '\n';
            headerWritten = true;
        }

        size_t rowCount{0};
        for(const auto &row : csv){
            bool firstCell{true};
            for(const auto &cell : row){
                std::string value;
                cell.read_value(value);
                if(!firstCell) out << ',';
                out << value;
                firstCell = false;
            }
            out << '\n';
            rowCount++;
        }

        totalRows += rowCount;

        if(filesProcessed % constants::system::FileProgressInterval == 0){
            fmt::println("merged {} files ({} rows)", filesProcessed, totalRows);
        }
    }

    fmt::println("done:  {} files, {} total rows in {}", filesProcessed, totalRows, outputFilePath);
}
