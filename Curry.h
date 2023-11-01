#ifndef CURRY_H
#define CURRY_H
#include <iostream>
#include <tuple>
#include <type_traits>
template <typename Functor> void ParamErgodic(Functor &&func){};

template <typename Functor, typename Type, typename... ArgTypes>
void ParamErgodic(Functor &&func, Type &&param, ArgTypes &&...args)
{
    func(std::forward<Type>(param));
    ParamErgodic(func, std::forward<ArgTypes>(args)...);
}

// ============================extract function into============================

enum class FunTypeEnum
{
    NormalFuncion,
    MemberFunction,
    OperatorFunc,
    NotFuntion
};
template <int Index, typename Type, typename... ArgTypes> struct ParamTypeHelper
{

    using type = typename ParamTypeHelper<Index - 1, ArgTypes...>::type;
};

template <typename Type, typename... ArgTypes> struct ParamTypeHelper<0, Type, ArgTypes...>
{
    using type = Type;
};
template <int Index, typename... ArgTypes> struct ParamType
{
    static_assert(Index >= 0 && Index < sizeof...(ArgTypes),
                  "error,Index must bigger than zero,and less than sizeof argument");
    using type = typename ParamTypeHelper<Index, ArgTypes...>::type;
};
template <bool flag, typename TrueType, typename FalseType> struct IF
{
};

template <typename TrueType, typename FalseType> struct IF<true, TrueType, FalseType>
{
    using type = TrueType;
};

template <typename TrueType, typename FalseType> struct IF<false, TrueType, FalseType>
{
    using type = FalseType;
};

template <typename Ret, typename... ArgTypes> struct Signature
{
    using retType = Ret;

#ifdef _MSC_VER // FK U MSVC!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    using tupleType = std::tuple<ArgTypes...>;
#else
    template <int Index> using argType = ParamType<Index, ArgTypes...>;
#endif
    static constexpr int argNum = sizeof...(ArgTypes);
};
template <typename T> struct HasOperator
{
    using funcType = typename std::remove_reference<T>::type;
    template <typename U> static constexpr bool check(decltype(&funcType::operator()) *)
    {
        return true;
    }
    template <typename U> static constexpr bool check(...)
    {
        return false;
    }
    static constexpr bool value = check<funcType>(nullptr);
};

struct NoFuntion
{
    static constexpr FunTypeEnum funType = FunTypeEnum::NotFuntion;
};

template <typename T> struct FunctorHelper
{
};

template <typename Ret, typename Cls, typename... ArgTypes>
struct FunctorHelper<Ret (Cls::*)(ArgTypes...)> : public Signature<Ret, ArgTypes...>
{
    using clsType = Cls;
    static constexpr FunTypeEnum funType = FunTypeEnum::OperatorFunc;
};

template <typename Ret, typename Cls, typename... ArgTypes>
struct FunctorHelper<Ret (Cls::*)(ArgTypes...) const> : public Signature<Ret, ArgTypes...>
{
    using clsType = Cls;
    static constexpr FunTypeEnum funType = FunTypeEnum::OperatorFunc;
};

template <typename T>
struct FunctionInfo : public IF<HasOperator<T>::value,
                                FunctorHelper<decltype(&std::remove_reference<T>::type::operator())>, NoFuntion>::type
{
};

template <typename Ret, typename Cls, typename... ArgTypes>
struct FunctionInfo<Ret (Cls::*)(ArgTypes...)> : public Signature<Ret, ArgTypes...>
{
    using clsType = Cls;
    static constexpr FunTypeEnum funType = FunTypeEnum::MemberFunction;
};

template <typename Ret, typename Cls, typename... ArgTypes>
struct FunctionInfo<Ret (Cls::*)(ArgTypes...) const> : public Signature<Ret, ArgTypes...>
{
    using clsType = Cls;
    static constexpr FunTypeEnum funType = FunTypeEnum::MemberFunction;
};

template <typename Ret, typename... ArgTypes> struct FunctionInfo<Ret(ArgTypes...)> : public Signature<Ret, ArgTypes...>
{
    static constexpr FunTypeEnum funType = FunTypeEnum::NormalFuncion;
};

// ============================extract function info============================

// ============================wrap function============================
template <typename Cls, typename Func, typename... ArgTypes> auto MemberInvoke(Cls *obj, Func func, ArgTypes &&...args)
{
    return (obj->*func)(std::forward<ArgTypes>(args)...);
}

