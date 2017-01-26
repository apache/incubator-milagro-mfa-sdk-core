# Core API for the MIRACL MFA Platform - `MfaSDK`

This flavor of the SDK should be used to build apps that authenticate users against the _MIRACL MFA Platform_.
It massively resembles the _Apache Milagro_ flavor, while incorporating some functionality is specific to the MIRACL Platform.
Similarly to `MPinSDK`, the `MfaSDK` needs to be instantiated and initialized.
Most of the methods return a `MfaSDK::Status` object, which is identical to the one used by `MfaSDK`.

The methods that return `Status`, will always return `Status::OK` if successful.
Many methods expect the provided `User` object to be in a certain state, and if it is not, the method will return `Status::FLOW_ERROR`

##### `Status Init(const StringMap& config, IN IContext* ctx);`
Identical and analogical to [`MPinSDK::Init()`](core-api-mpinsdk.md#status-initconst-stringmap-config-in-icontext-ctx)

##### `void Destroy();`
Identical and analogical to [`MPinSDK::Destroy()`](core-api-mpinsdk.md#void-destroy)

##### `void AddCustomHeaders(const StringMap& customHeaders);`
Identical and analogical to [`MPinSDK::AddCustomHeaders()`](core-api-mpinsdk.md#void-addcustomheadersconst-stringmap-customheaders)

##### `void ClearCustomHeaders();`
Identical and analogical to [`MPinSDK::ClearCustomHeaders()`](core-api-mpinsdk.md#void-clearcustomheaders)

##### `void AddTrustedDomain(const String& domain);`
Identical and analogical to [`MPinSDK::AddTrustedDomain()`](core-api-mpinsdk.md#void-addtrusteddomainconst-string-domain)

##### `void ClearTrustedDomains();`
Identical and analogical to [`MPinSDK::ClearTrustedDomains()`](core-api-mpinsdk.md#void-cleartrusteddomains)

##### `void SetCID(const String& cid);`
This method will set a specific _Client/Customer ID_ which the SDK should use when sending requests to the backend.
The MIRACL MFA Platform generates _Client IDs_ (sometimes also referred as _Customer IDs_) for the platform customers.
The customers can see those IDs through the _Platform Portal_.
When customers use the SDK to build their own applications to authenticate users using the Platform, the _Client ID_ has to be provided using this method.

##### `Status TestBackend(const String& server, const String& rpsPrefix = "rps") const;`
Identical and analogical to [`MPinSDK::TestBackend()`](core-api-mpinsdk.md#status-testbackendconst-string-server-const-string-rpsprefix--rps-const)

##### `Status SetBackend(const String& server, const String& rpsPrefix = "rps");`
Identical and analogical to [`MPinSDK::SetBackend()`](core-api-mpinsdk.md#status-setbackendconst-string-server-const-string-rpsprefix--rps)

##### `UserPtr MakeNewUser(const String& id, const String& deviceName = "") const;`
Identical and analogical to [`MPinSDK::MakeNewUser()`](core-api-mpinsdk.md#userptr-makenewuserconst-string-id-const-string-devicename---const)

##### `bool IsUserExisting(const String& id);`
Identical and analogical to [`MPinSDK::IsUserExisting()`](core-api-mpinsdk.md#bool-isuserexistingconst-string-id)

##### `void DeleteUser(INOUT UserPtr user);`
Identical and analogical to [`MPinSDK::DeleteUser()`](core-api-mpinsdk.md#void-deleteuserinout-userptr-user)

##### `Status ListUsers(OUT std::vector<UserPtr>& users) const;`
This method will populate the provided `users` vector with ALL the users known to the SDK. This is similar to the [`MPinSDK::ListAllUsers()`](core-api-mpinsdk.md#status-listallusersout-stdvectoruserptr-users-const).
After the list is returned to the caller, the users might be filtered out using their properties `User::GetBackend()`, `User::GetCustomerId()` and `User::GetAppId()`.

##### `Status GetServiceDetails(const String& url, OUT ServiceDetails& serviceDetails);`
This method is provided for applications working with the _MIRACL MFA Platform_.
After scanning a QR Code from the platform login page, the app should extract the URL from it and call this method to retrieve the service details.
The service details include the _backend URL_ which needs to be set back to the SDK in order connect it to the platform.
In such a scenario, the SDK could be initialized without setting a backend, and `SetBackend()` could be used after the backend URL has been retrieved through this method.
The returned `ServiceDetails` looks as follows:
```c++
class ServiceDetails
{
public:
    String name;
    String backendUrl;
    String rpsPrefix;
    String logoUrl;
};
```
* `name` is the service readable name
* `backendUrl` is the URL of the service backend. This URL has to be set either via the SDK `Init()` method or using  `SetBackend()`
* `rpsPrefix` is RPS prefix setting which is also provided together with `backendUrl` while setting a backend
* `logoUrl` is the URL of the service logo. The logo is a UI element that could be used by the app.

##### `Status GetSessionDetails(const String& accessCode, OUT SessionDetails& sessionDetails);`
This method is used to retrieve details regarding a browser session when the SDK is used to authenticate users to an online service, such as the _MIRACL MFA Platform_.
In this case an `accessCode` is transferred to the mobile device out-of-band e.g. via scanning a graphical code.
The code is then provided to this method to retrieve the session details.
This method will also notify the backend that the `accessCode` was read from the browser session.
The returned `SessionDetails` looks as follows:
```c++
class SessionDetails
{
public:
    void Clear();

    String prerollId;
    String appName;
    String appIconUrl;
    String customerId;
    String customerName;
    String customerIconUrl;
};
```
During the online browser session an optional user identity might be provided meaning that this is the user that wants to register/authenticate to the online service.
* The `prerollId` will carry that user ID, or it will be empty if no such ID was provided.
* `appName` is the name of the web application to which the service will authenticate the user.
* `appIconUrl` is the URL from which the icon for web application could be downloaded.
* `customerId` is the _Customer ID_ (aka _Client ID_) of the web app owner.
* `customerName` is the name of the web app owner.
* `customerIconUrl` is the URL from which the icon for the web app owner could be downloaded.

##### `Status AbortSession(const String& accessCode);`
This method should be used to inform the Platform that the current authentication/registration session has been aborted.
A session starts with obtaining the _Access Code_, usually after scanning and decoding a graphical image, such as QR Code.
Then the mobile client might retrieve the session details using `GetSessionDetails()`, after which it can either start registering a new end-user or start authentication.
This process might be interrupted by either the end-user disagreeing on the consent page, or by just hitting a Back button on the device, or by even closing the app.
For all those cases, it is recommended to use `AbortSession()` to inform the Platform.

##### `Status GetAccessCode(const String& authzUrl, OUT String& accessCode);`
This method should be used when the mobile app needs to login an end-user into the app itself.
In this case there's no browser session involved and the Access Code cannot be obtained by scanning an image from the browser.
Instead, the mobile app should initially get from its backend an _Authorization URL_. This URL could be formed at the app backend using one of the _MFA Platform SDK_ flavors.
When the mobile app has the Authorization URL, it can pass it to this method as `authzUrl`, and get back an `accessCode` that can be further used to register or authenticate end-users.
Note that the Authorization URL contains a parameter that identifies the app.
This parameter is validated by the Platform and it should correspond to the Customer ID, set via `SetCID()`.

##### `Status StartRegistration(INOUT UserPtr user, const String& accessCode, const String& pushToken);`
This method initializes the registration for a User that has already been created.
The SDK starts the Setup flow, sending the necessary requests to the back-end service.
The State of the User instance will change to `STARTED_REGISTRATION`.
The status will indicate whether the operation was successful or not.
During this call, an _M-Pin ID_ for the end-user will be issued by the Platform and stored within the user object.
The Platform will also start a user identity verification procedure, by sending a verification e-mail.

The `accessCode` should be obtained from a browser session, and session details are retrieved before starting the registration.
This way the mobile app can show to the end-user the respective details for the customer, which the identity is going to be associated to.
 
Optionally, the application might pass additional `pushToken`, which is a unique token for sending _Push Messages_ to the mobile app.
When such token is provided, the Platform might use additional verification step by sending a Push Message to the app.

##### `Status RestartRegistration(INOUT UserPtr user);`
This method is analogic to [`MPinSDK::RestartRegistration()`](core-api-mpinsdk.md#status-restartregistrationinout-userptr-user-const-string-userdata--)
without the additional optional parameter that is not used by the MFA Platform.

##### `Status ConfirmRegistration(INOUT UserPtr user);`
This method is analogic to [`MPinSDK::ConfirmRegistration()`](core-api-mpinsdk.md#status-confirmregistrationinout-userptr-user-const-string-pushtoken--)
without the additional optional parameter that is not used by the MFA Platform.

##### `Status FinishRegistration(INOUT UserPtr user, const String& pin)`
This method is analogic to [`MPinSDK::ConfirmRegistration()`](core-api-mpinsdk.md#status-finishregistrationinout-userptr-user-const-string-pin)

##### `Status StartAuthentication(INOUT UserPtr user, const String& accessCode);`
This method starts the authentication process for a given `user`.
It attempts to retrieve the _Time Permits_ for the user, and if successful, will return `Status::OK`.
If they cannot be retrieved, the method will return `Status::REVOKED`.
If this method is successfully completed, the app should read the PIN/secret from the end-user and call one of the `FinishAuthentication()` variants to authenticate the user.

The `accessCode` indicates which browser session the authentication is bound to.
This access code is retrieved out-of-band from a browser session. For custom authentication apps, the access code could be also retrieved using the `GetAccessCode()` method.
Having the access code, the SDK will notify the platform that authentication associated with it has started for the given user.

##### `Status FinishAuthentication(INOUT UserPtr user, const String& pin, const String& accessCode);`
This method authenticates the end-user for logging into a Web App in a browser session.
The `user` to be authenticated is passed as a parameter, along with his/her `pin` (secret).
The `accessCode` associates the authentication with the browser session from which it was obtained.
The method is analogical to [`MPinSDK::FinishAuthenticationAN()`](core-api-mpinsdk.md#status-finishauthenticationaninout-userptr-user-const-string-pin-const-string-accessnumber),
while the Access Code is used instead of an Access Number.
The returned status might be:
* `Status::OK` - Successful authentication.
* `Status::INCORRECT_PIN` - The authentication failed because of incorrect PIN.
After the 3rd unsuccessful authentication attempt, the method will still return `Status::INCORRECT_PIN` but the User State will be set to `BLOCKED`.

##### `Status FinishAuthentication(INOUT UserPtr user, const String& pin, const String& accessCode, OUT String& authzCode);`
This method authenticates an end-user in a way that allows the mobile app to log the user into the app itself after verifying the authentication against its own backend. 
When using this flow, the mobile app would first retrieve the `accessCode` using the `GetAccessCode()` method,
and when authentication the user it will receive an _Authorization Code_, `authzCode`.
Using this Authorization Code, the mobile app can make a request to its own backend, so the backend can validate it using one of the _MFA Platform SDK_ flavors,
and create a session token.
This token could be used further as an authentication element in the communication between the app and its backend.

See also [MIRACL MFA Platform - Main Flows](mfasdk-flows.md)