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
 * MPinSDKBase::IHttpRequest implementation used for to record test http requests
 */

#include "http_recorder.h"
#include <cassert>

typedef MPinSDKBase::String String;
typedef MPinSDKBase::StringMap StringMap;
typedef HttpRecordedData::Request Request;
typedef HttpRecordedData::Response Response;
typedef HttpRecordedData::PredefinedResponse PredefinedResponse;
typedef HttpRecordedData::ResponseQueue ResponseQueue;

void HttpRecorder::SetHeaders(const StringMap & headers)
{
    m_request.SetHeaders(headers);
}

void HttpRecorder::SetQueryParams(const StringMap & queryParams)
{
    assert(false);
}

void HttpRecorder::SetContent(const String & data)
{
    m_requestData = data;
    m_request.SetContent(data);
}

void HttpRecorder::SetTimeout(int seconds)
{
    m_request.SetTimeout(seconds);
}

bool HttpRecorder::Execute(Method method, const String & url)
{
    Request requset(method, url, m_requestData, m_context);
    PredefinedResponse pr;

    ResponseQueue& queue = m_recorder.GetPredefinedResponses();
    if (!queue.empty())
    {
        pr = queue.front();
        queue.pop();
    }

    if (pr.IsEmpty())
    {
        bool res = m_request.Execute(method, url);
        m_response = Response(res, m_request.GetExecuteErrorMessage(), m_request.GetHttpStatusCode(), m_request.GetResponseHeaders(), m_request.GetResponseData());
        m_recorder.Record(requset, m_response);
    }
    else
    {
        m_response = pr;
    }

    return m_response.success;
}

const String & HttpRecorder::GetExecuteErrorMessage() const
{
    return m_response.error;
}

int HttpRecorder::GetHttpStatusCode() const
{
    return m_response.httpStatus;
}

const StringMap & HttpRecorder::GetResponseHeaders() const
{
    return m_response.headers;
}

const String & HttpRecorder::GetResponseData() const
{
    return m_response.data;
}
