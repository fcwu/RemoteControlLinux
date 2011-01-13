///////////////////////////////////////////////////////////////////////////////
//
// Name: error.h
// Author: doro_wu@asus.com
// 
///////////////////////////////////////////////////////////////////////////////

#ifndef __RCTERROR_H__
#define __RCTERROR_H__

#include <exception>
#include <string>

#define RCT_MAX_BIT               ((unsigned int )0x01 << ((sizeof(unsigned int)*8) - 1))
#define RCTERR(a)                 (RCT_MAX_BIT | (a))
#define RCTWARN(a)                (a)
#define RCT_ERROR(a)              (((int) (a)) < 0)
#define RCT_SUCCESS               0
#define RCT_LOAD_ERROR            RCTERR (1)
#define RCT_INVALID_PARAMETER     RCTERR (2)
#define RCT_UNSUPPORTED           RCTERR (3)
#define RCT_BAD_BUFFER_SIZE       RCTERR (4)
#define RCT_BUFFER_TOO_SMALL      RCTERR (5)
#define RCT_NOT_READY             RCTERR (6)
#define RCT_DEVICE_ERROR          RCTERR (7)
#define RCT_WRITE_PROTECTED       RCTERR (8)
#define RCT_OUT_OF_RESOURCES      RCTERR (9)
#define RCT_VOLUME_CORRUPTED      RCTERR (10)
#define RCT_VOLUME_FULL           RCTERR (11)
#define RCT_NO_MEDIA              RCTERR (12)
#define RCT_MEDIA_CHANGED         RCTERR (13)
#define RCT_NOT_FOUND             RCTERR (14)
#define RCT_ACCESS_DENIED         RCTERR (15)
#define RCT_NO_RESPONSE           RCTERR (16)
#define RCT_NO_MAPPING            RCTERR (17)
#define RCT_TIMEOUT               RCTERR (18)
#define RCT_NOT_STARTED           RCTERR (19)
#define RCT_ALREADY_STARTED       RCTERR (20)
#define RCT_ABORTED               RCTERR (21)
#define RCT_ICMP_ERROR            RCTERR (22)
#define RCT_TFTP_ERROR            RCTERR (23)
#define RCT_PROTOCOL_ERROR        RCTERR (24)
#define RCT_INCOMPATIBLE_VERSION  RCTERR (25)
#define RCT_SECURITY_VIOLATION    RCTERR (26)
#define RCT_CRC_ERROR             RCTERR (27)
#define RCT_WARN_UNKNOWN_GLYPH    RCTWARN (1)
#define RCT_WARN_DELETE_FAILURE   RCTWARN (2)
#define RCT_WARN_WRITE_FAILURE    RCTWARN (3)
#define RCT_WARN_BUFFER_TOO_SMALL RCTWARN (4)

namespace RCT {

class Exception
    : std::exception 
{
public:
    Exception(unsigned int error_code, const char *msg);
    ~Exception() throw();

    virtual const char *what() const throw();
private:
    Exception();
    std::string mMessage;
};

}
#endif


