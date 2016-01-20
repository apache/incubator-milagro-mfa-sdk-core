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

#ifndef _AUTO_CONTEXT_V2_H_
#define _AUTO_CONTEXT_V2_H_

#include "mpin_sdk_v2.h"

class AutoContextV2 : public MPinSDKv2::IContext
{
public:
    typedef MPinSDKv2::String String;
    typedef MPinSDKv2::IHttpRequest IHttpRequest;
    typedef MPinSDKv2::IStorage IStorage;
    typedef MPinSDKv2::CryptoType CryptoType;

    AutoContextV2();
    ~AutoContextV2();
    virtual IHttpRequest * CreateHttpRequest() const;
    virtual void ReleaseHttpRequest(IN IHttpRequest *request) const;
    virtual IStorage * GetStorage(IStorage::Type type) const;
    virtual CryptoType GetMPinCryptoType() const;

private:
    IStorage *m_nonSecureStorage;
    IStorage *m_secureStorage;
};

#endif // _AUTO_CONTEXT_V2_H_
