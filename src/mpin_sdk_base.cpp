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
* M-Pin SDK Base implementation
*/

#include "mpin_sdk_base.h"
#include "mpin_crypto_non_tee.h"
#include "json/visitor.h"
#include <sstream>

typedef MPinSDKBase::Status Status;
typedef MPinSDKBase::User User;
typedef MPinSDKBase::UserPtr UserPtr;
typedef MPinSDKBase::String String;
typedef MPinSDKBase::StringMap StringMap;
typedef MPinSDKBase::IHttpRequest::Method HttpMethod;
using util::Url;

/*
* Status class
*/

Status::Status() : m_statusCode(OK)
{
}

Status::Status(Code statucCode) : m_statusCode(statucCode)
{
}

Status::Status(Code statucCode, const String& error) : m_statusCode(statucCode), m_errorMessage(error)
{
}

Status::Code Status::GetStatusCode() const
{
    return m_statusCode;
}

const String& Status::GetErrorMessage() const
{
    return m_errorMessage;
}

void Status::SetStatusCode(Code statusCode)
{
    m_statusCode = statusCode;
}

void Status::SetErrorMessage(const String& error)
{
    m_errorMessage = error;
}

bool Status::operator==(Code statusCode) const
{
    return m_statusCode == statusCode;
}

bool Status::operator!=(Code statusCode) const
{
    return m_statusCode != statusCode;
}


/*
* TimePermitCache class
*/

MPinSDKBase::TimePermitCache::TimePermitCache() : m_date(0)
{
}

const String& MPinSDKBase::TimePermitCache::GetTimePermit() const
{
    return m_timePermit;
}

int MPinSDKBase::TimePermitCache::GetDate() const
{
    return m_date;
}

void MPinSDKBase::TimePermitCache::Set(const String& timePermit, int date)
{
    m_timePermit = timePermit;
    m_date = date;
}

void MPinSDKBase::TimePermitCache::Invalidate()
{
    m_timePermit.Overwrite();
    m_date = 0;
}


/*
* User class
*/

User::User(const String& id, const String& deviceName) : m_id(id), m_deviceName(deviceName), m_state(INVALID)
{
}

String User::GetKey() const
{
    return String().Format("%s@%s", m_id.c_str(), m_backend.c_str());
}

const String& User::GetId() const
{
    return m_id;
}

const String& User::GetBackend() const
{
    return m_backend;
}

const String& User::GetCustomerId() const
{
    return m_customerId;
}

const String& User::GetAppId() const
{
    return m_appId;
}

const String& User::GetDeviceName() const
{
    return m_deviceName;
}

User::State User::GetState() const
{
    return m_state;
}

const String& User::GetMPinId() const
{
    return m_mpinId;
}

const String& User::GetMPinIdHex() const
{
    return m_mpinIdHex;
}

const String& User::GetRegOTT() const
{
    return m_regOTT;
}

const MPinSDKBase::TimePermitCache& User::GetTimePermitCache() const
{
    return m_timePermitCache;
}

void User::CacheTimePermit(const String& timePermit, int date)
{
    m_timePermitCache.Set(timePermit, date);
}

void User::SetBackend(const String& backend)
{
    m_backend = backend;
}

void User::SetStartedRegistration(const String& mpinIdHex, const String& regOTT, const String& customerId, const String& appId)
{
    m_state = STARTED_REGISTRATION;
    m_mpinIdHex = mpinIdHex;
    m_mpinId = util::HexDecode(mpinIdHex);
    m_regOTT = regOTT;
    m_customerId = customerId;
    m_appId = appId;
}


void User::SetActivated()
{
    m_state = ACTIVATED;
}

void User::SetRegistered()
{
    m_regOTT.Overwrite();
    m_state = REGISTERED;
}

void User::Invalidate()
{
    m_regOTT.Overwrite();
    m_timePermitCache.Invalidate();
    m_state = INVALID;
}

void User::Block()
{
    Invalidate();
    m_state = BLOCKED;
}

Status User::RestoreState(const String& stateString, const String& mpinIdHex, const String& regOTT, const String& backend,
    const String& customerId, const String& appId)
{
    SetBackend(backend);
    SetStartedRegistration(mpinIdHex, regOTT, customerId, appId);

    State state = StringToState(stateString);
    switch (state)
    {
    case INVALID:
        return Status(Status::STORAGE_ERROR, String().Format("Invalid user state found for user '%s': '%s'", m_id.c_str(), stateString.c_str()));
    case ACTIVATED:
        SetActivated();
    case STARTED_REGISTRATION:
        // regOTT *must* be valid
        if (regOTT.empty())
        {
            return Status(Status::STORAGE_ERROR, String().Format("Corrupted data for user '%s': state is '%s' but no regOTT was found", m_id.c_str(), stateString.c_str()));
        }
        return Status(Status::OK);
    case REGISTERED:
        SetRegistered();
        // regOTT *must* be empty
        if (!regOTT.empty())
        {
            return Status(Status::STORAGE_ERROR, String().Format("Corrupted data for user '%s': state is '%s' but regOTT is still saved", m_id.c_str(), stateString.c_str()));
        }
        return Status(Status::OK);
    case BLOCKED:
        Block();
        return Status(Status::OK);
    default:
        assert(false);
        return Status(Status::STORAGE_ERROR, String().Format("Internal error: invalid state %d returned from User::StringToState(%s)", (int)state, stateString.c_str()));
    }
}

String User::GetStateString() const
{
    return User::StateToString(m_state);
}

String User::StateToString(State state)
{
    switch (state)
    {
    case STARTED_REGISTRATION:
        return "STARTED_REGISTRATION";
    case ACTIVATED:
        return "ACTIVATED";
    case REGISTERED:
        return "REGISTERED";
    case BLOCKED:
        return "BLOCKED";
    default:
        assert(false);
        return "INVALID";
    }
}

User::State User::StringToState(const String& stateString)
{
    if (stateString == StateToString(STARTED_REGISTRATION))
    {
        return STARTED_REGISTRATION;
    }
    if (stateString == StateToString(ACTIVATED))
    {
        return ACTIVATED;
    }
    if (stateString == StateToString(REGISTERED))
    {
        return REGISTERED;
    }
    if (stateString == StateToString(BLOCKED))
    {
        return BLOCKED;
    }

    return INVALID;
}


