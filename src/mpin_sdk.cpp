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
 * M-Pin SDK implementation
 */


#include "mpin_sdk.h"
#include "mpin_crypto_non_tee.h"
#include "json/visitor.h"
#include <sstream>
#include <set>

typedef MPinSDKBase::Status Status;
typedef MPinSDKBase::User User;
typedef MPinSDKBase::UserPtr UserPtr;
typedef MPinSDKBase::String String;
typedef MPinSDKBase::StringMap StringMap;
typedef MPinSDKBase::IHttpRequest::Method HttpMethod;

MPinSDK::MPinSDK() : MPinSDKBase() {}

Status MPinSDK::StartRegistration(UserPtr user, const String& activateCode, const String& userData)
{
    return MPinSDKBase::StartRegistration(user, activateCode, userData, "", "");
}

Status MPinSDK::RestartRegistration(UserPtr user, const String& userData)
{
    return MPinSDKBase::RestartRegistration(user, userData);
}

Status MPinSDK::ConfirmRegistration(INOUT UserPtr user, const String& pushToken)
{
    return MPinSDKBase::ConfirmRegistration(user, pushToken);
}

Status MPinSDK::FinishRegistration(INOUT UserPtr user, const String& pin)
{
    return MPinSDKBase::FinishRegistration(user, pin);
}

Status MPinSDK::StartAuthentication(INOUT UserPtr user)
{
    return MPinSDKBase::StartAuthentication(user, "");
}

Status MPinSDK::CheckAccessNumber(const String& accessNumber)
{
    if(accessNumber.empty() || !ValidateAccessNumber(accessNumber))
    {
        return Status(Status::INCORRECT_ACCESS_NUMBER, "Invalid access number");
    }

    return Status::OK;
}

Status MPinSDK::FinishAuthentication(INOUT UserPtr user, const String& pin)
{
    util::JsonObject authResult;
    return FinishAuthenticationImpl(user, pin, "", NULL, authResult);
}

Status MPinSDK::FinishAuthentication(INOUT UserPtr user, const String& pin, OUT String& authResultData)
{
    util::JsonObject authResult;

    Status s = FinishAuthenticationImpl(user, pin, "", NULL, authResult);

    authResultData = authResult.ToString();
    return s;
}

Status MPinSDK::FinishAuthenticationOTP(INOUT UserPtr user, const String& pin, OUT OTP& otp)
{
    return MPinSDKBase::FinishAuthenticationOTP(user, pin, otp);
}

Status MPinSDK::FinishAuthenticationAN(INOUT UserPtr user, const String& pin, const String& accessNumber)
{
    return MPinSDKBase::FinishAuthenticationAC(user, pin, accessNumber);
}

bool MPinSDK::ValidateAccessNumber(const String& accessNumber)
{
    bool accessNumberUseCheckSum = m_clientSettings.GetBoolParam("accessNumberUseCheckSum", true);
    int accessNumberDigits = m_clientSettings.GetIntParam("accessNumberDigits", AN_WITH_CHECKSUM_LEN);

    if(!accessNumberUseCheckSum || accessNumberDigits != AN_WITH_CHECKSUM_LEN)
    {
        return true;
    }

    return ValidateAccessNumberChecksum(accessNumber);
}

bool MPinSDK::ValidateAccessNumberChecksum(const String& accessNumber)
{
    size_t len = accessNumber.length();
    const char *data = accessNumber.data();
    // Checksum is the last number digit (0 - 9)
    int checkSum = data[len - 1];
    // We are working with AN_WITH_CHECKSUM_LEN *digit* access numbers only
    if(len != AN_WITH_CHECKSUM_LEN || !isdigit(checkSum))
    {
        return false;
    }
    checkSum -= '0';

    int calculatedCheckSum = 0;
    for( ; len > 1; --len, ++data)
    {
        if(!isdigit(*data))
        {
            return false;
        }

        calculatedCheckSum += (*data - '0') * len;
    }

    calculatedCheckSum = ((11 - calculatedCheckSum % 11) % 11) % 10;
    return calculatedCheckSum == checkSum;
}

namespace
{
    class BackendFilter
    {
    public:
        BackendFilter(const String& backend) : m_backend(backend) {}

        bool operator()(const UserPtr& user) const
        {
            return user->GetBackend() == m_backend;
        }

    private:
        String m_backend;
    };
}

Status MPinSDK::ListUsers(OUT std::vector<UserPtr>& users, const String& backend) const
{
    return FilterUsers(users, BackendFilter(MakeBackendKey(backend)));
}

Status MPinSDK::ListUsers(OUT std::vector<UserPtr>& users) const
{
    Status s = CheckIfBackendIsSet();
    if(s != Status::OK)
    {
        return s;
    }

    return ListUsers(users, m_RPAServer);
}

Status MPinSDK::ListAllUsers(OUT std::vector<UserPtr>& users) const
{
    return MPinSDKBase::ListAllUsers(users);
}

Status MPinSDK::ListBackends(OUT std::vector<String>& backends) const
{
    Status s = CheckIfIsInitialized();
    if(s != Status::OK)
    {
        return s;
    }

    std::set<String> backendsSet;
    for (UsersMap::const_iterator i = m_users.begin(); i != m_users.end(); ++i)
    {
        backendsSet.insert(i->second->GetBackend());
    }

    backends.clear();
    backends.insert(backends.end(), backendsSet.begin(), backendsSet.end());

    return Status::OK;
}
