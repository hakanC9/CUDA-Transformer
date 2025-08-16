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

#include "plot_builder.hpp"

PlotBuilder::PlotBuilder() {
    gp_ = new Gnuplot();
    initPlot();
    setStyles();
}

PlotBuilder::PlotBuilder(std::string name) :
    outputFileName_(name)
{
    gp_ = new Gnuplot();
    initPlot();
    setStyles();
}

PlotBuilder::~PlotBuilder() {
    delete gp_;
}

void PlotBuilder::initPlot(int width, int height) {
    *gp_ << "set terminal png size " << width << "," << height << " enhanced\n";
    *gp_ << "set output \"" << outputFileName_ << "\"\n";
    *gp_ << "set key horizontal font \",15\"\n";
    *gp_ << "set grid\n";
}

void PlotBuilder::plot(std::vector<Series> sv) {
    *gp_ << "plot ";
    for (int i = 0; i < sv.size(); i++) {
        *gp_ << "\"" << sv[i].inputFileName << "\" using "
        << sv[i].xSeriesID << ":"
        << sv[i].ySeriesID
        << " ls "
        << stylesVec_[i % stylesVec_.size()]
        << " title \""
        << sv[i].seriesName
        << "\" with lines";
        if(i==sv.size()-1) {
            break;
        }
        *gp_ << ", ";
    }
    *gp_ << "\n";
}

void PlotBuilder::plotRelMetr(std::vector<Series> sv) {
    initPlot(1000, 400);
    setXlabel("Power cap [W]", 15);
    setYlabel("relative difference static v. dynamic [%]", 15);
    // setLegend(Legend::OUTSIDE);
    *gp_ << "set ytics font \",15\"\n";
    *gp_ << "plot ";
    for (int i = 0; i < sv.size(); i++) {
        *gp_ << "\"" << sv[i].inputFileName << "\" using "
        << sv[i].xSeriesID << ":"
        << sv[i].ySeriesID
        << " ls "
        << std::vector<std::string>{"1111", "7777", "3333", "11111"}[i % 4]
        << " title \""
        << sv[i].seriesName
        << "\" with linespoints";
        if(i==sv.size()-1) {
            break;
        }
        *gp_ << ", ";
    }
    *gp_ << "\n";
}

void PlotBuilder::plotPowerLog(std::vector<Series> sv)
{
    initPlot(1000, 500);
    setXlabel("Time [s]", 16);
    setYlabel("Power [W]", 16);
    setLegend(Legend::OUTSIDE);
    // *gp_ << "set yrange [0:220]\n"
    //      << "set ytics 20\n";


    *gp_ << "plot ";
    for (int i = 0; i < sv.size(); i++) {
        *gp_ << "\"" << sv[i].inputFileName << "\" using "
        << "($" << sv[i].xSeriesID << "/1000):"
        << sv[i].ySeriesID
        << " ls "
        << grayStylesVec_[i % grayStylesVec_.size()]
        << " linewidth 4 title \""
        << sv[i].seriesName
        << "\" with lines";
        if(i==sv.size()-1) {
            break;
        }
        *gp_ << ", ";
    }
    *gp_ << "\n";
}

