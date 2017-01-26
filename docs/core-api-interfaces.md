# Core API Interfaces

The interfaces are:

* `IHttpRequest` - for making HTTP requests
* `IStorage`     - for storing data on the device
* `IContext`     - for grouping the rest of the interfaces into a single bundle

Although the SDK Core API (part of it) is the de-facto SDK API, it is not exposed to the application developer.
It is an internal API to the Platform Adaptation Layer, which presents the SDK API to the application in a way that is native to the platform.

#### HTTP Request Interface (`IHttpRequest`)
The Core uses this interface to make HTTP requests.
It should be implemented in the Platform Adaptation Layer.
The Core creates an HTTP Request object via `IContext::CreateHttpRequest()` method and when done, releases the request via `IContext::ReleaseHttpRequest()`.

##### `virtual void SetHeaders(const StringMap& headers);`
This method sets the headers for the HTTP Request.
The headers are passed in the `headers` key/value map, which is a standard `std::map<std::string, std::string>` object.
This method should be called prior to executing the HTTP Request.

##### `virtual void SetQueryParams(const StringMap& queryParams);`
This method sets the query parameters for the HTTP Request.
The query parameters are passed in the `queryParams` key/value map, which is a standard `std::map<std::string, std::string>` object.
This method is called prior to executing the HTTP Request.

##### `virtual void SetContent(const String& data);`
This method sets the content (the data) of the HTTP Request.
It is passed in the `data` parameter as a string.
This method is called prior to executing the HTTP Request.

##### `virtual void SetTimeout(int seconds);`
This method sets a timeout for the HTTP Request.
The timeout is set in `seconds`.
If not set, the timeout is expected to be infinite.
This method is called prior to executing the HTTP Request.

##### `virtual bool Execute(Method method, const String& url);`
This method executes the HTTP Request with the provided `method` and to the given `url`.
The `Method` enumerator is defined as follows:
```c++
enum Method
{
    GET,
    POST,
    PUT,
    DELETE,
    OPTIONS,
    PATCH
};
```
The request is made with the previously set headers, query parameters and data.
If the HTTP request was successfully executed, and a response received, the return value will be `true`.
If the execution failed, the return value would be `false`.

**NOTE** that a non-2xx HTTP response does not mean that the request has failed, but that it succeeded while the return status code was not a 2xx HTTP response.
If the `Execute()` request failed, the `GetExecuteErrorMessage()` will return more information on the reason for the failure.

##### `virtual const String& GetExecuteErrorMessage() const;`
Returns an error message describing the failure of a preceding `Execute()` request.

##### `virtual int GetHttpStatusCode() const;`
Returns the status code received in response to the preceding successfully executed HTTP request.

##### `virtual const StringMap& GetResponseHeaders() const;`
Returns the headers received in response to the preceding successfully executed HTTP request.
The headers are returned in a key/value map, which is a standard `std::map<std::string, std::string>` object.

##### `virtual const String& GetResponseData() const;`
Returns the data received as a response to the preceding successfully executed HTTP request.
The data is returned as a string.

#### Storage Interface (`IStorage`)
The Core uses this interface to store data on the specific platform.
As different platforms provide different storage options, the implementation of the interface should be part of the Platform Adaptation Layer.
There are two kinds of storages that the Core uses, Secure and Non-secure.
In the Secure Storage, the Core stores the _regOTT_ (during registration) and _M-Pin Token_ for every user that is registered through the device.
In the Non-Secure Storage, the Core stores all non-sensitive data, like cached Time Permits.
The `IContext` provides the interface to the correct Storage via the `IContext::GetStorage(IStorage::Type)` method.

##### `virtual bool SetData(const String& data);`
This method sets/writes the whole storage data.
The data is provided in the data parameter, as a string.
The return value is `true` on success or `false` on failure.
If the method fails, further information regarding the error can be obtained through the `GetErrorMessage()` method.

##### `virtual bool GetData(OUT String& data);`
This method gets/reads the whole storage data.
The data is returned in the `data` parameter, as a string.
The return value is `true` on success or `false` on failure.
If the method fails, further information regarding the error can be obtained through the `GetErrorMessage()` method.

##### `virtual const String& GetErrorMessage() const;`
Returns the error from the preceding failed `GetData()` or `SetData()` methods.

#### Context Interface (`IContext`)
The Context Interface "bundles" the rest of the interfaces.
This is the only interface that is provided to the Core where the other interfaces are used/accessed through it.

##### `virtual IHttpRequest* CreateHttpRequest() const;`
This request creates a new HTTP request instance that conforms with `IHttpRequest`, and returns a pointer to it.
The Core calls `CreateHttpRequest()` when it needs to make an HTTP request.
After receiving a pointer to such an instance, the Core sets the needed headers, query parameters and data, and then executes the request.
The Core will then check for the status code, the headers, the data of the response, and will finally release the HTTP request.

**NOTE** that the HTTP request instance should remain "alive" until calling `ReleaseHttpRequest()`.

**NOTE** As the Core might create two or more HTTP request instances in parallel, the implementation should not use global or local (stack) HTTP request objects.

##### `virtual void ReleaseHttpRequest(IN IHttpRequest* request) const;`
This request destroys/releases a previously created HTTP request instance.
The Core calls `CreateHttpRequest()` when it needs to make an HTTP request.
After receiving a pointer to such an instance, the Core sets the needed headers, query parameters and data, and then executes the request.
The Core checks for the status code, the headers, the data of the response, and finally releases the HTTP request.

**NOTE** that the HTTP request instance should remain "alive" until calling `ReleaseHttpRequest()`.

**NOTE** Also, as the Core might create two or more HTTP request instances in parallel, the implementation should not use global or local (stack) HTTP request objects.

##### `virtual IStorage* GetStorage(IStorage::Type type) const;`
This method returns a pointer to the storage implementation, which conforms to `IStorage`.
There are two storage types, Secure and Non-Secure, where you can pass your desired storage type as a parameter.
The `IStorage::Type` enumerator is defined as follows:
```c++
enum Type
{
    SECURE,
    NONSECURE
};
```

##### `virtual CryptoType GetMPinCryptoType() const;`
This method provides information regarding the supported Crypto Type on the specific platform.
Currently, this method could return a different value than Non-TEE Crypto, only on an Android platform.
Other platforms will always return a Non-TEE Crypto value.
On an Android, the Platform Adaptation needs to check if TEE is supported, and then return TEE or Non-TEE Crypto type accordingly.
The `CryptoType` enumerator is defined as follows:
```c++
enum CryptoType
{
    CRYPTO_TEE,
    CRYPTO_NON_TEE
};
```
