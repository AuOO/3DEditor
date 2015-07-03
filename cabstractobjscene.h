#ifndef CABSTRACTOBJSCENE_H
#define CABSTRACTOBJSCENE_H
#include "windows.h"
#include "windowsx.h"
#include "vector"
#include "matrix.h"
#include "math.h"

using namespace std;

class CAbstractObjScene
{
protected:
    Matrix basis;
public:
    CAbstractObjScene();
    void RotationX(double,bool);
    void RotationY(double,bool);
    void RotationZ(double,bool);
    void MoveX(int);
    void MoveY(int);
    void MoveZ(int);
    void ResetPos();
    Matrix GetBasis() const;
    void SetBasis(const Matrix &value);
};

#endif // CABSTRACTOBJSCENE_H
