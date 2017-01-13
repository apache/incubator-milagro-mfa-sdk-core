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

#include "test_mfa_sdk.h"
#include "test_context.h"

typedef MfaSDK::Status Status;

TestMfaSDK::TestMfaSDK(TestContext & testContext) : m_testContext(testContext), m_accessCodeCounter(0)
{
}

Status TestMfaSDK::Init(const StringMap & config)
{
    return MfaSDK::Init(config, &m_testContext);
}

Status TestMfaSDK::GetAccessCode(const String & authzUrl, OUT String & accessCode)
{
    m_testContext.SetRequestContextData(String().Format("GetAccessCode%03d", m_accessCodeCounter++));
    Status s = MfaSDK::GetAccessCode(authzUrl, accessCode);
    m_testContext.SetRequestContextData("");
    return s;
}

Status TestMfaSDK::GetSessionDetails(const String & accessCode, OUT SessionDetails & sessionDetails)
{
    m_testContext.SetRequestContextData(accessCode);
    Status s = MfaSDK::GetSessionDetails(accessCode, sessionDetails);
    m_testContext.SetRequestContextData("");
    return s;
}

Status TestMfaSDK::AbortSession(const String & accessCode)
{
    m_testContext.SetRequestContextData(accessCode);
    Status s = MfaSDK::AbortSession(accessCode);
    m_testContext.SetRequestContextData("");
    return s;
}

Status TestMfaSDK::StartRegistration(INOUT UserPtr user, const String & accessCode, const String & pushToken)
{
    m_testContext.SetRequestContextData(user->GetId() + "-" + accessCode + "-" + pushToken);
    Status s = MfaSDK::StartRegistration(user, accessCode, pushToken);
    m_testContext.SetRequestContextData("");
    return s;
}

Status TestMfaSDK::RestartRegistration(INOUT UserPtr user)
{
    m_testContext.SetRequestContextData(user->GetId());
    Status s = MfaSDK::RestartRegistration(user);
    m_testContext.SetRequestContextData("");
    return s;
}

Status TestMfaSDK::ConfirmRegistration(INOUT UserPtr user)
{
    m_testContext.SetRequestContextData(user->GetId());
    Status s = MfaSDK::ConfirmRegistration(user);
    m_testContext.SetRequestContextData("");
    return s;
}

Status TestMfaSDK::FinishRegistration(INOUT UserPtr user, const String & pin)
{
    m_testContext.SetRequestContextData(user->GetId() + "-" + pin);
    Status s = MfaSDK::FinishRegistration(user, pin);
    m_testContext.SetRequestContextData("");
    return s;
}

Status TestMfaSDK::StartAuthentication(INOUT UserPtr user, const String & accessCode)
{
    m_testContext.SetRequestContextData(user->GetId() + "-" + accessCode);
    Status s = MfaSDK::StartAuthentication(user, accessCode);
    m_testContext.SetRequestContextData("");
    return s;
}

Status TestMfaSDK::FinishAuthentication(INOUT UserPtr user, const String & pin, const String & accessCode)
{
    m_testContext.SetRequestContextData(user->GetId() + "-" + pin + "-" + accessCode);
    Status s = MfaSDK::FinishAuthentication(user, pin, accessCode);
    m_testContext.SetRequestContextData("");
    return s;
}

Status TestMfaSDK::FinishAuthentication(INOUT UserPtr user, const String & pin, const String & accessCode, OUT String & authzCode)
{
    m_testContext.SetRequestContextData(user->GetId() + "-" + pin + "-" + accessCode + "-AUTHZ");
    Status s = MfaSDK::FinishAuthentication(user, pin, accessCode);
    m_testContext.SetRequestContextData("");
    return s;
}

bool TestMfaSDK::Logout(IN UserPtr user)
{
    m_testContext.SetRequestContextData(user->GetId());
    bool res = MfaSDK::Logout(user);
    m_testContext.SetRequestContextData("");
    return res;
}
