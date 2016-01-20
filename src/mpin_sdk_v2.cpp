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
 * M-Pin SDK version 2 implementation
 */

#include "mpin_sdk_v2.h"
#include "mpin_crypto.h"
#include "version.h"

typedef MPinSDKv2::Status Status;
typedef MPinSDKv2::User User;
typedef MPinSDKv2::UserPtr UserPtr;
typedef MPinSDKv2::String String;
typedef MPinSDKv2::IHttpRequest IHttpRequest;
typedef MPinSDKv2::IPinPad IPinPad;
typedef MPinSDKv2::CryptoType CryptoType;



MPinSDKv2::Context::Context() : m_appContext(NULL)
{
    m_pinpad = new Pinpad();
}

MPinSDKv2::Context::~Context()
{
    delete m_pinpad;
}

void MPinSDKv2::Context::Init(MPinSDKv2::IContext *appContext)
{
    m_appContext = appContext;
}

void MPinSDKv2::Context::SetPin(const String& pin)
{
    m_pinpad->SetPin(pin);
}

IHttpRequest * MPinSDKv2::Context::CreateHttpRequest() const
{
    return m_appContext->CreateHttpRequest();
}

void MPinSDKv2::Context::ReleaseHttpRequest(IN IHttpRequest *request) const
{
    m_appContext->ReleaseHttpRequest(request);
}

MPinSDKv2::IStorage * MPinSDKv2::Context::GetStorage(IStorage::Type type) const
{
    return m_appContext->GetStorage(type);
}

IPinPad * MPinSDKv2::Context::GetPinPad() const
{
    return m_pinpad;
}

CryptoType MPinSDKv2::Context::GetMPinCryptoType() const
{
    return m_appContext->GetMPinCryptoType();
}






MPinSDKv2::MPinSDKv2()
{
}

MPinSDKv2::~MPinSDKv2()
{
}

Status MPinSDKv2::Init(const StringMap& config, IN IContext* ctx)
{
    m_context.Init(ctx);
    return m_v1Sdk.Init(config, &m_context);
}

void MPinSDKv2::Destroy()
{
    m_v1Sdk.Destroy();
}

void MPinSDKv2::ClearUsers()
{
    m_v1Sdk.ClearUsers();
}

Status MPinSDKv2::TestBackend(const String& server, const String& rpsPrefix) const
{
    return m_v1Sdk.TestBackend(server, rpsPrefix);
}

Status MPinSDKv2::SetBackend(const String& server, const String& rpsPrefix)
{
    return m_v1Sdk.SetBackend(server, rpsPrefix);
}

UserPtr MPinSDKv2::MakeNewUser(const String& id, const String& deviceName) const
{
    return m_v1Sdk.MakeNewUser(id, deviceName);
}

Status MPinSDKv2::StartRegistration(INOUT UserPtr user, const String& userData)
{
    return m_v1Sdk.StartRegistration(user, userData);
}

Status MPinSDKv2::RestartRegistration(INOUT UserPtr user, const String& userData)
{
    return m_v1Sdk.RestartRegistration(user, userData);
}

Status MPinSDKv2::VerifyUser(INOUT UserPtr user, const String& mpinId, const String& activationKey)
{
    return m_v1Sdk.VerifyUser(user, mpinId, activationKey);
}

