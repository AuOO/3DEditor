#ifndef CCAMERA_H
#define CCAMERA_H
#include "cabstractobjscene.h"

class CCamera : public CAbstractObjScene
{
public:
    CCamera();
    void Zoom(double);
};

#endif // CCAMERA_H
