#pragma once

#include "PreCompile.h"
#include "Property.h"
#include "Type.h"

#define REGISTER_PROPERTY(Name)\
inline static struct RegistPropertyExecutor_##Name\
{\
    RegistPropertyExecutor_##Name()\
    {\
        static PropertyRegister<decltype(Name)> property_register_##Name{ \
            #Name, typeid(Name).name(), ThisType::StaticTypeInfo() \
        }; \
    }\
} regist_##Name;