Status MPinSDKv2::ConfirmRegistration(INOUT UserPtr user, const String& pushMessageIdentifier)
{
    Status s = m_v1Sdk.CheckIfBackendIsSet();
    if(s != Status::OK)
    {
        return s;
    }

	// A user can get here either in STARTED_REGISTRATION state or in ACTIVATED state (force-activate flow)
	// In the first case, the method might fail if the user identity has not been verified yet, and the user state
	// should stay as it was - STARTED_REGISTRATION
    s = m_v1Sdk.CheckUserState(user, User::STARTED_REGISTRATION);
    if(s != Status::OK)
    {
		Status sSave = s;
		s = m_v1Sdk.CheckUserState(user, User::ACTIVATED);
		if ( s != Status::OK )
		{
			return sSave;
		}
    }

    // Request a client secret share from the customer's D-TA and a signed request for a client secret share from CertiVox's D-TA.
    String mpinId = user->GetMPinId();
    String mpinIdHex = user->GetMPinIdHex();
    String regOTT = user->GetRegOTT();

    String url = String().Format("%s/%s?regOTT=%s", m_v1Sdk.m_clientSettings.GetStringParam("signatureURL"), mpinIdHex.c_str(), regOTT.c_str());
    if(!pushMessageIdentifier.empty())
    {
        url += "&pmiToken=" + pushMessageIdentifier;
    }

    HttpResponse response = m_v1Sdk.MakeGetRequest(url);
    if(response.GetStatus() != HttpResponse::HTTP_OK)
    {
        return response.TranslateToMPinStatus(HttpResponse::GET_CLIENT_SECRET1);
    }

    user->m_clientSecret1 = util::HexDecode(response.GetJsonData().GetStringParam("clientSecretShare"));

    // Request the client secret share from CertiVox's D-TA.
    String cs2Params = response.GetJsonData().GetStringParam("params");
    url.Format("%sclientSecret?%s", m_v1Sdk.m_clientSettings.GetStringParam("certivoxURL"), cs2Params.c_str());
    response = m_v1Sdk.MakeGetRequest(url);
    if(response.GetStatus() != HttpResponse::HTTP_OK)
    {
        return response.TranslateToMPinStatus(HttpResponse::GET_CLIENT_SECRET2);
    }

    user->m_clientSecret2 = util::HexDecode(response.GetJsonData().GetStringParam("clientSecret"));

    return Status::OK;
}

Status MPinSDKv2::FinishRegistration(INOUT UserPtr user, const String& pin)
{
    Status s = m_v1Sdk.CheckIfBackendIsSet();
    if(s != Status::OK)
    {
        return s;
    }

	// A user can get here either in STARTED_REGISTRATION state or in ACTIVATED state (force-activate flow)
	// In the first case, the method might fail if the user identity has not been verified yet, and the user state
	// should stay as it was - STARTED_REGISTRATION
    s = m_v1Sdk.CheckUserState(user, User::STARTED_REGISTRATION);
    if(s != Status::OK)
    {
		Status sSave = s;
		s = m_v1Sdk.CheckUserState(user, User::ACTIVATED);
		if ( s != Status::OK )
		{
			return sSave;
		}
    }

    // In addition, client secret shares must be retrieved
    if(user->m_clientSecret1.empty() || user->m_clientSecret2.empty())
    {
        return Status(Status::FLOW_ERROR, String().Format("Cannot finish user '%s' registration: User identity not verified", user->GetId().c_str()));
    }

    // Set pin to the wrapper context
    m_context.SetPin(pin);

    s = m_v1Sdk.m_crypto->OpenSession();
    if(s != Status::OK)
    {
        return s;
    }

    std::vector<String> clientSecretShares;
    clientSecretShares.push_back(user->m_clientSecret1);
    clientSecretShares.push_back(user->m_clientSecret2);

    s = m_v1Sdk.m_crypto->Register(user, clientSecretShares);
    if(s != Status::OK)
    {
        m_v1Sdk.m_crypto->CloseSession();
        return s;
    }

    m_v1Sdk.m_crypto->CloseSession();

    user->SetRegistered();
    s = m_v1Sdk.WriteUsersToStorage();
    if(s != Status::OK)
    {
        return s;
    }

    return Status::OK;
}

