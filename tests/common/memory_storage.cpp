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

#include "memory_storage.h"
#include <iostream>

typedef MPinSDKBase::String String;
using std::cout;
using std::endl;

bool MemoryStorage::SetData(const String& data)
{
        m_data = data;
        return true;
}

bool MemoryStorage::GetData(String &data)
{
    data = m_data;
    return true;
}

bool MemoryStorage::ClearData()
{
    cout << "Clearing old version of 'memory' storage." << endl;
    SetData("");
    return true;
}

const String& MemoryStorage::GetErrorMessage() const
{
    return m_errorMessage;
}