void PlotBuilder::plotPowerLogWithDynamicMetrics(std::vector<Series> top, std::vector<Series> bottom)
{
    initPlot(1000, 800);
    setYlabel("Power [W]", 16);
    setLegend(Legend::OUTSIDE);
    *gp_ << "set multiplot layout 2, 1\n";
    *gp_ << "set size 1, 0.55\n";
    *gp_ << "set origin 0, 0.45\n";
    *gp_ << "set xtics font \",18\"\n";
    *gp_ << "set ytics font \",18\"\n";

    *gp_ << "plot ";
    for (int i = 0; i < top.size(); i++)
    {
        *gp_ << "\"" << top[i].inputFileName << "\" using "
        << "($" << top[i].xSeriesID << "/1000):"
        << top[i].ySeriesID
        << " ls "
        << grayStylesVec_[i % grayStylesVec_.size()]
        << " linewidth 4 title \""
        << top[i].seriesName
        << "\" with lines";
        if(i==top.size()-1) {
            break;
        }
        *gp_ << ", ";
    }
    *gp_ << "\n";
    setXlabel("Time [s]", 16);
    setYlabel("Relative metric value", 16);
    *gp_ << "unset title\n";
    *gp_ << "set size 1, 0.45\n";
    *gp_ << "set origin 0, 0\n";
    *gp_ << "set xrange [0:]\n";
    *gp_ << "set yrange [:2.0]\n";
    *gp_ << "plot ";
    for (int i = 0; i < bottom.size(); i++)
    {
        *gp_ << "\"" << bottom[i].inputFileName << "\" using "
        << "($" << bottom[i].xSeriesID << "/1000):"
        << bottom[i].ySeriesID
        << " ls "
        << grayStylesVec_[i % grayStylesVec_.size()]
        << " linewidth 4 title \""
        << bottom[i].seriesName
        << "\" with lines";
        if(i==bottom.size()-1) {
            break;
        }
        *gp_ << ", ";
    }
    *gp_ << "\n";
    *gp_ << "unset multiplot \n";
}

void PlotBuilder::setStyles() {
    *gp_ << "set style line 11  linecolor rgb \"red\" linewidth 2.5 linetype 1 pointtype 1 pointsize 1 pointinterval 0"
        << "\n"
        << "set style line 12  linecolor rgb \"green\" linewidth 2.5 linetype \"___\" pointtype 2 pointsize 1 pointinterval 0"
        << "\n"
        << "set style line 13  linecolor rgb \"pink\" linewidth 2.5 linetype \"_\" pointtype 4 pointsize 1 pointinterval 0"
        << "\n"
        << "set style line 21  linecolor rgb \"blue\" linewidth 2.5 linetype 1 pointtype 7 pointsize 1 pointinterval 0"
        << "\n"
        << "set style line 22  linecolor rgb \"brown\" linewidth 2.5 linetype \"___\" pointtype 9 pointsize 1 pointinterval 0"
        << "\n"
        << "set style line 23  linecolor rgb \"grey\" linewidth 2.5 linetype \"_\" pointtype 13 pointsize 1 pointinterval 0"
        << "\n"
        << "set style line 31  linecolor rgb \"black\" linewidth 2.5 linetype 1 pointtype 48 pointsize 1 pointinterval 0"
        << "\n"
        << "set style line 32  linecolor rgb \"orange\" linewidth 2.5 linetype \"___\" pointtype 9 pointsize 1 pointinterval 0"
        << "\n"
        << "set style line 33  linecolor rgb \"orange\" linewidth 2.5 linetype \"_\" pointtype 15 pointsize 1 pointinterval 0"
        << "\n"
        << "set style line 4  linecolor rgb \"#006400\" linewidth 2.0 linetype 5 pointtype 21 pointsize 1 pointinterval 0"
        << "\n"
        << "set style line 5  linecolor rgb \"yellow\" linewidth 2.0 linetype 5 pointtype 12 pointsize 1 pointinterval 0"
        << "\n"
        << "set style line 6  linecolor rgb \"magenta\" linewidth 2.0 linetype 5 pointtype 31 pointsize 1 pointinterval 0"
        << "\n"
        << "set style line 7  linecolor rgb \"brown\" linewidth 2.0 linetype 2 pointtype 1 pointsize 0 pointinterval 10"
        << "\n";
    *gp_ << "set style line 111 lc rgb 'gray30' lt 1 lw 1\n"
        << "set style line 222 lc rgb 'gray40' lt 1 lw 1\n"
        << "set style line 333 lc rgb 'gray70' lt 1 lw 1\n"
        << "set style line 444 lc rgb 'gray90' lt 1 lw 1\n"
        << "set style line 555 lc rgb 'black' lt 1 lw 1\n"
        << "set style fill solid 1.0 border rgb 'grey30'\n";
    *gp_ << "set style line 1111 lc rgb 'gray20' lt 1 pt 1 ps 1 lw 3\n"
        << "set style line 2222 lc rgb 'gray90' lt 1 pt 2 ps 1 lw 1.5\n"
        << "set style line 3333 lc rgb 'gray70' lt 1 pt 4 ps 1 lw 2\n"
        << "set style line 4444 lc rgb 'gray40' lt 1 pt 7 ps 1 lw 2\n"
        << "set style line 5555 lc rgb 'black' lt 1 pt 9 ps 1 lw 2.5\n"
        << "set style line 6666 lc rgb 'gray30' lt \"_\" pt 13 ps 1 lw 2.5\n"
        << "set style line 7777 lc rgb 'gray40' lt \"_\" pt 15 ps 1 lw 2.5\n"
        << "set style line 8888 lc rgb 'gray70' lt \"_\" pt 17 ps 1 lw 2.5\n"
        << "set style line 9999 lc rgb 'gray90' lt \"_\" pt 19 ps 1 lw 2.5\n"
        << "set style line 11111 lc rgb 'black' lt \"_\" pt 21 ps 1 lw 2.5\n"
        << "set style fill solid 1.0 border rgb 'grey30'\n";
}

