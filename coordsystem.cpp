#include "coordsystem.h"

CCoordSystem::CCoordSystem(int value)
{
    basis.isIdentity();
    lenght = value;
}

void CCoordSystem::Draw(HDC hDc,CCamera cam)
{
    //экранные координаты
    int xs,ys,zs;
    //ось Х
    MoveToEx(hDc,cam.GetBasis().GetIh(),cam.GetBasis().GetJh(),NULL);
    xs = cam.GetBasis().GetIx()*lenght + cam.GetBasis().GetIh();
    ys = cam.GetBasis().GetJx()*lenght + cam.GetBasis().GetJh();
    TextOutA(hDc,xs+10,ys,"x",strlen("x"));
    LineTo(hDc,xs,ys);

    //ось Y
    MoveToEx(hDc,cam.GetBasis().GetIh(),cam.GetBasis().GetJh(),NULL);
    xs = cam.GetBasis().GetIy()*lenght + cam.GetBasis().GetIh();
    ys = cam.GetBasis().GetJy()*lenght + cam.GetBasis().GetJh();
    TextOutA(hDc,xs+10,ys,"y",strlen("y"));
    LineTo(hDc,xs,ys);

    //ось Z
    MoveToEx(hDc,cam.GetBasis().GetIh(),cam.GetBasis().GetJh(),NULL);
    xs = cam.GetBasis().GetIz()*lenght + cam.GetBasis().GetIh();
    ys = cam.GetBasis().GetJz()*lenght + cam.GetBasis().GetJh();
    TextOutA(hDc,xs,ys-10,"z",strlen("z"));
    LineTo(hDc,xs,ys); 
}

int CCoordSystem::GetLenght() const
{
    return lenght;
}

void CCoordSystem::SetLenght(int value)
{
    lenght = value;
}
