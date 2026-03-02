#pragma once

enum class ErrorCode {
    SUCCESS = 0,
    ENABLE_CHANNEL_FAILED = 1,
    TX_CHANNEL_CREATE_FAILED = 2,
    RX_CHANNEL_CREATE_FAILED = 3,
    RX_CALLBACK_REGISTER_FAILED = 4,
    RMT_RECEIVE_FAILED = 5,
    INDEX_OUT_OF_RANGE = 6,
    NOT_NEC_PROTOCOL = 7,
    ADDRESSES_DONT_MATCH = 8,
    COMMANDS_DONT_MATCH = 9,
    RMT_CHANNEL_WAS_FREED = 10,
    XQUEUE_CREATE_FAILED = 11
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