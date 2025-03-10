// ------------------------------------------------------------------------------------------------
#include "Library/String.hpp"
#include "Core/Utility.hpp"
#include "Core/Buffer.hpp"

// ------------------------------------------------------------------------------------------------
#include <cctype>
#include <cstdlib>
#include <cstring>

// ------------------------------------------------------------------------------------------------
#include <algorithm>

// ------------------------------------------------------------------------------------------------
namespace SqMod {

// ------------------------------------------------------------------------------------------------
static LightObj SqLeftStr(SQChar f, uint32_t w, StackStrF & s)
{
    // Is the specified width valid?
    if (!w)
    {
        return LightObj(_SC(""), 0); // Default to an empty string!
    }
    // Allocate a buffer with the requested width
    Buffer b(w + 1); // + null terminator
    // Is the specified string valid?
    if (!s.mLen)
    {
        std::memset(b.Data(), f, w); // Insert only the fill character
    }
    else
    {
        // Insert only the fill character first
        std::memset(b.Data(), f, w);
        // Overwrite with the specified string
        std::strncpy(b.Data(), s.mPtr, s.mLen);
    }
    // Obtain the initial stack size
    const StackGuard sg(s.mVM);
    // Push the string onto the stack
    sq_pushstring(s.mVM, b.Data(), w);
    // Obtain the object from the stack and return it
    return std::move(Var< LightObj >(s.mVM, -1).value);
}

// ------------------------------------------------------------------------------------------------
static LightObj SqLeftOffsetStr(SQChar f, uint32_t w, uint32_t o, StackStrF & s)
{
    // Is the specified width valid?
    if (!w)
    {
        return LightObj(_SC(""), 0); // Default to an empty string!
    }
    // Is the specified offset within width range?
    else if (o > w)
    {
        STHROWF("Offset is out of bounds");
    }
    // Allocate a buffer with the requested width
    Buffer b(w + 1); // + null terminator
    // Is the specified string valid?
    if (!s.mLen)
    {
        std::memset(b.Data(), f, w); // Insert only the fill character
    }
    else
    {
        // Calculate the string length
        const uint32_t n = ConvTo< uint32_t >::From(s.mLen);
        // Insert the fill character first
        std::memset(b.Data(), f, w);
        // Overwrite with the specified string
        if (n > (w - o))
        {
            std::strncpy(b.Data() + o, s.mPtr, n);
        }
        else
        {
            std::memcpy(b.Data() + o, s.mPtr, n * sizeof(SQChar));
        }
    }
    // Obtain the initial stack size
    const StackGuard sg(s.mVM);
    // Push the string onto the stack
    sq_pushstring(s.mVM, b.Data(), w);
    // Obtain the object from the stack and return it
    return std::move(Var< LightObj >(s.mVM, -1).value);
}

// ------------------------------------------------------------------------------------------------
static LightObj SqRightStr(SQChar f, uint32_t w, StackStrF & s)
{
    // Is the specified width valid?
    if (!w)
    {
        return LightObj(_SC(""), 0); // Default to an empty string!
    }
    // Allocate a buffer with the requested width
    Buffer b(w + 1); // + null terminator
    // Is the specified string valid?
    if (!s.mLen)
    {
        std::memset(b.Data(), f, w); // Insert only the fill character
    }
    else
    {
        // Calculate the string length
        const uint32_t n = ConvTo< uint32_t >::From(s.mLen);
        // Insert the fill character first
        std::memset(b.Data(), f, w);
        // Overwrite with the specified string
        if (n >= w)
        {
            std::strncpy(b.Data(), s.mPtr, w);
        }
        else
        {
            std::strncpy(b.Data() + (w - n), s.mPtr, n);
        }
    }
    // Obtain the initial stack size
    const StackGuard sg(s.mVM);
    // Push the string onto the stack
    sq_pushstring(s.mVM, b.Data(), w);
    // Obtain the object from the stack and return it
    return std::move(Var< LightObj >(s.mVM, -1).value);
}

// ------------------------------------------------------------------------------------------------
static LightObj SqRightOffsetStr(SQChar f, uint32_t w, uint32_t o, StackStrF & s)
{
    // Is the specified width valid?
    if (!w)
    {
        return LightObj(_SC(""), 0); // Default to an empty string!
    }
    // Is the specified offset within width range?
    else if (o > w)
    {
        STHROWF("Offset is out of bounds");
    }
    // Allocate a buffer with the requested width
    Buffer b(w + 1); // + null terminator
    // Is the specified string valid?
    if (!s.mLen)
    {
        std::memset(b.Data(), f, w); // Insert only the fill character
    }
    else
    {
        // Calculate the string length
        const uint32_t n = ConvTo< uint32_t >::From(s.mLen);
        // Insert the fill character first
        std::memset(b.Data(), f, w);
        // Overwrite with the specified string
        if (n >= w || (n + o) >= w)
        {
            std::strncpy(b.Data(), s.mPtr, w - o);
        }
        else
        {
            std::strncpy(b.Data() + ((w - n) - o), s.mPtr, n);
        }
    }
    // Obtain the initial stack size
    const StackGuard sg(s.mVM);
    // Push the string onto the stack
    sq_pushstring(s.mVM, b.Data(), w);
    // Obtain the object from the stack and return it
    return std::move(Var< LightObj >(s.mVM, -1).value);
}

// ------------------------------------------------------------------------------------------------
static LightObj SqCenterStr(SQChar f, uint32_t w, StackStrF & s)
{
    // Is the specified width valid?
    if (!w)
    {
        return LightObj(_SC(""), 0); // Default to an empty string!
    }
    // Allocate a buffer with the requested width
    Buffer b(w + 1); // + null terminator
    // Is the specified string valid?
    if (!s.mLen)
    {
        std::memset(b.Data(), f, w); // Insert only the fill character
    }
    else
    {
        // Calculate the insert position
        const auto p = ((w/2) - (s.mLen/2));
        // Insert only the fill character first
        std::memset(b.Data(), f, w);
        // Overwrite with the specified string
        std::strncpy(b.Data() + (p < 0 ? 0 : p), s.mPtr, s.mLen);
    }
    // Obtain the initial stack size
    const StackGuard sg(s.mVM);
    // Push the string onto the stack
    sq_pushstring(s.mVM, b.Data(), w);
    // Obtain the object from the stack and return it
    return std::move(Var< LightObj >(s.mVM, -1).value);
}

// ------------------------------------------------------------------------------------------------
static Buffer StrJustAlphaNumImpl(const SQChar * str, uint32_t len)
{
    // See if we actually have something to search for
    if(!len)
    {
        return Buffer(); // Default to an empty buffer!
    }
    // Obtain a temporary buffer
    Buffer b(len + 1); // + null terminator
    // Resulted string size
    uint32_t n = 0;
    // Currently processed character
    SQChar c;
    // Process characters
    while ((c = *(str++)) != '\0')
    {
        // Is this an alpha-numeric character?
        if (std::isalnum(c) != 0)
        {
            // Save it and move to the next one
            b.At(n++) = c;
        }
    }
    // End the resulted string
    b.At(n) = '\0';
    // Move the cursor to the end
    b.Move(n);
    // Return ownership of the buffer
    return b;
}

// ------------------------------------------------------------------------------------------------
Buffer StrJustAlphaNumB(const SQChar * str)
{
    // See if we actually have something to search for
    if(!str || *str == '\0')
    {
        return Buffer(); // Default to an empty string!
    }
    // Attempt to convert and return the result
    return StrJustAlphaNumImpl(str, std::strlen(str));
}

// ------------------------------------------------------------------------------------------------
const SQChar * StrJustAlphaNum(const SQChar * str)
{
    // See if we actually have something to search for
    if(!str || *str == '\0')
    {
        return _SC(""); // Default to an empty string!
    }
    // Attempt to convert and return the result
    return StrJustAlphaNumImpl(str, std::strlen(str)).Get< SQChar >();
}

// ------------------------------------------------------------------------------------------------
static LightObj SqJustAlphaNum(StackStrF & s)
{
    const Buffer b(StrJustAlphaNumImpl(s.mPtr, s.mLen));
    // Obtain the initial stack size
    const StackGuard sg(s.mVM);
    // Push the string onto the stack
    sq_pushstring(s.mVM, b.Data(), b.Position());
    // Obtain the object from the stack and return it
    return std::move(Var< LightObj >(s.mVM, -1).value);
}

// ------------------------------------------------------------------------------------------------
static Buffer StrToLowercaseImpl(const SQChar * str, uint32_t len)
{
    // See if we actually have something to search for
    if(!len)
    {
        return Buffer(); // Default to an empty buffer!
    }
    // Obtain a temporary buffer
    Buffer b(len + 1); // + null terminator
    // Resulted string size
    uint32_t n = 0;
    // Process characters
    while (*str != '\0')
    {
        // Convert it and move to the next one
        b.At(n++) = static_cast< char >(std::tolower(*(str++)));
    }
    // End the resulted string
    b.At(n) = '\0';
    // Move the cursor to the end
    b.Move(n);
    // Return ownership of the buffer
    return b;
}

// ------------------------------------------------------------------------------------------------
Buffer StrToLowercaseB(const SQChar * str)
{
    // See if we actually have something to search for
    if(!str || *str == '\0')
    {
        return Buffer(); // Default to an empty string!
    }
    // Attempt to convert and return the result
    return StrToLowercaseImpl(str, std::strlen(str));
}

// ------------------------------------------------------------------------------------------------
const SQChar * StrToLowercase(const SQChar * str)
{
    // See if we actually have something to search for
    if(!str || *str == '\0')
    {
        return _SC(""); // Default to an empty string!
    }
    // Attempt to convert and return the result
    return StrToLowercaseImpl(str, std::strlen(str)).Get< SQChar >();
}

// ------------------------------------------------------------------------------------------------
static LightObj SqToLowercase(StackStrF & s)
{
    const Buffer b(StrToLowercaseImpl(s.mPtr, s.mLen));
    // Obtain the initial stack size
    const StackGuard sg(s.mVM);
    // Push the string onto the stack
    sq_pushstring(s.mVM, b.Data(), b.Position());
    // Obtain the object from the stack and return it
    return std::move(Var< LightObj >(s.mVM, -1).value);
}

// ------------------------------------------------------------------------------------------------
static Buffer StrToUppercaseImpl(const SQChar * str, uint32_t len)
{
    // See if we actually have something to search for
    if(!len)
    {
        return Buffer(); // Default to an empty buffer!
    }
    // Obtain a temporary buffer
    Buffer b(len + 1); // + null terminator
    // Resulted string size
    uint32_t n = 0;
    // Process characters
    while (*str != '\0')
    {
        // Convert it and move to the next one
        b.At(n++) = static_cast< char >(std::toupper(*(str++)));
    }
    // End the resulted string
    b.At(n) = '\0';
    // Move the cursor to the end
    b.Move(n);
    // Return ownership of the buffer
    return b;
}

// ------------------------------------------------------------------------------------------------
Buffer StrToUppercaseB(const SQChar * str)
{
    // See if we actually have something to search for
    if(!str || *str == '\0')
    {
        return Buffer(); // Default to an empty string!
    }
    // Attempt to convert and return the result
    return StrToUppercaseImpl(str, std::strlen(str));
}

// ------------------------------------------------------------------------------------------------
const SQChar * StrToUppercase(const SQChar * str)
{
    // See if we actually have something to search for
    if(!str || *str == '\0')
    {
        return _SC(""); // Default to an empty string!
    }
    // Attempt to convert and return the result
    return StrToUppercaseImpl(str, std::strlen(str)).Get< SQChar >();
}

// ------------------------------------------------------------------------------------------------
static LightObj SqToUppercase(StackStrF & s)
{
    const Buffer b(StrToUppercaseImpl(s.mPtr, s.mLen));
    // Obtain the initial stack size
    const StackGuard sg(s.mVM);
    // Push the string onto the stack
    sq_pushstring(s.mVM, b.Data(), b.Position());
    // Obtain the object from the stack and return it
    return std::move(Var< LightObj >(s.mVM, -1).value);
}

/* ------------------------------------------------------------------------------------------------
 * Checks if all the characters in the specified string are of the specified class or not.
*/
template < int (*Fn)(int) > static bool SqAllChars(StackStrF & s)
{
    // See if we actually have something to search for
    if (!s.mLen)
    {
        // Since there are no other different character then
        // we count this as all characters being of this type
        return true;
    }
    // Start iterating characters and find the first that doesn't match
    for (const SQChar * itr = s.mPtr; *itr != '\0'; ++itr)
    {
        // Call the predicate with the current character
        if (Fn(*itr) == 0)
        {
            return false; // This character did not pass the test
        }
    }
    // All characters passed the test
    return true;
}

/* ------------------------------------------------------------------------------------------------
 * Find the position of the first character that matches the specified class.
*/
template < int (*Fn)(int), bool Neg > static LightObj SqFirstChar(StackStrF & s)
{
    // See if we actually have something to search for
    if (s.mLen)
    {
        // Start iterating characters and find the first that doesn't match
        for (const SQChar * itr = s.mPtr; *itr != '\0'; ++itr)
        {
            // Call the predicate with the current character
            if ((Fn(*itr) == 0) == Neg)
            {
                // Obtain the initial stack size
                const StackGuard sg(s.mVM);
                // This character passed our test, push it's position
                sq_pushinteger(s.mVM, itr - s.mPtr);
                // Obtain the object from the stack and return it
                return std::move(Var< LightObj >(s.mVM, -1).value);
            }
        }
    }
    // Since there are no other different character then
    // we count this as all characters being of this type
    return LightObj();
}

/* ------------------------------------------------------------------------------------------------
 * Find the position of the last character that matches the specified class.
*/
template < int (*Fn)(int), bool Neg > static LightObj SqLastChar(StackStrF & s)
{
    // See if we actually have something to search for
    if (s.mLen)
    {
        // Start iterating characters and find the first that doesn't match
        for (const SQChar * itr = (s.mPtr + s.mLen) - 1; itr >= s.mPtr; --itr)
        {
            // Call the predicate with the current character
            if ((Fn(*itr) == 0) == Neg)
            {
                // Obtain the initial stack size
                const StackGuard sg(s.mVM);
                // This character passed our test, push it's position
                sq_pushinteger(s.mVM, itr - s.mPtr);
                // Obtain the object from the stack and return it
                return std::move(Var< LightObj >(s.mVM, -1).value);
            }
        }
    }
    // Since there are no other different character then
    // we count this as all characters being of this type
    return LightObj();
}

/* ------------------------------------------------------------------------------------------------
 * Split the string into chunks wherever a character matches or not the specified class.
*/
static SQInteger SplitWhereCharImpl(HSQUIRRELVM vm, int(*fn)(int), bool neg)
{
    static const SQInteger top = sq_gettop(vm);
    // Is there a value to process?
    if (top <= 1)
    {
        return sq_throwerror(vm, "Missing string or value");
    }

    // Attempt to retrieve the value from the stack as a string
    StackStrF val(vm, 2);
    // Have we failed to retrieve the string?
    if (SQ_FAILED(val.Proc(true)))
    {
        return val.mRes; // Propagate the error!
    }

    // Create a new array on the stack
    sq_newarray(vm, 0);

    // See if we actually have something to search for
    if (!(val.mPtr) || *(val.mPtr) == '\0')
    {
        return 1; // Return an empty array
    }

    // Remember the position of the last found match
    const SQChar * last = val.mPtr;

    // Start iterating characters and slice where a match is found
    for (const SQChar * itr = val.mPtr; *itr != '\0'; ++itr)
    {
        // Call the predicate with the current character
        if ((fn(*itr) == 0) == neg)
        {
            // Are there any characters before this match?
            if ((itr - last) > 0 && (*last != '\0'))
            {
                // Push this chunk of string on the stack
                sq_pushstring(vm, last, itr - last);
                // Insert this element into the array on the stack
                const SQRESULT r = sq_arrayappend(vm, -2);
                // Did we fail to append the element?
                if (SQ_FAILED(r))
                {
                    return r; // We're done here
                }
            }
            // Push this character as an integer on the stack
            sq_pushinteger(vm, *itr);
            // Insert this element into the array on the stack
            const SQRESULT r = sq_arrayappend(vm, -2);
            // Did we fail to append the element?
            if (SQ_FAILED(r))
            {
                return r; // We're done here
            }
            // Update the position of the last found match
            last = (itr + 1);
        }
    }

    // Push the remaining chunk, if any
    if (*last != '\0')
    {
        // Push this chunk of string on the stack
        sq_pushstring(vm, last, -1);
        // Insert this element into the array on the stack
        const SQRESULT r = sq_arrayappend(vm, -2);
        // Did we fail to append the element?
        if (SQ_FAILED(r))
        {
            return r; // We're done here
        }
    }

    // We have a value to return on the stack
    return 1;
}

/* ------------------------------------------------------------------------------------------------
 * Simple forwarder to minimize templated functions with large body and reduce executable size.
*/
template < int (*Fn)(int), bool Neg > static SQInteger SplitWhereCharProxy(HSQUIRRELVM vm)
{
    return SplitWhereCharImpl(vm, Fn, Neg);
}

/* ------------------------------------------------------------------------------------------------
 * Checks if the specified character is of the specified class.
*/
template < int (*Fn)(int) > static bool IsCharOfType(int c)
{
    return (Fn(c) != 0);
}

// ------------------------------------------------------------------------------------------------
static bool OnlyDelimiter(const SQChar * str, SQChar chr)
{
    while (*str != '\0')
    {
        // Is this different from the delimiter?
        if (*(str++) != chr)
        {
            // Another character was found
            return false;
        }
    }
    // No other character was found
    return true;
}

// ------------------------------------------------------------------------------------------------
static SQInteger SqStrExplode(HSQUIRRELVM vm)
{
    const auto top = sq_gettop(vm);
    // Was the delimiter character specified?
    if (top <= 1)
    {
        return sq_throwerror(vm, _SC("Missing delimiter character"));
    }
    // Was the boolean empty specified?
    else if (top <= 2)
    {
        return sq_throwerror(vm, _SC("Missing boolean empty"));
    }
    // Was the string value specified?
    else if (top <= 3)
    {
        return sq_throwerror(vm, _SC("Missing string value"));
    }
    // Attempt to generate the string value
    StackStrF val(vm, 4);
    // Have we failed to retrieve the string?
    if (SQ_FAILED(val.Proc(true)))
    {
        return val.mRes; // Propagate the error!
    }
    // The delimiter character and boolean empty
    SQChar delim;
    bool empty;
    // Attempt to retrieve the remaining arguments from the stack
    try
    {
        delim = Var< SQChar >(vm, 2).value;
        empty = Var< bool >(vm, 3).value;
    }
    catch (const std::exception & e)
    {
        return sq_throwerror(vm, e.what());
    }
    catch (...)
    {
        return sq_throwerror(vm, _SC("Unable to retrieve arguments"));
    }
    // Grab the string value to a shorter name
    const SQChar * str = val.mPtr;
    // Create an empty array on the stack
    sq_newarray(vm, 0);
    // See if we actually have something to explode
    if(!str || *str == '\0')
    {
        // Specify that we have an argument on the stack
        return 1;
    }
    // Don't modify the specified string pointer
    const SQChar * itr = str, * last = str;
    // The number of delimiter occurrences
    uint32_t num = 0;
    // Pre-count how many delimiters of this type exist
    while (*itr != '\0')
    {
        // Is this our delimiter?
        if (*(itr++) == delim)
        {
            // Are we allowed to include empty elements?
            if (empty || (itr - last) > 1)
            {
                // Increase the count
                ++num;
            }
            // Update the last delimiter position
            last = itr;
        }
    }
    // Were there no delimiters found and can we include empty elements?
    if (num == 0 && !empty && (str[1] == '\0' || OnlyDelimiter(str, delim)))
    {
        // Specify that we have an argument on the stack
        return 1;
    }
    // Have we found any delimiters?
    else if (num == 0)
    {
        // Test against strings with only delimiters
        if (str[1] == '\0' || OnlyDelimiter(str, delim))
        {
            sq_pushstring(vm, _SC(""), 0); // Add an empty string
        }
        else
        {
            sq_pushstring(vm, val.mPtr, val.mLen); // Add the whole string
        }
        // Append the string on the stack to the array
        const SQRESULT r = sq_arrayappend(vm, -2);
        // Check the result
        if (SQ_FAILED(r))
        {
            return r; // Propagate the error
        }
        // Specify that we have an argument on the stack
        return 1;
    }
    // Is there anything after the last delimiter?
    if (itr != last && *last != delim)
    {
        ++num; // Add it to the counter
    }
    SQRESULT r = SQ_OK;
    // Pre-allocate an array with the number of found delimiters
    r = sq_arrayresize(vm, -1, num);
    // Check the result
    if (SQ_FAILED(r))
    {
        return r; // Propagate the error
    }
    // Don't modify the specified string pointer
    itr = str, last = str;
    // Reset the counter and use it as the element index
    num = 0;
    // Process the string again, this time slicing the actual elements
    while (*itr != '\0')
    {
        // Is this our delimiter?
        if (*itr++ == delim)
        {
            // Are we allowed to include empty elements?
            if (empty || (itr - last) > 1)
            {
                // Push the element index on the stack and advance to the next one
                sq_pushinteger(vm, num++);
                // Push the string portion on the stack
                sq_pushstring(vm, last, itr - last - 1);
                // Assign the string onto the
                r = sq_set(vm, -3);
                // Check the result
                if (SQ_FAILED(r))
                {
                    return r; // Propagate the error
                }
            }
            // Update the last delimiter position
            last = itr;
        }
    }
    // Is there anything after the last delimiter?
    if (itr != last && *last != delim)
    {
        // Push the element index on the stack
        sq_pushinteger(vm, num);
        // Add the remaining string as an element
        sq_pushstring(vm, last, itr - last);
        // Assign the string onto the
        r = sq_set(vm, -3);
        // Check the result
        if (SQ_FAILED(r))
        {
            return r; // Propagate the error
        }
    }
    // Specify that we have an argument on the stack
    return 1;
}

// ------------------------------------------------------------------------------------------------
static String StrImplode(StackStrF & sep, Array & arr)
{
    // String buffer
    String buf;
    // Determine array size
    const auto length = static_cast< size_t >(arr.Length());
    // Is there anything to implode?
    if (length <= 0)
    {
        return buf; // Default to an empty string
    }
    // Estimate length
    buf.reserve(length * 32);
    // Process array elements
    const SQRESULT res = arr.Foreach([&](HSQUIRRELVM vm, SQInteger i) -> SQRESULT {
        StackStrF str(vm, -1);
        // Retrieve the element value as string
        if (SQ_FAILED(str.Proc(false)))
        {
            return str.mRes; // Failed!
        }
        // Append the value to the buffer
        buf.append(str.mPtr, static_cast< size_t >(str.mLen));
        // Append the delimiter
        buf.append(sep.mPtr, static_cast< size_t >(sep.mLen));
        // Successfully processed
        return SQ_OK;
    });
    // Succeeded?
    if (SQ_FAILED(res))
    {
        buf.clear(); // NO!
    }
    else
    {
        for (SQInteger n = 0; n < sep.mLen; ++n)
        {
            buf.pop_back(); // Remove trailing separator
        }
    }
    // Return the string
    return buf;
}

// ------------------------------------------------------------------------------------------------
static String StrImplodeChar(SQChar chr, Array & arr)
{
    // String buffer
    String buf;
    // Determine array size
    const auto length = static_cast< size_t >(arr.Length());
    // Is there anything to implode?
    if (length <= 0)
    {
        return buf; // Default to an empty string
    }
    // Estimate length
    buf.reserve(length * 32);
    // Process array elements
    const SQRESULT res = arr.Foreach([&](HSQUIRRELVM vm, SQInteger i) -> SQRESULT {
        StackStrF str(vm, -1);
        // Retrieve the element value as string
        if (SQ_FAILED(str.Proc(false)))
        {
            return str.mRes; // Failed!
        }
        // Append the value to the buffer
        buf.append(str.mPtr, static_cast< size_t >(str.mLen));
        // Append the delimiter
        buf.push_back(chr);
        // Successfully processed
        return SQ_OK;
    });
    // Succeeded?
    if (SQ_FAILED(res))
    {
        buf.clear(); // NO!
    }
    else
    {
        buf.pop_back(); // Remove trailing separator
    }
    // Return the string
    return buf;
}

// ------------------------------------------------------------------------------------------------
static const SQChar * FromArray(Array & arr)
{
    // Determine array size
    const int32_t length = ConvTo< int32_t >::From(arr.Length());
    // Obtain a temporary buffer
    Buffer b(length * sizeof(int32_t));
    // Get array elements as integers
    arr.GetArray< int32_t >(b.Get< int32_t >(), length);
    // Overwrite integers with characters
    for (int32_t n = 0; n < length; ++n)
    {
        b.At(n) = ConvTo< SQChar >::From(b.At< int32_t >(n * sizeof(int32_t)));
    }
    // Terminate the resulted string
    b.At(length) = '\0';
    // Return the string
    return b.Get< SQChar >();
}

// ------------------------------------------------------------------------------------------------
static SQInteger StdPrintF(HSQUIRRELVM vm)
{
    // Attempt to retrieve the value from the stack as a string
    StackStrF val(vm, 2);
    // Have we failed to retrieve the string?
    if (SQ_FAILED(val.Proc(true)))
    {
        return val.mRes; // Propagate the error!
    }
    // Send the resulted string to console as a user message
    LogUsr("%s", val.mPtr);
    // This function doesn't return anything
    return 0;
}

// ------------------------------------------------------------------------------------------------
static String StrCharacterSwap(SQInteger a, SQInteger b, StackStrF & val)
{
    // Have we failed to retrieve the string?
    if (SQ_FAILED(val.Proc(true)))
    {
        STHROWLASTF("Invalid string");
    }
    // Is the string empty?
    else if (!val.mLen)
    {
        return String();
    }
    // Turn it into a string that we can edit
    String str(val.mPtr, val.mLen);
    // Replace all occurrences of the specified character
    std::replace(str.begin(), str.end(),
                    static_cast< String::value_type >(a), static_cast< String::value_type >(b));
    // Return the new string
    return str;
}

// ------------------------------------------------------------------------------------------------
// Returns a size_t, depicting the difference between `a` and `b`. See: https://github.com/wooorm/levenshtein.c
// See <https://en.wikipedia.org/wiki/Levenshtein_distance> for more information.
SQMOD_NODISCARD static size_t Levenshtein_n(const char * a, const size_t a_length, const char * b, const size_t b_length) noexcept
{
    // Shortcut optimizations / degenerate cases.
    if (a == b)
    {
        return 0;
    }
    else if (a_length == 0)
    {
        return b_length;
    }
    else if (b_length == 0)
    {
        return a_length;
    }
    auto cache = reinterpret_cast< size_t * >(calloc(a_length, sizeof(size_t)));
    size_t index = 0;
    size_t b_index = 0;
    size_t distance;
    size_t b_distance;
    size_t result;
    char code;
    // initialize the vector.
    while (index < a_length)
    {
        cache[index] = index + 1;
        index++;
    }
    // Loop
    while (b_index < b_length)
    {
        code = b[b_index];
        result = distance = b_index++;
        index = SIZE_MAX;

        while (++index < a_length)
        {
            b_distance = code == a[index] ? distance : distance + 1;
            distance = cache[index];

            cache[index] = result = distance > result
                ? b_distance > result
                    ? result + 1
                    : b_distance
                : b_distance > distance
                    ? distance + 1
                    : b_distance;
        }
    }
    free(cache);
    return result;
}

// ------------------------------------------------------------------------------------------------
static SQInteger SqLevenshtein(StackStrF & a, StackStrF & b)
{
    return static_cast< SQInteger >(Levenshtein_n(a.mPtr, static_cast< size_t >(a.mLen), b.mPtr, static_cast< size_t >(b.mLen)));
}

// ------------------------------------------------------------------------------------------------
static SQInteger SqStrToI(SQInteger base, StackStrF & s)
{
#ifdef _SQ64
	return std::stoll(s.mPtr, nullptr, ConvTo< int >::From((base)));
#else
	return std::stoi(s.mPtr, nullptr, ConvTo< int >::From((base)));
#endif
}

// ------------------------------------------------------------------------------------------------
static SQFloat SqStrToF(StackStrF & s)
{
#ifdef SQUSEDOUBLE
	return std::strtod(s.mPtr, nullptr);
#else
	return std::strtof(s.mPtr, nullptr);
#endif
}

// ================================================================================================
void Register_String(HSQUIRRELVM vm)
{
    Table strns(vm);

    strns.Func(_SC("FromArray"), &FromArray)
    .SquirrelFunc(_SC("Explode"), &SqStrExplode)
    .Func(_SC("Implode"), &StrImplode)
    .Func(_SC("ImplodeChar"), &StrImplodeChar)
    .FmtFunc(_SC("Center"), &SqCenterStr)
    .FmtFunc(_SC("Left"), &SqLeftStr)
    .FmtFunc(_SC("Right"), &SqRightStr)
    .FmtFunc(_SC("LeftEx"), &SqLeftOffsetStr)
    .FmtFunc(_SC("RightEx"), &SqRightOffsetStr)
    .FmtFunc(_SC("ToLower"), &SqToLowercase)
    .FmtFunc(_SC("ToUpper"), &SqToUppercase)
    .FmtFunc(_SC("CharSwap"), &StrCharacterSwap)
    .FmtFunc(_SC("Lowercase"), &SqToLowercase)
    .FmtFunc(_SC("Uppercase"), &SqToUppercase)
    .FmtFunc(_SC("JustAlnum"), &SqJustAlphaNum)
    .FmtFunc(_SC("ToInt"), &SqStrToI)
    .FmtFunc(_SC("ToFloat"), &SqStrToF)
    .FmtFunc(_SC("Levenshtein"), &SqLevenshtein)
    .FmtFunc(_SC("AreAllSpace"), &SqAllChars< std::isspace >)
    .FmtFunc(_SC("AreAllPrint"), &SqAllChars< std::isprint >)
    .FmtFunc(_SC("AreAllCntrl"), &SqAllChars< std::iscntrl >)
    .FmtFunc(_SC("AreAllUpper"), &SqAllChars< std::isupper >)
    .FmtFunc(_SC("AreAllLower"), &SqAllChars< std::islower >)
    .FmtFunc(_SC("AreAllAlpha"), &SqAllChars< std::isalpha >)
    .FmtFunc(_SC("AreAllDigit"), &SqAllChars< std::isdigit >)
    .FmtFunc(_SC("AreAllPunct"), &SqAllChars< std::ispunct >)
    .FmtFunc(_SC("AreAllXdigit"), &SqAllChars< std::isxdigit >)
    .FmtFunc(_SC("AreAllAlnum"), &SqAllChars< std::isalnum >)
    .FmtFunc(_SC("AreAllGraph"), &SqAllChars< std::isgraph >)
    .FmtFunc(_SC("AreAllBlank"), &SqAllChars< std::isblank >)
    .FmtFunc(_SC("FirstSpace"), &SqFirstChar< std::isspace, false >)
    .FmtFunc(_SC("FirstPrint"), &SqFirstChar< std::isprint, false >)
    .FmtFunc(_SC("FirstCntrl"), &SqFirstChar< std::iscntrl, false >)
    .FmtFunc(_SC("FirstUpper"), &SqFirstChar< std::isupper, false >)
    .FmtFunc(_SC("FirstLower"), &SqFirstChar< std::islower, false >)
    .FmtFunc(_SC("FirstAlpha"), &SqFirstChar< std::isalpha, false >)
    .FmtFunc(_SC("FirstDigit"), &SqFirstChar< std::isdigit, false >)
    .FmtFunc(_SC("FirstPunct"), &SqFirstChar< std::ispunct, false >)
    .FmtFunc(_SC("FirstXdigit"), &SqFirstChar< std::isxdigit, false >)
    .FmtFunc(_SC("FirstAlnum"), &SqFirstChar< std::isalnum, false >)
    .FmtFunc(_SC("FirstGraph"), &SqFirstChar< std::isgraph, false >)
    .FmtFunc(_SC("FirstBlank"), &SqFirstChar< std::isblank, false >)
    .FmtFunc(_SC("FirstNotSpace"), &SqFirstChar< std::isspace, true >)
    .FmtFunc(_SC("FirstNotPrint"), &SqFirstChar< std::isprint, true >)
    .FmtFunc(_SC("FirstNotCntrl"), &SqFirstChar< std::iscntrl, true >)
    .FmtFunc(_SC("FirstNotUpper"), &SqFirstChar< std::isupper, true >)
    .FmtFunc(_SC("FirstNotLower"), &SqFirstChar< std::islower, true >)
    .FmtFunc(_SC("FirstNotAlpha"), &SqFirstChar< std::isalpha, true >)
    .FmtFunc(_SC("FirstNotDigit"), &SqFirstChar< std::isdigit, true >)
    .FmtFunc(_SC("FirstNotPunct"), &SqFirstChar< std::ispunct, true >)
    .FmtFunc(_SC("FirstNotXdigit"), &SqFirstChar< std::isxdigit, true >)
    .FmtFunc(_SC("FirstNotAlnum"), &SqFirstChar< std::isalnum, true >)
    .FmtFunc(_SC("FirstNotGraph"), &SqFirstChar< std::isgraph, true >)
    .FmtFunc(_SC("FirstNotBlank"), &SqFirstChar< std::isblank, true >)
    .FmtFunc(_SC("LastSpace"), &SqLastChar< std::isspace, false >)
    .FmtFunc(_SC("LastPrint"), &SqLastChar< std::isprint, false >)
    .FmtFunc(_SC("LastCntrl"), &SqLastChar< std::iscntrl, false >)
    .FmtFunc(_SC("LastUpper"), &SqLastChar< std::isupper, false >)
    .FmtFunc(_SC("LastLower"), &SqLastChar< std::islower, false >)
    .FmtFunc(_SC("LastAlpha"), &SqLastChar< std::isalpha, false >)
    .FmtFunc(_SC("LastDigit"), &SqLastChar< std::isdigit, false >)
    .FmtFunc(_SC("LastPunct"), &SqLastChar< std::ispunct, false >)
    .FmtFunc(_SC("LastXdigit"), &SqLastChar< std::isxdigit, false >)
    .FmtFunc(_SC("LastAlnum"), &SqLastChar< std::isalnum, false >)
    .FmtFunc(_SC("LastGraph"), &SqLastChar< std::isgraph, false >)
    .FmtFunc(_SC("LastBlank"), &SqLastChar< std::isblank, false >)
    .FmtFunc(_SC("LastNotSpace"), &SqLastChar< std::isspace, true >)
    .FmtFunc(_SC("LastNotPrint"), &SqLastChar< std::isprint, true >)
    .FmtFunc(_SC("LastNotCntrl"), &SqLastChar< std::iscntrl, true >)
    .FmtFunc(_SC("LastNotUpper"), &SqLastChar< std::isupper, true >)
    .FmtFunc(_SC("LastNotLower"), &SqLastChar< std::islower, true >)
    .FmtFunc(_SC("LastNotAlpha"), &SqLastChar< std::isalpha, true >)
    .FmtFunc(_SC("LastNotDigit"), &SqLastChar< std::isdigit, true >)
    .FmtFunc(_SC("LastNotPunct"), &SqLastChar< std::ispunct, true >)
    .FmtFunc(_SC("LastNotXdigit"), &SqLastChar< std::isxdigit, true >)
    .FmtFunc(_SC("LastNotAlnum"), &SqLastChar< std::isalnum, true >)
    .FmtFunc(_SC("LastNotGraph"), &SqLastChar< std::isgraph, true >)
    .FmtFunc(_SC("LastNotBlank"), &SqLastChar< std::isblank, true >)
    .SquirrelFunc(_SC("SplitWhereSpace"), &SplitWhereCharProxy< std::isspace, false >)
    .SquirrelFunc(_SC("SplitWherePrint"), &SplitWhereCharProxy< std::isprint, false >)
    .SquirrelFunc(_SC("SplitWhereCntrl"), &SplitWhereCharProxy< std::iscntrl, false >)
    .SquirrelFunc(_SC("SplitWhereUpper"), &SplitWhereCharProxy< std::isupper, false >)
    .SquirrelFunc(_SC("SplitWhereLower"), &SplitWhereCharProxy< std::islower, false >)
    .SquirrelFunc(_SC("SplitWhereAlpha"), &SplitWhereCharProxy< std::isalpha, false >)
    .SquirrelFunc(_SC("SplitWhereDigit"), &SplitWhereCharProxy< std::isdigit, false >)
    .SquirrelFunc(_SC("SplitWherePunct"), &SplitWhereCharProxy< std::ispunct, false >)
    .SquirrelFunc(_SC("SplitWhereXdigit"), &SplitWhereCharProxy< std::isxdigit, false >)
    .SquirrelFunc(_SC("SplitWhereAlnum"), &SplitWhereCharProxy< std::isalnum, false >)
    .SquirrelFunc(_SC("SplitWhereGraph"), &SplitWhereCharProxy< std::isgraph, false >)
    .SquirrelFunc(_SC("SplitWhereBlank"), &SplitWhereCharProxy< std::isblank, false >)
    .SquirrelFunc(_SC("SplitWhereNotSpace"), &SplitWhereCharProxy< std::isspace, true >)
    .SquirrelFunc(_SC("SplitWhereNotPrint"), &SplitWhereCharProxy< std::isprint, true >)
    .SquirrelFunc(_SC("SplitWhereNotCntrl"), &SplitWhereCharProxy< std::iscntrl, true >)
    .SquirrelFunc(_SC("SplitWhereNotUpper"), &SplitWhereCharProxy< std::isupper, true >)
    .SquirrelFunc(_SC("SplitWhereNotLower"), &SplitWhereCharProxy< std::islower, true >)
    .SquirrelFunc(_SC("SplitWhereNotAlpha"), &SplitWhereCharProxy< std::isalpha, true >)
    .SquirrelFunc(_SC("SplitWhereNotDigit"), &SplitWhereCharProxy< std::isdigit, true >)
    .SquirrelFunc(_SC("SplitWhereNotPunct"), &SplitWhereCharProxy< std::ispunct, true >)
    .SquirrelFunc(_SC("SplitWhereNotXdigit"), &SplitWhereCharProxy< std::isxdigit, true >)
    .SquirrelFunc(_SC("SplitWhereNotAlnum"), &SplitWhereCharProxy< std::isalnum, true >)
    .SquirrelFunc(_SC("SplitWhereNotGraph"), &SplitWhereCharProxy< std::isgraph, true >)
    .SquirrelFunc(_SC("SplitWhereNotBlank"), &SplitWhereCharProxy< std::isblank, true >);

    RootTable(vm).Bind(_SC("SqStr"), strns);
    RootTable(vm).SquirrelFunc(_SC("printf"), &StdPrintF);

    RootTable(vm)
    .Func(_SC("IsSpace"), &IsCharOfType< std::isspace >)
    .Func(_SC("IsPrint"), &IsCharOfType< std::isprint >)
    .Func(_SC("IsCntrl"), &IsCharOfType< std::iscntrl >)
    .Func(_SC("IsUpper"), &IsCharOfType< std::isupper >)
    .Func(_SC("IsLower"), &IsCharOfType< std::islower >)
    .Func(_SC("IsAlpha"), &IsCharOfType< std::isalpha >)
    .Func(_SC("IsDigit"), &IsCharOfType< std::isdigit >)
    .Func(_SC("IsPunct"), &IsCharOfType< std::ispunct >)
    .Func(_SC("IsXdigit"), &IsCharOfType< std::isxdigit >)
    .Func(_SC("IsAlnum"), &IsCharOfType< std::isalnum >)
    .Func(_SC("IsGraph"), &IsCharOfType< std::isgraph >)
    .Func(_SC("IsBlank"), &IsCharOfType< std::isblank >)
    .Func(_SC("ToLower"), &tolower)
    .Func(_SC("ToUpper"), &toupper);
}

} // Namespace:: SqMod