/*
* MPinSDK::IHttpRequest static string constants
*/

const char *MPinSDKBase::IHttpRequest::CONTENT_TYPE_HEADER = "Content-Type";
const char *MPinSDKBase::IHttpRequest::ACCEPT_HEADER = "Accept";
#define JSON_CONTENT_TYPE_STRING "application/json"
const char *MPinSDKBase::IHttpRequest::JSON_CONTENT_TYPE = JSON_CONTENT_TYPE_STRING "; charset=UTF-8";
const char *MPinSDKBase::IHttpRequest::TEXT_PLAIN_CONTENT_TYPE = "text/plain";


/*
* MPinSDK::HttpResponse class
*/

MPinSDKBase::HttpResponse::HttpResponse(const String& requestUrl, const String& requestBody)
    : m_httpStatus(HTTP_OK), m_dataType(JSON), m_requestUrl(requestUrl), m_requestBody(requestBody)
{
}

int MPinSDKBase::HttpResponse::GetStatus() const
{
    return m_httpStatus;
}

MPinSDKBase::HttpResponse::DataType MPinSDKBase::HttpResponse::GetDataType() const
{
    return m_dataType;
}

MPinSDKBase::HttpResponse::DataType MPinSDKBase::HttpResponse::DetermineDataType(const String& contentTypeStr) const
{
    if (contentTypeStr.compare(0, strlen(JSON_CONTENT_TYPE_STRING), JSON_CONTENT_TYPE_STRING) == 0)
    {
        return JSON;
    }

    return RAW;
}

bool MPinSDKBase::HttpResponse::SetData(const String& rawData, const StringMap& headers, DataType expectedType)
{
    m_rawData = rawData;
    m_headers = headers;

    String contentTypeStr = headers.Get(IHttpRequest::CONTENT_TYPE_HEADER);
    m_dataType = DetermineDataType(contentTypeStr);

    String data = rawData;
    data.Trim();
    if (data.length() > 0 && !m_jsonData.Parse(data.c_str()))
    {
        SetResponseJsonParseError(data, m_jsonData.GetParseError());
        return false;
    }

    return true;
}

const util::JsonObject& MPinSDKBase::HttpResponse::GetJsonData() const
{
    return m_jsonData;
}

const String& MPinSDKBase::HttpResponse::GetRawData() const
{
    return m_rawData;
}

const StringMap& MPinSDKBase::HttpResponse::GetHeaders() const
{
    return m_headers;
}

void MPinSDKBase::HttpResponse::SetNetworkError(const String& error)
{
    m_httpStatus = NON_HTTP_ERROR;
    m_mpinStatus.SetStatusCode(Status::NETWORK_ERROR);
    m_mpinStatus.SetErrorMessage(String().Format("HTTP request to '%s' failed. Error: '%s'", m_requestUrl.c_str(), error.c_str()));
}

void MPinSDKBase::HttpResponse::SetResponseJsonParseError(const String& jsonParseError)
{
    SetResponseJsonParseError(String(m_rawData).Trim(), jsonParseError);
}

void MPinSDKBase::HttpResponse::SetResponseJsonParseError(const String& responseJson, const String& jsonParseError)
{
    m_httpStatus = NON_HTTP_ERROR;
    m_mpinStatus.SetStatusCode(Status::RESPONSE_PARSE_ERROR);
    m_mpinStatus.SetErrorMessage(String().Format("Failed to parse '%s' response json in request to '%s' (body='%s'). Error: '%s'",
        responseJson.c_str(), m_requestUrl.c_str(), m_requestBody.c_str(), jsonParseError.c_str()));
}

void MPinSDKBase::HttpResponse::SetUnexpectedContentTypeError(DataType expectedType, const String& responseContentType, const String& responseRawData)
{
    m_httpStatus = NON_HTTP_ERROR;
    m_mpinStatus.SetStatusCode(Status::RESPONSE_PARSE_ERROR);
    assert(expectedType == JSON || expectedType == RAW);
    m_mpinStatus.SetErrorMessage(String().Format("HTTP request to '%s' (body='%s') returned unexpected content type '%s'. Expected was '%s'",
        m_requestUrl.c_str(), m_requestBody.c_str(), responseContentType.c_str(), (expectedType == JSON) ? "JSON" : "RAW"));
}

void MPinSDKBase::HttpResponse::SetHttpError(int httpStatus)
{
    m_httpStatus = httpStatus;

    m_mpinStatus.SetErrorMessage(String().Format("HTTP request to '%s' (body='%s') returned status code %d",
        m_requestUrl.c_str(), m_requestBody.c_str(), httpStatus));

    if (httpStatus >= 500)
    {
        m_mpinStatus.SetStatusCode(Status::HTTP_SERVER_ERROR);
    }
    else if (httpStatus >= 400)
    {
        if (httpStatus == HTTP_NOT_ACCEPTABLE)
        {
            m_mpinStatus.SetStatusCode(Status::BAD_USER_AGENT);
        }
        else
        {
            m_mpinStatus.SetStatusCode(Status::HTTP_REQUEST_ERROR);
        }
    }
    else if (httpStatus >= 300)
    {
        m_mpinStatus.SetStatusCode(Status::NETWORK_ERROR);
    }
    else
    {
        // TODO: What to do if server returns 2xx (but not 200) or 3xx?
        assert(false);
    }
}

void MPinSDKBase::HttpResponse::SetUntrustedDomainError(const String& error)
{
    m_httpStatus = NON_HTTP_ERROR;
    m_mpinStatus.SetStatusCode(Status::UNTRUSTED_DOMAIN_ERROR);
    m_mpinStatus.SetErrorMessage(String().Format("Request to untrusted url '%s' failed. Error: '%s'", m_requestUrl.c_str(), error.c_str()));
}

