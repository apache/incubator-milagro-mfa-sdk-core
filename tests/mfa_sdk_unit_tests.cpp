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

#include "common/test_mfa_sdk.h"
#include "contexts/auto_context.h"
#include "common/url_parsing_test.h"
#include "CvLogger.h"

#define BOOST_TEST_MODULE Simple testcases
#include "boost/test/included/unit_test.hpp"

typedef MfaSDK::User User;
typedef MfaSDK::UserPtr UserPtr;
typedef MfaSDK::Status Status;
typedef MfaSDK::String String;
using namespace boost::unit_test;

static char RECORDED_DATA_JSON[] = {
#include "mfa_sdk_unit_tests_data.inc"
};

namespace
{
    class MemBuf : public std::streambuf
    {
    public:
        MemBuf(char *buf, size_t len)
        {
            this->setg(buf, buf, buf + len);
        }
    };

    const char * GetRecordedDataFileName()
    {
        int argc = framework::master_test_suite().argc;
        if (argc > 1)
        {
            char **argv = framework::master_test_suite().argv;
            return argv[1];
        }
        return "mfa_sdk_unit_tests_data.json";
    }

    String GetCurrentTestName()
    {
        return framework::current_test_case().p_name.get();
    }

    class TestNameData : public TestContext::AutoContextData
    {
    public:
        virtual String Get() const
        {
            return GetCurrentTestName();
        }
    };

    void PrintTestStart()
    {
        BOOST_MESSAGE(String().Format("Starting test %s...", GetCurrentTestName().c_str()));
    }

    void PrintTestEnd()
    {
        BOOST_MESSAGE(String().Format("    test %s finished", GetCurrentTestName().c_str()));
    }

    TestNameData testNameData;
    AutoContext context(testNameData);
    TestMfaSDK sdk(context);
    MfaSDK::StringMap config;
    const char *backend = "https://api.dev.miracl.net";
    const char *authzUrl = "https://api.dev.miracl.net/authorize?client_id=nfoztuxb5dpn4&"
        "redirect_uri=https%3A%2F%2Fmcl.demo.dev.miracl.net%2Foidc&response_type=code&scope=openid+email+profile&"
        "state=ac5caf93cc4118d27bbe23ec4568b0dd&back_url=https%3A%2F%2Fmcl.demo.dev.miracl.net%2F&lang=en";
    const char *serviceDetailsUrl = "https://mcl.dev.mpin.io";
}

namespace std
{
    std::ostream& operator<<(std::ostream& ostr, const Status& s)
    {
        ostr << s.GetStatusCodeString();
        return ostr;
    }

    std::ostream& operator<<(std::ostream& ostr, User::State s)
    {
        ostr << User::StateToString(s);
        return ostr;
    }
}

BOOST_AUTO_TEST_CASE(NoInit)
{
    unit_test_log.set_threshold_level(log_messages);

    PrintTestStart();

    CvShared::InitLogger("cvlog.txt", CvShared::enLogLevel_None);

    //context.EnterRequestRecorderMode(GetRecordedDataFileName());
    MemBuf buf(RECORDED_DATA_JSON, sizeof(RECORDED_DATA_JSON));
    std::istream recordedDataInputStream(&buf);
    context.EnterRequestPlayerMode(recordedDataInputStream);

    MfaSDK::ServiceDetails serviceDetails;
    Status s = sdk.GetServiceDetails("12345", serviceDetails);
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);

    MfaSDK::SessionDetails sessionDetails;
    s = sdk.GetSessionDetails("12345", sessionDetails);
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);
    s = sdk.AbortSession("12345");
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);

    String accessCode;
    s = sdk.GetAccessCode("12345", accessCode);

    s = sdk.TestBackend("12354");
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);

    s = sdk.SetBackend("12354");
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);

    UserPtr user = sdk.MakeNewUser("testUser");

    s = sdk.StartRegistration(user, "", "");
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);
    BOOST_CHECK_EQUAL(user->GetState(), User::INVALID);

    s = sdk.RestartRegistration(user);
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);
    BOOST_CHECK_EQUAL(user->GetState(), User::INVALID);

    s = sdk.ConfirmRegistration(user);
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);
    BOOST_CHECK_EQUAL(user->GetState(), User::INVALID);

    s = sdk.FinishRegistration(user, "");
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);
    BOOST_CHECK_EQUAL(user->GetState(), User::INVALID);

    s = sdk.StartAuthentication(user, "");
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);
    BOOST_CHECK_EQUAL(user->GetState(), User::INVALID);

    s = sdk.FinishAuthentication(user, "", "");
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);
    BOOST_CHECK_EQUAL(user->GetState(), User::INVALID);

    String authzCode;
    s = sdk.FinishAuthentication(user, "", "", authzCode);
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);
    BOOST_CHECK_EQUAL(user->GetState(), User::INVALID);

    s = sdk.StartAuthenticationOTP(user);
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);
    BOOST_CHECK_EQUAL(user->GetState(), User::INVALID);

    MfaSDK::OTP otp;
    s = sdk.FinishAuthenticationOTP(user, "", otp);
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);
    BOOST_CHECK_EQUAL(user->GetState(), User::INVALID);

    PrintTestEnd();
}

