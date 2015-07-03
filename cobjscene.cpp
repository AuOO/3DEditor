#include "cobjscene.h"
#include "iostream"

char* CObjScene::GetName() const
{
    return (char*)name.c_str();
}

void CObjScene::SetName(const char *value)
{
    name.clear();
    name.append(value);
}
CObjScene::CObjScene()
{
    basis.isIdentity();
}

void CObjScene::DrawPolygon(HDC hDc, CCamera cam, int num, bool sel, bool sedge)
{
    POINT4D screen,scene,poly4D[3],n,t1,t2;
    POINT poly[3];
    FACE face;
    int light;
    HBRUSH hBrush;
    HPEN hPen;

    if(num<aFace.size())
    {
        face = aFace.at(num);
        PointsFace(face,poly4D[0],poly4D[1],poly4D[2]);
        //переводим координаты
        for(int j=0;j<3;j++)
        {
            scene = basis*poly4D[j];
            screen = cam.GetBasis()*scene;
            //вроде масштабирование ??
            if(screen.h!=0)
            {
                screen.x /= screen.h;
                screen.y /= screen.h;
                screen.z /= screen.h;
            }
            poly[j].x = screen.x;
            poly[j].y = screen.y;
        }

        t1.x = poly4D[1].x - poly4D[0].x;
        t1.y = poly4D[1].y - poly4D[0].y;
        t1.z = poly4D[1].z - poly4D[0].z;
        t1.h = 1;

        t2.x = poly4D[2].x - poly4D[0].x;
        t2.y = poly4D[2].y - poly4D[0].y;
        t2.z = poly4D[2].z - poly4D[0].z;
        t2.h = 1;

        n.x = t1.y*t2.z-t1.z*t2.y;
        n.y = -(t1.x*t2.z-t1.z*t2.x);
        n.z = t1.x*t2.y-t1.y*t2.x;

        n.x /= sqrt(n.x*n.x+n.y*n.y+n.z*n.z);
        n.y /= sqrt(n.x*n.x+n.y*n.y+n.z*n.z);
        n.z /= sqrt(n.x*n.x+n.y*n.y+n.z*n.z);

        n = basis*n;
        n = cam.GetBasis()*n;

        light = abs((int)(n.z*100/sqrt(n.x*n.x+n.y*n.y+n.z*n.z)));
        if(!sel)
        {
            hBrush = CreateSolidBrush(RGB(0,20,40+(int)light));
            if(!sedge)
                hPen = CreatePen(PS_ALTERNATE,1,RGB(0,20,40+(int)light));
            else
                hPen = CreatePen(PS_ALTERNATE,1,RGB(0,100,255));
        }
        else
        {
            hPen = CreatePen(PS_ALTERNATE,1,RGB(200,50,50));
            hBrush = CreateSolidBrush(RGB(50+(int)light,0,0));
        }
        SelectObject(hDc,hPen);
        SelectObject(hDc,hBrush);
        Polygon(hDc,poly,3);
        DeleteObject(hBrush);
        DeleteObject(hPen);
    }
}

void CObjScene::CreateRectangle(int width,int height,int part,POINT4D point)
{
    aVer.clear();
    aEdge.clear();
    aFace.clear();
    POINT4D temp = point;
    int stepX = width/part,stepY = height/part;
    int count = (part+1)*(part+1);
    for(int i=0;i<part+1;i++)
    {
        for(int j=0;j<part+1;j++)
        {
            AddVertex(temp);
            temp.x += stepX;
        }
        temp.x = point.x;
        temp.y += stepY;
    }

    for(int i=0;i<part;i++)
        for(int j=0;j<part;j++)
        {
            AddFaceV(j+(part+1)*i,j+(part+1)*i+1,j+(part+1)*(i+1));
            AddFaceV(count-j-1-(part+1)*i,count-j-2-(part+1)*i,count-j-1-(part+1)*(i+1));
        }

}

int CObjScene::GetDepth(CCamera cam,int num)
{
    POINT4D screen,scene,poly4D[3];
    FACE face;
    int avgZ = 0;
    face = aFace.at(num);
    PointsFace(face,poly4D[0],poly4D[1],poly4D[2]);
    //переводим координаты
    for(int i=0;i<3;i++)
    {
        scene = basis*poly4D[i];
        screen = cam.GetBasis()*scene;
        avgZ += screen.z;
    }
    avgZ = avgZ/3;
    return avgZ;
}