Status MPinSDKBase::HttpResponse::TranslateToMPinStatus(Context context)
{
    switch (context)
    {
    case GET_CLIENT_SETTINGS:
    case AUTHENTICATE_PASS1:
    case AUTHENTICATE_PASS2:
    case GET_SESSION_DETAILS:
    case ABORT_SESSION:
    case GET_ACCESS_CODE:
        break;
    case GET_SERVICE_DETAILS:
        if (m_httpStatus == HTTP_PRECONDITION_FAILED)
        {
            m_mpinStatus.SetStatusCode(Status::BAD_CLIENT_VERSION);
            m_mpinStatus.SetErrorMessage("Wrong client app version");
        }
        break;
    case REGISTER:
        if (m_httpStatus == HTTP_FORBIDDEN)
        {
            m_mpinStatus.SetStatusCode(Status::IDENTITY_NOT_AUTHORIZED);
            m_mpinStatus.SetErrorMessage("Identity not authorized");
        }
        break;
    case GET_CLIENT_SECRET1:
        if (m_httpStatus == HTTP_BAD_REQUEST || m_httpStatus == HTTP_UNAUTHORIZED)
        {
            m_mpinStatus.SetStatusCode(Status::IDENTITY_NOT_VERIFIED);
            m_mpinStatus.SetErrorMessage("Identity not verified");
        }
        break;
    case GET_CLIENT_SECRET2:
        if (m_httpStatus == HTTP_REQUEST_TIMEOUT)
        {
            m_mpinStatus.SetStatusCode(Status::REQUEST_EXPIRED);
            m_mpinStatus.SetErrorMessage("Request expired");
        }
        break;
    case GET_TIME_PERMIT1:
    case GET_TIME_PERMIT2:
        if (m_httpStatus == HTTP_GONE)
        {
            m_mpinStatus.SetStatusCode(Status::REVOKED);
            m_mpinStatus.SetErrorMessage("User revoked");
        }
        break;
    case AUTHENTICATE_RPA:
        if (m_httpStatus == HTTP_UNAUTHORIZED)
        {
            m_mpinStatus.SetStatusCode(Status::INCORRECT_PIN);
            m_mpinStatus.SetErrorMessage("Incorrect pin");
        }
        else if (m_httpStatus == HTTP_REQUEST_TIMEOUT)
        {
            m_mpinStatus.SetStatusCode(Status::REQUEST_EXPIRED);
            m_mpinStatus.SetErrorMessage("Request expired");
        }
        else if (m_httpStatus == HTTP_GONE)
        {
            m_mpinStatus.SetStatusCode(Status::INCORRECT_PIN);
            m_mpinStatus.SetErrorMessage("User blocked");
        }
        else if (m_httpStatus == HTTP_PRECONDITION_FAILED)
        {
            m_mpinStatus.SetStatusCode(Status::INCORRECT_ACCESS_NUMBER);
            m_mpinStatus.SetErrorMessage("Invalid access number");
        }
        else if (m_httpStatus == HTTP_FORBIDDEN)
        {
            m_mpinStatus.SetStatusCode(Status::IDENTITY_NOT_AUTHORIZED);
            m_mpinStatus.SetErrorMessage("Identity not authorized");
        }
        else if (m_httpStatus == HTTP_CONFLICT)
        {
            m_mpinStatus.SetStatusCode(Status::CLIENT_SECRET_EXPIRED);
            m_mpinStatus.SetErrorMessage("Client secret expired");
        }
        break;
    }

    return m_mpinStatus;
}


/*
* MPinSDKBase class
*/

const char *MPinSDKBase::DEFAULT_RPS_PREFIX = "rps";
const char *MPinSDKBase::CONFIG_BACKEND = "backend";
// TODO: Remove this
static const char *CONFIG_BACKEND_OLD = "RPA_server";
const char *MPinSDKBase::CONFIG_RPS_PREFIX = "rps_prefix";

MPinSDKBase::MPinSDKBase() : m_state(NOT_INITIALIZED), m_context(NULL), m_crypto(NULL)
{
}

MPinSDKBase::~MPinSDKBase()
{
    Destroy();
}

bool MPinSDKBase::IsInitilized() const
{
    return m_state != NOT_INITIALIZED;
}

bool MPinSDKBase::IsBackendSet() const
{
    return m_state == BACKEND_SET;
}

Status MPinSDKBase::CheckIfIsInitialized() const
{
    if (IsInitilized())
    {
        return Status(Status::OK);
    }

    return Status(Status::FLOW_ERROR, "MPinSDK not initialized");
}

Status MPinSDKBase::CheckIfBackendIsSet() const
{
    if (IsBackendSet())
    {
        return Status(Status::OK);
    }

    return Status(Status::FLOW_ERROR, "MPinSDK backend was not set");
}

Status MPinSDKBase::CheckUrl(const String & url) const
{
    if (m_trustedDomains.empty())
    {
        return Status::OK;
    }

    Url parsedUrl(url);
    if (parsedUrl.GetScheme() != "https")
    {
        return Status(Status::UNTRUSTED_DOMAIN_ERROR, "Request must be made over https");
    }

    const String& host = parsedUrl.GetHost();
    for (UrlVector::const_iterator domain = m_trustedDomains.begin(); domain != m_trustedDomains.end(); ++domain)
    {
        if (host.EndsWith(domain->GetHost()))
        {
            return Status::OK;
        }
    }

    return Status(Status::UNTRUSTED_DOMAIN_ERROR, "Not in trusted domains list");
}

MPinSDKBase::HttpResponse MPinSDKBase::MakeRequest(const String& url, HttpMethod method, const util::JsonObject& bodyJson, HttpResponse::DataType expectedResponseType) const
{
    String requestBody = bodyJson.ToString();
    HttpResponse response(url, requestBody);

    Status s = CheckUrl(url);
    if (s != Status::OK)
    {
        response.SetUntrustedDomainError(s.GetErrorMessage());
        return response;
    }

    IHttpRequest *r = m_context->CreateHttpRequest();

    StringMap headers = m_customHeaders;
    if (method != IHttpRequest::GET)
    {
        headers.Put(IHttpRequest::CONTENT_TYPE_HEADER, IHttpRequest::JSON_CONTENT_TYPE);
        headers.Put(IHttpRequest::ACCEPT_HEADER, IHttpRequest::TEXT_PLAIN_CONTENT_TYPE);
    }

    if (!headers.empty())
    {
        r->SetHeaders(headers);
    }

    if (method != IHttpRequest::GET)
    {
        r->SetContent(requestBody);
    }

    if (!r->Execute(method, url))
    {
        response.SetNetworkError(r->GetExecuteErrorMessage());
        m_context->ReleaseHttpRequest(r);
        return response;
    }

    int httpStatus = r->GetHttpStatusCode();
    if (httpStatus != HttpResponse::HTTP_OK)
    {
        response.SetHttpError(httpStatus);
        m_context->ReleaseHttpRequest(r);
        return response;
    }

    if (!r->GetResponseData().empty())
    {
        response.SetData(r->GetResponseData(), r->GetResponseHeaders(), expectedResponseType);
    }

    m_context->ReleaseHttpRequest(r);
    return response;
}