BOOST_AUTO_TEST_CASE(InitNoCID)
{
    PrintTestStart();

    config.Put(MfaSDK::CONFIG_BACKEND, backend);

    Status s = sdk.Init(config);
    BOOST_CHECK_EQUAL(s, Status::BAD_USER_AGENT);

    PrintTestEnd();
}

BOOST_AUTO_TEST_CASE(Init)
{
    PrintTestStart();

    sdk.SetCID("mcl");
    Status s = sdk.Init(config);
    BOOST_CHECK_EQUAL(s, Status::OK);

    PrintTestEnd();
}

BOOST_AUTO_TEST_CASE(ServiceAndSessionDetails)
{
    PrintTestStart();

    BOOST_REQUIRE(sdk.IsInitilized());

    MfaSDK::ServiceDetails serviceDetails;
    Status s = sdk.GetServiceDetails(serviceDetailsUrl, serviceDetails);
    BOOST_CHECK_EQUAL(s, Status::OK);

    String accessCode;
    s = sdk.GetAccessCode(authzUrl, accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    MfaSDK::SessionDetails sessionDetails;
    s = sdk.GetSessionDetails(accessCode, sessionDetails);
    BOOST_CHECK_EQUAL(s, Status::OK);

    s = sdk.AbortSession(accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    PrintTestEnd();
}

BOOST_AUTO_TEST_CASE(TestBackend)
{
    PrintTestStart();

    BOOST_REQUIRE(sdk.IsInitilized());

    Status s = sdk.TestBackend("https://m-pindemo.certivox.org");
    BOOST_CHECK_EQUAL(s, Status::OK);

    s = sdk.TestBackend("https://blabla.certivox.org");
    BOOST_CHECK_NE(s, Status::OK);

    PrintTestEnd();
}

BOOST_AUTO_TEST_CASE(SetBackend)
{
    PrintTestStart();

    BOOST_REQUIRE(sdk.IsInitilized());

    Status s = sdk.SetBackend("https://blabla.certivox.org");
    BOOST_CHECK_NE(s, Status::OK);

    s = sdk.SetBackend(backend);
    BOOST_CHECK_EQUAL(s, Status::OK);

    PrintTestEnd();
}

BOOST_AUTO_TEST_CASE(Users)
{
    PrintTestStart();

    BOOST_REQUIRE(sdk.IsBackendSet());

    UserPtr user = sdk.MakeNewUser("testUser1@example.com");
    BOOST_CHECK_EQUAL(user->GetState(), User::INVALID);

    Status s = sdk.StartRegistration(user, "", "");
    BOOST_CHECK_EQUAL(s, Status::HTTP_REQUEST_ERROR); // Invalid accessCode
    BOOST_CHECK_EQUAL(user->GetState(), User::INVALID);

    String accessCode;
    s = sdk.GetAccessCode(authzUrl, accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    s = sdk.StartRegistration(user, accessCode, "");
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::STARTED_REGISTRATION);
    MfaSDK::Expiration rex = user->GetRegistrationExpiration();
    BOOST_CHECK(rex.expireTimeSeconds > 0 && rex.nowTimeSeconds > 0 && rex.expireTimeSeconds > rex.nowTimeSeconds);

    BOOST_CHECK(!sdk.CanLogout(user));
    BOOST_CHECK(!sdk.Logout(user));

    std::vector<UserPtr> users;
    sdk.ListUsers(users);
    BOOST_CHECK_EQUAL(users.size(), 1);

    s = sdk.FinishRegistration(user, "");
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);
    s = sdk.StartAuthentication(user, accessCode);
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);
    s = sdk.FinishAuthentication(user, "", accessCode);
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);

    BOOST_CHECK_EQUAL(user->GetState(), User::STARTED_REGISTRATION);

    sdk.DeleteUser(user);
    BOOST_CHECK_EQUAL(user->GetState(), User::INVALID);

    sdk.ListUsers(users);
    BOOST_CHECK(users.empty());

    s = sdk.RestartRegistration(user);
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);
    BOOST_CHECK_EQUAL(user->GetState(), User::INVALID);

    int count = 10;
    for (int i = 0; i < count; ++i)
    {
        user = sdk.MakeNewUser(String().Format("testUser%03d@example.com", i));
        s = sdk.StartRegistration(user, accessCode, "");
        BOOST_CHECK_EQUAL(s, Status::OK);
        BOOST_CHECK_EQUAL(user->GetState(), User::STARTED_REGISTRATION);
    }

    sdk.ListUsers(users);
    BOOST_CHECK_EQUAL(users.size(), count);

    for (std::vector<UserPtr>::iterator i = users.begin(); i != users.end(); ++i)
    {
        sdk.DeleteUser(*i);
    }

    sdk.ListUsers(users);
    BOOST_CHECK(users.empty());

    PrintTestEnd();
}