template <typename Cls, typename Func, typename... ArgTypes> auto MemberInvoke(Cls &&obj, Func func, ArgTypes &&...args)
{
    return (obj.*func)(std::forward<ArgTypes>(args)...);
}

template <typename Ret, typename... ArgTypes> auto WrapFunction(Ret(func)(ArgTypes...))
{
    return [=](ArgTypes... args) { return func(std::forward<ArgTypes>(args)...); };
}

template <typename Ret, typename Cls, typename... ArgTypes> auto WrapFunction(Ret (Cls::*func)(ArgTypes...) const)
{
    return [=](const Cls *obj, ArgTypes... args) {
        return MemberInvoke(std::forward<decltype(obj)>(obj), func, std::forward<ArgTypes>(args)...);
    };
}

template <typename Ret, typename Cls, typename... ArgTypes> auto WrapFunction(Ret (Cls::*func)(ArgTypes...))
{
    return [=](Cls *obj, ArgTypes... args) {
        return MemberInvoke(std::forward<decltype(obj)>(obj), func, std::forward<ArgTypes>(args)...);
    };
}

template <typename Ret, typename Cls, typename... ArgTypes>
auto WrapFunction(Cls &&obj, Ret (Cls::*func)(ArgTypes...) const)
{
#if __cplusplus >= 201402L || _MSVC_LANG >= 201402L
    return [obj = std::move(obj), func](ArgTypes... args) { return (obj.*func)(std::forward<ArgTypes>(args)...); };
#else
    return [obj, func](ArgTypes... args) { return (obj.*func)(std::forward<ArgTypes>(args)...); };
#endif
}

template <typename Ret, typename Cls, typename... ArgTypes> auto WrapFunction(Cls &&obj, Ret (Cls::*func)(ArgTypes...))
{
#if __cplusplus >= 201402L || _MSVC_LANG >= 201402L
    return [obj = std::move(obj), func](ArgTypes... args) { return (obj.*func)(std::forward<ArgTypes>(args)...); };
#else
    return [obj, func](ArgTypes... args) { return (obj.*func)(std::forward<ArgTypes>(args)...); };
#endif
}

template <typename Ret, typename Cls, typename... ArgTypes>
auto WrapFunction(const Cls &obj, Ret (Cls::*func)(ArgTypes...) const)
{
    return [&obj, func](ArgTypes... args) { return (obj.*func)(std::forward<ArgTypes>(args)...); };
}

template <typename Ret, typename Cls, typename... ArgTypes> auto WrapFunction(Cls &obj, Ret (Cls::*func)(ArgTypes...))
{
    return [&obj, func](ArgTypes... args) { return (obj.*func)(std::forward<ArgTypes>(args)...); };
}

template <typename Ret, typename Cls, typename... ArgTypes>
auto WrapFunction(const Cls *obj, Ret (Cls::*func)(ArgTypes...) const)
{
    return [obj, func](ArgTypes... args) { return (obj->*func)(std::forward<ArgTypes>(args)...); };
}

template <typename Ret, typename Cls, typename... ArgTypes> auto WrapFunction(Cls *obj, Ret (Cls::*func)(ArgTypes...))
{
    return [obj, func](ArgTypes... args) { return (obj->*func)(std::forward<ArgTypes>(args)...); };
}

template <typename Type> auto WrapFunction(Type &&obj)
{
    using rmrefType = typename std::remove_reference<Type>::type;
    using rmConstType = typename std::remove_cv<rmrefType>::type;
    using rmPointType = typename std::remove_pointer<rmConstType>::type;
    return WrapFunction(std::forward<Type>(obj), &rmPointType::operator());
}

template <typename Type> Type CopyFunction(Type &&func)
{
    return func;
}

template <typename Type> auto WrapFunctionS(Type &&obj)
{
    return WrapFunction(CopyFunction(obj));
}
// ============================wrap function============================

// ============================apply============================
template <int N> struct ApplyHelper
{
    template <typename Functor, typename Tuple, typename... ArgTypes>
    static auto Call(Functor &&func, Tuple &&container, ArgTypes &&...args)
    {
        constexpr int paramIndex = std::tuple_size<typename std::remove_reference<Tuple>::type>::value - N;
#ifdef _MSC_VER
        using tupleType = typename FunctionInfo<Functor>::tupleType;

        using paramType = typename std::tuple_element<paramIndex, tupleType>::type;
#else
        using paramType = typename FunctionInfo<Functor>::argType<paramIndex>::type;
#endif
        auto &&param = std::get<paramIndex>(container);

        return ApplyHelper<N - 1>::Call(std::forward<Functor>(func), std::forward<Tuple>(container),
                                        std::forward<ArgTypes>(args)..., std::forward<paramType>(param));
    }
};

