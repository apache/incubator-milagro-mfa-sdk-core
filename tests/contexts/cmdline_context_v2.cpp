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
 * MPinSDK::IContext and all related interfaces implementation for command line test client
 */

#include "cmdline_context_v2.h"
#include "../common/http_request.h"
#include "../common/file_storage.h"

#include <iostream>
#include <fstream>

typedef MPinSDKv2::String String;
typedef MPinSDKv2::IHttpRequest IHttpRequest;
typedef MPinSDKv2::CryptoType CryptoType;
typedef MPinSDKv2::UserPtr UserPtr;

/*
 * Context class impl
 */

CmdLineContextV2::CmdLineContextV2(const String& usersFile, const String& tokensFile)
{
    m_nonSecureStorage = new FileStorage(usersFile);
    m_secureStorage = new FileStorage(tokensFile);
}

CmdLineContextV2::~CmdLineContextV2()
{
    delete m_nonSecureStorage;
    delete m_secureStorage;
}

IHttpRequest * CmdLineContextV2::CreateHttpRequest() const
{
    return new HttpRequest();
}

void CmdLineContextV2::ReleaseHttpRequest(IN IHttpRequest *request) const
{
    delete request;
}

MPinSDK::IStorage * CmdLineContextV2::GetStorage(IStorage::Type type) const
{
    if(type == IStorage::SECURE)
    {
        return m_secureStorage;
    }

    return m_nonSecureStorage;
}

CryptoType CmdLineContextV2::GetMPinCryptoType() const
{
    return MPinSDK::CRYPTO_NON_TEE;
}
