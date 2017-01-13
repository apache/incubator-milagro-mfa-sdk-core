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

#ifndef _TEST_MFA_SDK_H_
#define _TEST_MFA_SDK_H_

#include "mfa_sdk.h"

class TestContext;

class TestMfaSDK : public MfaSDK
{
public:
    TestMfaSDK(TestContext& testContext);

    Status Init(const StringMap& config);

    Status GetAccessCode(const String& authzUrl, OUT String& accessCode);

    Status GetSessionDetails(const String& accessCode, OUT SessionDetails& sessionDetails);
    Status AbortSession(const String& accessCode);

    Status StartRegistration(INOUT UserPtr user, const String& accessCode, const String& pushToken);
    Status RestartRegistration(INOUT UserPtr user);
    Status ConfirmRegistration(INOUT UserPtr user);
    Status FinishRegistration(INOUT UserPtr user, const String& pin);

    Status StartAuthentication(INOUT UserPtr user, const String& accessCode);
    Status FinishAuthentication(INOUT UserPtr user, const String& pin, const String& accessCode);
    Status FinishAuthentication(INOUT UserPtr user, const String& pin, const String& accessCode, OUT String& authzCode);

    bool Logout(IN UserPtr user);

private:
    TestContext& m_testContext;
    int m_accessCodeCounter;
};

#endif // _TEST_MPIN_SDK_H_