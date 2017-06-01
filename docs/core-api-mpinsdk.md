# Core API for Apache Milagro - `MPinSDK`

The `MPinSDK` is the main SDK Core class for Apache Milagro.
In order to use the Core, one should create an instance of the `MPinSDK` class and initialize it.
Most of the methods return a `MPinSDK::Status` object, which is defined as follows:

```c++
class Status
{
public:
    enum Code
    {
        OK,
        PIN_INPUT_CANCELLED,     // Local error, returned when a user cancels entering a pin
        CRYPTO_ERROR,            // Local error in crypto functions
        STORAGE_ERROR,           // Local storage related error
        NETWORK_ERROR,           // Local error - unable to connect to remote server (no internet, or invalid server/port)
        RESPONSE_PARSE_ERROR,    // Local error - unable to parse json response from remote server (invalid json or unexpected json structure)
        FLOW_ERROR,              // Local error - improper MPinSDK class usage
        IDENTITY_NOT_AUTHORIZED, // Remote error - remote server refuses user registration or authentication
        IDENTITY_NOT_VERIFIED,   // Remote error - remote server refuses user registration because identity is not verified
        REQUEST_EXPIRED,         // Remote error - register/authentication request expired
        REVOKED,                 // Remote error - unable to get time permit (probably the user is temporarily suspended)
        INCORRECT_PIN,           // Remote error - user entered wrong pin
        INCORRECT_ACCESS_NUMBER, // Remote/local error - wrong access number (checksum failed or RPS returned 412)
        HTTP_SERVER_ERROR,       // Remote error, when the error does not apply to any of the above - the remote server returned internal server error status (5xx)
        HTTP_REQUEST_ERROR,      // Remote error, where the error does not apply to any of the above - invalid data sent to server, the remote server returned 4xx error status
        BAD_USER_AGENT,          // Remote error - user agent not supported
        CLIENT_SECRET_EXPIRED,   // Remote error - re-registration required because server master secret expired
        BAD_CLIENT_VERSION,      // Remote error - wrong client app version
        UNTRUSTED_DOMAIN_ERROR,  // Local error - a request to a domain, that is not in the trusted list was attempted		
    };

    Code GetStatusCode() const;
    String GetStatusCodeString() const;
    const String& GetErrorMessage() const;
    bool operator==(Code statusCode) const;
    bool operator!=(Code statusCode) const;
    ...
};
```
The methods that return `Status`, will always return `Status::OK` if successful.
Many methods expect the provided `User` object to be in a certain state, and if it is not, the method will return `Status::FLOW_ERROR`