Status MPinSDKv2::StartAuthentication(INOUT UserPtr user)
{
    Status s = m_v1Sdk.CheckIfBackendIsSet();
    if(s != Status::OK)
    {
        return s;
    }

    // Check if the user is already registered
    s = m_v1Sdk.CheckUserState(user, User::REGISTERED);
    if(s != Status::OK)
    {
        return s;
    }

    // Request a time permit share from the customer's D-TA and a signed request for a time permit share from CertiVox's D-TA.
    String mpinIdHex = user->GetMPinIdHex();
    String url = String().Format("%s/%s", m_v1Sdk.m_clientSettings.GetStringParam("timePermitsURL"), mpinIdHex.c_str());
    HttpResponse response = m_v1Sdk.MakeGetRequest(url);
    if(response.GetStatus() != HttpResponse::HTTP_OK)
    {
        return response.TranslateToMPinStatus(HttpResponse::GET_TIME_PERMIT1);
    }

    user->m_timePermitShare1 = util::HexDecode(response.GetJsonData().GetStringParam("timePermit"));

    // Request time permit share from CertiVox's D-TA (Searches first in user cache, than in S3 cache)
    s = m_v1Sdk.GetCertivoxTimePermitShare(user, response.GetJsonData(), user->m_timePermitShare2);
    if(s != Status::OK)
    {
        return s;
    }

    return Status::OK;
}

Status MPinSDKv2::CheckAccessNumber(const String& accessNumber)
{
    if(accessNumber.empty() || !m_v1Sdk.CheckAccessNumber(accessNumber))
    {
        return Status(Status::INCORRECT_ACCESS_NUMBER, "Invalid access number");
    }

    return Status::OK;
}

Status MPinSDKv2::FinishAuthentication(INOUT UserPtr user, const String& pin)
{
    util::JsonObject authResult;
    return FinishAuthenticationImpl(user, pin, "", NULL, authResult);
}

Status MPinSDKv2::FinishAuthentication(INOUT UserPtr user, const String& pin, OUT String& authResultData)
{
    util::JsonObject authResult;

    Status s = FinishAuthenticationImpl(user, pin, "", NULL, authResult);

    authResultData = authResult.ToString();
    return s;
}

Status MPinSDKv2::FinishAuthenticationOTP(INOUT UserPtr user, const String& pin, OUT OTP& otp)
{
    util::JsonObject authResult;
    String otpNumber;

    Status s = FinishAuthenticationImpl(user, pin, "", &otpNumber, authResult);

    otp.ExtractFrom(otpNumber, authResult);
    return s;
}

Status MPinSDKv2::FinishAuthenticationAN(INOUT UserPtr user, const String& pin, const String& accessNumber)
{
    util::JsonObject authResult;

    Status s = FinishAuthenticationImpl(user, pin, accessNumber, NULL, authResult);

    LogoutData logoutData;
    if(logoutData.ExtractFrom(authResult))
    {
        m_v1Sdk.m_logoutData.insert(std::make_pair(user, logoutData));
    }

    return s;
}

