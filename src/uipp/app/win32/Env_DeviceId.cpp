#include "../EnvExt.hpp"
#include "iphlpapi.h"

#pragma comment(lib,"Iphlpapi.lib")

namespace UI { namespace App {

const char * EnvExt::deviceId(void) const {
    static char s_device[128] = {0};

    if (s_device[0] == 0) {
        PIP_ADAPTER_INFO pAdapterInfo;
        PIP_ADAPTER_INFO pAdapter = NULL;
        DWORD dwRetVal = 0;

        pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );

        ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
        if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS) {
            free(pAdapterInfo);
            pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen);
        }

        if ((dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
            pAdapter = pAdapterInfo;
            while (pAdapter) {
                if(strstr(pAdapter->Description,"PCI") > 0 // pAdapter->Description中包含"PCI"为：物理网卡
                    || pAdapter->Type == 71                   // pAdapter->Type是71为：无线网卡
                    )
                {
                    printf("------------------------------------------------------------\n");
                    printf("Adapter Name: \t%s\n", pAdapter->AdapterName);
                    printf("Adapter Desc: \t%s\n", pAdapter->Description);
                    printf("Adapter Addr: \t");
                    for (UINT i = 0; i < pAdapter->AddressLength; i++)
                    {
                        printf("%02X%c", pAdapter->Address[i],
                               i == pAdapter->AddressLength - 1 ? '\n' : '-');
                    }
                    printf("Adapter Type: \t%d\n", pAdapter->Type);
                    printf("IP Address: \t%s\n", pAdapter->IpAddressList.IpAddress.String);
                    printf("IP Mask: \t%s\n", pAdapter->IpAddressList.IpMask.String);
                }
                pAdapter = pAdapter->Next;
            }
        }
        else {
            printf("Call to GetAdaptersInfo failed.\n");
        }
    }

    return s_device;
}

}}
