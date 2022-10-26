/*
* Actions.h
* Author: Fuad Rashid
* All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
* Redwood City CA, Ownership remains with Blue Jungle Inc, 
* All rights reserved worldwide. 
*/

#ifndef _ACTIONUTIL_H_
#define _ACTIONUTIL_H_

#include "Actions.h"

/**
 * @param ulOperation operation.
 * @return string representation of ulOperation
 */

WCHAR* getActionString (ULONG ulOperation)
{
    if (ulOperation == OPEN_READ_ACTION )
    {
        return (OPEN_NAME);
    }
    else if (ulOperation == OPEN_READ_WRITE_ACTION)
    {
        return (EDIT_NAME);
    }
    else if (ulOperation == DELETE_ACTION)
    {
        return (DELETE_NAME);
    }
    else if (ulOperation == READ_ACTION)
    {
        return (OPEN_NAME);
    }
    else if (ulOperation == WRITE_ACTION)
    {
        return (EDIT_NAME);
    }
    else if (ulOperation == CLOSE_ACTION)
    {
        return (CLOSE_NAME);
    }
    else if (ulOperation == RENAME_ACTION)
    {
        return (RENAME_NAME);
    }
    else if (ulOperation == CREATE_NEW_ACTION)
    {
        return (CREATE_NEW_NAME);
    }
    else if (ulOperation == CHANGE_PROPERTIES_ACTION)
    {
        return (CHANGE_PROPERTIES_NAME);
    }
    else if (ulOperation == CHANGE_SECURITY_ACTION)
    {
        return (CHANGE_SECURITY_NAME);
    }
    else if (ulOperation == SAVE_AS_ACTION)
    {
        return (COPY_NAME);
    }
    else if (ulOperation == SEND_IM_ACTION)
    {
        return (SEND_IM_NAME);
    }
    else if (ulOperation == CUT_PASTE_ACTION)
    {
        return (CUT_PASTE_NAME);
    }
    else if (ulOperation == COPY_PASTE_ACTION)
    {
        return (COPY_PASTE_NAME);
    }
    else if (ulOperation == BATCH_ACTION)
    {
        return (BATCH_NAME);
    }
    else if (ulOperation == BURN_ACTION)
    {
        return (BURN_NAME);
    }
    else if (ulOperation == COPY_ACTION)
    {
        return (COPY_NAME);
    }
    else if (ulOperation == MOVE_ACTION)
    {
        return (MOVE_NAME);
    }
    else if (ulOperation == SHARE_ACTION)
    {
        return (SHARE_NAME);
    }
    else if (ulOperation == EMAIL_ACTION)
    {
        return (EMAIL_NAME);
    }
    else if (ulOperation == PRINT_ACTION)
    {
        return (PRINT_NAME);
    }
    else if (ulOperation == EMBED_ACTION)
    {
        return (EMBED_NAME);
    }
    else if (ulOperation == STOP_AGENT_ACTION)
    {
        return (STOP_AGENT_NAME);
    }
    return UNKNOWN_NAME;
}

#endif