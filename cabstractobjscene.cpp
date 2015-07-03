#include "cabstractobjscene.h"

CAbstractObjScene::CAbstractObjScene()
{
}

Matrix CAbstractObjScene::GetBasis() const
{
    return basis;
}

void CAbstractObjScene::SetBasis(const Matrix &value)
{
    basis = value;
}

void CAbstractObjScene::RotationX(double angle, bool flag)
{
    Matrix rotxM;
    rotxM.isIdentity();
    rotxM.SetValue(1,1,cos(angle));
    rotxM.SetValue(1,2,-sin(angle));
    rotxM.SetValue(2,1,sin(angle));
    rotxM.SetValue(2,2,cos(angle));
    if(flag)
        basis = basis*rotxM;    //�������� ������������ ������� ���� (x ����������)
    else
        basis = rotxM*basis;    //�������� ������������ ������ k
}

void CAbstractObjScene::RotationY(double angle,bool flag)
{
    Matrix rotyM;
    rotyM.isIdentity();
    rotyM.SetValue(0,0,cos(angle));
    rotyM.SetValue(0,2,-sin(angle));
    rotyM.SetValue(2,0,sin(angle));
    rotyM.SetValue(2,2,cos(angle));
    if(flag)
        basis = basis*rotyM;    //�������� ������������ ������� ���� (y ����������)
    else
        basis = rotyM*basis;    //�������� ������������ ������ k
}

void CAbstractObjScene::RotationZ(double angle,bool flag)
{
    Matrix rotzM;
    rotzM.isIdentity();
    rotzM.SetValue(0,0,cos(angle));
    rotzM.SetValue(0,1,-sin(angle));
    rotzM.SetValue(1,0,sin(angle));
    rotzM.SetValue(1,1,cos(angle));
    if(flag)
        basis = basis*rotzM;    //�������� ������������ ������� ���� (z ����������)
    else
        basis = rotzM*basis;    //�������� ������������ ������ k
}

void CAbstractObjScene::MoveX(int value)
{
    Matrix movxM;
    movxM.isIdentity();
    movxM.SetValue(0,3,value);
    basis = movxM*basis;
}

void CAbstractObjScene::MoveY(int value)
{
    Matrix movyM;
    movyM.isIdentity();
    movyM.SetValue(1,3,value);
    basis = movyM*basis;
}

void CAbstractObjScene::MoveZ(int value)
{
    Matrix movzM;
    movzM.isIdentity();
    movzM.SetValue(2,3,value);
    basis = movzM*basis;
}

void CAbstractObjScene::ResetPos()
{
    basis.isIdentity();
}
