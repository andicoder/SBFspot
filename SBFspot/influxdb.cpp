#include "influxdb.h"
#include <curl/curl.h>
#include <chrono>

const int MAX_INVERTERS = 20;

namespace
{
void applyHttpAuth(CURL* curl, const std::string& auth)
{
	curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
	curl_easy_setopt(curl, CURLOPT_USERPWD, auth.c_str());
}

curl_slist* applyHttpHeaders(CURL* curl, const std::vector<std::string>& headers)
{
	curl_slist* curlHeaderList{};
	if (!headers.empty())
	{
		for (const auto& header : headers)
		{
			curlHeaderList = curl_slist_append(curlHeaderList, header.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curlHeaderList);
	}

	return curlHeaderList;
}

size_t writeDataCb(void* ptr, size_t size, size_t nmemb, std::string* data)
{
	data->append((char*)ptr, size * nmemb);
	return size * nmemb;
}

bool Post(const std::string& url, const std::string& data, const std::string& httpAuth)
{
	std::vector<std::string> httpHeaders;

	curl_global_init(CURL_GLOBAL_ALL);
	auto curl = curl_easy_init();

	/* First set the URL that is about to receive our POST. This URL can
	   just as well be a https:// URL if that is what should receive the
	   data. */
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	/* Now specify the POST data */
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

	curl_slist* curlHeaderList = applyHttpHeaders(curl, httpHeaders);
	applyHttpAuth(curl, httpAuth);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCb);

	std::string response;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	/* Perform the request, res will get the return code */
	auto curlRes = curl_easy_perform(curl);
	long httpCode{};
	curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &httpCode);
	bool success = (httpCode < 300 && httpCode >= 200);

	curl_slist_free_all(curlHeaderList); /* free the list again */
	curl_easy_cleanup(curl);
	return success && CURLE_OK == curlRes;
}

void AppendInverterData(InverterData* inverter, std::string& sendBuffer, std::chrono::nanoseconds spotTime)
{
#define APPEND_VALUE(value, factor) \
    sendBuffer.append(#value).append(1, '=').append(std::to_string(inverter->value / factor))

#define APPEND_VALUE_NAME(value, factor, name) \
    sendBuffer.append(#name).append(1, '=').append(std::to_string(inverter->value / factor))

	sendBuffer.append("spot,"); // measurement
	sendBuffer.append("DeviceName=").append(inverter->DeviceName).append(1, ',');
	sendBuffer.append("DeviceType=").append(inverter->DeviceType).append(1, ',');
	sendBuffer.append("Serial=").append(std::to_string(inverter->Serial)).append(1, ' ');

	APPEND_VALUE(Pdc1, 1.f).append(1, ',');
	APPEND_VALUE(Pdc2, 1.f).append(1, ',');
	APPEND_VALUE(Idc1, 1000.f).append(1, ',');
	APPEND_VALUE(Idc2, 1000.f).append(1, ',');
	APPEND_VALUE(Udc1, 100.f).append(1, ',');
	APPEND_VALUE(Udc2, 100.f).append(1, ',');
	APPEND_VALUE(Pac1, 1.f).append(1, ',');
	APPEND_VALUE(Pac2, 1.f).append(1, ',');
	APPEND_VALUE(Pac3, 1.f).append(1, ',');
	APPEND_VALUE(Iac1, 1000.f).append(1, ',');
	APPEND_VALUE(Iac2, 1000.f).append(1, ',');
	APPEND_VALUE(Iac3, 1000.f).append(1, ',');
	APPEND_VALUE(Uac1, 100.f).append(1, ',');
	APPEND_VALUE(Uac2, 100.f).append(1, ',');
	APPEND_VALUE(Uac3, 100.f).append(1, ',');
	APPEND_VALUE(EToday, 1000.f).append(1, ',');
	APPEND_VALUE(ETotal, 1000.f).append(1, ',');
	APPEND_VALUE(GridFreq, 100.f).append(1, ',');
	APPEND_VALUE(OperationTime, 3600).append(1, ',');
	APPEND_VALUE(FeedInTime, 3600).append(1, ',');
	APPEND_VALUE(BT_Signal, 1.f).append(1, ',');
	APPEND_VALUE(Temperature, 100.f).append(1, ',');
	APPEND_VALUE_NAME(calPdcTot, 1.f, PdcTot).append(1, ',');
	APPEND_VALUE_NAME(TotalPac, 1.f, PacTot).append(1, ',');
	APPEND_VALUE_NAME(calEfficiency, 1.f, Efficiency).append(1, ' ');
	sendBuffer.append(std::to_string(spotTime.count()));
}
}

int ExportSpotDataToInfluxdb(const Config* cfg, InverterData* inverters[])
{
	using namespace std::chrono;

	auto spotTime = seconds(time(nullptr));
	auto spotTimeNs = duration_cast<nanoseconds>(spotTime);

	std::string sendBuffer;
	sendBuffer.reserve(512);

	InverterData total{};

	for (int inv = 0; inverters[inv] != nullptr && inv < MAX_INVERTERS; inv++)
	{
		if (inv)
		{
			sendBuffer.append(1, '\n');
		}

		strcpy(total.DeviceType, inverters[inv]->DeviceType);
		total.Pdc1 += inverters[inv]->Pdc1;
		total.Pdc2 += inverters[inv]->Pdc2;
		total.Idc1 += inverters[inv]->Idc1;
		total.Idc2 += inverters[inv]->Idc2;
		total.Udc1 = std::max(inverters[inv]->Udc1, total.Udc1);
		total.Udc2 = std::max(inverters[inv]->Udc2, total.Udc2);
		total.Pac1 += inverters[inv]->Pac1;
		total.Pac2 += inverters[inv]->Pac2;
		total.Pac3 += inverters[inv]->Pac3;
		total.Iac1 += inverters[inv]->Iac1;
		total.Iac2 += inverters[inv]->Iac2;
		total.Iac3 += inverters[inv]->Iac3;
		total.Uac1 = std::max(inverters[inv]->Uac1, total.Uac1);
		total.Uac2 = std::max(inverters[inv]->Uac2, total.Uac2);
		total.Uac3 = std::max(inverters[inv]->Uac3, total.Uac3);
		total.calPacTot += inverters[inv]->calPacTot;
		total.calPdcTot += inverters[inv]->calPdcTot;
		total.TotalPac += inverters[inv]->TotalPac;
		total.EToday += inverters[inv]->EToday;
		total.ETotal += inverters[inv]->ETotal;
		total.GridFreq = std::max(inverters[inv]->GridFreq, total.GridFreq);
		total.OperationTime = std::max(inverters[inv]->OperationTime, total.OperationTime);
		total.FeedInTime = std::max(inverters[inv]->FeedInTime, total.FeedInTime);
		total.BT_Signal = std::max(inverters[inv]->BT_Signal, total.BT_Signal);
		total.Temperature = std::max(inverters[inv]->Temperature, total.Temperature);

		AppendInverterData(inverters[inv], sendBuffer, spotTimeNs);
	}

	strcpy(total.DeviceName, cfg->plantname);
	total.calEfficiency = total.calPdcTot == 0 ? 0.f : 100.f * total.calPacTot / total.calPdcTot;
	sendBuffer.append(1, '\n');
	AppendInverterData(&total, sendBuffer, spotTimeNs);

	std::string url = "http://" + cfg->influxdb_host + ":" + cfg->influxdb_port + "/write?db=" + cfg->influxdb_database;
	return Post(url, sendBuffer, cfg->influxdb_user + ":" + cfg->influxdb_password) ? 0 : 1;
}
