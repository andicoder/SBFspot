#include "influxdb.h"

int main()
{
	InverterData inverter{
		.Serial = 12356,
		.Pac1 = 239,
		.Uac1 = 23900,
		.Iac1 = 1000,
		.EToday = 5200,
		.ETotal = 4600000,
	};

	strcpy(inverter.DeviceName, "Test");
	strcpy(inverter.DeviceType, "Inverter");

	InverterData inverter2{
		.Serial = 562356,
		.Pac1 = 500,
		.Uac1 = 25000,
		.Iac1 = 2000,
		.EToday = 1000,
		.ETotal = 2600000,
	};

	strcpy(inverter2.DeviceName, "Test2");
	strcpy(inverter2.DeviceType, "Inverter");

	Config cfg = {
		.influxdb_host = "localhost",
		.influxdb_port = "8086",
		.influxdb_database = "smartmeter",
		.influxdb = 1,
	};

	strcpy(cfg.plantname, "MyPlant");

	InverterData *inverters[MAX_INVERTERS];
	memset(inverters, 0, sizeof(inverters));
	inverters[0] = &inverter;
	inverters[1] = &inverter2;
	return ExportSpotDataToInfluxdb(&cfg, inverters);
}