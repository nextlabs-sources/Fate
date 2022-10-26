

#ifndef _BASE_OBJ_H_
#define _BASE_OBJ_H_

class CBaseObj
{
public:
    CBaseObj(){};
    virtual ~CBaseObj(){};

protected:
    virtual HRESULT OnConnection(LPDISPATCH Application, int ConnectMode, LPDISPATCH AddInInst, SAFEARRAY * * custom) = 0;
    virtual HRESULT OnDisconnection(int RemoveMode, SAFEARRAY * * custom) = 0;

private:
};

#endif