int CObjScene::GetDepthV(CCamera cam, int value)
{
    POINT4D screen,scene;
    scene = aVer.at(value).point;

    scene = basis*scene;
    screen = cam.GetBasis()*scene;
    return screen.z;
}

bool CObjScene::ComparePoint(POINT4D val1, POINT4D val2)
{
    if(val1.x==val2.x && val1.y==val2.y && val1.z==val2.z && val1.h==val2.h)
        return true;
    else
        return false;
}

void CObjScene::AssociateEdge(int value)
{
    //связаываем массив ребер и граней
    FACE face;
    //ищем грани у которых есть такое ребро (граней может быть много)
    //проверяем есть ли связь ребра с гранью, если нет, то добавляем
    for(int i=0;i<aFace.size();i++)
    {
        face = aFace.at(i);
        for(int j=0;j<3;j++)
            if(face.edge[j] == value)
            {
                if(!IsEdgeWithFace(value,i))
                {
                    aEdge[value].face.push_back(i);
                    for(int k=0;k<3;k++)
                    {
                        if(face.edge[k]!=value)
                            aEdge[value].edge.push_back(face.edge[k]);
                    }
                }
            }
    }
}

int CObjScene::FindVertex(POINT4D value)
{
    VERTEX vertex;
    for(int i=0;i<aVer.size();i++)
    {
        vertex = aVer.at(i);
        if(ComparePoint(vertex.point,value))
            return i;
    }
    return -1;
}

int CObjScene::FindEdge(EDGE value)
{
    EDGE edge;
    for(int i=0;i<aEdge.size();i++)
    {
        edge = aEdge.at(i);
        if(edge.ver[0]==value.ver[0]&&edge.ver[1]==value.ver[1])
            return i;
        if(edge.ver[0]==value.ver[1]&&edge.ver[1]==value.ver[0])
            return i;
    }
    return -1;
}

int CObjScene::FindFace(FACE value)
{
    FACE face;
    for(int i=0;i<aFace.size();i++)
    {
        face = aFace.at(i);
        for(int j=0;j<3;j++)
        {
            if(face.edge[j]==value.edge[0]||face.edge[j]==value.edge[1]||face.edge[j]==value.edge[2])
            {
                if(j==2)
                    return i;
            }
            else
                return -1;
        }
    }
    return -1;
}

bool CObjScene::IsEdgeWithFace(int iEdge, int iFace)
{
    EDGE edge = aEdge.at(iEdge);
    for(int i=0;i<edge.face.size();i++)
        if(edge.face.at(i)==iFace)
            return true;
    return false;
}

int CObjScene::GetCountVertex()
{
    return aVer.size();
}

int CObjScene::GetCountEdge()
{
    return aEdge.size();
}

int CObjScene::GetCountFace()
{
    return aFace.size();
}

void CObjScene::MoveVertex(int index, POINT4D point)
{
    aVer.at(index).point.x += point.x;
    aVer.at(index).point.y += point.y;
    aVer.at(index).point.z += point.z;
    aVer.at(index).point.h += point.h;
}

void CObjScene::CopyVertex(int index)
{
    VERTEX temp = aVer.at(index);
    temp.edge.clear();
    aVer.push_back(temp);
}

void CObjScene::PointsFace(FACE face, POINT4D &point1, POINT4D &point2, POINT4D &point3)
{
    POINT4D point;
    EDGE edge;
    //берем первое ребро
    edge = aEdge.at(face.edge[0]);
    //достаем первые 2 координаты грани
    point1 = aVer[edge.ver[0]].point;
    point2 = aVer[edge.ver[1]].point;
    //берем след. ребро и достаем 3ю координату
    edge = aEdge.at(face.edge[1]);
    point = aVer[edge.ver[0]].point;
    //если такой координаты нет, то берем ее
    if(!ComparePoint(point,point1)&&!ComparePoint(point,point2))
        point3 = point;
    else
    {
        //если не подошла первая координата, то 2ая подойдет
        point = aVer[edge.ver[1]].point;
        point3 = point;
    }
}