BOOST_AUTO_TEST_CASE(Registration)
{
    PrintTestStart();

    BOOST_REQUIRE(sdk.IsBackendSet());

    String accessCode;
    Status s = sdk.GetAccessCode(authzUrl, accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    // Normal registration flow (with invalid RestartRegistration at the end)
    UserPtr user = sdk.MakeNewUser("testUser1@example.com");

    s = sdk.StartRegistration(user, accessCode, "");
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::STARTED_REGISTRATION);

    s = sdk.ConfirmRegistration(user);
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::STARTED_REGISTRATION);

    s = sdk.FinishRegistration(user, "1234");
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::REGISTERED);

    s = sdk.RestartRegistration(user);
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);
    BOOST_CHECK_EQUAL(user->GetState(), User::REGISTERED);

    // Normal registration flow with successfull RestartRegistration
    user = sdk.MakeNewUser("testUser2@example.com");

    s = sdk.StartRegistration(user, accessCode, "");
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::STARTED_REGISTRATION);

    s = sdk.RestartRegistration(user);
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::STARTED_REGISTRATION);

    s = sdk.ConfirmRegistration(user);
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::STARTED_REGISTRATION);

    s = sdk.FinishRegistration(user, "1234");
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::REGISTERED);

    // Duplicating user id register attempt
    user = sdk.MakeNewUser("testUser1@example.com");

    s = sdk.StartRegistration(user, accessCode, "test");
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);
    BOOST_CHECK_EQUAL(user->GetState(), User::INVALID);

    // Invalid user id register attempt
    user = sdk.MakeNewUser("");

    s = sdk.StartRegistration(user, accessCode, "");
    BOOST_CHECK_EQUAL(s, Status::HTTP_REQUEST_ERROR);
    BOOST_CHECK_EQUAL(user->GetState(), User::INVALID);

    // Registration flow with missing ConfirmRegistration
    user = sdk.MakeNewUser("testUser3@example.com");

    s = sdk.StartRegistration(user, accessCode, "");
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::STARTED_REGISTRATION);

    s = sdk.FinishRegistration(user, "");
    BOOST_CHECK_EQUAL(s, Status::FLOW_ERROR);
    BOOST_CHECK_EQUAL(user->GetState(), User::STARTED_REGISTRATION);

    // Registration with PIN input cancellation
    user = sdk.MakeNewUser("testUser4@example.com");

    s = sdk.StartRegistration(user, accessCode, "");
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::STARTED_REGISTRATION);

    s = sdk.ConfirmRegistration(user);
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::STARTED_REGISTRATION);

    s = sdk.FinishRegistration(user, "");
    BOOST_CHECK_EQUAL(s, Status::PIN_INPUT_CANCELED);
    BOOST_CHECK_EQUAL(user->GetState(), User::STARTED_REGISTRATION);

    PrintTestEnd();
}

namespace
{
    UserPtr FindUser(MfaSDK& sdk, const std::string& id, User::State expectedState)
    {
        std::vector<UserPtr> users;
        sdk.ListUsers(users);
        UserPtr user;
        for (std::vector<UserPtr>::iterator i = users.begin(); i != users.end(); ++i)
        {
            if ((*i)->GetId() == id)
            {
                user = *i;
            }
        }

        BOOST_REQUIRE_MESSAGE(user.get(), "Failed to find '" << id << "' user");
        BOOST_REQUIRE_EQUAL(user->GetState(), expectedState);

        return user;
    }
}