void PlotBuilder::setLegend(Legend option) {
    switch (option) {
        case Legend::OUTSIDE :
            *gp_ << "set key outside center bottom"
               << "\n";
            break;
        case Legend::INSIDE :
        default :
            *gp_ << "set key inside center top"
               << "\n";
            break;
    }
}

void PlotBuilder::setPlotTitle(std::string title, int fontSize) {
    *gp_ << "set title \""
       << title
       << "\"font \"," << fontSize << "\"\n";
}

void PlotBuilder::setSimpleSubtitle(std::string subtitleText, int fontSize, float screenVerticalPosition)
{
    *gp_ << "set label 1 \"" << subtitleText << "\" font \"," << fontSize << "\" at screen 0.5, " << screenVerticalPosition << " center\n";
}

void PlotBuilder::setXlabel(std::string xLabel, int fontSize) {
    *gp_ << "set xlabel \"" << xLabel << "\" font \"," << fontSize << "\" offset 2,0\n";
}

void PlotBuilder::setYlabel(std::string xLabel, int fontSize) {
    *gp_ << "set ylabel \"" << xLabel << "\" font \"," << fontSize << "\" offset 2,0\n";
}

std::string PlotBuilder::printErrBar(
    std::string pathToFile,
    double xPosition,
    int seriesColumn,
    int stddevColumn,
    std::string styleName,
    int dataIndex,
    std::string baseValueVar)
{
    std::stringstream ss;
    ss << " \"" << pathToFile << "\""
       << " index " << dataIndex
       << " using ($0+(" << xPosition << ")):($"
       << seriesColumn << "/" << baseValueVar << "):" << stddevColumn
       << " notitle with yerrorb ls " << styleName << ",";
    return ss.str();
}

std::string PlotBuilder::printBar(
    std::string pathToFile,
    double xPosition,
    double barWidth,
    int seriesColumn,
    std::string styleName,
    int dataIndex,
    bool printTitle,
    std::string plotTitle,
    std::string baseValueVar, // if data has to be normalized, the default value as a gnuplot variable or converted to string has to be provided
    int yOffsetSeriesColumn) // if bar value is a sum of two input series than yOffset might be provided
{
    std::stringstream ss;
    ss << " \"" << pathToFile << "\""
       << " index " << dataIndex
       << " using ($0+(" << xPosition << ")):(($"
       << seriesColumn << "+$" << yOffsetSeriesColumn << ")/" << baseValueVar << "):(" << barWidth << ") "
       << (printTitle ? "title '" + plotTitle + "'" : "notitle")
       << " with boxes ls " << styleName << ",";
    return ss.str();
}

