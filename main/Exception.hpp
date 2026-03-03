#pragma once

#include "esp_err.h"

enum class ErrorCode {
    SUCCESS = 0,
    INDEX_OUT_OF_RANGE = 6,
    XQUEUE_CREATE_FAILED = 11
};

enum class IRErrorCode {
    SUCCESS = 0,
    ENABLE_CHANNEL_FAILED = 1,
    TX_CHANNEL_CREATE_FAILED = 2,
    RX_CHANNEL_CREATE_FAILED = 3,
    RX_CALLBACK_REGISTER_FAILED = 4,
    RMT_RECEIVE_FAILED = 5,
    NOT_NEC_PROTOCOL = 7,
    ADDRESSES_DONT_MATCH = 8,
    COMMANDS_DONT_MATCH = 9,
    RMT_CHANNEL_WAS_FREED = 10
};

class EspException
{
public:
    EspException(const esp_err_t err) :
        _err_code(err)
    {   
    }

public:
    esp_err_t get() const { return _err_code; }

private:
    const esp_err_t _err_code;
};

class Exception
{
public:
    Exception(const ErrorCode err) :
        _err_code(err)
    {   
    }

public:
    ErrorCode get() const { return _err_code; }

private:
    const ErrorCode _err_code;
};

class IRException
{
public:
    IRException(const IRErrorCode err) :
        _err_code(err)
    {   
    }

public:
    IRErrorCode get() const { return _err_code; }

private:
    const IRErrorCode _err_code;
};
