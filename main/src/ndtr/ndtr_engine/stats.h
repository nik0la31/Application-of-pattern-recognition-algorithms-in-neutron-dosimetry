#ifndef STATS_H
#define STATS_H

class Stats
{
public:
    Stats();

    int TracesCount = 0;
    int MinDiameter = -1;
    int MaxDiameter = -1;
    int AverageDiameter = -1;
    int MinIntensity = -1;
    int MaxIntensity = -1;
    int AverageIntensity = -1;
};

#endif // STATS_H