std::string PlotBuilder::printLabel(
    std::string pathToFile,
    double xPosition,
    int seriesColumn,
    double yPositionRel,
    int dataIndex,
    int labelPrecision,
    int fontSize,
    std::string baseValueVar) // if bar value is a sum of two input series than yOffset might be provided
{
    std::stringstream ss;
    ss << " \"" << pathToFile << "\""
       << " index " << dataIndex
       << " using ($0+(" << xPosition << ")):(($"
       << seriesColumn << "/" << baseValueVar << ")+"
       << yPositionRel << "):(sprintf(\"%3." << labelPrecision << "f\",$" << seriesColumn << ")) "
       << "notitle with labels font \"," << fontSize << "\" rotate left,";
    return ss.str();
}

std::string PlotBuilder::prindBarWithErrAndLabels(
    std::string pathToFile,
    double xPosition,
    int seriesColumn,
    int stddevColumn,
    double barWidth,
    double yLabelPositionRel,
    int dataIndex,
    std::string styleName,
    bool printTitle,
    std::string plotTitle,
    int labelPrecision,
    int fontSize,
    std::string baseValueVar) // if bar value is a sum of two input series than yOffset might be provided
{
    std::stringstream ss;
    ss << printErrBar(pathToFile, xPosition, seriesColumn, stddevColumn, styleName, dataIndex,
                      baseValueVar)
       << printBar(pathToFile, xPosition, barWidth, seriesColumn, styleName, dataIndex,
                   printTitle, plotTitle, baseValueVar)
       << printLabel(pathToFile, xPosition, seriesColumn, yLabelPositionRel, dataIndex,
                     labelPrecision, fontSize, baseValueVar);
    return ss.str();
}

void PlotBuilder::plotTmp(std::string name /*std::vector<Series> sv*/) {
    int power = 2;
    int powerDev = 5;
    int energy = 6;
    int energyDev = 9;
    int time = 11;
    int timeDev = 14;
    int waitT = 16;
    int testT = 17;
    int et = 18;
    int etDev = 20;
    int eds = 21;
    int edsDev = etDev;
    double barWidth = 0.16;
    double labelDistanceY = 0.1;
    int labelFontSize = 16;

    *gp_ << "first(x) = ($0 > 0 ? base : base = x)\n";

    // # Syntax: u 0:($0==RowIndex?(VariableName=$ColumnIndex):$ColumnIndex)
    // # RowIndex starts with 0, ColumnIndex starts with 1
    // # 'u' is an abbreviation for the 'using' modifier 

    *gp_ << "set table\n";
    *gp_ << "plot"
       << " \"" << name << "\" i 0 u 0:($0==0?(def_P=$" << power << "):$" << power << "),"
       << " \"" << name << "\" i 0 u 0:($0==0?(def_E=$" << energy << "):$" << energy << "),"
       << " \"" << name << "\" i 0 u 0:($0==0?(def_t=$" << time << "):$" << time << "),"
       << " \"" << name << "\" i 0 u 0:($0==0?(def_et=$" << et << "):$" << et << "),\n";

    *gp_ << "unset table\n";
    initPlot(2000, 800);
    *gp_ << "print def_P, def_E, def_t\n";
    *gp_ << "set yrange [0:]\n";
    *gp_ << "set arrow 1 from 0.0,1 to 7.5,1 nohead ls 11\n"
        << "set label 'default' at 0.3,1.03 center\n";
    *gp_ << "set key font \",16\"\n";
    *gp_ << "set offsets graph 0.0, 0.0, 0.4, 0.0\n";
    *gp_ << "set xtics font \",18\"\n"
        << "set ytics 0.1\n";
    *gp_ << "set xrange [0:7.5]\n";
    *gp_ << "set xtics nomirror out ("
       << "'default' 1,"
       << "'LS E' 3,"
       << "'LS EDP' 5,"
       << "'LS EDS' 7,"
       << "'GSS E' 2,"
       << "'GSS EDP' 4,"
       << "'GSS EDS' 6)\n";
    *gp_ << "set ylabel \"normalized result [-]\" font \",20\" offset 2,0\n";

    *gp_ << "# Size of one box\n"
       << "plot";
       std::vector<int> plotOrder {0, 1, 4, 2, 5, 3, 6};
       int xIndex = 1;
       for (auto&& i : plotOrder) {
        // power
           *gp_ << prindBarWithErrAndLabels(name, xIndex - 2*barWidth, power, powerDev, barWidth,
                                           labelDistanceY, i, "111", i==1, "av.P [W]",
                                           2, labelFontSize, "def_P");
        // energy
           *gp_ << prindBarWithErrAndLabels(name, xIndex - barWidth, energy, energyDev, barWidth,
                                           labelDistanceY, i, "222", i==1, "E [J]", 1, labelFontSize,
                                           "def_E");
        // time
           *gp_ << prindBarWithErrAndLabels(name, xIndex, time, timeDev, barWidth, labelDistanceY,
                                           i, "333", i==1, "exec. time [s]", 2, labelFontSize, "def_t");
        // test time
           *gp_ << printBar(name, xIndex + barWidth/4, barWidth/2, testT, "222 fs pattern 6", i, i==1,
                           "test phase", "$" + std::to_string(time), waitT);
        // wait time
           *gp_ << printBar(name, xIndex + barWidth/4, barWidth/2, waitT, "222 fs pattern 13", i, i==1,
                           "wait phase", "$" + std::to_string(time));
        // energy*time
           *gp_ << prindBarWithErrAndLabels(name, xIndex + barWidth, et, etDev, barWidth,
                                           labelDistanceY, i, "444", i==1, "EDP [kJs]",
                                           1, labelFontSize, "def_et");
        // M+ / EDS
           *gp_ << prindBarWithErrAndLabels(name, xIndex + 2*barWidth, eds, edsDev, barWidth,
                                           labelDistanceY, i, "555", i==1, "EDS(k=2) [-]",
                                           3, labelFontSize);
            xIndex++;
       }
    *gp_ << "\n";
}

