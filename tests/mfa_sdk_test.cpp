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

#include <mfa_sdk.h>
#include "contexts/cmdline_context.h"
#include "common/url_parsing_test.h"
#include <CvLogger.h>
#include <iostream>
#include <conio.h>
#include <vector>

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::vector;
typedef MfaSDK::Status Status;
typedef MfaSDK::String String;
typedef MfaSDK::StringMap StringMap;
typedef MfaSDK::User User;
typedef MfaSDK::UserPtr UserPtr;

string GetStringFromStdin(const char *prompt)
{
    cout << prompt;
    string str;
    cin >> str;
    return str;
}

int main(int argc, char *argv[])
{
    CvShared::InitLogger("cvlog.txt", CvShared::enLogLevel_None);

    if (true)
    {
        cout << "URL Parsing test:" << endl << endl;
        DoStandaloneUrlParsingTest();
        cout << endl;
    }

    const char *authzUrl = "https://api.dev.miracl.net/authorize?client_id=ojmlslsgnaax2&redirect_uri=https%3A%2F%2Fdemo.dev.miracl.net%2Foidc&response_type=code&scope=openid+email+profile&state=096aa4d129464939886a7a0d8fe1e212&back_url=https%3A%2F%2Fdemo.dev.miracl.net%2F&lang=en";

    const char *backend = "https://api.dev.miracl.net";

    CmdLineContext context("mfa_sdk_test_users.json", "mfa_sdk_test_tokens.json");
    MfaSDK sdk;

    sdk.AddTrustedDomain("miracl.net");

    // TODO: Shouldn't CID setting be moved to Init?
    sdk.SetCID("dd");

    Status s = sdk.Init(StringMap(), &context);
    if (s != Status::OK)
    {
        cout << "Failed to initialize SDK: status code = " << s.GetStatusCode() << ", error: " << s.GetErrorMessage() << endl;
        _getch();
        return 0;
    }

    s = sdk.SetBackend(backend);
    if (s != Status::OK)
    {
        cout << "Failed to set SDK backend: status code = " << s.GetStatusCode() << ", error: " << s.GetErrorMessage() << endl;
        _getch();
        return 0;
    }

    vector<UserPtr> users;
    sdk.ListUsers(users);
    UserPtr user;
    if (!users.empty())
    {
        user = users[0];
        cout << "Authenticating user '" << user->GetId() << "'" << endl;
    }
    else
    {
        user = sdk.MakeNewUser("slav.klenov@miracl.com");
        cout << "Did not found any registered users. Will register new user '" << user->GetId() << "'" << endl;

        //string accessCode = GetStringFromStdin("Enter access code: ");
        String accessCode;
        s = sdk.GetAccessCode(authzUrl, accessCode);
        if (s != Status::OK)
        {
            cout << "Failed to get access code: status code = " << s.GetStatusCode() << ", error: " << s.GetErrorMessage() << endl;
            _getch();
            return 0;
        }

        s = sdk.StartRegistration(user, accessCode, "");
        if (s != Status::OK)
        {
            cout << "Failed to start user registration: status code = " << s.GetStatusCode() << ", error: " << s.GetErrorMessage() << endl;
            _getch();
            return 0;
        }

        if (user->GetState() == User::ACTIVATED)
        {
            cout << "User registered and force activated" << endl;
        }
        else
        {
            cout << "Registration started. Press any key after activation is confirmed..." << endl;
            _getch();
        }

        s = sdk.ConfirmRegistration(user);
        if (s != Status::OK)
        {
            cout << "Failed to confirm user registration: status code = " << s.GetStatusCode() << ", error: " << s.GetErrorMessage() << endl;
            _getch();
            return 0;
        }

        string pin = GetStringFromStdin("Enter pin: ");

        s = sdk.FinishRegistration(user, pin);
        if (s != Status::OK)
        {
            cout << "Failed to finish user registration: status code = " << s.GetStatusCode() << ", error: " << s.GetErrorMessage() << endl;
            _getch();
            return 0;
        }

        cout << "User successfuly registered. Press any key to authenticate user..." << endl;
        _getch();
    }

    //string accessCode = GetStringFromStdin("Enter access code: ");
    String accessCode;
    s = sdk.GetAccessCode(authzUrl, accessCode);
    if (s != Status::OK)
    {
        cout << "Failed to get access code: status code = " << s.GetStatusCode() << ", error: " << s.GetErrorMessage() << endl;
        _getch();
        return 0;
    }

    s = sdk.StartAuthentication(user, accessCode);
    if (s != Status::OK)
    {
        cout << "Failed to start user authentication: status code = " << s.GetStatusCode() << ", error: " << s.GetErrorMessage() << endl;
        _getch();
        return 0;
    }

    string pin = GetStringFromStdin("Enter pin: ");
    s = sdk.FinishAuthentication(user, pin, accessCode);
    if (s != Status::OK)
    {
        cout << "Failed to authenticate user: status code = " << s.GetStatusCode() << ", error: " << s.GetErrorMessage() << endl;
        _getch();
        return 0;
    }

    cout << "User successfuly authenticated!" << endl;
    cout << "Press any key to exit..." << endl;

    _getch();

    return 0;
}
