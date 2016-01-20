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
 * M-Pin SDK version 2 interface
 */

#ifndef _MPIN_SDK_V2_H_
#define _MPIN_SDK_V2_H_

#include "mpin_sdk.h"

class MPinSDKv2
{
public:
    typedef util::String String;
    typedef util::StringMap StringMap;
    typedef MPinSDK::User User;
    typedef MPinSDK::UserPtr UserPtr;
    typedef MPinSDK::CryptoType CryptoType;
    typedef MPinSDK::IHttpRequest IHttpRequest;
    typedef MPinSDK::IStorage IStorage;
    typedef MPinSDK::IPinPad IPinPad;
    typedef MPinSDK::Status Status;
    typedef MPinSDK::OTP OTP;

    class IContext
    {
    public:
        virtual ~IContext() {}
        virtual IHttpRequest * CreateHttpRequest() const = 0;
        virtual void ReleaseHttpRequest(IN IHttpRequest *request) const = 0;
        virtual IStorage * GetStorage(IStorage::Type type) const = 0;
        virtual CryptoType GetMPinCryptoType() const = 0;
    };

    MPinSDKv2();
    ~MPinSDKv2();

    Status Init(const StringMap& config, IN IContext* ctx);
    void Destroy();
    void ClearUsers();

    Status TestBackend(const String& server, const String& rpsPrefix = MPinSDK::DEFAULT_RPS_PREFIX) const;
    Status SetBackend(const String& server, const String& rpsPrefix = MPinSDK::DEFAULT_RPS_PREFIX);
    UserPtr MakeNewUser(const String& id, const String& deviceName = "") const;

    Status StartRegistration(INOUT UserPtr user, const String& userData = "");
    Status RestartRegistration(INOUT UserPtr user, const String& userData = "");
    Status VerifyUser(INOUT UserPtr user, const String& mpinId, const String& activationKey);
    Status ConfirmRegistration(INOUT UserPtr user, const String& pushMessageIdentifier = "");
    Status FinishRegistration(INOUT UserPtr user, const String& pin);

    Status StartAuthentication(INOUT UserPtr user);
    Status CheckAccessNumber(const String& accessNumber);
    Status FinishAuthentication(INOUT UserPtr user, const String& pin);
    Status FinishAuthentication(INOUT UserPtr user, const String& pin, OUT String& authResultData);
    Status FinishAuthenticationOTP(INOUT UserPtr user, const String& pin, OUT OTP& otp);
    Status FinishAuthenticationAN(INOUT UserPtr user, const String& pin, const String& accessNumber);

    void DeleteUser(INOUT UserPtr user);
    void ListUsers(OUT std::vector<UserPtr>& users);
    const char * GetVersion();
    bool CanLogout(IN UserPtr user);
    bool Logout(IN UserPtr user);
	String GetClientParam(const String& key);

private:
    Status FinishAuthenticationImpl(INOUT UserPtr user, const String& pin, const String& accessNumber, OUT String *otp, OUT util::JsonObject& authResultData);

    typedef MPinSDK::TimePermitCache TimePermitCache;
    typedef MPinSDK::HttpResponse HttpResponse;
    typedef MPinSDK::State State;
    typedef MPinSDK::LogoutData LogoutData;

    class Context : public MPinSDK::IContext
    {
    public:
        Context();
        ~Context();
        void Init(MPinSDKv2::IContext *appContext);
        void SetPin(const String& pin);
        virtual IHttpRequest * CreateHttpRequest() const;
        virtual void ReleaseHttpRequest(IN IHttpRequest *request) const;
        virtual IStorage * GetStorage(IStorage::Type type) const;
        virtual IPinPad * GetPinPad() const;
        virtual CryptoType GetMPinCryptoType() const;

    private:
        class Pinpad : public IPinPad
        {
        public:
            void SetPin(const String& pin) { m_pin = pin; }
            virtual String Show(UserPtr user, Mode mode) { return m_pin; }
        private:
            String m_pin;
        };

        MPinSDKv2::IContext *m_appContext;
        Pinpad *m_pinpad;
    };

    MPinSDK m_v1Sdk;
    Context m_context;
};

#endif // _MPIN_SDK_V2_H_