##### `Status Init(const StringMap& config, IN IContext* ctx);`
This method initializes the `MPinSDK` instance.
It receives a map of configuration parameters and a pointer to a [Context Interface](#Context-Interface).
The configuration map is a key-value map into which different configuration options can be inserted.
This is a flexible way of passing configurations into the Core, as the method parameters will not change when new configuration parameters are added.
Unsupported parameters will be ignored.
Currently, the Core recognizes the following parameters:

* `backend` - the URL of the Milagro MFA back-end service (Mandatory)
* `rpsPrefix` - the prefix that should be added for requests to the RPS (Optional). The default value is `"rps"`.

Example:
```c++
MPinSDK::StringMap config;
config["backend"] = "http://ec2-54-77-232-113.eu-west-1.compute.amazonaws.com";
 
MPinSDK* sdk = new MPinSDK;
MPinSDK::Status s = sdk->Init( config, Context::Instance() );
```

##### `void Destroy();`
This method clears the `MPinSDK` instance and releases any allocated data by it.
After calling this method, one should use `Init()` again in order to re-use the `MPinSDK` instance.

##### `void AddCustomHeaders(const StringMap& customHeaders);`
This method allows the SDK user to set a map of custom headers, which will be added to any HTTP request that the SDK executes.
The `customHeaders` parameter is a map of header names mapped to their respective value.
Subsequent calls of this method will add headers on top of the already added ones.

##### `void ClearCustomHeaders();`
This method will clear all the currently set custom headers.

##### `void AddTrustedDomain(const String& domain);`
For better security, the SDK user might want to limit the SDK to make outgoing requests only to URLs that belong to one or more trusted domains.
This method can be used to add such trusted domains, one by one.
When trusted doamins are added, the SDK will verify that any outgoing request is done over the `https` protocol and the host belongs to one of the trusted domains.
If for some reason a request is about to be done to a non-trusted domain, the SDK will return `Status::UNTRUSTED_DOMAIN_ERROR`.

##### `void ClearTrustedDomains();`
This method will clear all the currently set trusted domains.

##### `Status TestBackend(const String& server, const String& rpsPrefix = "rps") const;`
This method will test whether `server` is a valid back-end URL by trying to retrieve Client Settings from it.
Optionally, a custom RPS prefix might be specified if it was customized at the back-end and is different than the default `"rps"`.
If the back-end URL is a valid one, the method will return `Status::OK`.

##### `Status SetBackend(const String& server, const String& rpsPrefix = "rps");`
This method will change the currently configured back-end in the Core.
Initially the back-end might be set through the `Init()` method, but then it might be change using this method.
`server` is the new back-end URL that should be used.
Optionally, a custom RPS prefix might be specified if it was customized at the back-end and is different than the default `"rps"`.
If successful, the method will return `Status::OK`.

##### `UserPtr MakeNewUser(const String& id, const String& deviceName = "") const;`
This method creates a new `User` object.
The User object represents an end-user of the Milagro authentication.
The user has its own unique identity, which is passed as the id parameter to this method.
Additionally, an optional `deviceName` might be specified.
The _Device Name_ is passed to the RPA, which might store it and use it later to determine which _M-Pin ID_ is associated with this device.
The returned value is of type `UserPtr`, which is a reference counting shared pointer to the newly created `User` instance.
The User class itself looks like this:
```c++
class User
{
public:
    enum State
    {
        INVALID,
        STARTED_REGISTRATION,
        ACTIVATED,
        REGISTERED,
        BLOCKED
    };

    const String& GetId() const;
    const String& GetBackend() const;
    State GetState() const;

private:
    ....
};
```
The newly created user is in the `INVALID` state.
The `User` class is defined in the namespace of the `MPinSDK` class.

##### `bool IsUserExisting(const String& id);`
This method will retutn `true` if there is a user with the given identity, associated with the currently set backend.
If no such user is found, the method will return `false`.

##### `void DeleteUser(INOUT UserPtr user);`
This method deletes a user from the users list that the SDK maintains.
All the user data including its _M-Pin ID_, its state and _M-Pin Token_ will be deleted.
A new user with the same identity can be created later with the `MakeNewUser()` method.

##### `Status ListUsers(OUT std::vector<UserPtr>& users) const;`
This method populates the provided vector with all the users that are associated with the currently set backend.
Different users might be in different states, reflecting their registration status.
The method will return `Status::OK` on success and `Status::FLOW_ERROR` if no backend is set through the `Init()` or `SetBackend()` methods.

##### `Status ListUsers(OUT std::vector<UserPtr>& users, const String& backend) const;`
This method populates the provided vector with all the users that are associated with the provided `backend`.
Different users might be in different states, reflecting their registration status.
The method will return `Status::OK` on success and `Status::FLOW_ERROR` if the SDK was not initialized.

##### `Status ListAllUsers(OUT std::vector<UserPtr>& users) const;`
This method populates the provided vector with all the users associated with all the backends know to the SDK.
Different users might be in different states, reflecting their registration status.
The user association to a backend could be retrieved through the `User::GetBackend()` method.
The method will return `Status::OK` on success and `Status::FLOW_ERROR` if the SDK was not initialized.

##### `Status ListBackends(OUT std::vector<String>& backends) const;`
This method will populate the provided vector with all the backends known to the SDK.
The method will return `Status::OK` on success and `Status::FLOW_ERROR` if the SDK was not initialized.

##### `Status StartRegistration(INOUT UserPtr user, const String& activateCode = "", const String& userData = "");`
This method initializes the registration for a User that has already been created.
The Core starts the Milagro Setup flow, sending the necessary requests to the back-end service.
The State of the User instance will change to `STARTED_REGISTRATION`.
The status will indicate whether the operation was successful or not.
During this call, an _M-Pin ID_ for the end-user will be issued by the RPS and stored within the user object.
The RPA could also start a user identity verification procedure, by sending a verification e-mail.

The optional `activateCode` parameter might be provided if the registration process requires such.
In cases when the user verification is done through a _One-Time-Code_ (OTC) or through an SMS that carries such code, this OTC should be passed as the `activateCode` parameter.
In those cases, the identity verification should be completed instantly and the User State will be set to `ACTIVATED`.
 
Optionally, the application might pass additional `userData` which might help the RPA to verify the user identity.
The RPA might decide to verify the identity without starting a verification process.
In this case, the `Status` of the call will still be `Status::OK`, but the User State will be set to `ACTIVATED`.

##### `Status RestartRegistration(INOUT UserPtr user, const String& userData = "");`
This method re-initializes the registration process for a user, where registration has already started.
The difference between this method and `StartRegistration()` is that it will not generate a new _M-Pin ID_, but will use the one that was already generated.
Besides that, the methods follow the same procedures, such as getting the RPA to re-start the user identity verification procedure of sending a verification email to the user.

The application could also pass additional `userData` to help the RPA to verify the user identity.
The RPA might decide to verify the identity without starting a verification process.
In this case, the `Status` of the call will still be `Status::OK`, but the User State will be set to `ACTIVATED`.

##### `Status ConfirmRegistration(INOUT UserPtr user, const String& pushToken = "");`
This method allows the application to check whether the user identity verification process has been finalized or not.
The provided `user` object is expected to be either in the `STARTED_REGISTRATION` state or in the `ACTIVATED` state.
The latter is possible if the RPA activated the user immediately with the call to `StartRegistration()` and no verification process was started.
During the call to `ConfirmRegistration()` the SDK will make an attempt to retrieve _Client Key_ for the user.
This attempt will succeed if the user has already been verified/activated but will fail otherwise.
The method will return `Status::OK` if the Client Key has been successfully retrieved and `Status::IDENTITY_NOT_VERIFIED` if the identity has not been verified yet.
If the method has succeeded, the application is expected to get the desired PIN/secret from the end-user and then call `FinishRegistration()`, and provide the PIN.

**Note** Using the optional parameter `pushToken`, the application can provide a platform specific token for sending _Push Messages_ to the device.
Such push messages might be utilized as an alternative to the _Access Number_, as part of the authentication flow.

##### `Status FinishRegistration(INOUT UserPtr user, const String& pin)`
This method finalizes the user registration process.
It extracts the _M-Pin Token_ from the _Client Key_ for the provided `pin` (secret), and then stores the token in the secure storage.
On successful completion, the User state will be set to `REGISTERED` and the method will return `Status::OK`

##### `Status StartAuthentication(INOUT UserPtr user);`
This method starts the authentication process for a given `user`.
It attempts to retrieve the _Time Permits_ for the user, and if successful, will return `Status::OK`.
If they cannot be retrieved, the method will return `Status::REVOKED`.
If this method is successfully completed, the app should read the PIN/secret from the end-user and call one of the `FinishAuthentication()` variants to authenticate the user.

##### `Status CheckAccessNumber(const String& accessNumber);`
This method is used only when a user needs to be authenticated to a remote (browser) session, using _Access Number_.
The access numbers might have a check-sum digit in them and this check-sum needs to be verified on the client side, in order to prevent calling the back-end with non-compliant access numbers.
The method will return `Status::OK` if successful, and `Status::INCORRECT_ACCESS_NUMBER` if not successful.

##### `Status FinishAuthentication(INOUT UserPtr user, const String& pin);`
##### `Status FinishAuthentication(INOUT UserPtr user, const String& pin, OUT String& authResultData);`
This method performs end-user authentication where the `user` to be authenticated is passed as a parameter, along with his `pin` (secret).
The method performs the authentication against the _Milagro MFA Server_ using the provided PIN and the stored _M-Pin Token_, and then logs into the RPA.
The RPA responds with the authentication _User Data_ which is returned to the application through the `authResultData` parameter.
If successful, the returned status will be `Status::OK`, and if the authentication fails, the return status would be `INCORRECT_PIN`.
After the 3rd (configurable in the RPS) unsuccessful authentication attempt, the method will return `Status::INCORRECT_PIN` and the User State will be set to `BLOCKED`.

##### `Status FinishAuthenticationOTP(INOUT UserPtr user, const String& pin, OUT OTP& otp);`
This method performs end-user authentication for an OTP.
The authentication process is similar to `FinishAuthentication()`, but the RPA issues an OTP instead of logging the user into the application.
The returned status is analogical to the `FinishAuthentication()` method, but in addition to that, an `OTP` structure is returned.
The `OTP` structure looks like this:
```c++
class OTP
{
public:
    String otp;
    long expireTime;
    int ttlSeconds;
    long nowTime;
    Status status;
};
```
* The `otp` string is the issued OTP.
* The `expireTime` is the Milagro MFA system time when the OTP will expire.
* The `ttlSeconds` is the expiration period in seconds.
* The `nowTime` is the current Milagro MFA system time.
* `status` is the status of the OTP generation. The status will be `Status::OK` if the OTP was successfully generated, or `Status::FLOW_ERROR` if not.

**NOTE** that OTP might be generated only by RPA that supports that functionality, such as the MIRACL M-Pin SSO.
Other RPA's might not support OTP generation where the `status` inside the returned `otp` structure will be `Status::FLOW_ERROR`.

##### `Status FinishAuthenticationAN(INOUT UserPtr user, const String& pin, const String& accessNumber);`
This method authenticates the end-user using an _Access Number_, provided by a PC/Browser session.
After this authentication, the end-user can log into the PC/Browser which provided the Access Number, while the authentication itself is done on the Mobile Device.
`accessNumber` is the Access Number from the browser session.
The returned status might be:
* `Status::OK` - Successful authentication.
* `Status::INCORRECT_PIN` - The authentication failed because of incorrect PIN. After the 3rd (configurable in the RPS) unsuccessful authentication attempt, the method will still return `Status::INCORRECT_PIN` but the User State will be set to `BLOCKED`.
* `Status::INCORRECT_ACCESS_NUMBER` - The authentication failed because of incorrect Access Number.

##### `bool CanLogout(IN UserPtr user);`
This method is used after authentication with an Access Number through `FinishAuthenticationAN()`.
After such an authentication, the Mobile Device can log out the end-user from the Browser session, if the RPA supports that functionality.
This method checks whether logout information was provided by the RPA and the remote (Browser) session can be terminated from the Mobile Device.
The method will return `true` if the user can be logged-out from the remote session, and `false` otherwise.

##### `bool Logout(IN UserPtr user);`
This method tries to log out the end-user from a remote (Browser) session after a successful authentication through `FinishAuthenticationAN()`.
Before calling this method, it is recommended to ensure that logout data was provided by the RPA and that the logout operation can be actually performed.
The method will return `true` if the logged-out request to the RPA was successful, and `false` otherwise.

##### `const char* GetVersion();`
This method returns a pointer to a null-terminated string with the version number of the SDK.

##### `String GetClientParam(const String& key);`
This method returns the value for a _Client Setting_ with the given key.
The value is returned as a String always, i.e. when a numeric or a boolean value is expected, the conversion should be handled by the application.
Client settings that might interest the applications are:
* `accessNumberDigits` - The number of Access Number digits that should be entered by the user, prior to calling `FinishAuthenticationAN()`.
* `setDeviceName` - Indicator (`true/false`) whether the application should ask the user to insert a _Device Name_ and pass it to the `MakeNewUser()` method.
* `appID` - The _App ID_ used by the backend. The App ID is a unique ID assigned to each customer or application. It is a hex-encoded long numeric value. The App ID can be used only for information purposes and it does not affect the application's behavior in any way.

See also [Apache Milagro - Main Flows](mpinsdk-flows.md)