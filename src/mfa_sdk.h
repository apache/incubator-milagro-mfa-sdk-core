/*
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
*/

/*
* MFA SDK interface
*/

#ifndef _MFA_SDK_H_
#define _MFA_SDK_H_

#include "mpin_sdk_base.h"

class MfaSDK : public MPinSDKBase
{
public:
    class ServiceDetails
    {
    public:
        String name;
        String backendUrl;
        String rpsPrefix;
        String logoUrl;
    };

    class SessionDetails
    {
    public:
        void Clear();

        String prerollId;
        String appName;
        String appIconUrl;
        String customerId;
        String customerName;
        String customerIconUrl;
    };

    MfaSDK();

    Status GetServiceDetails(const String& url, OUT ServiceDetails& serviceDetails);
    void SetCID(const String& cid);
    void ClearCustomHeaders();

    Status GetSessionDetails(const String& accessCode, OUT SessionDetails& sessionDetails);
    Status AbortSession(const String& accessCode);

    Status StartRegistration(INOUT UserPtr user, const String& accessCode, const String& pushToken);
    Status RestartRegistration(INOUT UserPtr user);
    Status ConfirmRegistration(INOUT UserPtr user);
    Status FinishRegistration(INOUT UserPtr user, const String& pin);

    Status StartAuthentication(INOUT UserPtr user, const String& accessCode);
    Status FinishAuthentication(INOUT UserPtr user, const String& pin, OUT String& authzCode);
    Status FinishAuthenticationAC(INOUT UserPtr user, const String& pin, const String& accessCode);

    Status ListUsers(OUT std::vector<UserPtr>& users) const;
};

#endif // _MFA_SDK_H_

#include "mpin_sdk_base.h"