void PlotBuilder::plotTmpGSS(std::string name /*std::vector<Series> sv*/) {
    int power = 2;
    int powerDev = 5;
    int energy = 6;
    int energyDev = 9;
    int time = 11;
    int timeDev = 14;
    int waitT = 16;
    int testT = 17;
    int et = 18;
    int etDev = 20;
    int eds = 21;
    int edsDev = etDev;
    double barWidth = 0.16;
    double labelDistanceY = 0.1;
    int labelFontSize = 16;

    *gp_ << "first(x) = ($0 > 0 ? base : base = x)\n";

    // # Syntax: u 0:($0==RowIndex?(VariableName=$ColumnIndex):$ColumnIndex)
    // # RowIndex starts with 0, ColumnIndex starts with 1
    // # 'u' is an abbreviation for the 'using' modifier 

    *gp_ << "set table\n";
    *gp_ << "plot"
       << " \"" << name << "\" i 0 u 0:($0==0?(def_P=$" << power << "):$" << power << "),"
       << " \"" << name << "\" i 0 u 0:($0==0?(def_E=$" << energy << "):$" << energy << "),"
       << " \"" << name << "\" i 0 u 0:($0==0?(def_t=$" << time << "):$" << time << "),"
       << " \"" << name << "\" i 0 u 0:($0==0?(def_et=$" << et << "):$" << et << "),\n";

    *gp_ << "unset table\n";
    initPlot(1000, 800);
    *gp_ << "print def_P, def_E, def_t\n";
    *gp_ << "set yrange [0:]\n";
    *gp_ << "set arrow 1 from 0.0,1 to 4.5,1 nohead ls 11\n"
        << "set label 'default' at 0.3,1.03 center\n";
    *gp_ << "set key font \",16\"\n";
    *gp_ << "set offsets graph 0.0, 0.0, 0.5, 0.0\n";
    *gp_ << "set xtics font \",18\"\n"
        << "set ytics 0.1\n";
    *gp_ << "set xrange [0:4.5]\n";
    *gp_ << "set xtics nomirror out ("
       << "'default' 1,"
       << "'GSS E' 2,"
       << "'GSS EDP' 3,"
       << "'GSS EDS' 4)\n";
    *gp_ << "set ylabel \"normalized result [-]\" font \",20\" offset 2,0\n";

    *gp_ << "# Size of one box\n"
       << "plot";
       std::vector<int> plotOrder {0, 1, 2, 3};
       int xIndex = 1;
       for (auto&& i : plotOrder) {
        // power
           *gp_ << prindBarWithErrAndLabels(name, xIndex - 2*barWidth, power, powerDev, barWidth,
                                           labelDistanceY, i, "111", i==1, "av.P [W]",
                                           2, labelFontSize, "def_P");
        // energy
           *gp_ << prindBarWithErrAndLabels(name, xIndex - barWidth, energy, energyDev, barWidth,
                                           labelDistanceY, i, "222", i==1, "E [J]", 1, labelFontSize,
                                           "def_E");
        // time
           *gp_ << prindBarWithErrAndLabels(name, xIndex, time, timeDev, barWidth, labelDistanceY,
                                           i, "333", i==1, "exec. time [s]", 2, labelFontSize, "def_t");
        // test time
           *gp_ << printBar(name, xIndex + barWidth/4, barWidth/2, testT, "222 fs pattern 6", i, i==1,
                           "test phase", "$" + std::to_string(time), waitT);
        // wait time
           *gp_ << printBar(name, xIndex + barWidth/4, barWidth/2, waitT, "222 fs pattern 13", i, i==1,
                           "wait phase", "$" + std::to_string(time));
        // energy*time
           *gp_ << prindBarWithErrAndLabels(name, xIndex + barWidth, et, etDev, barWidth,
                                           labelDistanceY, i, "444", i==1, "EDP [kJs]",
                                           1, labelFontSize, "def_et");
        // M+ / EDS
           *gp_ << prindBarWithErrAndLabels(name, xIndex + 2*barWidth, eds, edsDev, barWidth,
                                           labelDistanceY, i, "555", i==1, "EDS(k=2) [-]",
                                           3, labelFontSize);
            xIndex++;
       }
    *gp_ << "\n";
}

