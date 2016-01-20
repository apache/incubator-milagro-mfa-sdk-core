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
typedef MPinSDK::IPinPad IPinPad;
typedef MPinSDK::CryptoType CryptoType;
typedef MPinSDK::UserPtr UserPtr;

/*
 * Pinpad class impl
 */

class AutoPinpad : public MPinSDK::IPinPad
{
public:
    void SetPin(const String& pin)
    {
        m_pin = pin;
    }

    virtual String Show(UserPtr user, Mode mode)
    {
        return m_pin;
    }

private:
    String m_pin;
};

/*
 * Context class impl
 */

AutoContext::AutoContext()
{
    m_nonSecureStorage = new MemoryStorage();
    m_secureStorage = new MemoryStorage();
    m_pinpad = new AutoPinpad();
}

AutoContext::~AutoContext()
{
    delete m_nonSecureStorage;
    delete m_secureStorage;
    delete m_pinpad;
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

IPinPad * AutoContext::GetPinPad() const
{
    return m_pinpad;
}

CryptoType AutoContext::GetMPinCryptoType() const
{
    return MPinSDK::CRYPTO_NON_TEE;
}

void AutoContext::SetPin(const String& pin)
{
    ((AutoPinpad *) m_pinpad)->SetPin(pin);
}