MPinSDKBase::HttpResponse MPinSDKBase::MakeGetRequest(const String& url, HttpResponse::DataType expectedResponseType) const
{
    return MakeRequest(url, IHttpRequest::GET, util::JsonObject(), expectedResponseType);
}

namespace
{
    class RewriteUrlVisitor : public json::Visitor
    {
    public:
        RewriteUrlVisitor(const String& rpaServer) : m_rpaServer(rpaServer) {}
        virtual ~RewriteUrlVisitor() {}

        virtual void Visit(json::Array& array) {}
        virtual void Visit(json::Object& object) {}
        virtual void Visit(json::Number& number) {}
        virtual void Visit(json::Boolean& boolean) {}
        virtual void Visit(json::Null& null) {}

        virtual void Visit(json::String& string)
        {
            String url = string.Value();
            if (url[0] == '/')
            {
                url.insert(0, m_rpaServer);
            }
            else
            {
                // Replace wss:// with https:// and ws:// with http://
                url.ReplaceAll("wss://", "https://");
                url.ReplaceAll("ws://", "http://");
            }
            string = url;
        }

    private:
        String m_rpaServer;
    };
}

Status MPinSDKBase::RewriteRelativeUrls()
{
    try
    {
        RewriteUrlVisitor visitor(m_RPAServer);
        for (json::Object::iterator i = m_clientSettings.Begin(); i != m_clientSettings.End(); ++i)
        {
            i->element.Accept(visitor);
        }
    }
    catch (json::Exception&)
    {
        return Status(Status::RESPONSE_PARSE_ERROR, String().Format("Unexpected client settings json: '%s'", m_clientSettings.ToString().c_str()));
    }

    return Status(Status::OK);
}

Status MPinSDKBase::Init(const StringMap& config, IContext* ctx)
{
    if (IsInitilized())
    {
        return Status(Status::OK);
    }

    m_context = ctx;

    if (ctx->GetMPinCryptoType() == CRYPTO_NON_TEE)
    {
        MPinCryptoNonTee *nonteeCrypto = new MPinCryptoNonTee();
        Status s = nonteeCrypto->Init(ctx->GetStorage(IStorage::SECURE));
        if (s != Status::OK)
        {
            delete nonteeCrypto;
            return s;
        }
        m_crypto = nonteeCrypto;
    }
    else
    {
        return Status(Status::FLOW_ERROR, String("CRYPTO_TEE crypto type is currently not supported"));
    }

    Status s = LoadUsersFromStorage();
    if (s != Status::OK)
    {
        return s;
    }

    m_state = INITIALIZED;

    String backend = config.Get(CONFIG_BACKEND);
    if (backend.empty())
    {
        backend = config.Get(CONFIG_BACKEND_OLD);
        if (backend.empty())
        {
            return Status(Status::OK);
        }
    }

    StringMap::const_iterator i = config.find(CONFIG_RPS_PREFIX);
    String rpsPrefix = (i != config.end()) ? i->second : DEFAULT_RPS_PREFIX;

    return SetBackend(backend, rpsPrefix);
}

void MPinSDKBase::Destroy()
{
    if (!IsInitilized())
    {
        return;
    }

    ClearUsers();

    delete m_crypto;
    m_crypto = NULL;
    m_context = NULL;

    ClearCustomHeaders();

    m_state = NOT_INITIALIZED;
}

void MPinSDKBase::AddCustomHeaders(const StringMap& customHeaders)
{
    m_customHeaders.PutAll(customHeaders);
}

void MPinSDKBase::ClearCustomHeaders()
{
    m_customHeaders.clear();
}

void MPinSDKBase::AddTrustedDomain(const String & domain)
{
    m_trustedDomains.push_back(Url(domain));
}

void MPinSDKBase::ClearTrustedDomains()
{
    m_trustedDomains.clear();
}

Status MPinSDKBase::GetClientSettings(const String& backend, const String& rpsPrefix, OUT util::JsonObject *clientSettings) const
{
    HttpResponse response = MakeGetRequest(String().Format("%s/%s/clientSettings", backend.c_str(), String(rpsPrefix).Trim("/").c_str()));
    if (response.GetStatus() != HttpResponse::HTTP_OK)
    {
        return response.TranslateToMPinStatus(HttpResponse::GET_CLIENT_SETTINGS);
    }

    if (clientSettings != NULL)
    {
        *clientSettings = response.GetJsonData();
    }


    return Status(Status::OK);
}

Status MPinSDKBase::TestBackend(const String& backend, const String& rpsPrefix) const
{
    Status s = CheckIfIsInitialized();
    if (s != Status::OK)
    {
        return s;
    }

    return GetClientSettings(String(backend).TrimRight("/"), rpsPrefix, NULL);
}

Status MPinSDKBase::SetBackend(const String& backend, const String& rpsPrefix)
{
    Status s = CheckIfIsInitialized();
    if (s != Status::OK)
    {
        return s;
    }

    m_RPAServer = backend;
    m_RPAServer.TrimRight("/");
    s = GetClientSettings(m_RPAServer, rpsPrefix, &m_clientSettings);
    if (s != Status::OK)
    {
        return s;
    }

    s = RewriteRelativeUrls();

    // Check to see if the old access number algorithm is used and if yes, disable check sum validation
    if (m_clientSettings.GetIntParam("cSum", 0) == 0)
    {
        m_clientSettings["accessNumberUseCheckSum"] = json::Boolean(false);
    }

    if (s != Status::OK)
    {
        return s;
    }

    m_state = BACKEND_SET;
    return Status(Status::OK);
}

UserPtr MPinSDKBase::MakeNewUser(const String& id, const String& deviceName) const
{
    return UserPtr(new User(id, deviceName));
}

void MPinSDKBase::DeleteUser(UserPtr user)
{
    Status s = CheckIfIsInitialized();
    if (s != Status::OK)
    {
        return;
    }

    UsersMap::iterator i = m_users.find(user->GetKey());
    if (i == m_users.end() || user != i->second)
    {
        return;
    }

    m_crypto->DeleteRegOTT(i->second->GetMPinId());
    m_crypto->DeleteToken(i->second->GetMPinId());
    i->second->Invalidate();
    m_logoutData.erase(i->second);
    m_users.erase(i);
    WriteUsersToStorage();
}