void PlotBuilder::submitPlot() {
    // this allows for releasing the binary file with output plot
    delete gp_;
    gp_ = new Gnuplot(); 
}

void PlotBuilder::plotEPet(std::string name) {
    int energy = 2;
    int time = 4;
    int instr = 10;
    int powercap = 1;
    int power = 3;
    double k1 = 2.0;
    double k2 = 1.5;
    double k3 = 1.25;
    *gp_ << "set table\n";
    *gp_ << "plot"
       << " \"" << name << "\" i 0 u 0:($0==0?(def_P=$" << power << "):$" << power << "),"
       << " \"" << name << "\" i 0 u 0:($0==0?(def_E=$" << energy << "):$" << energy << "),"
       << " \"" << name << "\" i 0 u 0:($0==0?(def_t=$" << time << "):$" << time << "),\n";
    *gp_ << "unset table\n";
    *gp_ << "print def_P, def_E, def_t\n";
    *gp_ << "inv(x) = 1/x\n";
    *gp_ << "k1=" << k1 << "\n";
    *gp_ << "k2=" << k2 << "\n";
    *gp_ << "k3=" << k3 << "\n";
    *gp_ << "b1= k1/(k1-1)\n";
    *gp_ << "b2= k2/(k2-1)\n";
    *gp_ << "b3= k3/(k3-1)\n";
    *gp_ << "eds1(x) = -b1/k1 * x + b1\n";
    *gp_ << "eds2(x) = -b2/k2 * x + b2\n";
    *gp_ << "eds3(x) = -b3/k3 * x + b3\n";
    *gp_ << "x_max = 2.4\n";
    initPlot(800, 800);
    *gp_ << "first(x) = ($0 > 0 ? base : base = x)\n";
    *gp_ << "set yrange [0:1.4]\n";
    *gp_ << "set xrange [0.8:x_max]\n";
    *gp_ << "set key vert\n";
    *gp_ << "set xlabel \"Normalized execution time [-]\" font \",25\"\n"
        << "set ylabel \"Normalized energy consumption [-]\" font \",25\" offset 2,0\n";
    // *gp << "print first(" << time << ")\n";
    *gp_ << "plot \"" << name << "\" using ($"
              << time
              << "/def_t):($"
              << energy
              << "/def_E) "
              << "title 'single result' with points lc rgb 'black' ps 2,"
              << "[0.01:x_max] inv(x) with lines ls 8888 title 'EDP = 1',"
              << "[0:x_max] 1 with lines ls 1111 title 'E = 1',"
              << "[0:x_max] eds1(x) with lines ls 3333 title 'EDS, k = 2.0',"
              << "[0:x_max] eds2(x) with lines ls 4444 title 'EDS, k = 1.5',"
              << "[0:x_max] eds3(x) with lines ls 11111 title 'EDS, k = 1.25'"
              << "\n";

}

