#ifndef COBJSCENE_H
#define COBJSCENE_H

#include "cabstractobjscene.h"
#include "ccamera.h"
#include "vector"
#include "algorithm"
#include "string"

//режимы вывода полигонов:
#define MOD_VERTEX 0
#define MOD_EDGE 1
#define MOD_POLYGON 2

//крылатая структура хранения полигонной сетки
struct VERTEX
{
    POINT4D point;
    vector<int> edge;
};

struct EDGE
{
    int ver[2];
    vector<int> face;
    vector<int> edge;
};

struct FACE
{
    int edge[3];
};

using namespace std;

class CObjScene : public CAbstractObjScene
{
private:
    vector <VERTEX> aVer;
    vector <EDGE> aEdge;
    vector <FACE> aFace;
protected:
    string name;
public:
    CObjScene();
    void DrawPolygon(HDC,CCamera,int,bool,bool);  //отрисовка полигона
    int GetDepth(CCamera,int);          //глубина отдельного полигона
    int GetDepthV(CCamera,int);         //глубина вершины

    //Примитивы:
    void CreateRectangle(int,int,int,POINT4D);  //создание прямоугольника

    bool ComparePoint(POINT4D,POINT4D); //сравнение вершины, true - если одинаковые
    void AssociateEdge(int);            //создает связи для "крылатой" структуры хранения

    void AddVertex(POINT4D);            //добавление новой вершины
    void AddEdge(POINT4D,POINT4D);      //добавление ребра и новых вершин
    void AddEdge(int,int);              //добавление ребра по существующим вершинам
    void AddFaceE(int,int,int);         //добавление грани по существующим ребрам
    void AddFaceV(int,int,int);         //добавление грани по существующим вершинам

    void DeleteFace(int);               //удаление грани по индексу

    int FindVertex(POINT4D);            //поиск вершины в aVer, -1 - если ненайдена, index - если найдена
    int FindEdge(EDGE);                 //поиск ребра в aEdge, -1 - если ненайден, index - если найден
    int FindFace(FACE);                 //поиск грани в aFace, -1 - если ненайдена, index - если найдена

    bool IsEdgeWithFace(int,int);       //true - если ребро имеет связь с гранью (в EDGE проверяем массив face)

    int GetCountVertex();               //количество вершин
    int GetCountEdge();                 //количество ребер
    int GetCountFace();                 //количество граней

    void MoveVertex(int,POINT4D);       //перемещение вершины
    void CopyVertex(int);               //копирует вершину

    void PointsFace(FACE,POINT4D&,POINT4D&,POINT4D&);   //возвращает вершины грани
    void PointsFace(FACE,int&,int&,int&);               //возвращает индексы

    VERTEX GetVertex(int);              //возвращает вершину по индексу
    EDGE GetEdge(int);                  //возвращает ребро по индексу
    FACE GetFace(int);                  //возвращает грань по индексу

    char* GetName() const;              //возвращает имя объекта
    void SetName(const char *value);   //задает имя объекта
};

#endif // COBJSCENE_H