void CObjScene::PointsFace(FACE face, int &point1, int &point2, int &point3)
{
    POINT4D point,p1,p2;
    EDGE edge;
    //берем первое ребро
    edge = aEdge.at(face.edge[0]);
    //достаем индексы первых 2 вершин
    p1 = aVer[edge.ver[0]].point;
    p2 = aVer[edge.ver[1]].point;
    point1 = edge.ver[0];
    point2 = edge.ver[1];

    //берем след. ребро и достаем 3ю координату
    edge = aEdge.at(face.edge[1]);
    point = aVer[edge.ver[0]].point;
    //если такой координаты нет, то берем ее
    if(!ComparePoint(point,p1)&&!ComparePoint(point,p2))
        point3 = edge.ver[0];
    else
        point3 = edge.ver[1];
}

VERTEX CObjScene::GetVertex(int index)
{
    return aVer.at(index);
}

EDGE CObjScene::GetEdge(int index)
{
    return aEdge.at(index);
}

FACE CObjScene::GetFace(int index)
{
    return aFace.at(index);
}

void CObjScene::AddEdge(POINT4D beg,POINT4D end)
{
    VERTEX begV,endV;
    EDGE edge;
    int index;
    //если вершины не равны
    if(!ComparePoint(beg,end))
    {
        begV.point = beg;
        endV.point = end;
        //ищем похожие вершины в массиве вершин
        index = FindVertex(begV.point);
        //если не нашли, то добавляем вершины, иначе ставим уже существующие индексы вершин
        if(index==-1)
        {
            aVer.push_back(begV);
            edge.ver[0] = aVer.size()-1;
        }
        else
            edge.ver[0] = index;

        //для второй вершины
        index = FindVertex(endV.point);
        if(index==-1)
        {
            aVer.push_back(endV);
            edge.ver[1] = aVer.size()-1;
        }
        else
            edge.ver[1] = index;

        //ищем похожие ребра в массиве ребер
        index = FindEdge(edge);
        //если не нашли, то добавляем новое, иначе удаляем из массива вершин 2 последнии добавленные
        if(index==-1)
        {
            aEdge.push_back(edge);
            //добавляем двум новым вершинам, индекс последнего ребра
            aVer[edge.ver[0]].edge.push_back(aEdge.size()-1);
            aVer[edge.ver[1]].edge.push_back(aEdge.size()-1);
        }
        else
        {
            aVer.pop_back();
            aVer.pop_back();
        }
    }
}

void CObjScene::AddEdge(int beg, int end)
{
    EDGE edge;
    edge.ver[0] = beg;
    edge.ver[1] = end;
    if(FindEdge(edge)==-1)
    {
        aEdge.push_back(edge);
        aVer[beg].edge.push_back(aEdge.size()-1);
        aVer[end].edge.push_back(aEdge.size()-1);
    }
}

void CObjScene::AddFaceV(int val1, int val2, int val3)
{
    POINT4D p1,p2,p3;
    FACE face;
    EDGE edge;
    //индексы ребер
    int iEdge1,iEdge2,iEdge3;

    //если ребра существуют то используем их, если нет, то создаем новые
    edge.ver[0] = val1;
    edge.ver[1] = val2;
    iEdge1 = FindEdge(edge);
    if(iEdge1==-1)
    {
        AddEdge(val1,val2);
        iEdge1 = aEdge.size()-1;
    }

    edge.ver[0] = val1;
    edge.ver[1] = val3;
    iEdge2 = FindEdge(edge);
    if(iEdge2==-1)
    {
        AddEdge(val1,val3);
        iEdge2 = aEdge.size()-1;
    }

    edge.ver[0] = val2;
    edge.ver[1] = val3;
    iEdge3 = FindEdge(edge);
    if(iEdge3==-1)
    {
        AddEdge(val2,val3);
        iEdge3 = aEdge.size()-1;
    }

    face.edge[0] = iEdge1;
    face.edge[1] = iEdge2;
    face.edge[2] = iEdge3;
    if(FindFace(face)==-1)
    {
        p1 = aVer.at(val1).point;
        p2 = aVer.at(val2).point;
        p3 = aVer.at(val3).point;

        p2.x -= p1.x;
        p2.y -= p1.y;
        p2.z -= p1.z;

        p3.x -= p1.x;
        p3.y -= p1.y;
        p3.z -= p1.z;


        aFace.push_back(face);
        AssociateEdge(iEdge1);
        AssociateEdge(iEdge2);
        AssociateEdge(iEdge3);
    }
}

