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
* MFA SDK implementation
*/

#include "mfa_sdk.h"
#include "mpin_crypto_non_tee.h"
#include "json/visitor.h"
#include <sstream>

typedef MPinSDKBase::Status Status;
typedef MPinSDKBase::User User;
typedef MPinSDKBase::UserPtr UserPtr;
typedef MPinSDKBase::String String;
typedef MPinSDKBase::StringMap StringMap;
typedef MPinSDKBase::IHttpRequest::Method HttpMethod;

MfaSDK::MfaSDK() {}

Status MfaSDK::GetServiceDetails(const String& url, OUT ServiceDetails& serviceDetails)
{
    HttpResponse response = MakeGetRequest(String().Format("%s/service", String(url).TrimRight("/").c_str()));
    if (response.GetStatus() != HttpResponse::HTTP_OK)
    {
        return response.TranslateToMPinStatus(HttpResponse::GET_SERVICE_DETAILS);
    }

    try
    {
        const util::JsonObject& data = response.GetJsonData();
        serviceDetails.name = ((const json::String&) data["name"]).Value();
        serviceDetails.backendUrl = ((const json::String&) data["url"]).Value();
        serviceDetails.rpsPrefix = ((const json::String&) data["rps_prefix"]).Value();
        serviceDetails.logoUrl = ((const json::String&) data["logo_url"]).Value();
        String type = ((const json::String&) data["type"]).Value();
        if (type != "online")
        {
            throw json::Exception(String().Format("Unexpected service details type: '%s'. Must be 'online'", type.c_str()));
        }

        return Status::OK;
    }
    catch (json::Exception& e)
    {
        response.SetResponseJsonParseError(e.what());
        return response.TranslateToMPinStatus(HttpResponse::GET_SERVICE_DETAILS);
    }
}

namespace
{
    const char *CID_HEADER = "X-MIRACL-CID";
}

void MfaSDK::SetCID(const String& clientId)
{
    m_customHeaders[CID_HEADER] = clientId;
}

void MfaSDK::ClearCustomHeaders()
{
    String cid = m_customHeaders.Get(CID_HEADER);
    MPinSDKBase::ClearCustomHeaders();
    SetCID(cid);
}

void MfaSDK::SessionDetails::Clear()
{
    prerollId.clear();
    appName.clear();
    appIconUrl.clear();
    customerId.clear();
    customerName.clear();
    customerIconUrl.clear();
}

Status MfaSDK::GetSessionDetails(const String& accessCode, OUT SessionDetails& sessionDetails)
{
    sessionDetails.Clear();

    Status s = CheckIfBackendIsSet();
    if (s != Status::OK)
    {
        return s;
    }

    String codeStatusUrl = m_clientSettings.GetStringParam("codeStatusURL");
    if (codeStatusUrl.empty())
    {
        return Status::OK;
    }

    util::JsonObject data;
    data["status"] = json::String("wid");
    data["wid"] = json::String(accessCode);

    HttpResponse response = MakeRequest(codeStatusUrl, IHttpRequest::POST, data);
    if (response.GetStatus() != HttpResponse::HTTP_OK)
    {
        return response.TranslateToMPinStatus(HttpResponse::GET_SESSION_DETAILS);
    }

    const util::JsonObject& json = response.GetJsonData();
    sessionDetails.prerollId = json.GetStringParam("prerollId");
    sessionDetails.appName = json.GetStringParam("appName");
    sessionDetails.appIconUrl = json.GetStringParam("appLogoURL");
    sessionDetails.customerId = json.GetStringParam("customerId");
    sessionDetails.customerName = json.GetStringParam("customerName");
    sessionDetails.customerIconUrl = json.GetStringParam("customerLogoURL");

    return Status::OK;
}

Status MfaSDK::AbortSession(const String& accessCode)
{
    Status s = CheckIfBackendIsSet();
    if (s != Status::OK)
    {
        return s;
    }

    String codeStatusUrl = m_clientSettings.GetStringParam("codeStatusURL");
    if (codeStatusUrl.empty())
    {
        return Status::OK;
    }

    util::JsonObject data;
    data["status"] = json::String("abort");
    data["wid"] = json::String(accessCode);

    HttpResponse response = MakeRequest(codeStatusUrl, IHttpRequest::POST, data);
    if (response.GetStatus() != HttpResponse::HTTP_OK)
    {
        return response.TranslateToMPinStatus(HttpResponse::ABORT_SESSION);
    }

    return Status::OK;
}

Status MfaSDK::StartRegistration(INOUT UserPtr user, const String& accessCode, const String& pushToken)
{
    return MPinSDKBase::StartRegistration(user, "", "", accessCode, pushToken);
}

Status MfaSDK::RestartRegistration(INOUT UserPtr user)
{
    return MPinSDKBase::RestartRegistration(user, "");
}

Status MfaSDK::ConfirmRegistration(INOUT UserPtr user)
{
    return MPinSDKBase::ConfirmRegistration(user, "");
}

Status MfaSDK::FinishRegistration(INOUT UserPtr user, const String& pin)
{
    return MPinSDKBase::FinishRegistration(user, pin);
}

Status MfaSDK::GetAccessCode(const String& authzUrl, OUT String& accessCode)
{
    Status s = CheckIfIsInitialized();
    if (s != Status::OK)
    {
        return s;
    }

    HttpResponse response = MakeRequest(authzUrl, IHttpRequest::POST, util::JsonObject());
    if (response.GetStatus() != HttpResponse::HTTP_OK)
    {
        return response.TranslateToMPinStatus(HttpResponse::GET_ACCESS_CODE);
    }

    try
    {
        const util::JsonObject& data = response.GetJsonData();
        String qrURL = ((const json::String&) data["qrURL"]).Value();
        size_t index = qrURL.find_last_of('#');
        if (qrURL.empty() || index >= qrURL.length() - 1)
        {
            throw json::Exception(String().Format("Failed to extract accessCode from qrURL: '%s'", qrURL.c_str()));
        }
        accessCode = qrURL.substr(index + 1);
        return Status::OK;
    }
    catch (json::Exception& e)
    {
        response.SetResponseJsonParseError(e.what());
        return response.TranslateToMPinStatus(HttpResponse::GET_ACCESS_CODE);
    }
}

Status MfaSDK::StartAuthentication(INOUT UserPtr user, const String& accessCode)
{
    return MPinSDKBase::StartAuthentication(user, accessCode);
}

Status MfaSDK::FinishAuthentication(INOUT UserPtr user, const String& pin, const String& accessCode)
{
    return MPinSDKBase::FinishAuthenticationAC(user, pin, accessCode);
}

Status MfaSDK::FinishAuthentication(INOUT UserPtr user, const String& pin, const String& accessCode, OUT String& authzCode)
{
    util::JsonObject authResult;

    Status s = FinishAuthenticationImpl(user, pin, accessCode, NULL, authResult);

    authzCode = authResult.GetStringParam("code");
    return s;
}

Status MfaSDK::ListUsers(OUT std::vector<UserPtr>& users) const
{
    return MPinSDKBase::ListAllUsers(users);
}
