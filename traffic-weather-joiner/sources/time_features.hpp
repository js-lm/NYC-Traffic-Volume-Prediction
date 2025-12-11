#pragma once

#include <cmath>
#include <unordered_set>

#include "units.hpp"

namespace feature_engineering{

    struct TimeFeatures{
        double monthCosine;
        double monthSine;
        double hourCosine;
        double hourSine;
        double minuteCosine;
        double minuteSine;
    };

    inline TimeFeatures encodeTime(const units::Timestamp &timestamp){
        TimeFeatures features;

        // month encoding (1-12)
        double monthAngle{2.0 * constants::Pi * timestamp.month / 12.0};
        features.monthCosine = std::cos(monthAngle);
        features.monthSine = std::sin(monthAngle);

        // hour encoding (0-23)
        double hourAngle{2.0 * constants::Pi * timestamp.hour / 24.0};
        features.hourCosine = std::cos(hourAngle);
        features.hourSine = std::sin(hourAngle);

        // minute encoding (0-59)
        double minuteAngle{2.0 * constants::Pi * timestamp.minute / 60.0};
        features.minuteCosine = std::cos(minuteAngle);
        features.minuteSine = std::sin(minuteAngle);

        return features;
    }

    inline bool isWeekend(const units::Timestamp &timestamp){
        // Zeller's congruence
        int year    {timestamp.year};
        int month   {timestamp.month};
        int day     {timestamp.day};

        if(month < 3){
            month += 12;
            year -= 1;
        }

        int century{year / 100};
        int yearOfCentury{year % 100};
        
        int dayOfWeek{
            (day + (13 * (month + 1)) / 5 + yearOfCentury + yearOfCentury / 4 + century / 4 - 2 * century) % 7
        };
        
        // saturday = 0, sunday = 1
        return dayOfWeek == 0 || dayOfWeek == 1;
    }

