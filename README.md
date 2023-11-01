# Curry

C++ Curry


## describe
可以对任意C++可调用对象进行柯里化，包括全局函数，成员函数，lambda

```
    int func(int a,int b,int c)
    {
        return a+b*c;
    }
    auto curry=Curry(func);
    int result=curry(1)(2)(3);
    result=curry(1,2)(3);
    result=curry(1)(2,3);
    result=curry(1,2,3);
```