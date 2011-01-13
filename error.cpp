///////////////////////////////////////////////////////////////////////////////
//
// Name: error.cpp
// Author: doro_wu@asus.com
// 
///////////////////////////////////////////////////////////////////////////////

#include "error.h"

#include <stdio.h>
#include <string>
#include <exception>

using namespace RCT;

typedef struct {
    unsigned int code;
    const char * msg;
} ErrorMessage;

static ErrorMessage g_def_error_msg[] = {
    {RCT_LOAD_ERROR, "LOAD_ERROR"},
    {RCT_INVALID_PARAMETER ,"INVALID_PARAMETER"},
    {RCT_UNSUPPORTED ,"UNSUPPORTED"},
    {RCT_BAD_BUFFER_SIZE ,"BAD_BUFFER_SIZE"},
    {RCT_BUFFER_TOO_SMALL ,"BUFFER_TOO_SMALL"},
    {RCT_NOT_READY ,"NOT_READY"},
    {RCT_DEVICE_ERROR ,"DEVICE_ERROR"},
    {RCT_WRITE_PROTECTED ,"WRITE_PROTECTED"},
    {RCT_OUT_OF_RESOURCES ,"OUT_OF_RESOURCES"},
    {RCT_VOLUME_CORRUPTED ,"VOLUME_CORRUPTED"},
    {RCT_VOLUME_FULL ,"VOLUME_FULL"},
    {RCT_NO_MEDIA ,"NO_MEDIA"},
    {RCT_MEDIA_CHANGED ,"MEDIA_CHANGED"},
    {RCT_NOT_FOUND ,"NOT_FOUND"},
    {RCT_ACCESS_DENIED ,"ACCESS_DENIED"},
    {RCT_NO_RESPONSE ,"NO_RESPONSE"},
    {RCT_NO_MAPPING ,"NO_MAPPING"},
    {RCT_TIMEOUT ,"TIMEOUT"},
    {RCT_NOT_STARTED ,"NOT_STARTED"},
    {RCT_ALREADY_STARTED ,"ALREADY_STARTED"},
    {RCT_ABORTED ,"ABORTED"},
    {RCT_ICMP_ERROR ,"ICMP_ERROR"},
    {RCT_TFTP_ERROR ,"TFTP_ERROR"},
    {RCT_PROTOCOL_ERROR ,"PROTOCOL_ERROR"},
    {RCT_INCOMPATIBLE_VERSION ,"INCOMPATIBLE_VERSION"},
    {RCT_SECURITY_VIOLATION ,"SECURITY_VIOLATION"},
    {RCT_CRC_ERROR ,"CRC_ERROR"}
};


Exception::Exception(unsigned int error_code, const char *msg) {

    // write default error message
    int i;
    char tmp[64];
    const char * error_code_msg = 0;
    const int num_error_message = sizeof(g_def_error_msg)/sizeof(ErrorMessage);
    for (i = 0; i < num_error_message; ++i) {
        if (error_code == g_def_error_msg[i].code) {
            error_code_msg = g_def_error_msg[i].msg;
            break;
        }
    }
    if (i == num_error_message) {
        error_code ^= RCT_MAX_BIT;
        snprintf(tmp, sizeof(tmp) - 1, "[ERROR CODE: %d]", error_code);
        error_code_msg = tmp;
    }
    mMessage.append(error_code_msg);

    // write custom message
    if (msg != NULL) {
        mMessage.append(" ");
        mMessage.append(msg);
    } 
}

Exception::~Exception() throw() {
}

const char * Exception::what() const throw() {
    return mMessage.c_str();
}

