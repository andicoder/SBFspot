#include "influxdb.h"
#include <curl/curl.h>

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
    if (!headers.empty()) {
        for (const auto& header : headers) {
            curlHeaderList = curl_slist_append(curlHeaderList, header.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curlHeaderList);
    }

    return curlHeaderList;
}

//void readResultInto(CURL* curl, CURLcode resultCode, net::Result& out_result)
//{
//    long httpCode{};
//    curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &httpCode);
//    out_result.httpCode = static_cast<uint16_t>(httpCode);
//    out_result.success = (httpCode < 300 && httpCode >= 200);
//}

size_t writeDataCb(void* ptr, size_t size, size_t nmemb, std::string* data)
{
    data->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

int Post(const std::string& url, const std::string& data, const std::string& httpAuth)
{
    std::vector<std::string> httpHeaders;

    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();

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
    auto curl_res = curl_easy_perform(curl);
//    readResultInto(curl, curl_res, res);

    curl_slist_free_all(curlHeaderList); /* free the list again */
    
    curl_easy_cleanup(curl);
    return 0;
}

void AppendInverterData(InverterData *inverter, std::string& sendBuffer, time_t spotTime)
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
    sendBuffer.append(std::to_string(spotTime));
}
}

int ExportSpotDataToInfluxdb(const Config *cfg, InverterData *inverters[])
{
    std::string sendBuffer;
    sendBuffer.reserve(512);
    for (int inv=0; inverters[inv]!=NULL && inv<MAX_INVERTERS; inv++)
    {
        if (inv)
        {
            sendBuffer.append(1, '\n');
        }

        // Take time from computer instead of inverter
        time_t spottime = cfg->SpotTimeSource == 0 ? inverters[0]->InverterDatetime : time(NULL);
        spottime *= 1000000000; // ns
        AppendInverterData(inverters[inv], sendBuffer, spottime);
    }

    Post("", sendBuffer, "");

    return 0;
}
