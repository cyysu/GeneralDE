#include "../EnvExt.hpp"
#include "SSKeychain.h"

namespace UI { namespace App {

const char * EnvExt::deviceId(void) const {

    if (m_deviceId.empty()) {
        NSError* error = nil;
        NSString* randomID = [SSKeychain passwordForService:@"T4.The9.DeviceID" account:@"commonuser" error:&error];
        if ([error code] == SSKeychainErrorNotFound) {
            //NSLog(@"Passwordnot found");
            srand((unsigned)time(NULL));
            randomID = [NSString stringWithFormat:@"%d%d", rand(), rand()];
            [SSKeychain setPassword: [NSString stringWithFormat:@"%@", randomID] forService:@"T4.The9.DeviceID" account:@"commonuser"];
        }

        m_deviceId = [randomID UTF8String];
    }

	return m_deviceId.c_str();
}

}}
