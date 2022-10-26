#ifndef _NotificationInfo_h_
#define _NotificationInfo_h_


struct NotificationInfo
{
    tstring time;
    tstring event;
    tstring file;
    tstring message;
    int     enforcement;        /* store DENY/ALLOW */
};

typedef std::vector< NotificationInfo* > NotificationVector;

#endif