Status MPinSDKv2::FinishAuthenticationImpl(INOUT UserPtr user, const String& pin, const String& accessNumber, OUT String *otp, OUT util::JsonObject& authResultData)
{
    Status s = m_v1Sdk.CheckIfBackendIsSet();
    if(s != Status::OK)
    {
        return s;
    }

    // Check if the user is already registered
    s = m_v1Sdk.CheckUserState(user, User::REGISTERED);
    if(s != Status::OK)
    {
        return s;
    }

    // Check if time permit was obtained from StartAuthentication
    if(user->m_timePermitShare1.empty() || user->m_timePermitShare2.empty())
    {
        return Status(Status::FLOW_ERROR, String().Format("Cannot finish user '%s' authentication: Invalid time permit", user->GetId().c_str()));
    }

    String mpinIdHex = user->GetMPinIdHex();

    // Set pin to the wrapper context
    m_context.SetPin(pin);

    s = m_v1Sdk.m_crypto->OpenSession();
    if(s != Status::OK)
    {
        return s;
    }

    std::vector<String> timePermitShares;
    timePermitShares.push_back(user->m_timePermitShare1);
    timePermitShares.push_back(user->m_timePermitShare2);

    // Authentication pass 1
    String u, ut;
    s = m_v1Sdk.m_crypto->AuthenticatePass1(user, timePermitShares, u, ut);
    if(s != Status::OK)
    {
        m_v1Sdk.m_crypto->CloseSession();
        return s;
    }

    util::JsonObject requestData;
    requestData["pass"] = json::Number(1);
    requestData["mpin_id"] = json::String(mpinIdHex);
    requestData["UT"] = json::String(util::HexEncode(ut));
    requestData["U"] = json::String(util::HexEncode(u));

    String mpinAuthServerURL = m_v1Sdk.m_clientSettings.GetStringParam("mpinAuthServerURL");
    String url = String().Format("%s/pass1", mpinAuthServerURL.c_str());
    HttpResponse response = m_v1Sdk.MakeRequest(url, IHttpRequest::POST, requestData);
    if(response.GetStatus() != HttpResponse::HTTP_OK)
    {
        m_v1Sdk.m_crypto->CloseSession();
        return response.TranslateToMPinStatus(HttpResponse::AUTHENTICATE_PASS1);
    }

    String y = util::HexDecode(response.GetJsonData().GetStringParam("y"));

    // Authentication pass 2
    String v;
    m_v1Sdk.m_crypto->AuthenticatePass2(user, y, v);
    if(s != Status::OK)
    {
        m_v1Sdk.m_crypto->CloseSession();
        return s;
    }

    requestData.Clear();
    requestData["pass"] = json::Number(2);
    requestData["OTP"] = json::Boolean(otp != NULL ? true : false);
    requestData["WID"] = json::String(accessNumber.empty() ? "0" : accessNumber);
    requestData["V"] = json::String(util::HexEncode(v));
    requestData["mpin_id"] = json::String(mpinIdHex);

    url.Format("%s/pass2", mpinAuthServerURL.c_str());
    response = m_v1Sdk.MakeRequest(url, IHttpRequest::POST, requestData);
    if(response.GetStatus() != HttpResponse::HTTP_OK)
    {
        m_v1Sdk.m_crypto->CloseSession();
        return response.TranslateToMPinStatus(HttpResponse::AUTHENTICATE_PASS2);
    }

    // Save OTP data to be used if otp was requested
    if(otp != NULL)
    {
        *otp = response.GetJsonData().GetStringParam("OTP");
    }

    // Send response data from M-Pin authentication server to RPA
    url = m_v1Sdk.m_clientSettings.GetStringParam(accessNumber.empty() ? "authenticateURL" : "mobileAuthenticateURL");
    requestData.Clear();
    requestData["mpinResponse"] = response.GetJsonData();
    response = m_v1Sdk.MakeRequest(url, IHttpRequest::POST, requestData);
    if(response.GetStatus() != HttpResponse::HTTP_OK)
    {
        m_v1Sdk.m_crypto->CloseSession();
        s = response.TranslateToMPinStatus(HttpResponse::AUTHENTICATE_RPA);

        if(response.GetStatus() == HttpResponse::HTTP_GONE)
        {
            user->Block();
            m_v1Sdk.m_crypto->DeleteToken(user->GetMPinId());
            m_v1Sdk.WriteUsersToStorage();
        }

        return s;
    }
	
    // You are now logged in with M-Pin!
    m_v1Sdk.m_crypto->CloseSession();

    authResultData = response.GetJsonData();

    return Status::OK;
}

void MPinSDKv2::DeleteUser(INOUT UserPtr user)
{
    m_v1Sdk.DeleteUser(user);
}

void MPinSDKv2::ListUsers(OUT std::vector<UserPtr>& users)
{
    m_v1Sdk.ListUsers(users);
}

const char * MPinSDKv2::GetVersion()
{
    return MPIN_SDK_V2_VERSION;
}

bool MPinSDKv2::CanLogout(IN UserPtr user)
{
    return m_v1Sdk.CanLogout(user);
}

bool MPinSDKv2::Logout(IN UserPtr user)
{
    return m_v1Sdk.Logout(user);
}

String MPinSDKv2::GetClientParam(const String& key)
{
    return m_v1Sdk.GetClientParam(key);
}