void MPinSDKBase::ClearUsers()
{
    for (UsersMap::iterator i = m_users.begin(); i != m_users.end(); ++i)
    {
        i->second->Invalidate();
    }
    m_users.clear();
    m_logoutData.clear();
}

bool MPinSDKBase::CanLogout(UserPtr user)
{
    LogoutDataMap::iterator i = m_logoutData.find(user);
    if (i == m_logoutData.end()) return false;
    if (i->second.logoutURL.empty()) return false;
    return true;
}

bool MPinSDKBase::Logout(UserPtr user)
{
    LogoutDataMap::iterator i = m_logoutData.find(user);
    if (i == m_logoutData.end()) return false;
    if (i->second.logoutURL.empty()) return false;
    util::JsonObject logoutData;
    if (!logoutData.Parse(i->second.logoutData.c_str()))
    {
        return false;
    }

    String url = String().Format("%s%s", m_RPAServer.c_str(), i->second.logoutURL.c_str());
    HttpResponse response = MakeRequest(url, IHttpRequest::POST, logoutData);

    if (response.GetStatus() != HttpResponse::HTTP_OK)
    {
        return false;
    }
    m_logoutData.erase(i);
    return true;
}

namespace
{
    class StringVisitor :public json::Visitor
    {
    public:
        virtual ~StringVisitor() {}

        virtual void Visit(json::Array& array) {}
        virtual void Visit(json::Object& object) {}
        virtual void Visit(json::Null& null) {}

        virtual void Visit(json::Number& number)
        {
            data << (int)number.Value();
        }

        virtual void Visit(json::String& string)
        {
            data << string.Value();
        }

        virtual void Visit(json::Boolean& boolean)
        {
            data << (boolean.Value() ? "true" : "false");
        }

        String GetData()
        {
            return data.str();
        }

    private:
        std::stringstream data;
    };
}

String MPinSDKBase::GetClientParam(const String& key)
{
    StringVisitor sv;
    m_clientSettings[key].Accept(sv);
    return sv.GetData();
}

String MPinSDKBase::MakeBackendKey(const String& backendServer) const
{
    String backend = backendServer;
    backend.ReplaceAll("https://", "");
    backend.ReplaceAll("http://", "");
    backend.TrimRight("/");
    return backend;
}

Status MPinSDKBase::CheckUserState(UserPtr user, User::State expectedState)
{
    UsersMap::iterator i = m_users.find(user->GetKey());
    if (expectedState == User::INVALID)
    {
        if (i != m_users.end())
        {
            return Status(Status::FLOW_ERROR, String().Format("User '%s' was already added", user->GetId().c_str()));
        }

        if (user->GetState() != User::INVALID)
        {
            return Status(Status::FLOW_ERROR, String().Format("Invalid '%s' user state: current state=%s, expected state=%s",
                user->GetId().c_str(), User::StateToString(user->GetState()).c_str(), User::StateToString(expectedState).c_str()));
        }

        return Status(Status::OK);
    }

    if (i == m_users.end())
    {
        return Status(Status::FLOW_ERROR, String().Format("User '%s' was not added or has been deleted", user->GetId().c_str()));
    }

    if (user != i->second)
    {
        return Status(Status::FLOW_ERROR, String().Format("Different user object with the '%s' id was previously added", user->GetId().c_str()));
    }

    if (user->GetBackend() != MakeBackendKey(m_RPAServer))
    {
        return Status(Status::FLOW_ERROR, String().Format("User '%s' is registered within a different backend than the current one", user->GetId().c_str()));
    }

    if (user->GetState() != expectedState)
    {
        return Status(Status::FLOW_ERROR, String().Format("Invalid '%s' user state: current state=%s, expected state=%s",
            user->GetId().c_str(), User::StateToString(user->GetState()).c_str(), User::StateToString(expectedState).c_str()));
    }

    return Status(Status::OK);
}

Status MPinSDKBase::StartRegistration(INOUT UserPtr user, const String& activateCode, const String& userData, const String& accessCode, const String& pushToken)
{
    Status s = CheckIfBackendIsSet();
    if (s != Status::OK)
    {
        return s;
    }

    s = CheckUserState(user, User::INVALID);
    if (s != Status::OK)
    {
        return s;
    }

    return RequestRegistration(user, activateCode, accessCode, pushToken, userData);
}

Status MPinSDKBase::RestartRegistration(INOUT UserPtr user, const String& userData)
{
    Status s = CheckIfBackendIsSet();
    if (s != Status::OK)
    {
        return s;
    }

    s = CheckUserState(user, User::STARTED_REGISTRATION);
    if (s != Status::OK)
    {
        return s;
    }

    return RequestRegistration(user, "", "", "", userData);
}

Status MPinSDKBase::RequestRegistration(UserPtr user, const String& activateCode, const String& accessCode, const String& pushToken, const String& userData)
{
    // Make request to RPA to add M-Pin ID
    util::JsonObject data;
    data["userId"] = json::String(user->GetId());
    data["mobile"] = json::Number(1);
    data.PutIfNotEmpty("deviceName", user->GetDeviceName());
    data.PutIfNotEmpty("userData", userData);
    data.PutIfNotEmpty("activateCode", activateCode);
    data.PutIfNotEmpty("wid", accessCode);
    data.PutIfNotEmpty("pushToken", pushToken);

    String url;
    if (user->GetState() == User::STARTED_REGISTRATION)
    {
        data["regOTT"] = json::String(user->GetRegOTT());
        url.Format("%s/%s", m_clientSettings.GetStringParam("registerURL"), user->GetMPinIdHex().c_str());
    }
    else
    {
        url = m_clientSettings.GetStringParam("registerURL");
    }

    HttpResponse response = MakeRequest(url, IHttpRequest::PUT, data);
    if (response.GetStatus() != HttpResponse::HTTP_OK)
    {
        // TODO: BAD_USER_AGENT will be returned if access number doesn't match CID. Probably a better name must be used
        return response.TranslateToMPinStatus(HttpResponse::REGISTER);
    }

    bool writeUsersToStorage = false;

    bool userIsNew = (user->GetState() == User::INVALID);
    if (userIsNew)
    {
        user->SetBackend(MakeBackendKey(m_RPAServer));
        m_users[user->GetKey()] = user;
    }

    String mpinIdHex = response.GetJsonData().GetStringParam("mpinId");
    String regOTT = response.GetJsonData().GetStringParam("regOTT");
    bool userDataChanged = (regOTT != user->GetRegOTT() || mpinIdHex != user->GetMPinIdHex());
    // TODO: Check customerId and appId with Stan
    String customerId = response.GetJsonData().GetStringParam("customerId");
    String appId = response.GetJsonData().GetStringParam("appId");

    if (userIsNew || userDataChanged)
    {
        user->SetStartedRegistration(mpinIdHex, regOTT, customerId, appId);
        writeUsersToStorage = true;
    }

    if (response.GetJsonData().GetBoolParam("active"))
    {
        user->SetActivated();
        writeUsersToStorage = true;
    }

    if (writeUsersToStorage)
    {
        Status s = WriteUsersToStorage();
        if (s != Status::OK)
        {
            return s;
        }
    }

    return Status(Status::OK);
}

