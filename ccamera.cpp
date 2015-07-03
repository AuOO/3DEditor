#include "ccamera.h"

CCamera::CCamera()
{
    basis.isIdentity();
}

void CCamera::Zoom(double value)
{
    basis.SetValue(0,0,basis.GetIx()/value);
    basis.SetValue(1,1,basis.GetJy()/value);
    basis.SetValue(2,2,basis.GetKz()/value);
    basis.SetValue(3,3,basis.GetCh()/value);
}
