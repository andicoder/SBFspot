#ifndef INFLUXDB_H
#define INFLUXDB_H

#include "osselect.h"
#include "SBFspot.h"
#include "EventData.h"

int ExportSpotDataToInfluxdb(const Config *cfg, InverterData *inverters[]);
int ExportDayDataToInfluxdb(const Config *cfg, InverterData *inverters[]);

#endif //INFLUXDB_H