Status MPinSDKBase::ConfirmRegistration(INOUT UserPtr user, const String& pushToken)
{
    Status s = CheckIfBackendIsSet();
    if (s != Status::OK)
    {
        return s;
    }

    // A user can get here either in STARTED_REGISTRATION state or in ACTIVATED state (force-activate flow)
    // In the first case, the method might fail if the user identity has not been verified yet, and the user state
    // should stay as it was - STARTED_REGISTRATION
    s = CheckUserState(user, User::STARTED_REGISTRATION);
    if (s != Status::OK)
    {
        Status sSave = s;
        s = CheckUserState(user, User::ACTIVATED);
        if (s != Status::OK)
        {
            return sSave;
        }
    }

    // Request a client secret share from the customer's D-TA and a signed request for a client secret share from CertiVox's D-TA.
    String mpinId = user->GetMPinId();
    String mpinIdHex = user->GetMPinIdHex();
    String regOTT = user->GetRegOTT();

    String url = String().Format("%s/%s?regOTT=%s", m_clientSettings.GetStringParam("signatureURL"), mpinIdHex.c_str(), regOTT.c_str());
    if (!pushToken.empty())
    {
        url += "&pmiToken=" + pushToken;
    }

    HttpResponse response = MakeGetRequest(url);
    if (response.GetStatus() != HttpResponse::HTTP_OK)
    {
        return response.TranslateToMPinStatus(HttpResponse::GET_CLIENT_SECRET1);
    }

    user->m_clientSecret1 = util::HexDecode(response.GetJsonData().GetStringParam("clientSecretShare"));

    // Request the client secret share from CertiVox's D-TA.
    String cs2Params = response.GetJsonData().GetStringParam("params");
    url.Format("%sclientSecret?%s", m_clientSettings.GetStringParam("certivoxURL"), cs2Params.c_str());
    response = MakeGetRequest(url);
    if (response.GetStatus() != HttpResponse::HTTP_OK)
    {
        return response.TranslateToMPinStatus(HttpResponse::GET_CLIENT_SECRET2);
    }

    user->m_clientSecret2 = util::HexDecode(response.GetJsonData().GetStringParam("clientSecret"));

    return Status::OK;
}

Status MPinSDKBase::FinishRegistration(INOUT UserPtr user, const String& pin)
{
    Status s = CheckIfBackendIsSet();
    if (s != Status::OK)
    {
        return s;
    }

    // A user can get here either in STARTED_REGISTRATION state or in ACTIVATED state (force-activate flow)
    // In the first case, the method might fail if the user identity has not been verified yet, and the user state
    // should stay as it was - STARTED_REGISTRATION
    s = CheckUserState(user, User::STARTED_REGISTRATION);
    if (s != Status::OK)
    {
        Status sSave = s;
        s = CheckUserState(user, User::ACTIVATED);
        if (s != Status::OK)
        {
            return sSave;
        }
    }

    // In addition, client secret shares must be retrieved
    if (user->m_clientSecret1.empty() || user->m_clientSecret2.empty())
    {
        return Status(Status::FLOW_ERROR, String().Format("Cannot finish user '%s' registration: User identity not verified", user->GetId().c_str()));
    }

    s = m_crypto->OpenSession();
    if (s != Status::OK)
    {
        return s;
    }

    std::vector<String> clientSecretShares;
    clientSecretShares.push_back(user->m_clientSecret1);
    clientSecretShares.push_back(user->m_clientSecret2);

    s = m_crypto->Register(user, pin, clientSecretShares);
    if (s != Status::OK)
    {
        m_crypto->CloseSession();
        return s;
    }

    m_crypto->CloseSession();

    user->SetRegistered();
    s = WriteUsersToStorage();
    if (s != Status::OK)
    {
        return s;
    }

    return Status::OK;
}

Status MPinSDKBase::StartAuthentication(INOUT UserPtr user, const String& accessCode)
{
    Status s = CheckIfBackendIsSet();
    if (s != Status::OK)
    {
        return s;
    }

    // Check if the user is already registered
    s = CheckUserState(user, User::REGISTERED);
    if (s != Status::OK)
    {
        return s;
    }

    String codeStatusURL = m_clientSettings.GetStringParam("codeStatusURL");
    if (!codeStatusURL.empty() && !accessCode.empty())
    {
        util::JsonObject data;
        data["status"] = json::String("user");
        data["wid"] = json::String(accessCode);
        data["userId"] = json::String(user->GetId());
        MakeRequest(codeStatusURL, IHttpRequest::POST, data);
    }

    bool useTimePermits = m_clientSettings.GetBoolParam("usePermits", true);
    if (!useTimePermits)
    {
        return Status::OK;
    }

    // Request a time permit share from the customer's D-TA and a signed request for a time permit share from CertiVox's D-TA.
    String mpinIdHex = user->GetMPinIdHex();
    String url = String().Format("%s/%s", m_clientSettings.GetStringParam("timePermitsURL"), mpinIdHex.c_str());
    HttpResponse response = MakeGetRequest(url);
    if (response.GetStatus() != HttpResponse::HTTP_OK)
    {
        return response.TranslateToMPinStatus(HttpResponse::GET_TIME_PERMIT1);
    }

    user->m_timePermitShare1 = util::HexDecode(response.GetJsonData().GetStringParam("timePermit"));

    // Request time permit share from CertiVox's D-TA (Searches first in user cache, than in S3 cache)
    s = GetCertivoxTimePermitShare(user, response.GetJsonData(), user->m_timePermitShare2);
    if (s != Status::OK)
    {
        return s;
    }

    return Status::OK;
}

