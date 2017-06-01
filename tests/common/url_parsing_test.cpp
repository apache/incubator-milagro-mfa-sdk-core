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
* Url utility class test
*/

#include "url_parsing_test.h"

namespace
{
    using util::Url;
    using std::cout;
    using std::endl;
    using std::ostream;

    UrlTestVector tests =
    {
        { "https://test.com:8080/path", Url("https", "test.com", "8080", "path") },
        { "https://test.com:8080", Url("https", "test.com", "8080", "") },
        { "test", Url("", "test", "", "") },
        { "test.com", Url("", "test.com", "", "") },
        { "test.com:80", Url("", "test.com", "80", "") },
        { "http://test", Url("http", "test", "", "") },
        { "http:///test:8080/", Url("http", "test", "8080", "") },
        { "https:/test.com", Url("", "https", "", "test.com") },
        { "https:test.com", Url("", "https", "test.com", "") },
        { "http://test.com/path", Url("http", "test.com", "", "path") },
        { "test.com/path", Url("", "test.com", "", "path") },
        { "", Url("", "", "", "") },
        { "http:/", Url("", "http", "", "") },
        { "https://", Url("https", "", "", "") },
        { "https:///", Url("https", "", "", "") },
        { "/test", Url("", "", "", "test") },
        { "://test.com:8080", Url("", "test.com", "8080", "") },
        { ":///test.com/", Url("", "test.com", "", "") },
        { ":///test.com:121/", Url("", "test.com", "121", "") },
        { "/:///", Url("/", "", "", "") },
        { "http://test.com/:80/path", Url("http", "test.com", "", ":80/path") },
        // Uncomment for unicode tolower test { "https://ПрОбА.net", Url("https", "проба.net", "", "") },
        { "https://проба.net", Url("https", "проба.net", "", "") },
        { "HtTpS://tESt.CoM/:80/path", Url("https", "test.com", "", ":80/path") },
        { "https://世丕且且世两上与丑万丣丕且丗丕.japan.com", Url("https", "世丕且且世两上与丑万丣丕且丗丕.japan.com", "", "") },
        { "https://ひほわれよう.japan.com:7130/存在する", Url("https", "ひほわれよう.japan.com", "7130", "存在する") },
        // Uncomment for unicode tolower test { "http://Über.STRAßE.com", Url("http", "über.straße.com", "", "") },
        { "http://über.straße.com", Url("http", "über.straße.com", "", "") },
        { "https://这是一份非常简单的说明书.china.com", Url("https", "这是一份非常简单的说明书.china.com", "", "") },
        { "HTTP://這是一份非常簡單的說明書.china.com", Url("http", "這是一份非常簡單的說明書.china.com", "", "") },
    };
}

namespace std
{
    ostream& operator<<(ostream& out, const Url& url)
    {
        out << "scheme='" << url.GetScheme() << "', host='" << url.GetHost() << "', port='" << url.GetPort() << "', path='" << url.GetPath() << "'";
        return out;
    }
}

bool UrlTest::Parse()
{
    return GetParsedUrl() == correctUrl;
}

util::Url UrlTest::GetParsedUrl() const
{
    return Url(urlString);
}

UrlTestVector& GetUrlTests()
{
    return tests;
}

void DoStandaloneUrlParsingTest()
{
    size_t failedCount = 0;
    for (UrlTestVector::iterator test = tests.begin(); test != tests.end(); ++test)
    {
        if (test->Parse())
        {
            cout << "success: '" << test->urlString << "' parsed as {" << test->GetParsedUrl() << "}" << endl;
        }
        else
        {
            cout << " * FAILED: '" << test->urlString << "' parsed as {" << test->GetParsedUrl() << "}" << endl << "   expected was {" << test->correctUrl << "}" << endl;
            ++failedCount;
        }
    }
    cout << endl << "Failed " << failedCount << " out of " << tests.size() << endl;
}