    inline const std::unordered_set<int> &getHolidaySet(){
        static std::unordered_set<int> holidays;

        // 2000 Federal Holidays
        holidays.insert(20000101); // New Year's Day
        holidays.insert(19991231); // New Year's Day (Observed)
        holidays.insert(20000117); // MLK Jr. Day
        holidays.insert(20000221); // Presidents' Day
        holidays.insert(20000529); // Memorial Day
        holidays.insert(20000704); // Independence Day
        holidays.insert(20000904); // Labor Day
        holidays.insert(20001009); // Columbus Day
        holidays.insert(20001110); // Veterans Day (Observed)
        holidays.insert(20001111); // Veterans Day
        holidays.insert(20001123); // Thanksgiving
        holidays.insert(20001225); // Christmas

        // 2001 Federal Holidays
        holidays.insert(20010101); // New Year's Day
        holidays.insert(20010115); // MLK Jr. Day
        holidays.insert(20010219); // Presidents' Day
        holidays.insert(20010528); // Memorial Day
        holidays.insert(20010704); // Independence Day
        holidays.insert(20010903); // Labor Day
        holidays.insert(20011008); // Columbus Day
        holidays.insert(20011111); // Veterans Day
        holidays.insert(20011112); // Veterans Day (Observed)
        holidays.insert(20011122); // Thanksgiving
        holidays.insert(20011225); // Christmas

        // 2002 Federal Holidays
        holidays.insert(20020101); // New Year's Day
        holidays.insert(20020121); // MLK Jr. Day
        holidays.insert(20020218); // Presidents' Day
        holidays.insert(20020527); // Memorial Day
        holidays.insert(20020704); // Independence Day
        holidays.insert(20020902); // Labor Day
        holidays.insert(20021014); // Columbus Day
        holidays.insert(20021111); // Veterans Day
        holidays.insert(20021128); // Thanksgiving
        holidays.insert(20021225); // Christmas

        // 2003 Federal Holidays
        holidays.insert(20030101); // New Year's Day
        holidays.insert(20030120); // MLK Jr. Day
        holidays.insert(20030217); // Presidents' Day
        holidays.insert(20030526); // Memorial Day
        holidays.insert(20030704); // Independence Day
        holidays.insert(20030901); // Labor Day
        holidays.insert(20031013); // Columbus Day
        holidays.insert(20031111); // Veterans Day
        holidays.insert(20031127); // Thanksgiving
        holidays.insert(20031225); // Christmas

        // 2004 Federal Holidays
        holidays.insert(20040101); // New Year's Day
        holidays.insert(20040119); // MLK Jr. Day
        holidays.insert(20040216); // Presidents' Day
        holidays.insert(20040531); // Memorial Day
        holidays.insert(20040704); // Independence Day
        holidays.insert(20040705); // Independence Day (Observed)
        holidays.insert(20040906); // Labor Day
        holidays.insert(20041011); // Columbus Day
        holidays.insert(20041111); // Veterans Day
        holidays.insert(20041125); // Thanksgiving
        holidays.insert(20041224); // Christmas (Observed)
        holidays.insert(20041225); // Christmas

        // 2005 Federal Holidays
        holidays.insert(20050101); // New Year's Day
        holidays.insert(20041231); // New Year's Day (Observed)
        holidays.insert(20050117); // MLK Jr. Day
        holidays.insert(20050221); // Presidents' Day
        holidays.insert(20050530); // Memorial Day
        holidays.insert(20050704); // Independence Day
        holidays.insert(20050905); // Labor Day
        holidays.insert(20051010); // Columbus Day
        holidays.insert(20051111); // Veterans Day
        holidays.insert(20051124); // Thanksgiving
        holidays.insert(20051225); // Christmas
        holidays.insert(20051226); // Christmas (Observed)

        // 2006 Federal Holidays
        holidays.insert(20060101); // New Year's Day
        holidays.insert(20060102); // New Year's Day (Observed)
        holidays.insert(20060116); // MLK Jr. Day
        holidays.insert(20060220); // Presidents' Day
        holidays.insert(20060529); // Memorial Day
        holidays.insert(20060704); // Independence Day
        holidays.insert(20060904); // Labor Day
        holidays.insert(20061009); // Columbus Day
        holidays.insert(20061110); // Veterans Day (Observed)
        holidays.insert(20061111); // Veterans Day
        holidays.insert(20061123); // Thanksgiving
        holidays.insert(20061225); // Christmas

        // 2007 Federal Holidays
        holidays.insert(20070101); // New Year's Day
        holidays.insert(20070115); // MLK Jr. Day
        holidays.insert(20070219); // Presidents' Day
        holidays.insert(20070528); // Memorial Day
        holidays.insert(20070704); // Independence Day
        holidays.insert(20070903); // Labor Day
        holidays.insert(20071008); // Columbus Day
        holidays.insert(20071111); // Veterans Day
        holidays.insert(20071112); // Veterans Day (Observed)
        holidays.insert(20071122); // Thanksgiving
        holidays.insert(20071225); // Christmas

        // 2008 Federal Holidays
        holidays.insert(20080101); // New Year's Day
        holidays.insert(20080121); // MLK Jr. Day
        holidays.insert(20080218); // Presidents' Day
        holidays.insert(20080526); // Memorial Day
        holidays.insert(20080704); // Independence Day
        holidays.insert(20080901); // Labor Day
        holidays.insert(20081013); // Columbus Day
        holidays.insert(20081111); // Veterans Day
        holidays.insert(20081127); // Thanksgiving
        holidays.insert(20081225); // Christmas

        // 2009 Federal Holidays
        holidays.insert(20090101); // New Year's Day
        holidays.insert(20090119); // MLK Jr. Day
        holidays.insert(20090216); // Presidents' Day
        holidays.insert(20090525); // Memorial Day
        holidays.insert(20090703); // Independence Day (Observed)
        holidays.insert(20090704); // Independence Day
        holidays.insert(20090907); // Labor Day
        holidays.insert(20091012); // Columbus Day
        holidays.insert(20091111); // Veterans Day
        holidays.insert(20091126); // Thanksgiving
        holidays.insert(20091225); // Christmas

        // 2010 Federal Holidays
        holidays.insert(20100101); // New Year's Day
        holidays.insert(20100118); // MLK Jr. Day
        holidays.insert(20100215); // Presidents' Day
        holidays.insert(20100531); // Memorial Day
        holidays.insert(20100704); // Independence Day
        holidays.insert(20100705); // Independence Day (Observed)
        holidays.insert(20100906); // Labor Day
        holidays.insert(20101011); // Columbus Day
        holidays.insert(20101111); // Veterans Day
        holidays.insert(20101125); // Thanksgiving
        holidays.insert(20101224); // Christmas (Observed)
        holidays.insert(20101225); // Christmas
        holidays.insert(20101231); // New Year's Day (Observed for 2011)

        // 2011 Federal Holidays
        holidays.insert(20110101); // New Year's Day
        holidays.insert(20110117); // MLK Jr. Day
        holidays.insert(20110221); // Presidents' Day
        holidays.insert(20110530); // Memorial Day
        holidays.insert(20110704); // Independence Day
        holidays.insert(20110905); // Labor Day
        holidays.insert(20111010); // Columbus Day
        holidays.insert(20111111); // Veterans Day
        holidays.insert(20111124); // Thanksgiving
        holidays.insert(20111225); // Christmas
        holidays.insert(20111226); // Christmas (Observed)

        // 2012 Federal Holidays
        holidays.insert(20120101); // New Year's Day
        holidays.insert(20120102); // New Year's Day (Observed)
        holidays.insert(20120116); // MLK Jr. Day
        holidays.insert(20120220); // Presidents' Day
        holidays.insert(20120528); // Memorial Day
        holidays.insert(20120704); // Independence Day
        holidays.insert(20120903); // Labor Day
        holidays.insert(20121008); // Columbus Day
        holidays.insert(20121111); // Veterans Day
        holidays.insert(20121112); // Veterans Day (Observed)
        holidays.insert(20121122); // Thanksgiving
        holidays.insert(20121225); // Christmas

        // 2013 Federal Holidays
        holidays.insert(20130101); // New Year's Day
        holidays.insert(20130121); // MLK Jr. Day
        holidays.insert(20130218); // Presidents' Day
        holidays.insert(20130527); // Memorial Day
        holidays.insert(20130704); // Independence Day
        holidays.insert(20130902); // Labor Day
        holidays.insert(20131014); // Columbus Day
        holidays.insert(20131111); // Veterans Day
        holidays.insert(20131128); // Thanksgiving
        holidays.insert(20131225); // Christmas

        // 2014 Federal Holidays
        holidays.insert(20140101); // New Year's Day
        holidays.insert(20140120); // MLK Jr. Day
        holidays.insert(20140217); // Presidents' Day
        holidays.insert(20140526); // Memorial Day
        holidays.insert(20140704); // Independence Day
        holidays.insert(20140901); // Labor Day
        holidays.insert(20141013); // Columbus Day
        holidays.insert(20141111); // Veterans Day
        holidays.insert(20141127); // Thanksgiving
        holidays.insert(20141225); // Christmas

        // 2015 Federal Holidays
        holidays.insert(20150101); // New Year's Day
        holidays.insert(20150119); // MLK Jr. Day
        holidays.insert(20150216); // Presidents' Day
        holidays.insert(20150525); // Memorial Day
        holidays.insert(20150703); // Independence Day (Observed)
        holidays.insert(20150704); // Independence Day
        holidays.insert(20150907); // Labor Day
        holidays.insert(20151012); // Columbus Day
        holidays.insert(20151111); // Veterans Day
        holidays.insert(20151126); // Thanksgiving
        holidays.insert(20151225); // Christmas

        // 2016 Federal Holidays
        holidays.insert(20160101); // New Year's Day
        holidays.insert(20160118); // MLK Jr. Day
        holidays.insert(20160215); // Presidents' Day
        holidays.insert(20160530); // Memorial Day
        holidays.insert(20160704); // Independence Day
        holidays.insert(20160905); // Labor Day
        holidays.insert(20161010); // Columbus Day
        holidays.insert(20161111); // Veterans Day
        holidays.insert(20161124); // Thanksgiving
        holidays.insert(20161225); // Christmas
        holidays.insert(20161226); // Christmas (Observed)

        // 2017 Federal Holidays
        holidays.insert(20170101); // New Year's Day
        holidays.insert(20170102); // New Year's Day (Observed)
        holidays.insert(20170116); // MLK Jr. Day
        holidays.insert(20170220); // Presidents' Day
        holidays.insert(20170529); // Memorial Day
        holidays.insert(20170704); // Independence Day
        holidays.insert(20170904); // Labor Day
        holidays.insert(20171009); // Columbus Day
        holidays.insert(20171110); // Veterans Day (Observed)
        holidays.insert(20171111); // Veterans Day
        holidays.insert(20171123); // Thanksgiving
        holidays.insert(20171225); // Christmas

        // 2018 Federal Holidays
        holidays.insert(20180101); // New Year's Day
        holidays.insert(20180115); // MLK Jr. Day
        holidays.insert(20180219); // Presidents' Day
        holidays.insert(20180528); // Memorial Day
        holidays.insert(20180704); // Independence Day
        holidays.insert(20180903); // Labor Day
        holidays.insert(20181008); // Columbus Day
        holidays.insert(20181111); // Veterans Day
        holidays.insert(20181112); // Veterans Day (Observed)
        holidays.insert(20181122); // Thanksgiving
        holidays.insert(20181225); // Christmas

        // 2019 Federal Holidays
        holidays.insert(20190101); // New Year's Day
        holidays.insert(20190121); // MLK Jr. Day
        holidays.insert(20190218); // Presidents' Day
        holidays.insert(20190527); // Memorial Day
        holidays.insert(20190704); // Independence Day
        holidays.insert(20190902); // Labor Day
        holidays.insert(20191014); // Columbus Day
        holidays.insert(20191111); // Veterans Day
        holidays.insert(20191128); // Thanksgiving
        holidays.insert(20191225); // Christmas

        // 2020 Federal Holidays
        holidays.insert(20200101); // New Year's Day
        holidays.insert(20200120); // MLK Jr. Day
        holidays.insert(20200217); // Presidents' Day
        holidays.insert(20200525); // Memorial Day
        holidays.insert(20200703); // Independence Day (Observed)
        holidays.insert(20200704); // Independence Day
        holidays.insert(20200907); // Labor Day
        holidays.insert(20201012); // Columbus Day
        holidays.insert(20201111); // Veterans Day
        holidays.insert(20201126); // Thanksgiving
        holidays.insert(20201225); // Christmas

        // 2021 Federal Holidays
        holidays.insert(20210101); // New Year's Day
        holidays.insert(20210118); // MLK Jr. Day
        holidays.insert(20210215); // Presidents' Day
        holidays.insert(20210531); // Memorial Day
        holidays.insert(20210618); // Juneteenth (Observed)
        holidays.insert(20210619); // Juneteenth
        holidays.insert(20210704); // Independence Day
        holidays.insert(20210705); // Independence Day (Observed)
        holidays.insert(20210906); // Labor Day
        holidays.insert(20211011); // Columbus Day
        holidays.insert(20211111); // Veterans Day
        holidays.insert(20211125); // Thanksgiving
        holidays.insert(20211224); // Christmas (Observed)
        holidays.insert(20211225); // Christmas
        holidays.insert(20211231); // New Year's Day (Observed for 2022)

        // 2022 Federal Holidays
        holidays.insert(20220101); // New Year's Day
        holidays.insert(20220117); // MLK Jr. Day
        holidays.insert(20220221); // Presidents' Day
        holidays.insert(20220530); // Memorial Day
        holidays.insert(20220619); // Juneteenth
        holidays.insert(20220620); // Juneteenth (Observed)
        holidays.insert(20220704); // Independence Day
        holidays.insert(20220905); // Labor Day
        holidays.insert(20221010); // Columbus Day
        holidays.insert(20221111); // Veterans Day
        holidays.insert(20221124); // Thanksgiving
        holidays.insert(20221225); // Christmas
        holidays.insert(20221226); // Christmas (Observed)

        // 2023 Federal Holidays
        holidays.insert(20230101); // New Year's Day
        holidays.insert(20230102); // New Year's Day (Observed)
        holidays.insert(20230116); // MLK Jr. Day
        holidays.insert(20230220); // Presidents' Day
        holidays.insert(20230529); // Memorial Day
        holidays.insert(20230619); // Juneteenth
        holidays.insert(20230704); // Independence Day
        holidays.insert(20230904); // Labor Day
        holidays.insert(20231009); // Columbus Day
        holidays.insert(20231110); // Veterans Day (Observed)
        holidays.insert(20231111); // Veterans Day
        holidays.insert(20231123); // Thanksgiving
        holidays.insert(20231225); // Christmas

        // 2024 Federal Holidays
        holidays.insert(20240101); // New Year's Day
        holidays.insert(20240115); // MLK Jr. Day
        holidays.insert(20240219); // Presidents' Day
        holidays.insert(20240527); // Memorial Day
        holidays.insert(20240619); // Juneteenth
        holidays.insert(20240704); // Independence Day
        holidays.insert(20240902); // Labor Day
        holidays.insert(20241014); // Columbus Day
        holidays.insert(20241111); // Veterans Day
        holidays.insert(20241128); // Thanksgiving
        holidays.insert(20241225); // Christmas

        // 2025 Federal Holidays
        holidays.insert(20250101); // New Year's Day
        holidays.insert(20250120); // MLK Jr. Day
        holidays.insert(20250217); // Presidents' Day
        holidays.insert(20250526); // Memorial Day
        holidays.insert(20250619); // Juneteenth
        holidays.insert(20250704); // Independence Day
        holidays.insert(20250901); // Labor Day
        holidays.insert(20251013); // Columbus Day
        holidays.insert(20251111); // Veterans Day
        holidays.insert(20251127); // Thanksgiving
        holidays.insert(20251225); // Christmas

        return holidays;
    }

    inline bool isHoliday(const units::Timestamp &timestamp){
        return getHolidaySet().count(timestamp.toPackedDate()) > 0;
    }

} // namespace feature_engineering