void PlotBuilder::plotEPall(std::string name) {
    int energy = 2;
    int time = 4;
    int instr = 10;
    int powercap = 1;
    int power = 3;
    int eds = 14;
    // double k1 = 2.0;
    // double k2 = 1.5;
    *gp_ << "set table\n";
    *gp_ << "plot"
       << " \"" << name << "\" i 0 u 0:($0==0?(def_P=$" << power << "):$" << power << "),"
       << " \"" << name << "\" i 0 u 0:($0==1?(max_P=$" << powercap << "):$" << powercap << "),"
       << " \"" << name << "\" i 0 u 0:($0==0?(def_E=$" << energy << "):$" << energy << "),"
       << " \"" << name << "\" i 0 u 0:($0==0?(def_t=$" << time << "):$" << time << "),"
       << " \"" << name << "\" i 0 u 0:($0==0?(def_i=$" << instr << "):$" << instr << "),\n";
    *gp_ << "unset table\n";
    *gp_ << "print def_P, def_E, def_t, def_i, max_P\n";
    // *gp << "t = [0.01:2]\n";
    // *gp << "inv(x) = 1/x\n";
    // *gp << "k1=" << k1 << "\n";
    // *gp << "k2=" << k2 << "\n";
    // *gp << "b1= k1/(k1-1)\n";
    // *gp << "b2= k2/(k2-1)\n";
    // *gp << "eds1(x) = -b1/k1 * x + b1\n";
    // *gp << "eds2(x) = -b2/k2 * x + b2\n";
    initPlot(800, 600);
    *gp_ << "first(x) = ($0 > 0 ? base : base = x)\n";
    *gp_ << "set xrange [:max_P]\n";
    *gp_ << "set key vert\n";
    *gp_ << "set xlabel \"power cap [W]\"\n";
    *gp_ << "set yrange [0:2.5]\n"
        << "set ytics 0.1 font \",15\"\n"
        << "set ylabel \"normalized result [-]\" font \",20\" offset 2,0\n";
    // *gp << "print first(" << time << ")\n";
    *gp_ << "plot \""
        << name << "\" using " << powercap
        << ":($" << energy << "/def_E) "
        << "title 'E [-]' with linespoints ls 11111, \""
        << name << "\" using " << powercap
        << ":($" << time << "/def_t) "
        << "title 't [-]' with linespoints ls 7777, \""
        << name << "\" using " << powercap
        << ":($" << power << "/def_P) "
        << "title 'P [-]' with linespoints ls 3333, \""
        << name << "\" using " << powercap
        << ":(($" << energy << ")*($" << time << ")/(def_E * def_t)) "
        << "title 'EDP [-]' with linespoints ls 4444, \""
        << name << "\" using " << powercap
        << ":($" << eds << ") "
        << "title 'EDS [-]' with linespoints ls 1111, \""
        << name << "\" using " << powercap
        << ":($" << instr << "/def_i) "
        << "title 'instr. [-]' with linespoints ls 6666,"
        << "\n";
}
