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
 * MPinSDKBase::IContext and all related interfaces implementation for command line test client
 */

#include "cmdline_context.h"
#include "../common/file_storage.h"

typedef MPinSDKBase::String String;
typedef MPinSDKBase::IHttpRequest IHttpRequest;
typedef MPinSDKBase::CryptoType CryptoType;

/*
 * Context class impl
 */

CmdLineContext::CmdLineContext(const String& usersFile, const String& tokensFile) :
    m_nonSecureStorage(new FileStorage(usersFile)), m_secureStorage(new FileStorage(tokensFile))
{
}

CmdLineContext::~CmdLineContext()
{
    delete m_nonSecureStorage;
    delete m_secureStorage;
}

MPinSDKBase::IStorage * CmdLineContext::GetStorage(IStorage::Type type) const
{
    if(type == IStorage::SECURE)
    {
        return m_secureStorage;
    }

    return m_nonSecureStorage;
}

CryptoType CmdLineContext::GetMPinCryptoType() const
{
    return MPinSDKBase::CRYPTO_NON_TEE;
}