void CObjScene::DeleteFace(int index)
{
    FACE face = aFace.at(index);
    EDGE edge;
    aFace.erase(aFace.begin()+index);
    //каскадное удаление ВСЕГО о_О
    //берем ребра удаляемой грани и в массиве aEdge удаляем связь с гранью
    for(int i=0;i<3;i++)
    {
        edge = aEdge.at(face.edge[i]);
        for(int j=0;j<edge.face.size();j++)
        {
            if(aEdge.at(face.edge[i]).face.at(j)==index)
            {
                aEdge.at(face.edge[i]).face.erase(aEdge.at(face.edge[i]).face.begin()+j);
                break;
            }
        }
        //удаляем связи с другими ребрами в массиве aEdge ???
        for(int j=0;j<aEdge.size();j++)
        {
            for(int k=0;k<aEdge.at(j).edge.size();k++)
            {
                if(aEdge.at(j).edge.at(k)==face.edge[i]&&aEdge.at(face.edge[i]).face.empty())
                {
                    aEdge.at(j).edge.erase(aEdge.at(j).edge.begin()+k);
                    k--;
                }
            }
        }
    }
    //удаляем ребра у которых нету связи с гранями
    for(int i=0;i<aEdge.size();i++)
    {
        if(aEdge.at(i).face.empty())
        {
            //удаляем связи вершин с ребрами
            for(int j=0;j<2;j++)
            {
                for(int k=0;k<aVer.at(aEdge.at(i).ver[j]).edge.size();k++)
                {
                    if(aVer.at(aEdge.at(i).ver[j]).edge.at(k)==i)
                    {
                        aVer.at(aEdge.at(i).ver[j]).edge.erase(aVer.at(aEdge.at(i).ver[j]).edge.begin()+k);
                        break;
                    }
                }
            }
            //пересчитываем индексы ребер в массиве вершин
            for(int j=0;j<aVer.size();j++)
            {
                for(int k=0;k<aVer.at(j).edge.size();k++)
                {
                    if(aVer.at(j).edge.at(k)>i)
                        aVer.at(j).edge[k]--;
                }
            }
            //пересчитываем индексы ребер в массиве граней
            for(int j=0;j<aFace.size();j++)
            {
                for(int k=0;k<3;k++)
                {
                    if(aFace.at(j).edge[k]>i)
                        aFace.at(j).edge[k]--;
                }
            }
            //пересчитываем индексы ребер в массиве ребер ??
            /*for(int j=0;j<aEdge.size();j++)
            {
                for(int k=0;k<aEdge.at(j).edge.size();k++)
                {
                    if(aEdge.at(j).edge[k]>i)
                        aEdge.at(j).edge[k]--;
                }
            }*/
            aEdge.erase(aEdge.begin()+i);
            i--;
        }
    }
    //удаляем вершины у которых нету связи с ребрами
    for(int i=0;i<aVer.size();i++)
    {
        if(aVer.at(i).edge.empty())
        {
            //перестчиываем индексы вершин в массиве ребер
            for(int j=0;j<aEdge.size();j++)
            {
                if(aEdge.at(j).ver[0]>i)
                    aEdge.at(j).ver[0]--;
                if(aEdge.at(j).ver[1]>i)
                    aEdge.at(j).ver[1]--;
            }
            aVer.erase(aVer.begin()+i);
            i--;
        }
    }
    //устанавливаем новые индексы на грани в массиве ребер
    for(int i=0;i<aEdge.size();i++)
    {
        edge = aEdge.at(i);
        for(int j=0;j<edge.face.size();j++)
        {
            if(aEdge.at(i).face[j]>index)
                aEdge.at(i).face[j]--;
        }
    }
}

void CObjScene::AddVertex(POINT4D value)
{
    VERTEX vertex;
    vertex.point = value;
    if(FindVertex(value)==-1)
        aVer.push_back(vertex);
}

void CObjScene::AddFaceE(int val1, int val2, int val3)
{
    FACE face;
    face.edge[0] = val1;
    face.edge[1] = val2;
    face.edge[2] = val3;
    if(FindFace(face)==-1)
    {
        aFace.push_back(face);
        AssociateEdge(val1);
        AssociateEdge(val2);
        AssociateEdge(val3);
    }
}
