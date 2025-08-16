/*
   Copyright 2022, Adam Krzywaniak.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include <gnuplot-iostream.h>

enum class Legend {
    INSIDE,
    OUTSIDE
};

enum class SeriesStyle {
    LINES,
    LINES_WITH_POINTS,
    POINTS,
    BOX,
    Y_ERROR_BAR
};

struct Series {
    Series(std::string n, int x, int y, std::string sn) :
        inputFileName(n), xSeriesID(x), ySeriesID(y), seriesName(sn) {}

    std::string inputFileName;
    std::string seriesName;
    int xSeriesID;
    int ySeriesID;
};

class PlotBuilder {
public:
    PlotBuilder();
    PlotBuilder(std::string name);
    virtual ~PlotBuilder();

    void plot(std::vector<Series> sv);
    void plotPowerLog(std::vector<Series> sv);
    void plotPowerLogWithDynamicMetrics(std::vector<Series> top, std::vector<Series> bottom);
    void plotRelMetr(std::vector<Series> sv);
    void setLegend(Legend option);
    void plotTmp(std::string);
    void plotTmpGSS(std::string);
    void plotEPet(std::string);
    void plotEPall(std::string);
    void setPlotTitle(std::string title, int fontSize = 20);
    void setSimpleSubtitle(std::string subtitleText, int fontSize=16, float screenVerticalPosition=0.86);
    void submitPlot();
    void setXlabel(std::string xLabel, int fontSize = 20);
    void setYlabel(std::string xLabel, int fontSize = 20);
    void setOutputName(std::string name) { outputFileName_ = name; }

private:
    void setStyles();
    void initPlot(int = 1200, int = 800);
    std::string printErrBar(std::string pathToFile, double xPosition, int seriesColumn,
                            int stddevColumn, std::string styleName, int dataIndex,
                            std::string baseValueVar = "1");
    std::string printBar(std::string pathToFile, double xPosition, double barWidth,
                         int seriesColumn, std::string styleName, int dataIndex,
                         bool printTitle = false, std::string plotTitle = "",
                         std::string baseValueVar = "1", int yBarOffset = 0);
    std::string printLabel(std::string pathToFile, double xPosition, int seriesColumn,
                           double yPositionRel, int dataIndex, int labelPrecision = 1,
                           int fontSize = 12, std::string baseValueVar = "1");
    std::string prindBarWithErrAndLabels(std::string pathToFile, double xPosition,
                                         int seriesColumn, int stddevColumn, double barWidth,
                                         double yPositionRel, int dataIndex,
                                         std::string styleName, bool printTitle,
                                         std::string plotTitle = "", int labelPrecision = 1,
                                         int fontSize = 12, std::string baseValueVar = "1");
    std::vector<std::string> stylesVec_ {"11", "21", "31", "12", "22", "32", "13", "23", "33", "4", "5", "6", "7"};
    std::vector<std::string> grayStylesVec_ {"555", "222", "333", "444", "111"};
    std::string outputFileName_ {"plot.png"};
    Gnuplot *gp_;
};