Status MPinSDKBase::FinishAuthenticationImpl(INOUT UserPtr user, const String& pin, const String& accessCode, OUT String *otp, OUT util::JsonObject& authResultData)
{
    Status s = CheckIfBackendIsSet();
    if (s != Status::OK)
    {
        return s;
    }

    // Check if the user is already registered
    s = CheckUserState(user, User::REGISTERED);
    if (s != Status::OK)
    {
        return s;
    }

    bool useTimePermits = m_clientSettings.GetBoolParam("usePermits", true);

    // Check if time permit was obtained from StartAuthentication
    if (useTimePermits && (user->m_timePermitShare1.empty() || user->m_timePermitShare2.empty()))
    {
        return Status(Status::FLOW_ERROR, String().Format("Cannot finish user '%s' authentication: Invalid time permit", user->GetId().c_str()));
    }

    String mpinIdHex = user->GetMPinIdHex();

    s = m_crypto->OpenSession();
    if (s != Status::OK)
    {
        return s;
    }

    std::vector<String> timePermitShares;
    int date = 0;
    if (useTimePermits)
    {
        timePermitShares.push_back(user->m_timePermitShare1);
        timePermitShares.push_back(user->m_timePermitShare2);
        date = user->GetTimePermitCache().GetDate();
    }

    // Authentication pass 1
    String u, ut;
    s = m_crypto->AuthenticatePass1(user, pin, date, timePermitShares, u, ut);
    if (s != Status::OK)
    {
        m_crypto->CloseSession();
        return s;
    }

    util::JsonObject requestData;
    requestData["pass"] = json::Number(1);
    requestData["mpin_id"] = json::String(mpinIdHex);
    requestData["UT"] = json::String(util::HexEncode(ut));
    requestData["U"] = json::String(util::HexEncode(u));

    String mpinAuthServerURL = m_clientSettings.GetStringParam("mpinAuthServerURL");
    String url = String().Format("%s/pass1", mpinAuthServerURL.c_str());
    HttpResponse response = MakeRequest(url, IHttpRequest::POST, requestData);
    if (response.GetStatus() != HttpResponse::HTTP_OK)
    {
        m_crypto->CloseSession();
        return response.TranslateToMPinStatus(HttpResponse::AUTHENTICATE_PASS1);
    }

    String y = util::HexDecode(response.GetJsonData().GetStringParam("y"));

    // Authentication pass 2
    String v;
    m_crypto->AuthenticatePass2(user, y, v);
    if (s != Status::OK)
    {
        m_crypto->CloseSession();
        return s;
    }

    requestData.Clear();
    requestData["pass"] = json::Number(2);
    requestData["OTP"] = json::Boolean(otp != NULL ? true : false);
    requestData["WID"] = json::String(accessCode.empty() ? "0" : accessCode);
    requestData["V"] = json::String(util::HexEncode(v));
    requestData["mpin_id"] = json::String(mpinIdHex);

    url.Format("%s/pass2", mpinAuthServerURL.c_str());
    response = MakeRequest(url, IHttpRequest::POST, requestData);
    if (response.GetStatus() != HttpResponse::HTTP_OK)
    {
        m_crypto->CloseSession();
        return response.TranslateToMPinStatus(HttpResponse::AUTHENTICATE_PASS2);
    }

    // Save OTP data to be used if otp was requested
    if (otp != NULL)
    {
        *otp = response.GetJsonData().GetStringParam("OTP");
    }

    // Send response data from M-Pin authentication server to RPA
    url = m_clientSettings.GetStringParam(accessCode.empty() ? "authenticateURL" : "mobileAuthenticateURL");
    requestData.Clear();
    requestData["mpinResponse"] = response.GetJsonData();
    response = MakeRequest(url, IHttpRequest::POST, requestData);
    if (response.GetStatus() != HttpResponse::HTTP_OK)
    {
        m_crypto->CloseSession();
        s = response.TranslateToMPinStatus(HttpResponse::AUTHENTICATE_RPA);

        if (response.GetStatus() == HttpResponse::HTTP_GONE)
        {
            user->Block();
            m_crypto->DeleteToken(user->GetMPinId());
            WriteUsersToStorage();
        }

        return s;
    }

    // You are now logged in with M-Pin!
    m_crypto->CloseSession();

    authResultData = response.GetJsonData();

    return Status::OK;
}

Status MPinSDKBase::GetCertivoxTimePermitShare(INOUT UserPtr user, const util::JsonObject& cutomerTimePermitData, OUT String& resultTimePermit)
{
    // First check if we have cached time permit in user and if it is still valid (for today)
    int date = cutomerTimePermitData.GetIntParam("date");
    const TimePermitCache& userCache = user->GetTimePermitCache();
    const String& cachedTimePermit = userCache.GetTimePermit();
    if (!cachedTimePermit.empty() && userCache.GetDate() == date)
    {
        resultTimePermit = cachedTimePermit;
        return Status(Status::OK);
    }

    // No or too old cached time permit - try get time permit from S3
    String s3Url = m_clientSettings.GetStringParam("timePermitsStorageURL");
    String appId = m_clientSettings.GetStringParam("appID");
    String storageId = cutomerTimePermitData.GetStringParam("storageId");

    // Make GET request to s3Url/app_id/date/storageId
    String url = String().Format("%s/%s/%d/%s", s3Url.c_str(), appId.c_str(), date, storageId.c_str());
    HttpResponse response = MakeGetRequest(url, HttpResponse::RAW);
    if (response.GetStatus() == HttpResponse::HTTP_OK)
    {
        // OK - add time permit to user cache
        resultTimePermit = util::HexDecode(response.GetRawData());
        user->CacheTimePermit(resultTimePermit, date);
        WriteUsersToStorage();
        return Status(Status::OK);
    }

    // No cached time permit in S3 or something other went wrong
    // Finally request time permit share from CertiVox's D-TA
    String signature = cutomerTimePermitData.GetStringParam("signature");
    String t2Params = String().Format("hash_mpin_id=%s&app_id=%s&mobile=1&signature=%s",
        storageId.c_str(), appId.c_str(), signature.c_str());
    url.Format("%stimePermit?%s", m_clientSettings.GetStringParam("certivoxURL"), t2Params.c_str());
    response = MakeGetRequest(url);
    if (response.GetStatus() != HttpResponse::HTTP_OK)
    {
        return response.TranslateToMPinStatus(HttpResponse::GET_TIME_PERMIT2);
    }

    resultTimePermit = util::HexDecode(response.GetJsonData().GetStringParam("timePermit"));
    // OK - add time permit to user cache
    user->CacheTimePermit(resultTimePermit, date);
    WriteUsersToStorage();

    return Status(Status::OK);
}

