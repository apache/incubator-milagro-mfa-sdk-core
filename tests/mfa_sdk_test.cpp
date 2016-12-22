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

    const char *backend = "https://api.dev.miracl.net";
    StringMap config;
    config.Put(MfaSDK::CONFIG_BACKEND, backend);

    CmdLineContext context("mfa_sdk_test_users.json", "mfa_sdk_test_tokens.json");
    MfaSDK sdk;
    sdk.SetCID("dd");

    Status s = sdk.Init(config, &context);
    if (s != Status::OK)
    {
        cout << "Failed to initialize MPinSDK: status code = " << s.GetStatusCode() << ", error: " << s.GetErrorMessage() << endl;
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

        string accessCode = GetStringFromStdin("Enter access code: ");

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

    string accessCode = GetStringFromStdin("Enter access code: ");
    s = sdk.StartAuthentication(user, accessCode);
    if (s != Status::OK)
    {
        cout << "Failed to start user authentication: status code = " << s.GetStatusCode() << ", error: " << s.GetErrorMessage() << endl;
        _getch();
        return 0;
    }

    string pin = GetStringFromStdin("Enter pin: ");
    s = sdk.FinishAuthenticationAC(user, pin, accessCode);
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
