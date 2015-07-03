#ifndef COORDSYSTEM_H
#define COORDSYSTEM_H
#include "cabstractobjscene.h"
#include "ccamera.h"

class CCoordSystem : public CAbstractObjScene
{
private:
    int lenght; //длинна осей
public:
    CCoordSystem(int value);
    void Draw(HDC,CCamera);
    int GetLenght() const;
    void SetLenght(int value);
};

#endif // COORDSYSTEM_H