Status MPinSDKBase::FinishAuthenticationAC(INOUT UserPtr user, const String& pin, const String& accessCode)
{
    util::JsonObject authResult;

    Status s = FinishAuthenticationImpl(user, pin, accessCode, NULL, authResult);

    LogoutData logoutData;
    if (logoutData.ExtractFrom(authResult))
    {
        m_logoutData.insert(std::make_pair(user, logoutData));
    }

    return s;
}

void MPinSDKBase::OTP::ExtractFrom(const String& otpData, const util::JsonObject& json)
{
    if (otpData.empty())
    {
        status = Status(Status::RESPONSE_PARSE_ERROR, "OTP not issued");
        return;
    }

    otp = otpData;

    ttlSeconds = json.GetIntParam("ttlSeconds");
    int64_t tmp = json.GetInt64Param("expireTime");
    expireTime = (long)(tmp / 1000);
    tmp = json.GetInt64Param("nowTime");
    nowTime = (long)(tmp / 1000);

    if (expireTime == 0 || ttlSeconds == 0 || nowTime == 0)
    {
        status = Status(Status::RESPONSE_PARSE_ERROR, "OTP data is malformed");
    }
}

bool MPinSDKBase::LogoutData::ExtractFrom(const util::JsonObject& json)
{
    logoutURL = json.GetStringParam("logoutURL");
    json::Object::const_iterator i = json.Find("logoutData");

    if (i == json.End())
    {
        return false;
    }

    try
    {
        logoutData = util::JsonObject(i->element).ToString();
    }
    catch (json::Exception&)
    {
        logoutData = "";
    }

    return true;
}

namespace
{
    bool AllUsersFilter(const UserPtr& user)
    {
        return true;
    }
}

Status MPinSDKBase::ListAllUsers(OUT std::vector<UserPtr>& users) const
{
    return FilterUsers(users, AllUsersFilter);
}

Status MPinSDKBase::WriteUsersToStorage() const
{
    try
    {
        json::Object rootObject;
        for (UsersMap::const_iterator i = m_users.begin(); i != m_users.end(); ++i)
        {
            UserPtr user = i->second;
            util::JsonObject userObject;

            userObject["id"] = json::String(user->GetId());
            userObject["backend"] = json::String(user->GetBackend());
            userObject.PutIfNotEmpty("customerId", user->GetCustomerId());
            userObject.PutIfNotEmpty("appId", user->GetAppId());
            userObject.PutIfNotEmpty("deviceName", user->GetDeviceName());
            userObject["state"] = json::String(user->GetStateString());

            json::Object timePermitCacheObject;
            timePermitCacheObject["date"] = json::Number(user->GetTimePermitCache().GetDate());
            timePermitCacheObject["timePermit"] = json::String(util::HexEncode(user->GetTimePermitCache().GetTimePermit()));
            userObject["timePermitCache"] = timePermitCacheObject;

            rootObject[user->GetMPinIdHex()] = userObject;

            Status s;
            switch (user->GetState())
            {
            case User::STARTED_REGISTRATION:
            case User::ACTIVATED:
                s = m_crypto->SaveRegOTT(user->GetMPinId(), user->GetRegOTT());
                break;
            case User::REGISTERED:
                s = m_crypto->DeleteRegOTT(user->GetMPinId());
                break;
            default:
                break;
            }
            if (s != Status::OK)
            {
                return s;
            }
        }

        std::stringstream strOut;
        json::Writer::Write(rootObject, strOut);
        m_context->GetStorage(IStorage::NONSECURE)->SetData(strOut.str());
    }
    catch (const json::Exception& e)
    {
        return Status(Status::STORAGE_ERROR, e.what());
    }

    return Status::OK;
}

Status MPinSDKBase::LoadUsersFromStorage()
{
    ClearUsers();

    String data;
    m_context->GetStorage(IStorage::NONSECURE)->GetData(data);
    data.Trim();
    if (data.empty())
    {
        return Status(Status::OK);
    }

    try
    {
        json::Object rootObject;
        std::istringstream str(data);
        json::Reader::Read(rootObject, str);

        for (json::Object::const_iterator usersIter = rootObject.Begin(); usersIter != rootObject.End(); ++usersIter)
        {
            const String& mpinIdHex = usersIter->name;
            String mpinId = util::HexDecode(mpinIdHex);

            const json::Object& userObject = (const json::Object&) usersIter->element;
            std::string id = ((const json::String&) userObject["id"]).Value();
            std::string backend = ((const json::String&) userObject["backend"]).Value();
            std::string customerId = util::GetOptionalStringParam(userObject, "customerId");
            std::string appId = util::GetOptionalStringParam(userObject, "appId");
            std::string deviceName = util::GetOptionalStringParam(userObject, "deviceName");
            std::string state = ((const json::String&) userObject["state"]).Value();

            String regOTT;
            Status s = m_crypto->LoadRegOTT(mpinId, regOTT);
            if (s != Status::OK)
            {
                return s;
            }

            UserPtr user = MakeNewUser(id, deviceName);
            s = user->RestoreState(state, mpinIdHex, regOTT, backend, customerId, appId);
            if (s != Status::OK)
            {
                return s;
            }

            const json::Object& timePermitCacheObject = (const json::Object&) userObject["timePermitCache"];
            int date = (int)((const json::Number&) timePermitCacheObject["date"]).Value();
            const String& timePermit = util::HexDecode(((const json::String&) timePermitCacheObject["timePermit"]).Value());

            user->CacheTimePermit(timePermit, date);

            m_users[user->GetKey()] = user;
        }
    }
    catch (const json::Exception& e)
    {
        return Status(Status::STORAGE_ERROR, e.what());
    }

    return Status::OK;
}