template <> struct ApplyHelper<0>
{
    template <typename Functor, typename Tuple, typename... ArgTypes>
    static auto Call(Functor func, Tuple &&container, ArgTypes &&...args)
    {
        return func(std::forward<ArgTypes>(args)...);
    }
};

template <typename Functor, typename Tuple> auto Apply(Functor &&func, Tuple &&container)
{
    auto wrapFunc = WrapFunction(func);
    using wrapFuncType = decltype(wrapFunc);
    constexpr int containerSize = std::tuple_size<typename std::remove_reference<Tuple>::type>::value;
    constexpr int funcArgNum = FunctionInfo<wrapFuncType>::argNum;
    static_assert(containerSize == funcArgNum, "error ,number of tuple's argument must be equal to function argument");
    return ApplyHelper<containerSize>::Call(wrapFunc, std::forward<Tuple>(container));
}

// ============================apply============================

// ============================curry============================

template <int N /* , typename Functor, typename ContainerType */> struct CurryHelper
{
    static_assert(N > 0, "error ,number of paramter function must be bigger than container");

    template <typename Func, typename... ArgTypes> static auto Result(Func &&func, ArgTypes &&...oldArgs)
    {
        return [=](auto &&...args) mutable {
            return CurryHelper<N - sizeof...(args)>::Result(func, std::forward<ArgTypes>(oldArgs)...,
                                                            std::forward<decltype(args)>(args)...);
        };
    }

    // Functor _func;
    // ContainerType _container;

    // template <typename Func, typename Tuple> static auto Result(Func &&func, Tuple &&container)
    // {
    //     return CurryHelper<N, Functor, typename std::remove_reference<Tuple>::type>(std::forward<Func>(func),
    //                                                                                 std::forward<Tuple>(container));

    // }

    // template <typename T>
    // CurryHelper(const Functor &func, T &&container) : _func(func), _container(std::forward<T>(container))
    // {
    // }
    // CurryHelper(CurryHelper &&other) : _func(std::move(other._func)), _container(std::move(other._container))
    // {
    // }
    // CurryHelper(const CurryHelper &other) : _func(other._func), _container(other._container)
    // {
    // }
    // template <typename... ArgTypes> auto operator()(ArgTypes &&...args)
    // {`
    //     auto newContainer = std::tuple_cat(_container, std::make_tuple(std::forward<ArgTypes>(args)...));
    //     constexpr int argNum = FunctionInfo<Functor>::argNum;
    //     constexpr int containerSize = std::tuple_size<decltype(newContainer)>::value;
    //     return CurryHelper<argNum - containerSize, Functor, ContainerType>::Result(_func, std::move(newContainer));
    // }
};

template </* typename Functor, typename ContainerType */> struct CurryHelper<0 /* , Functor, ContainerType */>
{
    // template <typename Func, typename T> static auto Result(Func &&func, T &&container)
    // {
    //     return Apply(std::forward<Func>(func), std::forward<T>(container));
    // }

    template <typename Func, typename... ArgTypes> static auto Result(Func &&func, ArgTypes &&...args)
    {
        return func(std::forward<ArgTypes>(args)...);
    }
};

// template <int N> struct InitCurry
// {
//     template <typename Functor, typename... ArgTypes> static auto Curry(Functor &&func, ArgTypes &&...args)
//     {
//         // auto container = std::make_tuple(std::forward<ArgTypes>(args)...);
//         // return CurryHelper<N - sizeof...(ArgTypes), typename std::remove_reference<Functor>::type,
//         //                    decltype(container)>::Result(func, std::move(container));
//         return
//         CurryHelper<N-sizeof...(ArgTypes)>::Result(std::forward<Functor>(func),std::forward<ArgTypes>(args)...);
//     }

//     template <typename Functor> static auto Curry(Functor &&func)
//     {
//         return [=](auto &&...args) { return InitCurry<N>::Curry(func, std::forward<decltype(args)>(args)...); };
//     }
// };

// template <> struct InitCurry<0>
// {
//     template <typename Functor> static auto Curry(Functor func)
//     {
//         return WrapFunction(func);
//     }
// };

template <typename Functor> auto Curry(Functor &&func)
{
    auto wrapFunc = WrapFunctionS(func);
    constexpr int argNum = FunctionInfo<decltype(wrapFunc)>::argNum;
    return CurryHelper<argNum>::Result(std::move(wrapFunc));
}

// ============================curry============================

#endif