BOOST_AUTO_TEST_CASE(Authentication)
{
    PrintTestStart();

    BOOST_REQUIRE(sdk.IsBackendSet());

    UserPtr user = FindUser(sdk, "testUser1@example.com", User::REGISTERED);

    // Normal authentication
    String accessCode;
    Status s = sdk.GetAccessCode(authzUrl, accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    s = sdk.StartAuthentication(user, accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::REGISTERED);

    s = sdk.FinishAuthentication(user, "1234", accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::REGISTERED);

    // authz authentication
    s = sdk.GetAccessCode(authzUrl, accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    s = sdk.StartAuthentication(user, accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    String authzCode;
    s = sdk.FinishAuthentication(user, "1234", accessCode, authzCode);
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::REGISTERED);

    // OTP authentication
    s = sdk.StartAuthenticationOTP(user);
    BOOST_CHECK_EQUAL(s, Status::OK);

    MfaSDK::OTP otp;
    s = sdk.FinishAuthenticationOTP(user, "1234", otp);
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::REGISTERED);
    BOOST_CHECK_EQUAL(otp.status, Status::OK);

    // One incorrect pin, followed by correct pin authentication
    s = sdk.GetAccessCode(authzUrl, accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    s = sdk.StartAuthentication(user, accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    s = sdk.FinishAuthentication(user, "1111", accessCode);
    BOOST_CHECK_EQUAL(s, Status::INCORRECT_PIN);
    BOOST_CHECK_EQUAL(user->GetState(), User::REGISTERED);

    s = sdk.FinishAuthentication(user, "1234", accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    // Incorrect access code
    accessCode = "1234";

    s = sdk.StartAuthentication(user, accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK); // TODO: Is passing OK with wrong accessCode correct?

    s = sdk.FinishAuthentication(user, "1234", accessCode);
    BOOST_CHECK_EQUAL(s, Status::INCORRECT_ACCESS_NUMBER);

    // User blocking with 3 times incorrect pin
    s = sdk.GetAccessCode(authzUrl, accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    s = sdk.StartAuthentication(user, accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    s = sdk.FinishAuthentication(user, "1111", accessCode);
    BOOST_CHECK_EQUAL(s, Status::INCORRECT_PIN);
    BOOST_CHECK_EQUAL(user->GetState(), User::REGISTERED);

    s = sdk.FinishAuthentication(user, "2222", accessCode);
    BOOST_CHECK_EQUAL(s, Status::INCORRECT_PIN);
    BOOST_CHECK_EQUAL(user->GetState(), User::REGISTERED);

    s = sdk.FinishAuthentication(user, "3333", accessCode);
    BOOST_CHECK_EQUAL(s, Status::INCORRECT_PIN);
    BOOST_CHECK_EQUAL(user->GetState(), User::BLOCKED);

    PrintTestEnd();
}

BOOST_AUTO_TEST_CASE(TrustedDomains)
{
    PrintTestStart();

    // URL Parsing test
    UrlTestVector urlTests = GetUrlTests();
    for (UrlTestVector::iterator test = urlTests.begin(); test != urlTests.end(); ++test)
    {
        BOOST_CHECK_MESSAGE(test->Parse(),
            "Url parsing test failed: '" << test->urlString << "' parsed as {" << test->GetParsedUrl() << "} expected was {" << test->correctUrl << "}");
    }
    
    // Trusted domain tests
    BOOST_REQUIRE(sdk.IsInitilized());

    sdk.AddTrustedDomain("mpin.io");

    MfaSDK::ServiceDetails serviceDetails;
    Status s = sdk.GetServiceDetails(serviceDetailsUrl, serviceDetails);
    BOOST_CHECK_EQUAL(s, Status::OK);

    // No miracl.net trusted domain
    String accessCode;
    s = sdk.GetAccessCode(authzUrl, accessCode);
    BOOST_CHECK_EQUAL(s, Status::UNTRUSTED_DOMAIN_ERROR);

    MfaSDK::SessionDetails sessionDetails;
    s = sdk.GetSessionDetails(accessCode, sessionDetails);
    BOOST_CHECK_EQUAL(s, Status::UNTRUSTED_DOMAIN_ERROR);
    s = sdk.AbortSession(accessCode);
    BOOST_CHECK_EQUAL(s, Status::UNTRUSTED_DOMAIN_ERROR);

    UserPtr user = sdk.MakeNewUser("testUser5@example.com");
    s = sdk.StartRegistration(user, accessCode, "");
    BOOST_CHECK_EQUAL(s, Status::UNTRUSTED_DOMAIN_ERROR);
    BOOST_CHECK_EQUAL(user->GetState(), User::INVALID);

    // Add miracl.net trusted domain
    sdk.AddTrustedDomain("miracl.net");

    s = sdk.GetAccessCode(authzUrl, accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    s = sdk.GetSessionDetails(accessCode, sessionDetails);
    BOOST_CHECK_EQUAL(s, Status::OK);
    s = sdk.AbortSession(accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    s = sdk.GetAccessCode(authzUrl, accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    s = sdk.StartRegistration(user, accessCode, "");
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::STARTED_REGISTRATION);

    s = sdk.ConfirmRegistration(user);
    BOOST_CHECK_EQUAL(s, Status::OK);

    s = sdk.FinishRegistration(user, "1234");
    BOOST_CHECK_EQUAL(s, Status::OK);
    BOOST_CHECK_EQUAL(user->GetState(), User::REGISTERED);

    s = sdk.StartAuthentication(user, accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    s = sdk.FinishAuthentication(user, "1234", accessCode);
    BOOST_CHECK_EQUAL(s, Status::OK);

    PrintTestEnd();
}
