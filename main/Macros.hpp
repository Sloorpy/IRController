#pragma once

#define DELETE_COPY_CTOR(ClassName) \
    ClassName(const ClassName&) = delete;

#define DELETE_COPY_ASSIGN(ClassName) \
    ClassName& operator=(const ClassName&) = delete;

#define DELETE_MOVE_CTOR(ClassName) \
    ClassName(ClassName&&) = delete;

#define DELETE_MOVE_ASSIGN(ClassName) \
    ClassName& operator=(ClassName&&) = delete;

#define DELETE_COPY_MOVE(ClassName) \
    DELETE_COPY_CTOR(ClassName) \
    DELETE_COPY_ASSIGN(ClassName) \
    DELETE_MOVE_CTOR(ClassName) \
    DELETE_MOVE_ASSIGN(ClassName)
