//
// SqratClass: Class Binding
//

//
// Copyright (c) 2009 Brandon Jones
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source
//    distribution.
//

#pragma once

#include <squirrelex.h>

#include <typeinfo>

#include "sqratObject.h"
#include "sqratClassType.h"
#include "sqratMemberMethods.h"
#include "sqratAllocator.h"
#include "sqratTypes.h"

namespace Sqrat
{

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Facilitates exposing a C++ class with no base class to Squirrel
///
/// \tparam C Class type to expose
/// \tparam A An allocator to use when instantiating and destroying class instances of this type in Squirrel
///
/// \remarks
/// DefaultAllocator is used if no allocator is specified. This should be sufficent for most classes,
/// but if specific behavior is desired, it can be overridden. If the class should not be instantiated from
/// Squirrel the NoConstructor allocator may be used. See NoCopy and CopyOnly too.
///
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class C, class A = DefaultAllocator<C> >
class Class : public Object
{
private:

    static SQInteger cleanup_hook(SQUserPointer ptr, SQInteger size) {
        SQUNUSED(size);
        auto** ud = reinterpret_cast<ClassData<C>**>(ptr);
        delete *ud;
        return 0;
    }

public:

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Constructs the Class object
    ///
    /// A Class object doesnt do anything on its own.
    /// It must be told what methods and variables it contains.
    /// This is done using Class methods such as Class::Func and Class::Var.
    /// Then the Class must be exposed to Squirrel.
    /// This is usually done by calling TableBase::Bind on a RootTable with the Class.
    ///
    /// \param v           Squirrel virtual machine to create the Class for
    /// \param className   A necessarily unique name for the class that can appear in error messages
    /// \param createClass Should class type data be created? (almost always should be true - don't worry about it)
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Class(HSQUIRRELVM v, const string& className, bool createClass = true) : Object(false) {
        if (createClass && !ClassType<C>::hasClassData(v)) {
/*
            sq_pushregistrytable(v);
            sq_pushstring(v, "__classes", -1);
            if (SQ_FAILED(sq_rawget(v, -2))) {
                sq_newtable(v);
                sq_pushstring(v, "__classes", -1);
                sq_push(v, -2);
                sq_rawset(v, -4);
            }
            sq_pushstring(v, className.c_str(), static_cast<SQInteger>(className.size()));
            auto** ud = reinterpret_cast<ClassData<C>**>(sq_newuserdata(v, sizeof(ClassData<C>*)));
            *ud = new ClassData<C>;
            sq_setreleasehook(v, -1, &cleanup_hook);
            sq_rawset(v, -3);
            sq_pop(v, 2);
*/
            //ClassData<C>* cd = *ud;
            auto* cd = new ClassData<C>();
            VMContext::ClsPtr ptr(static_cast<AbstractClassData*>(cd));
            GetVMContext(v)->classes.try_emplace(className, std::move(ptr));

            //if (ClassType<C>::getStaticClassData().Expired()) {
            if (!ClassType<C>::getStaticClassData()) {
                cd->staticData.Init(new StaticClassData<C, void>);
                cd->staticData->copyFunc  = &A::Copy;
                cd->staticData->className = string(className);
                cd->staticData->baseClass = nullptr;

                //ClassType<C>::getStaticClassData() = cd->staticData;
            } else {
                //cd->staticData = ClassType<C>::getStaticClassData().Lock();
                throw Exception("investigate me! [Class]");
            }

            HSQOBJECT& classObj = cd->classObj;
            sq_resetobject(&classObj);

            sq_newclass(v, false);
            sq_getstackobj(v, -1, &classObj);
            sq_addref(v, &classObj); // must addref before the pop!
            sq_pop(v, 1);
            InitClass(cd);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Gets the Squirrel object for this Class (copy)
    ///
    /// \return Squirrel object representing the Squirrel class
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual HSQOBJECT GetObj() const {
        return ClassType<C>::getClassData(SqVM())->classObj;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Gets the Squirrel object for this Class (reference)
    ///
    /// \return Squirrel object representing the Squirrel class
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual HSQOBJECT& GetObj() {
        return ClassType<C>::getClassData(SqVM())->classObj;
    }

public:

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Retrieve the object itself.
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Class& Self() { return *this; }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Retrieve the object itself.
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const Class& Self() const { return *this; }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Assigns a static class slot a value
    ///
    /// \param name Name of the static slot
    /// \param val  Value to assign
    ///
    /// \tparam V Type of value (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /// \remarks
    /// Static values are read-only in Squirrel.
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class V>
    Class& SetStaticValue(const SQChar* name, const V& val) {
        BindValue<V>(name, val, true);
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Assigns a class slot a value
    ///
    /// \param name Name of the slot
    /// \param val  Value to assign
    ///
    /// \tparam V Type of value (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class V>
    Class& SetValue(const SQChar* name, const V& val) {
        BindValue<V>(name, val, false);
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a class variable
    ///
    /// \param name Name of the variable as it will appear in Squirrel
    /// \param var  Variable to bind
    ///
    /// \tparam V Type of variable (usually doesnt need to be defined explicitly)
    ///
    /// \remarks
    /// If V is not a pointer or reference, then it must have a default constructor.
    /// See Sqrat::Class::Prop to work around this requirement
    ///
    /// \return The Class itself so the call can be chained
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class V>
    Class& Var(const SQChar* name, V C::* var) {
        ClassData<C>* cd = ClassType<C>::getClassData(SqVM());

        // Add the getter
        BindAccessor(name, &var, sizeof(var), &sqDefaultGet<C, V>, cd->getTable);

        // Add the setter
        BindAccessor(name, &var, sizeof(var), &sqDefaultSet<C, V>, cd->setTable);

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a class variable without a setter
    ///
    /// \param name Name of the variable as it will appear in Squirrel
    /// \param var  Variable to bind
    ///
    /// \tparam V Type of variable (usually doesnt need to be defined explicitly)
    ///
    /// \remarks
    /// If V is not a pointer or reference, then it must have a default constructor.
    /// See Sqrat::Class::Prop to work around this requirement
    ///
    /// \return The Class itself so the call can be chained
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class V>
    Class& ConstVar(const SQChar* name, V C::* var) {
        ClassData<C>* cd = ClassType<C>::getClassData(SqVM());

        // Add the getter
        BindAccessor(name, &var, sizeof(var), &sqDefaultGet<C, V>, cd->getTable);

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Bind a class static variable
    ///
    /// \param name Name of the variable as it will appear in Squirrel
    /// \param var  Variable to bind
    ///
    /// \tparam V Type of variable (usually doesnt need to be defined explicitly)
    ///
    /// \remarks
    /// If V is not a pointer or reference, then it must have a default constructor.
    /// See Sqrat::Class::Prop to work around this requirement
    ///
    /// \return The Class itself so the call can be chained
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class V>
    Class& StaticVar(const SQChar* name, V* var) {
        ClassData<C>* cd = ClassType<C>::getClassData(SqVM());

        // Add the getter
        BindAccessor(name, &var, sizeof(var), &sqStaticGet<C, V>, cd->getTable);

        // Add the setter
        BindAccessor(name, &var, sizeof(var), &sqStaticSet<C, V>, cd->setTable);

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a class property
    ///
    /// \param name      Name of the variable as it will appear in Squirrel
    /// \param getMethod Getter for the variable
    /// \param setMethod Setter for the variable
    ///
    /// \tparam F1 Type of get function (usually doesnt need to be defined explicitly)
    /// \tparam F2 Type of set function (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /// \remarks
    /// This method binds setter and getter functions in C++ to Squirrel as if they are a class variable.
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class F1, class F2>
    Class& Prop(const SQChar* name, F1 getMethod, F2 setMethod) {
        ClassData<C>* cd = ClassType<C>::getClassData(SqVM());

        if(getMethod != nullptr) {
            // Add the getter
            BindAccessor(name, &getMethod, sizeof(getMethod), SqMemberOverloadedFunc(getMethod), cd->getTable);
        }

        if(setMethod != nullptr) {
            // Add the setter
            BindAccessor(name, &setMethod, sizeof(setMethod), SqMemberOverloadedFunc(setMethod), cd->setTable);
        }

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a class property (using global functions instead of member functions)
    ///
    /// \param name      Name of the variable as it will appear in Squirrel
    /// \param getMethod Getter for the variable
    /// \param setMethod Setter for the variable
    ///
    /// \tparam F1 Type of get function (usually doesnt need to be defined explicitly)
    /// \tparam F2 Type of set function (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /// \remarks
    /// This method binds setter and getter functions in C++ to Squirrel as if they are a class variable.
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class F1, class F2>
    Class& GlobalProp(const SQChar* name, F1 getMethod, F2 setMethod) {
        ClassData<C>* cd = ClassType<C>::getClassData(SqVM());

        if(getMethod != nullptr) {
            // Add the getter
            BindAccessor(name, &getMethod, sizeof(getMethod), SqMemberGlobalOverloadedFunc(getMethod), cd->getTable);
        }

        if(setMethod != nullptr) {
            // Add the setter
            BindAccessor(name, &setMethod, sizeof(setMethod), SqMemberGlobalOverloadedFunc(setMethod), cd->setTable);
        }

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a read-only class property
    ///
    /// \param name      Name of the variable as it will appear in Squirrel
    /// \param getMethod Getter for the variable
    ///
    /// \tparam F Type of get function (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /// \remarks
    /// This method binds a getter function in C++ to Squirrel as if it is a class variable.
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class F>
    Class& Prop(const SQChar* name, F getMethod) {
        // Add the getter
        BindAccessor(name, &getMethod, sizeof(getMethod), SqMemberOverloadedFunc(getMethod), ClassType<C>::getClassData(SqVM())->getTable);

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a read-only class property (using a global function instead of a member function)
    ///
    /// \param name      Name of the variable as it will appear in Squirrel
    /// \param getMethod Getter for the variable
    ///
    /// \tparam F Type of get function (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /// \remarks
    /// This method binds a getter function in C++ to Squirrel as if it is a class variable.
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class F>
    Class& GlobalProp(const SQChar* name, F getMethod) {
        // Add the getter
        BindAccessor(name, &getMethod, sizeof(getMethod), SqMemberGlobalOverloadedFunc(getMethod), ClassType<C>::getClassData(SqVM())->getTable);

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a class function
    ///
    /// \param name   Name of the function as it will appear in Squirrel
    /// \param method Function to bind
    ///
    /// \tparam F Type of function (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class F>
    Class& Func(const SQChar* name, F method) {
        BindFunc(name, &method, sizeof(method), SqMemberFunc(method));
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a class function with formatting support
    ///
    /// \param name   Name of the function as it will appear in Squirrel
    /// \param method Function to bind
    ///
    /// \tparam F Type of function (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class F>
    Class& FmtFunc(const SQChar* name, F method) {
        BindFunc(name, &method, sizeof(method), SqMemberFunc(method));
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a class function with callback support
    ///
    /// \param name   Name of the function as it will appear in Squirrel
    /// \param method Function to bind
    ///
    /// \tparam F Type of function (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class F>
    Class& CbFunc(const SQChar* name, F method) {
        BindFunc(name, &method, sizeof(method), SqMemberFunc(method));
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a class function with overloading enabled
    ///
    /// \param name   Name of the function as it will appear in Squirrel
    /// \param method Function to bind
    ///
    /// \tparam F Type of function (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /// \remarks
    /// Overloading in this context means to allow the function name to be used with functions
    /// of a different number of arguments. This definition differs from others (e.g. C++'s).
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class F>
    Class& Overload(const SQChar* name, F method) {
        BindOverload(name, &method, sizeof(method), SqMemberOverloadedFunc(method), SqOverloadFunc(method), SqGetArgCount(method));
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a global function as a class function
    ///
    /// \param name   Name of the function as it will appear in Squirrel
    /// \param method Function to bind
    ///
    /// \tparam F Type of function (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class F>
    Class& GlobalFunc(const SQChar* name, F method) {
        BindFunc(name, &method, sizeof(method), SqMemberGlobalFunc(method));
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a global function as a class function with formatting support
    ///
    /// \param name   Name of the function as it will appear in Squirrel
    /// \param method Function to bind
    ///
    /// \tparam F Type of function (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class F>
    Class& GlobalFmtFunc(const SQChar* name, F method) {
        BindFunc(name, &method, sizeof(method), SqMemberGlobalFunc(method));
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a global function as a class function with formatting support
    ///
    /// \param name   Name of the function as it will appear in Squirrel
    /// \param method Function to bind
    ///
    /// \tparam F Type of function (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class F>
    Class& GlobalCbFunc(const SQChar* name, F method) {
        BindFunc(name, &method, sizeof(method), SqMemberGlobalFunc(method));
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a static class function
    ///
    /// \param name   Name of the function as it will appear in Squirrel
    /// \param method Function to bind
    ///
    /// \tparam F Type of function (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class F>
    Class& StaticFunc(const SQChar* name, F method) {
        BindFunc(name, &method, sizeof(method), SqGlobalFunc(method));
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a static class function with formatting support
    ///
    /// \param name   Name of the function as it will appear in Squirrel
    /// \param method Function to bind
    ///
    /// \tparam F Type of function (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class F>
    Class& StaticFmtFunc(const SQChar* name, F method) {
        BindFunc(name, &method, sizeof(method), SqGlobalFunc(method));
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a static class function with formatting support
    ///
    /// \param name   Name of the function as it will appear in Squirrel
    /// \param method Function to bind
    ///
    /// \tparam F Type of function (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class F>
    Class& StaticCbFunc(const SQChar* name, F method) {
        BindFunc(name, &method, sizeof(method), SqGlobalFunc(method));
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a global function as a class function with overloading enabled
    ///
    /// \param name   Name of the function as it will appear in Squirrel
    /// \param method Function to bind
    ///
    /// \tparam F Type of function (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /// \remarks
    /// Overloading in this context means to allow the function name to be used with functions
    /// of a different number of arguments. This definition differs from others (e.g. C++'s).
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class F>
    Class& GlobalOverload(const SQChar* name, F method) {
        BindOverload(name, &method, sizeof(method), SqMemberGlobalOverloadedFunc(method), SqOverloadFunc(method), SqGetArgCount(method) - 1);
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a static class function with overloading enabled
    ///
    /// \param name   Name of the function as it will appear in Squirrel
    /// \param method Function to bind
    ///
    /// \tparam F Type of function (usually doesnt need to be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /// \remarks
    /// Overloading in this context means to allow the function name to be used with functions
    /// of a different number of arguments. This definition differs from others (e.g. C++'s).
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class F>
    Class& StaticOverload(const SQChar* name, F method) {
        BindOverload(name, &method, sizeof(method), SqGlobalOverloadedFunc(method), SqOverloadFunc(method), SqGetArgCount(method));
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a Squirrel function as defined by the Squirrel documentation as a class function
    ///
    /// \param name Name of the function as it will appear in Squirrel
    /// \param func Function to bind
    ///
    /// \return The Class itself so the call can be chained
    ///
    /// \remarks
    /// Inside of the function, the class instance the function was called with will be at index 1 on the
    /// stack and all arguments will be after that index in the order they were given to the function.
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Class& SquirrelFunc(const SQChar* name, SQFUNCTION func) {
        HSQUIRRELVM vm = SqVM();
        sq_pushobject(vm, ClassType<C>::getClassData(vm)->classObj);
        sq_pushstring(vm, name, -1);
        sq_newclosure(vm, func, 0);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, name);
        sq_newslot(vm, -3, false);
        sq_pop(vm, 1); // pop table

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a Squirrel function as defined by the Squirrel documentation as a class function
    ///
    /// \param name Name of the function as it will appear in Squirrel
    /// \param func Function to bind
    /// \param pnum Number of parameters the function expects.
    /// \param mask Types of parameters the function expects.
    ///
    /// \return The Class itself so the call can be chained
    ///
    /// \remarks
    /// Inside of the function, the class instance the function was called with will be at index 1 on the
    /// stack and all arguments will be after that index in the order they were given to the function.
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Class& SquirrelFunc(const SQChar* name, SQFUNCTION func, SQInteger pnum, const SQChar * mask) {
        HSQUIRRELVM vm = SqVM();
        sq_pushobject(vm, ClassType<C>::getClassData(vm)->classObj);
        sq_pushstring(vm, name, -1);
        sq_newclosure(vm, func, 0);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, name);
        // Set parameter validation
        sq_setparamscheck(vm, pnum, mask);
        sq_newslot(vm, -3, false);
        sq_pop(vm, 1); // pop table

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Gets a Function from a name in the Class
    ///
    /// \param name The name in the class that contains the Function
    ///
    /// \return Function found in the Class (null if failed)
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Function GetFunction(const SQChar* name) {
        HSQUIRRELVM vm = SqVM();
        ClassData<C>* cd = ClassType<C>::getClassData(vm);
        HSQOBJECT funcObj;
        sq_pushobject(vm, cd->classObj);
        sq_pushstring(vm, name, -1);
#if !defined (SCRAT_NO_ERROR_CHECKING)
        if(SQ_FAILED(sq_get(vm, -2))) {
            sq_pop(vm, 1);
            return Function();
        }
        SQObjectType value_type = sq_gettype(vm, -1);
        if (value_type != OT_CLOSURE && value_type != OT_NATIVECLOSURE) {
            sq_pop(vm, 2);
            return Function();
        }
#else
        sq_get(vm, -2);
#endif
        sq_getstackobj(vm, -1, &funcObj);
        Function ret(vm, cd->classObj, funcObj); // must addref before the pop!
        sq_pop(vm, 2);
        return ret;
    }

protected:

/// @cond DEV

    static SQInteger ClassWeakref(HSQUIRRELVM vm) {
        sq_weakref(vm, -1);
        return 1;
    }

    static SQInteger ClassTypeof(HSQUIRRELVM vm) {
        sq_pushstring(vm, ClassType<C>::ClassName().c_str(), -1);
        return 1;
    }

    static SQInteger ClassCloned(HSQUIRRELVM vm) {
        SQTRY()
        Sqrat::Var<const C*> other(vm, 2);
        SQCATCH_NOEXCEPT(vm) {
            SQCLEAR(vm);
            return SQ_ERROR;
        }
#if !defined (SCRAT_NO_ERROR_CHECKING)
        return ClassType<C>::CopyFunc()(vm, 1, other.value);
#else
        ClassType<C>::CopyFunc()(vm, 1, other.value);
        return 0;
#endif
        SQCATCH(vm) {
#if defined (SCRAT_USE_EXCEPTIONS)
            SQUNUSED(e); // this is to avoid a warning in MSVC
#endif
            return SQ_ERROR;
        }
    }

    // Initialize the required data structure for the class
    void InitClass(ClassData<C>* cd) {
        HSQUIRRELVM vm = SqVM();
        cd->instances.Init(new std::unordered_map<C*, HSQOBJECT>);

        // push the class
        sq_pushobject(vm, cd->classObj);

        // set the typetag of the class
        sq_settypetag(vm, -1, cd->staticData.Get());

        // add the default constructor
        sq_pushstring(vm, _SC("constructor"), -1);
        sq_newclosure(vm, &A::New, 0);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, _SC("constructor"));
        sq_newslot(vm, -3, false);

        // add the set table (static)
        HSQOBJECT& setTable = cd->setTable;
        sq_resetobject(&setTable);
        sq_pushstring(vm, _SC("@set"), -1);
        sq_newtable(vm);
        sq_getstackobj(vm, -1, &setTable);
        sq_addref(vm, &setTable);
        sq_newslot(vm, -3, true);

        // add the get table (static)
        HSQOBJECT& getTable = cd->getTable;
        sq_resetobject(&getTable);
        sq_pushstring(vm, _SC("@get"), -1);
        sq_newtable(vm);
        sq_getstackobj(vm, -1, &getTable);
        sq_addref(vm, &getTable);
        sq_newslot(vm, -3, true);

        // override _set
        sq_pushstring(vm, _SC("_set"), -1);
        sq_pushobject(vm, setTable); // Push the set table as a free variable
        sq_newclosure(vm, &sqVarSet, 1);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, _SC("_set"));
        sq_newslot(vm, -3, false);

        // override _get
        sq_pushstring(vm, _SC("_get"), -1);
        sq_pushobject(vm, getTable); // Push the get table as a free variable
        sq_newclosure(vm, &sqVarGet, 1);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, _SC("_get"));
        sq_newslot(vm, -3, false);

        // add weakref (apparently not provided by default)
        sq_pushstring(vm, _SC("weakref"), -1);
        sq_newclosure(vm, &Class::ClassWeakref, 0);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, _SC("weakref"));
        sq_newslot(vm, -3, false);

        // add _typeof
        sq_pushstring(vm, _SC("_typeof"), -1);
        sq_newclosure(vm, &Class::ClassTypeof, 0);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, _SC("_typeof"));
        sq_newslot(vm, -3, false);

        // add _cloned
        sq_pushstring(vm, _SC("_cloned"), -1);
        sq_newclosure(vm, &Class::ClassCloned, 0);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, _SC("_cloned"));
        sq_newslot(vm, -3, false);

        // pop the class
        sq_pop(vm, 1);
    }

    // Helper function used to bind getters and setters
    inline void BindAccessor(const SQChar* name, void* var, size_t varSize, SQFUNCTION func, HSQOBJECT table) {
        HSQUIRRELVM vm = SqVM();
        // Push the get or set table
        sq_pushobject(vm, table);
        sq_pushstring(vm, name, -1);

        // Push the variable offset as a free variable
        SQUserPointer varPtr = sq_newuserdata(vm, static_cast<SQUnsignedInteger>(varSize));
        memcpy(varPtr, var, varSize);

        // Create the accessor function
        sq_newclosure(vm, func, 1);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, name);

        // Add the accessor to the table
        sq_newslot(vm, -3, false);

        // Pop get/set table
        sq_pop(vm, 1);
    }

    // constructor binding
    Class& BindConstructor(SQFUNCTION constructor, SQInteger nParams, const SQChar *name = 0) {
        HSQUIRRELVM vm = SqVM();
        // Decide whether to bind to a class or the root table
        bool alternative_global = false;
        if (name == 0)
            name = _SC("constructor");
        else alternative_global = true;
        // Generate the overload mangled name
        string overloadName;
        overloadName.reserve(15);
        SqOverloadName::Get(name, static_cast< int >(nParams), overloadName);
        // Should we push a class object?
        if (!alternative_global )
            sq_pushobject(vm, ClassType<C>::getClassData(vm)->classObj);
        // The containing environment is the root table??
        else sq_pushroottable(vm);

        // Bind overload handler name
        sq_pushstring(vm, name, -1);
        // function name is passed as a free variable
        //sq_pushstring(vm, name, -1);
        sq_push(vm, -1); // <- Let's cheat(?) by pushing the same object
        sq_newclosure(vm, &OverloadConstructionForwarder, 1);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, name);
        // Include it into the object
        sq_newslot(vm, -3, false);
        // Bind overloaded function
        sq_pushstring(vm, overloadName.c_str(), static_cast<SQInteger>(overloadName.size()));
        sq_newclosure(vm, constructor, 0);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, overloadName.c_str());
        // Include it into the object
        sq_newslot(vm, -3, false);
        // pop object/environment
        sq_pop(vm, 1);

        return *this;
    }

/// @endcond

public:

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a constructor with no arguments (there can only be one constructor of this many arguments for a given name)
    ///
    /// \param name Name of the constructor as it will appear in Squirrel (default value creates a traditional constructor)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Class& Ctor(const SQChar *name = 0) {
        return BindConstructor(A::iNew, 0, name);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Binds a constructor with 1 or more arguments (there can only be one constructor of this many arguments for a given name)
    ///
    /// \param name Name of the constructor as it will appear in Squirrel (default value creates a traditional constructor)
    ///
    /// \tparam P Type of arguments of the constructor (must be defined explicitly)
    ///
    /// \return The Class itself so the call can be chained
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<class... P>
    Class& Ctor(const SQChar *name = 0) {
        return BindConstructor(A::template iNew<P...>, sizeof...(P), name);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Facilitates exposing a C++ class with a base class to Squirrel
///
/// \tparam C Class type to expose
/// \tparam B Base class type (must already be bound)
/// \tparam A An allocator to use when instantiating and destroying class instances of this type in Squirrel
///
/// \remarks
/// Classes in Squirrel are single-inheritance only, and as such Sqrat only allows for single inheritance as well.
///
/// \remarks
/// DefaultAllocator is used if no allocator is specified. This should be sufficent for most classes,
/// but if specific behavior is desired, it can be overridden. If the class should not be instantiated from
/// Squirrel the NoConstructor allocator may be used. See NoCopy and CopyOnly too.
///
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class C, class B, class A = DefaultAllocator<C> >
class DerivedClass : public Class<C, A>
{
private:

    static SQInteger cleanup_hook(SQUserPointer ptr, SQInteger size) {
        SQUNUSED(size);
        auto** ud = reinterpret_cast<ClassData<C>**>(ptr);
        delete *ud;
        return 0;
    }

public:

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Retrieve the object itself.
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    DerivedClass& Self() { return *this; }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Retrieve the object itself.
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const DerivedClass& Self() const { return *this; }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Constructs the DerivedClass object
    ///
    /// A DerivedClass object doesnt do anything on its own.
    /// It must be told what methods and variables it contains.
    /// This is done using Class methods such as Class::Func and Class::Var.
    /// Then the DerivedClass must be exposed to Squirrel.
    /// This is usually done by calling TableBase::Bind on a RootTable with the DerivedClass.
    ///
    /// \param v         Squirrel virtual machine to create the DerivedClass for
    /// \param className A necessarily unique name for the class that can appear in error messages
    ///
    /// \remarks
    /// You MUST bind the base class fully before constructing a derived class.
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    DerivedClass(HSQUIRRELVM v, const string& className) : Class<C, A>(v, string(), false) {
        if (!ClassType<C>::hasClassData(v)) {
/*
            sq_pushregistrytable(v);
            sq_pushstring(v, "__classes", -1);
            if (SQ_FAILED(sq_rawget(v, -2))) {
                sq_newtable(v);
                sq_pushstring(v, "__classes", -1);
                sq_push(v, -2);
                sq_rawset(v, -4);
            }
            sq_pushstring(v, className.c_str(), -1);
            auto** ud = reinterpret_cast<ClassData<C>**>(sq_newuserdata(v, sizeof(ClassData<C>*)));
            *ud = new ClassData<C>;
            sq_setreleasehook(v, -1, &cleanup_hook);
            sq_rawset(v, -3);
            sq_pop(v, 2);
*/
            ClassData<B>* bd = ClassType<B>::getClassData(v);
            //ClassData<C>* cd = *ud;
            auto* cd = new ClassData<C>();
            VMContext::ClsPtr ptr(static_cast<AbstractClassData*>(cd));
            GetVMContext(v)->classes.try_emplace(className, std::move(ptr));

            //if (ClassType<C>::getStaticClassData()->Expired()) {
            if (!ClassType<C>::getStaticClassData()) {
                cd->staticData.Init(new StaticClassData<C, B>);
                cd->staticData->copyFunc  = &A::Copy;
                cd->staticData->className = string(className);
                cd->staticData->baseClass = bd->staticData.Get();

                //ClassType<C>::getStaticClassData() = cd->staticData;
            } else {
                //cd->staticData = ClassType<C>::getStaticClassData().Lock();
                throw Exception("investigate me! [DerivedClass]");
            }

            HSQOBJECT& classObj = cd->classObj;
            sq_resetobject(&classObj);

            sq_pushobject(v, bd->classObj);
            sq_newclass(v, true);
            sq_getstackobj(v, -1, &classObj);
            sq_addref(v, &classObj); // must addref before the pop!
            sq_pop(v, 1);
            InitDerivedClass(v, cd, bd);
        }
    }

protected:

/// @cond DEV

    void InitDerivedClass(HSQUIRRELVM vm, ClassData<C>* cd, ClassData<B>* bd) {
        cd->instances.Init(new std::unordered_map<C*, HSQOBJECT>);

        // push the class
        sq_pushobject(vm, cd->classObj);

        // set the typetag of the class
        sq_settypetag(vm, -1, cd->staticData.Get());

        // add the default constructor
        sq_pushstring(vm, _SC("constructor"), -1);
        sq_newclosure(vm, &A::New, 0);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, _SC("constructor"));
        sq_newslot(vm, -3, false);

        // clone the base classes set table (static)
        HSQOBJECT& setTable = cd->setTable;
        sq_resetobject(&setTable);
        sq_pushobject(vm, bd->setTable);
        sq_pushstring(vm, _SC("@set"), -1);
        sq_clone(vm, -2);
        sq_remove(vm, -3);
        sq_getstackobj(vm, -1, &setTable);
        sq_addref(vm, &setTable);
        sq_newslot(vm, -3, true);

        // clone the base classes get table (static)
        HSQOBJECT& getTable = cd->getTable;
        sq_resetobject(&getTable);
        sq_pushobject(vm, bd->getTable);
        sq_pushstring(vm, _SC("@get"), -1);
        sq_clone(vm, -2);
        sq_remove(vm, -3);
        sq_getstackobj(vm, -1, &getTable);
        sq_addref(vm, &getTable);
        sq_newslot(vm, -3, true);

        // override _set
        sq_pushstring(vm, _SC("_set"), -1);
        sq_pushobject(vm, setTable); // Push the set table as a free variable
        sq_newclosure(vm, sqVarSet, 1);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, _SC("_set"));
        sq_newslot(vm, -3, false);

        // override _get
        sq_pushstring(vm, _SC("_get"), -1);
        sq_pushobject(vm, getTable); // Push the get table as a free variable
        sq_newclosure(vm, sqVarGet, 1);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, _SC("_get"));
        sq_newslot(vm, -3, false);

        // add weakref (apparently not provided by default)
        sq_pushstring(vm, _SC("weakref"), -1);
        sq_newclosure(vm, &Class<C, A>::ClassWeakref, 0);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, _SC("weakref"));
        sq_newslot(vm, -3, false);

        // add _typeof
        sq_pushstring(vm, _SC("_typeof"), -1);
        sq_newclosure(vm, &Class<C, A>::ClassTypeof, 0);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, _SC("_typeof"));
        sq_newslot(vm, -3, false);

        // add _cloned
        sq_pushstring(vm, _SC("_cloned"), -1);
        sq_newclosure(vm, &Class<C, A>::ClassCloned, 0);
        // Set the closure name (for debug purposes)
        sq_setnativeclosurename(vm, -1, _SC("_cloned"));
        sq_newslot(vm, -3, false);

        // pop the class
        sq_pop(vm, 1);
    }

/// @endcond

};

}
