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

#include "auto_context.h"
#include "../common/http_request.h"
#include "../common/memory_storage.h"

#include <iostream>
#include <fstream>

typedef MPinSDK::String String;
typedef MPinSDK::IHttpRequest IHttpRequest;
typedef MPinSDK::CryptoType CryptoType;
typedef MPinSDK::UserPtr UserPtr;

/*
 * Context class impl
 */

AutoContext::AutoContext()
{
    m_nonSecureStorage = new MemoryStorage();
    m_secureStorage = new MemoryStorage();
}

AutoContext::~AutoContext()
{
    delete m_nonSecureStorage;
    delete m_secureStorage;
}

IHttpRequest * AutoContext::CreateHttpRequest() const
{
    return new HttpRequest();
}

void AutoContext::ReleaseHttpRequest(IN IHttpRequest *request) const
{
    delete request;
}

MPinSDK::IStorage * AutoContext::GetStorage(IStorage::Type type) const
{
    if(type == IStorage::SECURE)
    {
        return m_secureStorage;
    }

    return m_nonSecureStorage;
}

CryptoType AutoContext::GetMPinCryptoType() const
{
    return MPinSDK::CRYPTO_NON_TEE;
}
