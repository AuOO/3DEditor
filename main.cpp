#include "cabstractobjscene.h"
#include "cobjscene.h"
#include "ccamera.h"
#include "coordsystem.h"
#include "matrix.h"
#include "commctrl.h"
#include "string.h"
#include "iostream"
#include "sstream"

#define ID_CHILDWINDOW      1
#define ID_ROTATEBUTTONX    2
#define ID_ROTATEBUTTONY    3
#define ID_ROTATEBUTTONZ    4
#define ID_EDITANGLE        5
#define ID_MENUNEWFILE      6
#define ID_MENUOPENFILE     7
#define ID_MENUSAVE         8
#define ID_MENUSAVEAS       9
#define ID_MENUEXIT         10
#define ID_ROTATECHECK      11
#define ID_LISTVIEWOBJ      12
#define ID_LISTVIEWVER      13
#define ID_LISTVIEWEDGE     14
#define ID_LISTVIEWFACE     15
#define ID_COMBOBOXVEF      16
#define ID_COMBOBOXOP       17
#define ID_RADIOBPOLYGON    18
#define ID_RADIOBVERTEX     19
#define ID_STATICX          20
#define ID_STATICY          21
#define ID_STATICZ          22
#define ID_EDITX            23
#define ID_EDITY            24
#define ID_EDITZ            25
#define ID_BUTMOVE          26
#define ID_BUTADDFACE       27
#define ID_NEWVERTEX        28
#define ID_BUTCREATEPR      29
#define ID_EDITCOUNT        30
#define ID_EDITW            31
#define ID_EDITH            32
#define ID_EDITNAME         33
#define ID_BUTHELP          34

#define CBID_VERTEX         100
#define CBID_EDGE           101
#define CBID_FACE           102

#define CBID_CREATEPRIM     103
#define CBID_OBJECTS        104
#define CBID_MODIFICATION   105

#define POPID_ADDFACE       200
struct INDPOLYGON
{
    int iObj;
    int iFace;
    int depth;
    bool isSel;
};

struct INDVERTEX
{
    int iObj;
    int iVertex;
    int depth;
    bool isSel;
};

HWND hMainWnd,hChild,hListViewVer, hListViewEdge, hListViewFace, hListViewObj;
HWND hComboBoxVEF, hComboBoxOP;
HWND hEditX, hEditY, hEditZ, hStaticX, hStaticY, hStaticZ, hButPolygon, hButVertex, hButShowVer;
HWND hButMove, hButAddFace;
HWND hButCreatePr, hEditCount, hEditW, hEditH, hEditName;
HINSTANCE hInstance;
CCamera cam;

vector <CObjScene> allObj;
vector <INDPOLYGON> buffer;
vector <INDVERTEX> points;
int curObj;
bool bCsys;
bool bSEdge;
bool bAllSel;
bool bHelp;

LRESULT CALLBACK MainWinProc(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK ChildWinProc(HWND,UINT,WPARAM,LPARAM);

void FormScreenBuffer(CCamera);                 //формирует массив полигонов buffer на основе массива allObj
void FormPointsBuffer(CCamera);                 //формирует массив индексов точек
void AddPointToBuffer(CCamera,int,int);         //добавляет одну точку в массив индексов
void ReformDepthBuffer(CCamera);                //пересчитывает depth для каждого элемента в массиве buffer
void ReformIndexBuffer(int,int);                //смещает все индексы граней на 1 начиная с заданной у объекта
void FormListViewObj(HWND);                     //формирует лист объектов на основе массива allObj
void FormListViewVer(HWND,int);                 //формирует лист вершин на основе объекта из allObj по указанному индексу
void FormListViewEdge(HWND,int);                //формирует лист ребер на основе объекта из allObj по указанному индексу
void FormListViewFace(HWND,int);                //формирует лист грань на основе объекта из allObj по указанному индексу
void QuickSortBuffer(int,int);                  //быстрая сортировка массива buffer
void AddListElm(HWND,LPSTR[],int,int);          //добавление нового элемента в указанный лист
bool IsBelongsTriangle(POINT,INDPOLYGON,CCamera);  //проверка принадлежности точки треугольнику в системе экрана
void UnSelectAllPolygon();                      //снимает выделение со всех полигонов в buffer
void SelectAllPolygon(int);                     //выделяет все полигоны заданного объекта
void SelectAllVertex(int);                      //выделяет все вершины заданного объекта
void SaveObj(LPSTR,int);                        //сохранить в формате obj
void LoadObj(LPSTR);                            //загрузка формата obj

using namespace std;

void APIENTRY HandlePopMenu(HWND hWnd,POINT pt)
{
    HMENU hPopMenu;
    hPopMenu = CreatePopupMenu();
    AppendMenuA(hPopMenu,MF_STRING,POPID_ADDFACE,"Создать грань");
    AppendMenuA(hPopMenu,MF_STRING,0,"Удалить вершину");
    AppendMenuA(hPopMenu,MF_STRING,0,"Удалить выделенное");
    AppendMenuA(hPopMenu,MF_SEPARATOR,0,"");
    AppendMenuA(hPopMenu,MF_STRING,ID_MENUEXIT,"Выход");
    TrackPopupMenu(hPopMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x, pt.y,0, hWnd, NULL);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
    hInstance = hInst;
    WNDCLASSA wc;
    memset(&wc,0,sizeof(WNDCLASS));
    wc.style = CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc = MainWinProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(0,0,0));
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "MainWndClass";
    if(!RegisterClassA(&wc))
        return false;

    memset(&wc,0,sizeof(WNDCLASS));
    wc.lpfnWndProc = ChildWinProc;
    wc.hInstance = hInst;
    wc.hbrBackground = CreateSolidBrush(RGB(0,0,0));
    wc.lpszClassName = "ChildWndClass";
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    if(!RegisterClassA(&wc))
        return false;

    hMainWnd = CreateWindowA("MainWndClass","Example",WS_POPUPWINDOW,
        CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,(HWND)NULL,NULL,(HINSTANCE)hInst,NULL);
    if(!hMainWnd)
        return false;

    hChild = CreateWindowA("ChildWndClass",NULL,WS_CHILD|WS_BORDER|WS_VISIBLE ,0,0,
        1600,1055,hMainWnd,(HMENU)ID_CHILDWINDOW,(HINSTANCE)hInst,NULL);

    ShowWindow(hMainWnd,SW_SHOWMAXIMIZED);
    UpdateWindow(hMainWnd);

    MSG msg;
    while(GetMessageA(&msg,(HWND)NULL,0,0))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}
LRESULT CALLBACK MainWinProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lp)
{
    OPENFILENAMEA ofName;
    char szFile[260];
    string name;
    CObjScene temp;
    RECT rect;
    HMENU hMenu,hPopMenu;
    LVCOLUMNA lvC;
    LPNMLISTVIEW pnmLV;
    POINT4D point;
    char text[5];
    int angle;
    int index;
    bool check;

    switch(msg)
    {
        case WM_CREATE:
            bCsys = true;
            bSEdge = true;
            bAllSel = false;
            bHelp = false;
            SetWindowTextA(hWnd,"My 3DEditor");
            hMenu = CreateMenu();
            hPopMenu = CreatePopupMenu();

            AppendMenuA(hMenu,MF_STRING|MF_POPUP,(UINT)hPopMenu,"Файл");
            {
                AppendMenuA(hPopMenu,MF_STRING,ID_MENUNEWFILE,"Новый файл");
                AppendMenuA(hPopMenu,MF_STRING,ID_MENUOPENFILE,"Открыть файл");
                AppendMenuA(hPopMenu,MF_SEPARATOR,0,"");

                AppendMenuA(hPopMenu,MF_STRING,ID_MENUSAVE,"Сохранить");
                AppendMenuA(hPopMenu,MF_STRING,ID_MENUSAVEAS,"Сохранить как");

                AppendMenuA(hPopMenu,MF_SEPARATOR,0,"");
                AppendMenuA(hPopMenu,MF_STRING,ID_MENUEXIT,"Выход");
            }
            AppendMenuA(hMenu,MF_STRING,ID_BUTHELP,"Справка");
            SetMenu(hWnd,hMenu);

            CreateWindowA("static","",WS_CHILD|WS_VISIBLE,
                1605,0,315,1055,hWnd,0,NULL,NULL);

            CreateWindowA("static","Поворот камеры вокруг:",WS_CHILD|WS_VISIBLE,
                1615,10,240,30,hWnd,0,NULL,NULL);

            CreateWindowA("button","Оси X",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                1615,40,80,30,hWnd,(HMENU)ID_ROTATEBUTTONX,NULL,NULL);

            CreateWindowA("button","Оси Y",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                1700,40,80,30,hWnd,(HMENU)ID_ROTATEBUTTONY,NULL,NULL);

            CreateWindowA("button","Оси Z",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                1790,40,80,30,hWnd,(HMENU)ID_ROTATEBUTTONZ,NULL,NULL);

            CreateWindowA("edit","0",WS_CHILD|WS_VISIBLE|ES_LEFT|WS_BORDER,
                1615,80,80,30,hWnd,(HMENU)ID_EDITANGLE,NULL,NULL);

            CreateWindowA("button","Закрепить ось",WS_CHILD|WS_VISIBLE|BS_CHECKBOX,
                1700,80,180,30,hWnd,(HMENU)ID_ROTATECHECK,NULL,NULL);

            //вкладка "Модификация"
            hButPolygon = CreateWindowA("button","Полигоны",WS_CHILD|BS_RADIOBUTTON,
                1615,150,110,30,hWnd,(HMENU)ID_RADIOBPOLYGON,NULL,NULL);

            CheckDlgButton(hWnd,ID_RADIOBPOLYGON,BST_CHECKED);

            hButVertex = CreateWindowA("button","Вершины",WS_CHILD|BS_RADIOBUTTON,
                1615,180,110,30,hWnd,(HMENU)ID_RADIOBVERTEX,NULL,NULL);

            hButShowVer = CreateWindowA("button","Новая вершина",WS_CHILD|BS_CHECKBOX,
                1615,350,200,30,hWnd,(HMENU)ID_NEWVERTEX,NULL,NULL);

            hStaticX = CreateWindowA("static","x :",WS_CHILD,
                1615,210,80,30,hWnd,(HMENU)ID_STATICX,NULL,NULL);

            hStaticY = CreateWindowA("static","y :",WS_CHILD,
                1700,210,80,30,hWnd,(HMENU)ID_STATICY,NULL,NULL);

            hStaticZ = CreateWindowA("static","z :",WS_CHILD,
                1785,210,80,30,hWnd,(HMENU)ID_STATICZ,NULL,NULL);

            hEditX = CreateWindowA("edit","0",WS_CHILD|ES_LEFT|WS_BORDER,
                1615,240,80,30,hWnd,(HMENU)ID_EDITX,NULL,NULL);

            hEditY = CreateWindowA("edit","0",WS_CHILD|ES_LEFT|WS_BORDER,
                1700,240,80,30,hWnd,(HMENU)ID_EDITY,NULL,NULL);

            hEditZ = CreateWindowA("edit","0",WS_CHILD|ES_LEFT|WS_BORDER,
                1785,240,80,30,hWnd,(HMENU)ID_EDITZ,NULL,NULL);

            hButMove = CreateWindowA("button","Переместить",WS_CHILD|BS_PUSHBUTTON,
                1615,280,180,30,hWnd,(HMENU)ID_BUTMOVE,NULL,NULL);

            hButAddFace = CreateWindowA("button","Создать грань",WS_CHILD|BS_PUSHBUTTON,
                1615,315,180,30,hWnd,(HMENU)ID_BUTADDFACE,NULL,NULL);

            //вкладка "Создание примитивов"
            hEditName = CreateWindowA("edit","Название",WS_CHILD|ES_LEFT|WS_BORDER,
                1615,150,100,30,hWnd,(HMENU)ID_EDITNAME,NULL,NULL);

            hEditCount = CreateWindowA("edit","Части",WS_CHILD|ES_LEFT|WS_BORDER,
                1616,185,80,30,hWnd,(HMENU)ID_EDITCOUNT,NULL,NULL);

            hEditW = CreateWindowA("edit","Ширина",WS_CHILD|ES_LEFT|WS_BORDER,
                1615,220,80,30,hWnd,(HMENU)ID_EDITW,NULL,NULL);

            hEditH = CreateWindowA("edit","Высота",WS_CHILD|ES_LEFT|WS_BORDER,
                1615,255,80,30,hWnd,(HMENU)ID_EDITH,NULL,NULL);

            hButCreatePr = CreateWindowA("button","Поверхность",WS_CHILD|BS_PUSHBUTTON,
                1615,290,180,30,hWnd,(HMENU)ID_BUTCREATEPR,NULL,NULL);

            //создание списка граней
            hListViewFace = CreateWindowA(WC_LISTVIEWA,NULL,WS_CHILD|LVS_REPORT,1615,750,295,300,hWnd,(HMENU)ID_LISTVIEWFACE,NULL,NULL);
            ListView_SetExtendedListViewStyleEx(hListViewFace,LVS_EX_GRIDLINES,LVS_EX_GRIDLINES);
            ListView_SetExtendedListViewStyleEx(hListViewFace,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);
            lvC.mask = LVCF_TEXT|LVCF_WIDTH;
            lvC.pszText=(LPSTR)"№";
            lvC.cx=30;
            SendMessageA(hListViewFace,LVM_INSERTCOLUMNA,0,(LPARAM)&lvC);
            lvC.pszText=(LPSTR)"Ребро №1";
            lvC.cx=85;
            SendMessageA(hListViewFace,LVM_INSERTCOLUMNA,1,(LPARAM)&lvC);
            lvC.pszText=(LPSTR)"Ребро №2";
            lvC.cx=85;
            SendMessageA(hListViewFace,LVM_INSERTCOLUMNA,2,(LPARAM)&lvC);
            lvC.pszText=(LPSTR)"Ребро №3";
            lvC.cx=85;
            SendMessageA(hListViewFace,LVM_INSERTCOLUMNA,3,(LPARAM)&lvC);

            //создание списка ребер
            hListViewEdge = CreateWindowA(WC_LISTVIEWA,NULL,WS_CHILD|LVS_REPORT,1615,750,295,300,hWnd,(HMENU)ID_LISTVIEWEDGE,NULL,NULL);
            ListView_SetExtendedListViewStyleEx(hListViewEdge,LVS_EX_GRIDLINES,LVS_EX_GRIDLINES);
            ListView_SetExtendedListViewStyleEx(hListViewEdge,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);
            lvC.pszText=(LPSTR)"№";
            lvC.cx=30;
            SendMessageA(hListViewEdge,LVM_INSERTCOLUMNA,0,(LPARAM)&lvC);
            lvC.pszText=(LPSTR)"x, y, z";
            lvC.cx=120;
            SendMessageA(hListViewEdge,LVM_INSERTCOLUMNA,1,(LPARAM)&lvC);
            lvC.pszText=(LPSTR)"x, y, z";
            lvC.cx=120;
            SendMessageA(hListViewEdge,LVM_INSERTCOLUMNA,2,(LPARAM)&lvC);

            //создание списка вершин
            hListViewVer = CreateWindowA(WC_LISTVIEWA,NULL,WS_CHILD|LVS_REPORT,1615,750,295,300,hWnd,(HMENU)ID_LISTVIEWVER,NULL,NULL);
            ListView_SetExtendedListViewStyleEx(hListViewVer,LVS_EX_GRIDLINES,LVS_EX_GRIDLINES);
            ListView_SetExtendedListViewStyleEx(hListViewVer,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);          
            lvC.pszText=(LPSTR)"№";
            lvC.cx=30;
            SendMessageA(hListViewVer,LVM_INSERTCOLUMNA,0,(LPARAM)&lvC);
            lvC.pszText=(LPSTR)"x, y, z";
            lvC.cx=150;
            SendMessageA(hListViewVer,LVM_INSERTCOLUMNA,1,(LPARAM)&lvC);

            //создание списка объектов
            hListViewObj = CreateWindowA(WC_LISTVIEWA,NULL,WS_CHILD|LVS_REPORT,1615,400,295,300,hWnd,(HMENU)ID_LISTVIEWOBJ,NULL,NULL);
            ListView_SetExtendedListViewStyleEx(hListViewObj,LVS_EX_GRIDLINES,LVS_EX_GRIDLINES);
            ListView_SetExtendedListViewStyleEx(hListViewObj,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);
            lvC.pszText=(LPSTR)"№";
            lvC.cx=30;
            SendMessageA(hListViewObj,LVM_INSERTCOLUMNA,0,(LPARAM)&lvC);
            lvC.pszText=(LPSTR)"Название";
            lvC.cx=100;
            SendMessageA(hListViewObj,LVM_INSERTCOLUMNA,1,(LPARAM)&lvC);

            //создание бокса для списков вершин, ребер и граней
            hComboBoxVEF = CreateWindowA(WC_COMBOBOXA,NULL,WS_CHILD|CBS_DROPDOWNLIST,1615,710,295,500,hWnd,(HMENU)ID_COMBOBOXVEF,NULL,NULL);
            SendMessageA(hComboBoxVEF,CB_ADDSTRING,(WPARAM)CBID_VERTEX,(LPARAM)"Вершины");
            SendMessageA(hComboBoxVEF,CB_ADDSTRING,(WPARAM)CBID_EDGE,(LPARAM)"Ребра");
            SendMessageA(hComboBoxVEF,CB_ADDSTRING,(WPARAM)CBID_FACE,(LPARAM)"Грани");

            //создание бокса для выбора опций
            hComboBoxOP = CreateWindowA(WC_COMBOBOXA,NULL,WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST,1615,120,295,500,hWnd,(HMENU)ID_COMBOBOXOP,NULL,NULL);
            SendMessageA(hComboBoxOP,CB_ADDSTRING,(WPARAM)CBID_CREATEPRIM,(LPARAM)"Создание примитивов");
            SendMessageA(hComboBoxOP,CB_ADDSTRING,(WPARAM)CBID_OBJECTS,(LPARAM)"Объекты сцены");
            SendMessageA(hComboBoxOP,CB_ADDSTRING,(WPARAM)CBID_MODIFICATION,(LPARAM)"Модификация");
            return 0;
        case WM_NOTIFY:
            LPNMHDR lpnmHdr;
            lpnmHdr = (LPNMHDR)lp;
            if((lpnmHdr->idFrom==ID_LISTVIEWOBJ)&&(lpnmHdr->code==NM_CLICK))
            {
                 pnmLV = (LPNMLISTVIEW)lp;
                 if(pnmLV->iItem!=-1)
                 {
                    FormListViewVer(hListViewVer,pnmLV->iItem);
                    FormListViewEdge(hListViewEdge,pnmLV->iItem);
                    FormListViewFace(hListViewFace,pnmLV->iItem);
                    if(GetKeyState(VK_LCONTROL)<0)
                    {
                        if(IsDlgButtonChecked(hWnd,ID_RADIOBPOLYGON))
                            SelectAllPolygon(pnmLV->iItem);
                        else
                            SelectAllVertex(pnmLV->iItem);
                        curObj = pnmLV->iItem;
                        GetClientRect(hChild,&rect);
                        InvalidateRect(hChild,&rect,false);
                        UpdateWindow(hChild);
                        SetFocus(hWnd);
                    }
                 }
            }
            return 0;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case ID_BUTHELP:
                    if(bHelp)
                        bHelp=false;
                    else
                        bHelp=true;
                    GetClientRect(hChild,&rect);
                    InvalidateRect(hChild,&rect,false);
                    UpdateWindow(hChild);
                    SetFocus(hWnd);
                break;
                case ID_ROTATEBUTTONX:
                    GetWindowTextA(GetDlgItem(hWnd,ID_EDITANGLE),text,5);
                    angle = atoi(text);
                    check = IsDlgButtonChecked(hWnd,ID_ROTATECHECK);
                    cam.RotationX(angle*M_PI/180,check);
                    ReformDepthBuffer(cam);
                    GetClientRect(hChild,&rect);
                    InvalidateRect(hChild,&rect,false);
                    UpdateWindow(hChild);
                    SetFocus(hWnd);
                break;
                case ID_ROTATEBUTTONY:
                    GetWindowTextA(GetDlgItem(hWnd,ID_EDITANGLE),text,5);
                    angle = atoi(text);
                    check = IsDlgButtonChecked(hWnd,ID_ROTATECHECK);
                    cam.RotationY(angle*M_PI/180,check);
                    ReformDepthBuffer(cam);
                    GetClientRect(hChild,&rect);
                    InvalidateRect(hChild,&rect,false);
                    UpdateWindow(hChild);
                    SetFocus(hWnd);
                break;
                case ID_ROTATEBUTTONZ:
                    GetWindowTextA(GetDlgItem(hWnd,ID_EDITANGLE),text,5);
                    angle = atoi(text);
                    check = IsDlgButtonChecked(hWnd,ID_ROTATECHECK);
                    cam.RotationZ(angle*M_PI/180,check);
                    ReformDepthBuffer(cam);
                    GetClientRect(hChild,&rect);
                    InvalidateRect(hChild,&rect,false);
                    UpdateWindow(hChild);
                    SetFocus(hWnd);
                break;
                case ID_ROTATECHECK:
                    check = IsDlgButtonChecked(hWnd,ID_ROTATECHECK);
                    if(!check)
                        CheckDlgButton(hWnd,ID_ROTATECHECK,BST_CHECKED);
                    else
                        CheckDlgButton(hWnd,ID_ROTATECHECK,BST_UNCHECKED);
                    SetFocus(hWnd);
                break;
                case ID_COMBOBOXVEF:
                    index = SendMessageA(hComboBoxVEF,CB_GETCURSEL,0,0);
                    ShowWindow(hListViewVer,SW_HIDE);
                    ShowWindow(hListViewEdge,SW_HIDE);
                    ShowWindow(hListViewFace,SW_HIDE);
                    switch(index)
                    {
                        case 0:
                            ShowWindow(hListViewVer,SW_SHOW);
                        break;
                        case 1:
                            ShowWindow(hListViewEdge,SW_SHOW);
                        break;
                        case 2:
                            ShowWindow(hListViewFace,SW_SHOW);
                        break;
                    }
                break;
                case ID_RADIOBPOLYGON:
                    check = IsDlgButtonChecked(hWnd,ID_RADIOBPOLYGON);
                    if(!check)
                    {
                        CheckDlgButton(hWnd,ID_RADIOBPOLYGON,BST_CHECKED);
                        CheckDlgButton(hWnd,ID_RADIOBVERTEX,BST_UNCHECKED);
                    }
                    SetFocus(hWnd);
                break;
                case ID_RADIOBVERTEX:
                    check = IsDlgButtonChecked(hWnd,ID_RADIOBVERTEX);
                    if(!check)
                    {
                        CheckDlgButton(hWnd,ID_RADIOBVERTEX,BST_CHECKED);
                        CheckDlgButton(hWnd,ID_RADIOBPOLYGON,BST_UNCHECKED);
                    }
                    SetFocus(hWnd);
                break;
                case ID_NEWVERTEX:
                    check = IsDlgButtonChecked(hWnd,ID_NEWVERTEX);
                    if(!check)
                        CheckDlgButton(hWnd,ID_NEWVERTEX,BST_CHECKED);
                    else
                        CheckDlgButton(hWnd,ID_NEWVERTEX,BST_UNCHECKED);
                break;
                case ID_COMBOBOXOP:
                    index = SendMessageA(hComboBoxOP,CB_GETCURSEL,0,0);
                    ShowWindow(hListViewObj,SW_HIDE);
                    ShowWindow(hListViewVer,SW_HIDE);
                    ShowWindow(hListViewEdge,SW_HIDE);
                    ShowWindow(hListViewFace,SW_HIDE);
                    ShowWindow(hComboBoxVEF,SW_HIDE);
                    ShowWindow(hEditX,SW_HIDE);
                    ShowWindow(hEditY,SW_HIDE);
                    ShowWindow(hEditZ,SW_HIDE);
                    ShowWindow(hStaticX,SW_HIDE);
                    ShowWindow(hStaticY,SW_HIDE);
                    ShowWindow(hStaticZ,SW_HIDE);
                    ShowWindow(hButPolygon,SW_HIDE);
                    ShowWindow(hButVertex,SW_HIDE);
                    ShowWindow(hButMove,SW_HIDE);
                    ShowWindow(hButAddFace,SW_HIDE);
                    ShowWindow(hButShowVer,SW_HIDE);
                    ShowWindow(hButCreatePr,SW_HIDE);
                    ShowWindow(hEditCount,SW_HIDE);
                    ShowWindow(hEditW,SW_HIDE);
                    ShowWindow(hEditH,SW_HIDE);
                    ShowWindow(hEditName,SW_HIDE);
                    switch(index)
                    {
                        case 0:
                            ShowWindow(hButCreatePr,SW_SHOW);
                            ShowWindow(hEditCount,SW_SHOW);
                            ShowWindow(hEditW,SW_SHOW);
                            ShowWindow(hEditH,SW_SHOW);
                            ShowWindow(hEditName,SW_SHOW);
                        break;
                        case 1:
                            ShowWindow(hListViewObj,SW_SHOW);
                            ShowWindow(hComboBoxVEF,SW_SHOW);
                        break;
                        case 2:
                            ShowWindow(hEditX,SW_SHOW);
                            ShowWindow(hEditY,SW_SHOW);
                            ShowWindow(hEditZ,SW_SHOW);
                            ShowWindow(hStaticX,SW_SHOW);
                            ShowWindow(hStaticY,SW_SHOW);
                            ShowWindow(hStaticZ,SW_SHOW);
                            ShowWindow(hButPolygon,SW_SHOW);
                            ShowWindow(hButVertex,SW_SHOW);
                            ShowWindow(hButMove,SW_SHOW);
                            ShowWindow(hButAddFace,SW_SHOW);
                            ShowWindow(hButShowVer,SW_SHOW);
                        break;
                    }
                break;
                case ID_BUTMOVE:
                    GetWindowTextA(hEditX,text,5);
                    point.x = atoi(text);
                    GetWindowTextA(hEditY,text,5);
                    point.y = atoi(text);
                    GetWindowTextA(hEditZ,text,5);
                    point.z = atoi(text);
                    point.h = 0;

                    for(int i=0;i<points.size();i++)
                    {
                        if(points.at(i).isSel)
                            allObj.at(0).MoveVertex(i,point);
                    }
                    GetClientRect(hChild,&rect);
                    InvalidateRect(hChild,&rect,false);
                    UpdateWindow(hChild);
                    SetFocus(hWnd);
                break;
                case ID_BUTADDFACE:
                    index = 0;
                    int buf[3];
                    int iObj,count;
                    count=0;
                    for(int i=0;i<points.size();i++)
                    {
                        if(points.at(i).isSel)
                        {
                            iObj = points.at(i).iObj;
                            if(iObj==curObj)
                            {
                                buf[index] = points.at(i).iVertex;
                                index++;
                                count++;
                                if(count>=3)
                                {
                                    allObj[iObj].AddFaceV(buf[0],buf[1],buf[2]);
                                }
                                if(count%3==0)
                                {
                                    index=0;
                                }
                            }
                        }
                    }
                    FormScreenBuffer(cam);
                    GetClientRect(hChild,&rect);
                    InvalidateRect(hChild,&rect,false);
                    UpdateWindow(hChild);
                    SetFocus(hWnd);
                break;
                case ID_BUTCREATEPR:
                    int width,height;
                    point = {0,0,0,1};

                    GetWindowTextA(hEditCount,text,5);
                    index = atoi(text);
                    GetWindowTextA(hEditW,text,10);
                    width = atoi(text);
                    GetWindowTextA(hEditH,text,10);
                    height = atoi(text);
                    GetWindowTextA(hEditName,text,20);
                    temp.SetName(text);

                    temp.CreateRectangle(width,height,index,point);
                    allObj.push_back(temp);
                    curObj = allObj.size()-1;

                    FormListViewObj(hListViewObj);
                    FormScreenBuffer(cam);
                    FormPointsBuffer(cam);
                break;
                case ID_MENUNEWFILE:
                    allObj.clear();
                    buffer.clear();
                    points.clear();
                    curObj = -1;
                    FormListViewObj(hListViewObj);
                    GetClientRect(hChild,&rect);
                    InvalidateRect(hChild,&rect,false);
                    UpdateWindow(hChild);
                    SetFocus(hWnd);
                break;
                case ID_MENUSAVEAS:
                    name.clear();
                    ZeroMemory(&ofName, sizeof(OPENFILENAMEA));
                    ofName.lStructSize = sizeof(OPENFILENAMEA);
                    ofName.hwndOwner = hWnd;
                    ofName.lpstrFile = szFile;
                    ofName.nMaxFile = sizeof(szFile);
                    ofName.lpstrFilter = "Obj\0";
                    ofName.nFilterIndex = 1;
                    ofName.lpstrFileTitle = NULL;
                    ofName.nMaxFileTitle = 0;
                    ofName.lpstrInitialDir = NULL;
                    ofName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                    if(GetSaveFileNameA(&ofName))
                    {
                        name.append(ofName.lpstrFile);
                        name.append(".obj");
                        SaveObj((char*)name.c_str(),curObj);
                    }
                break;
                case ID_MENUSAVE:
                    if(curObj!=-1)
                        SaveObj("FastSave.obj",curObj);
                break;
                case ID_MENUOPENFILE:
                    name.clear();
                    ZeroMemory(&ofName, sizeof(OPENFILENAMEA));
                    ofName.lStructSize = sizeof(OPENFILENAMEA);
                    ofName.hwndOwner = hWnd;
                    ofName.lpstrFile = szFile;
                    ofName.nMaxFile = sizeof(szFile);
                    ofName.lpstrFilter = "Obj\0";
                    ofName.nFilterIndex = 1;
                    ofName.lpstrFileTitle = NULL;
                    ofName.nMaxFileTitle = 0;
                    ofName.lpstrInitialDir = NULL;
                    ofName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                    if(GetOpenFileNameA(&ofName))
                    {
                        name.append(ofName.lpstrFile);
                        LoadObj((char*)name.c_str());
                        FormListViewObj(hListViewObj);
                        FormScreenBuffer(cam);
                        FormPointsBuffer(cam);
                        GetClientRect(hChild,&rect);
                        InvalidateRect(hChild,&rect,false);
                        UpdateWindow(hChild);
                        SetFocus(hWnd);
                    }
                break;
                case ID_MENUEXIT:
                    SendMessageA(hWnd, WM_CLOSE, 0, 0);
                break;
            }
            return 0;
        case WM_KEYDOWN:
            switch(wParam)
            {
                case VK_F1:
                    cam.ResetPos();
                break;
                case VK_F2:
                    UnSelectAllPolygon();
                break;
                case VK_UP:
                    cam.MoveY(20);
                break;
                case VK_DOWN:
                    cam.MoveY(-20);
                break;
                case VK_LEFT:
                    cam.MoveX(-20);
                break;
                case VK_RIGHT:
                    cam.MoveX(20);
                break;
                case VK_ESCAPE:
                    SendMessageA(hWnd, WM_CLOSE, 0, 0);
                break;
            }
            ReformDepthBuffer(cam);
            GetClientRect(hChild,&rect);
            InvalidateRect(hChild,&rect,false);
            UpdateWindow(hChild);
            SetFocus(hWnd);
            return 0;
        case WM_ERASEBKGND:
            return 0;
        case WM_DESTROY:
            DeleteObject(hListViewVer);
            DeleteObject(hListViewEdge);
            DeleteObject(hListViewFace);
            DeleteObject(hListViewObj);
            DeleteObject(hChild);
            DeleteObject(hMainWnd);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hWnd,msg,wParam,lp);
}
LRESULT CALLBACK ChildWinProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lp)
{
    HDC hDc;
    HBITMAP hBmp;
    HDC hMemDc;
    PAINTSTRUCT ps;
    RECT rect;
    HPEN hPen1;
    POINT mouse,tmouse;
    static POINT prev = {0,0};
    HBRUSH hBrushSel;
    CCoordSystem csystem(200);
    CObjScene temp;
    POINT4D point;
    bool check;
    double x,y;
    switch(msg)
    {
        case WM_CREATE:
            curObj = -1;
            return 0;
        case WM_PAINT: 
            GetClientRect(hWnd,&rect);
            hDc = BeginPaint(hWnd,&ps);  
            SetMapMode(hDc,MM_ANISOTROPIC);
            SetWindowExtEx(hDc,rect.right,rect.bottom,NULL);
            SetViewportExtEx(hDc,rect.right,-rect.bottom,NULL);
            SetViewportOrgEx(hDc,rect.right/2,rect.bottom/2,NULL);

            hMemDc = CreateCompatibleDC(hDc);
            SetMapMode(hMemDc,MM_ANISOTROPIC);
            SetWindowExtEx(hMemDc,rect.right,rect.bottom,NULL);
            SetViewportExtEx(hMemDc,rect.right,-rect.bottom,NULL);
            SetViewportOrgEx(hMemDc,rect.right/2,rect.bottom/2,NULL);

            hBmp = CreateCompatibleBitmap(hDc,rect.right,rect.bottom);
            SelectObject(hMemDc,hBmp);

            SetTextColor(hMemDc,RGB(0,200,255));
            SetBkColor(hMemDc,RGB(0,0,0));
            hPen1 = CreatePen(PS_ALTERNATE,1,RGB(0,100,255));
            hBrushSel = CreateSolidBrush(RGB(200,50,50));

            if(!buffer.empty())
                QuickSortBuffer(0,buffer.size()-1);

            SelectObject(hMemDc,hPen1);
            for(int i=0;i<buffer.size();i++)
            {
                if(buffer.at(i).depth<0)
                {
                    if(buffer.at(i).isSel)
                        allObj.at(buffer.at(i).iObj).DrawPolygon(hMemDc,cam,buffer.at(i).iFace,true,bSEdge);
                    else
                        allObj.at(buffer.at(i).iObj).DrawPolygon(hMemDc,cam,buffer.at(i).iFace,false,bSEdge);
                }
            }
            SelectObject(hMemDc,hPen1);
            if(bCsys)
                csystem.Draw(hMemDc,cam);
            for(int i=0;i<buffer.size();i++)
            {
                if(buffer.at(i).depth>=0)
                {
                    if(buffer.at(i).isSel)
                        allObj.at(buffer.at(i).iObj).DrawPolygon(hMemDc,cam,buffer.at(i).iFace,true,bSEdge);
                    else
                        allObj.at(buffer.at(i).iObj).DrawPolygon(hMemDc,cam,buffer.at(i).iFace,false,bSEdge);
                }
            }

            if(IsDlgButtonChecked(hMainWnd,ID_RADIOBVERTEX))
            {
                SelectObject(hMemDc,hBrushSel);
                for(int i=0;i<points.size();i++)
                {
                    if(points.at(i).isSel)
                    {
                        temp = allObj.at(points.at(i).iObj);
                        point = temp.GetVertex(points.at(i).iVertex).point;
                        point = temp.GetBasis()*point;
                        point = cam.GetBasis()*point;
                        Ellipse(hMemDc,point.x-5,point.y-5,point.x+5,point.y+5);
                    }
                }
            }

            if(bHelp)
            {
                SetTextColor(hMemDc,RGB(255,255,255));
                SetBkColor(hMemDc,RGB(0,0,0));
                TextOutA(hMemDc,-rect.right/2+20,rect.bottom/2-20,"F1 - поставить камеру в начальное положение",strlen("F1 - поставить камеру в начальное положение"));
                TextOutA(hMemDc,-rect.right/2+20,rect.bottom/2-60,"F2 - отменить выделение",strlen("F2 - отменить выделение"));
                TextOutA(hMemDc,-rect.right/2+20,rect.bottom/2-100,"F3 - выделить все полигоны",strlen("F3 - выделить все полигоны"));
                TextOutA(hMemDc,-rect.right/2+20,rect.bottom/2-140,"F4 - копировать вершину",strlen("F4 - копировать вершину"));
                TextOutA(hMemDc,-rect.right/2+20,rect.bottom/2-180,"F5 - показать/спрятать систему координат",strlen("F5 - показать/спрятать систему координат"));
                TextOutA(hMemDc,-rect.right/2+20,rect.bottom/2-220,"F6 - показать/спрятать ребра полигонов",strlen("F6 - показать/спрятать ребра полигонов"));
            }

            BitBlt(hDc,-rect.right/2,-rect.bottom/2,rect.right,rect.bottom,hMemDc,-rect.right/2,-rect.bottom/2,SRCCOPY);

            DeleteObject(hPen1);
            DeleteObject(hBrushSel);
            DeleteObject(hBmp);
            DeleteDC(hMemDc);

            EndPaint(hWnd,&ps);
            return 0;
        case WM_LBUTTONDOWN:
            GetClientRect(hWnd,&rect);
            hDc = GetDC(hWnd);
            GetCursorPos(&mouse);
            ScreenToClient(hWnd,&mouse);

            tmouse.x = (mouse.x - rect.right/2);
            tmouse.y = (rect.bottom/2 - mouse.y);
            check = IsDlgButtonChecked(hMainWnd,ID_RADIOBPOLYGON);
            //выделение полигона
            if(GetKeyState(VK_LCONTROL)<0&&check)
            {
                for(int i=0;i<buffer.size();i++)
                {
                    if(IsBelongsTriangle(tmouse,buffer.at(buffer.size()-i-1),cam))
                    {
                        if(buffer.at(buffer.size()-i-1).isSel)
                            buffer.at(buffer.size()-i-1).isSel = false;
                        else
                            buffer.at(buffer.size()-i-1).isSel = true;
                        break;
                    }
                }
                GetClientRect(hWnd,&rect);
                InvalidateRect(hWnd,&rect,false);
                UpdateWindow(hWnd);
                SetFocus(hWnd);
            }
            check = IsDlgButtonChecked(hMainWnd,ID_RADIOBVERTEX);
            //выделение вершины
            if(GetKeyState(VK_LCONTROL)<0&&check)
            {
                check = false;  //главное не создавать лишних вершин
                INDVERTEX ind;
                for(int i=0;i<points.size();i++)
                {
                    if(curObj == points.at(i).iObj)
                    {
                        ind = points.at(i);
                        temp = allObj.at(ind.iObj);
                        point = temp.GetVertex(ind.iVertex).point;

                        point = temp.GetBasis()*point;
                        point = cam.GetBasis()*point;
                        int x = abs((int)(tmouse.x-point.x));
                        int y = abs((int)(tmouse.y-point.y));
                        int r = sqrt(x*x+y*y);
                        if(r<15)
                        {
                            check = true;
                            if(!points.at(i).isSel)
                                points.at(i).isSel = true;
                            else
                                points.at(i).isSel = false;
                        }
                    }
                }
                //нажатием мыши создаем новую вершину и выделяем ее
                if(IsDlgButtonChecked(hMainWnd,ID_NEWVERTEX)&&!check)
                {
                    point.x = tmouse.x;
                    point.y = tmouse.y;
                    point.z = 0;
                    point.h = 1;

                    for(int i=0;i<points.size();i++)
                    {
                        if(points.at(i).isSel)
                        {
                            point.z = points.at(i).depth;
                            break;
                        }
                    }
                    point = cam.GetBasis().Transposed()*point;

                    allObj.at(curObj).AddVertex(point);
                    AddPointToBuffer(cam,curObj,allObj.at(curObj).GetCountVertex()-1);
                    points.at(points.size()-1).isSel = true;
                }
                GetClientRect(hWnd,&rect);
                InvalidateRect(hWnd,&rect,false);
                UpdateWindow(hWnd);
                SetFocus(hWnd);
            }
            return 0;
        case WM_MOUSEMOVE:
            GetCursorPos(&mouse);
            if(prev.y>mouse.y)
                y =-1;
            else
                if(prev.y<mouse.y)
                    y = 1;
                else
                    y = 0;

            if(prev.x>mouse.x)
                x =-1;
            else
                if(prev.x<mouse.x)
                    x = 1;
                else
                    x = 0;
            prev = mouse;
            //движение камеры
            if(GetKeyState(VK_LBUTTON)<0&&GetKeyState(VK_LCONTROL)>=0&&GetKeyState(VK_LSHIFT)>=0)
            {
                cam.RotationY(-x*2*M_PI/360,false);
                cam.RotationX(y*2*M_PI/360,false);
            }
            //движение вершин
            if(GetKeyState(VK_LBUTTON)<0&&GetKeyState(VK_LSHIFT)<0)
            {
                for(int i=0;i<points.size();i++)
                {
                    if(points.at(i).isSel&&(curObj == points.at(i).iObj))
                    {
                        temp = allObj.at(points.at(i).iObj);
                        point.x = x;
                        point.y = -y;
                        point.z = 0;
                        point.h = 0;
                        point = cam.GetBasis().Transposed()*point;
                        allObj.at(points.at(i).iObj).MoveVertex(points.at(i).iVertex,point);
                    }
                }
            }
            ReformDepthBuffer(cam);
            GetClientRect(hWnd,&rect);
            InvalidateRect(hWnd,&rect,false);
            UpdateWindow(hWnd);
            SetFocus(hWnd);
            return 0;
        case WM_KEYDOWN:
            switch(wParam)
            {
                case VK_DELETE:
                    for(int i=0;i<buffer.size();i++)
                        if(buffer.at(i).isSel)
                        {
                            allObj.at(buffer.at(i).iObj).DeleteFace(buffer.at(i).iFace);
                            ReformIndexBuffer(buffer.at(i).iObj,buffer.at(i).iFace);
                            buffer.erase(buffer.begin()+i);
                            i--;
                        }
                    FormPointsBuffer(cam);
                    FormScreenBuffer(cam);
                break;
                case VK_F1:
                    cam.ResetPos();
                break;
                case VK_F2:
                    UnSelectAllPolygon();
                    bAllSel = false;
                break;
                case VK_F3:
                    if(!bAllSel)
                    {
                        if(IsDlgButtonChecked(hMainWnd,ID_RADIOBPOLYGON))
                            SelectAllPolygon(curObj);
                        else
                            SelectAllVertex(curObj);
                        bAllSel = true;
                    }
                    else
                    {
                        UnSelectAllPolygon();
                        bAllSel = false;
                    }
                break;
                case VK_F4:
                    int size;
                    size = points.size();
                    for(int i=0;i<size;i++)
                    {
                        if(points.at(i).isSel&&(curObj==points.at(i).iObj))
                        {
                            allObj.at(points.at(i).iObj).CopyVertex(points.at(i).iVertex);

                            AddPointToBuffer(cam,curObj,allObj.at(curObj).GetCountVertex()-1);
                            points.at(i).isSel = false;
                            points.at(points.size()-1).isSel = true;
                        }
                    }
                break;
                case VK_F5:
                    if(bCsys)
                        bCsys = false;
                    else
                        bCsys = true;
                break;
                case VK_F6:
                    if(bSEdge)
                        bSEdge = false;
                    else
                        bSEdge = true;
                break;
                case VK_DOWN:
                    for(int i=0;i<points.size();i++)
                    {
                        if(points.at(i).isSel&&points.at(i).iObj==curObj)
                        {
                            temp = allObj.at(curObj);
                            point.x = 0;
                            point.y = -10;
                            point.z = 0;
                            point.h = 0;
                            point = cam.GetBasis().Transposed()*point;
                            allObj.at(curObj).MoveVertex(points.at(i).iVertex,point);
                        }
                    }
                break;
                case VK_UP:
                    for(int i=0;i<points.size();i++)
                    {
                        if(points.at(i).isSel&&points.at(i).iObj==curObj)
                        {
                            temp = allObj.at(curObj);
                            point.x = 0;
                            point.y = 10;
                            point.z = 0;
                            point.h = 0;
                            point = cam.GetBasis().Transposed()*point;
                            allObj.at(curObj).MoveVertex(points.at(i).iVertex,point);
                        }
                    }
                break;
                case VK_LEFT:
                    for(int i=0;i<points.size();i++)
                    {
                        if(points.at(i).isSel&&points.at(i).iObj==curObj)
                        {
                            temp = allObj.at(curObj);
                            point.x = -10;
                            point.y = 0;
                            point.z = 0;
                            point.h = 0;
                            point = cam.GetBasis().Transposed()*point;
                            allObj.at(curObj).MoveVertex(points.at(i).iVertex,point);
                        }
                    }
                break;
                case VK_RIGHT:
                    for(int i=0;i<points.size();i++)
                    {
                        if(points.at(i).isSel&&points.at(i).iObj==curObj)
                        {
                            temp = allObj.at(curObj);
                            point.x = 10;
                            point.y = 0;
                            point.z = 0;
                            point.h = 0;
                            point = cam.GetBasis().Transposed()*point;
                            allObj.at(curObj).MoveVertex(points.at(i).iVertex,point);
                        }
                    }
                break;
                case VK_ESCAPE:
                    SendMessage(hWnd, WM_CLOSE, 0, 0);
                break;
            }
            ReformDepthBuffer(cam);
            GetClientRect(hWnd,&rect);
            InvalidateRect(hWnd,&rect,false);
            UpdateWindow(hWnd);
            SetFocus(hWnd);
            return 0;
        case WM_ERASEBKGND:
            return 0;
        case WM_RBUTTONDOWN:
            GetClientRect(hWnd,&rect);
            mouse.x = LOWORD(lp);
            mouse.y = HIWORD(lp)+25;
            HandlePopMenu(hWnd,mouse);
            return 0;
        case WM_SIZE:
            GetClientRect(hWnd,&rect);
            InvalidateRect(hWnd,&rect,true);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hWnd,msg,wParam,lp);
}
void FormScreenBuffer(CCamera cam)
{
    CObjScene tempObj;
    INDPOLYGON tempPoly;
    buffer.clear();
    for(int i=0;i<allObj.size();i++)
    {
        tempObj = allObj.at(i);
        tempPoly.iObj = i;
        for(int j=0;j<tempObj.GetCountFace();j++)
        {
            tempPoly.depth = tempObj.GetDepth(cam,j);
            tempPoly.iFace = j;
            tempPoly.isSel = false;
            buffer.push_back(tempPoly);
        }
    }
}
void ReformIndexBuffer(int iObj, int iFace)
{
    for(int i=0;i<buffer.size();i++)
    {
        if(buffer.at(i).iObj==iObj)
        {
            if(buffer.at(i).iFace>=iFace)
            {
                buffer.at(i).iFace--;
            }
        }
    }
}

void ReformDepthBuffer(CCamera cam)
{
    for(int i=0;i<buffer.size();i++)
        buffer[i].depth = allObj.at(buffer[i].iObj).GetDepth(cam,buffer[i].iFace);

    for(int i=0;i<points.size();i++)
        points[i].depth = allObj.at(points[i].iObj).GetDepthV(cam,points[i].iVertex);
}

void QuickSortBuffer(int first, int last)
{
    INDPOLYGON temp;
    int i = first;
    int j = last;
    int x = buffer.at((first+last)/2).depth;
    do
    {
        while (buffer.at(i).depth<x)
            i++;
        while (buffer.at(j).depth>x)
            j--;

        if(i <= j)
        {
            if (buffer.at(i).depth>buffer.at(j).depth)
            {
                temp = buffer.at(i);
                buffer[i] = buffer[j];
                buffer[j] = temp;
            }
            i++;
            j--;
        }
    }
    while (i<=j);

    if (i<last)
        QuickSortBuffer(i,last);
    if (first<j)
        QuickSortBuffer(first,j);

}

void AddListElm(HWND hListView,string text[],int count, int index)
{
    LVITEMA LvItem;
    memset(&LvItem,0,sizeof(LvItem));
    LvItem.mask=LVIF_TEXT;
    LvItem.cchTextMax = 256;
    LvItem.iItem=index;
    LvItem.iSubItem=0;
    SendMessageA(hListView,LVM_INSERTITEMA,0,(LPARAM)&LvItem);

    for(int i=0;i<count;i++)
    {
        LvItem.iSubItem=i;
        LvItem.pszText=(char*)text[i].c_str();
        SendMessageA(hListView,LVM_SETITEMA,0,(LPARAM)&LvItem);
    }
}

void FormListViewObj(HWND hListView)
{
    ListView_DeleteAllItems(hListView);
    string text[2];
    char num[3];
    for(int i=0;i<allObj.size();i++)
    {
        text[0].clear();
        text[1].clear();
        itoa(i,num,10);
        text[0].append(num);
        text[1].append(allObj.at(i).GetName());
        AddListElm(hListView,text,2,i);
    }
}

void FormListViewVer(HWND hListView, int index)
{
    ListView_DeleteAllItems(hListView);
    CObjScene temp = allObj.at(index);
    VERTEX vertex;
    POINT4D scene;
    string str,text[2];
    char buf[10];

    for(int i=0;i<temp.GetCountVertex();i++)
    {
        str.clear();
        text[0].clear();
        text[1].clear();

        itoa(i,buf,10);
        text[0].append(buf);

        vertex = temp.GetVertex(i);
        scene = temp.GetBasis()*vertex.point;

        str.clear();
        str.append("(");
        itoa(scene.x,buf,10);
        str.append(buf);
        str.append(", ");

        itoa(scene.y,buf,10);
        str.append(buf);
        str.append(", ");

        itoa(scene.z,buf,10);
        str.append(buf);
        str.append(")");

        text[1].append(str);
        AddListElm(hListView,text,2,i);
    }
}

void FormListViewEdge(HWND hListView, int index)
{
    ListView_DeleteAllItems(hListView);
    CObjScene temp = allObj.at(index);
    EDGE edge;
    POINT4D scene;
    string str,text[3];
    char buf[10];

    for(int i=0;i<temp.GetCountEdge();i++)
    {
        for(int j=0;j<2;j++)
            text[j].clear();

        edge = temp.GetEdge(i);

        itoa(i,buf,10);
        text[0].append(buf);

        for(int j=0;j<2;j++)
        {
            str.clear();
            scene = temp.GetBasis()*temp.GetVertex(edge.ver[j]).point;

            str.append("(");
            itoa(scene.x,buf,10);
            str.append(buf);
            str.append(", ");

            itoa(scene.y,buf,10);
            str.append(buf);
            str.append(", ");

            itoa(scene.z,buf,10);
            str.append(buf);
            str.append(")");

            text[j+1] = str;
        }
        AddListElm(hListView,text,3,i);
    }
}

void FormListViewFace(HWND hListView,int index)
{
    ListView_DeleteAllItems(hListView);
    CObjScene temp = allObj.at(index);
    FACE face;
    string text[4];
    char buf[10];

    for(int i=0;i<temp.GetCountFace();i++)
    {
        for(int j=0;j<4;j++)
            text[j].clear();

        face = temp.GetFace(i);

        itoa(i,buf,10);
        text[0].append(buf);

        for(int j=0;j<3;j++)
        {
            itoa(face.edge[j],buf,10);

            text[j+1].append(buf);
        }
        AddListElm(hListView,text,4,i);
    }
}

bool IsBelongsTriangle(POINT point,INDPOLYGON poly,CCamera cam)
{
    int res1,res2,res3;
    CObjScene temp = allObj.at(poly.iObj);
    FACE face = temp.GetFace(poly.iFace);
    POINT4D poly4D[3];
    temp.PointsFace(face,poly4D[0],poly4D[1],poly4D[2]);
    for(int i=0;i<3;i++)
    {
        poly4D[i] = temp.GetBasis()*poly4D[i];
        poly4D[i] = cam.GetBasis()*poly4D[i];
    }
    res1 = (poly4D[0].x - point.x)*(poly4D[1].y - poly4D[0].y) - (poly4D[1].x - poly4D[0].x)*(poly4D[0].y - point.y);
    res2 = (poly4D[1].x - point.x)*(poly4D[2].y - poly4D[1].y) - (poly4D[2].x - poly4D[1].x)*(poly4D[1].y - point.y);
    res3 = (poly4D[2].x - point.x)*(poly4D[0].y - poly4D[2].y) - (poly4D[0].x - poly4D[2].x)*(poly4D[2].y - point.y);

    if(res1>0&&res2>0&&res3>0)
        return true;
    else
        if(res1<0&&res2<0&&res3<0)
            return true;
        else
            return false;
}

void UnSelectAllPolygon()
{
    for(int i=0;i<buffer.size();i++)
        buffer.at(i).isSel = false;
    for(int i=0;i<points.size();i++)
        points.at(i).isSel = false;
}

void SelectAllPolygon(int value)
{
    if(value!=-1)
        for(int i=0;i<buffer.size();i++)
        {
            if(buffer.at(i).iObj==value)
                buffer.at(i).isSel = true;
        }
}


void FormPointsBuffer(CCamera cam)
{
    CObjScene tempObj;
    INDVERTEX tempVer;
    POINT4D point;
    points.clear();
    for(int i=0;i<allObj.size();i++)
    {
        tempObj = allObj.at(i);
        tempVer.iObj = i;
        for(int j=0;j<tempObj.GetCountVertex();j++)
        {
            point = tempObj.GetBasis()*tempObj.GetVertex(j).point;
            point = cam.GetBasis()*point;
            tempVer.depth = point.z;
            tempVer.iVertex = j;
            tempVer.isSel = false;
            points.push_back(tempVer);
        }
    }
}

void SelectAllVertex(int value)
{
    for(int i=0;i<points.size();i++)
    {
        if(points.at(i).iObj==value)
        {
            points.at(i).isSel = true;
        }
    }
}

void AddPointToBuffer(CCamera cam,int iObj,int iVertex)
{
    CObjScene tempObj;
    INDVERTEX tempVer;
    POINT4D point;
    tempObj = allObj.at(iObj);
    tempVer.iObj = iObj;
    point = tempObj.GetBasis()*tempObj.GetVertex(iVertex).point;
    point = cam.GetBasis()*point;
    tempVer.depth = point.z;
    tempVer.iVertex = iVertex;
    tempVer.isSel = false;
    points.push_back(tempVer);
}

void SaveObj(LPSTR fName,int iObj)
{
    DWORD dwTemp;
    HANDLE hFile;
    POINT4D point;
    FACE face;
    string text;
    char buf[20];
    int i1=0,i2=0,i3=0;
    text.clear();
    hFile = CreateFileA(fName, GENERIC_WRITE, 0, NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    //записываем информацию о вершинах
    for(int i=0;i<points.size();i++)
    {
        if(points.at(i).iObj==iObj)
        {
            text.clear();
            text.append("v ");
            point = allObj.at(points.at(i).iObj).GetVertex(points.at(i).iVertex).point;
            itoa(point.x,buf,10);
            text.append(buf);
            text.append(" ");

            itoa(point.y,buf,10);
            text.append(buf);
            text.append(" ");

            itoa(point.z,buf,10);
            text.append(buf);
            text.append(" ");

            itoa(point.h,buf,10);
            text.append(buf);
            text.append("\x0D\x0A");    //какой то ад, для текстового файла; \n неработает
            WriteFile(hFile,text.c_str(),text.size(),&dwTemp,NULL);
        }
    }
    text.clear();
    text.append("\x0D\x0A");
    WriteFile(hFile,text.c_str(),text.size(),&dwTemp,NULL);
    for(int i=0;i<buffer.size();i++)
    {
        if(buffer.at(i).iObj==iObj)
        {
            text.clear();
            text.append("f ");
            face = allObj.at(iObj).GetFace(buffer.at(i).iFace);
            allObj.at(iObj).PointsFace(face,i1,i2,i3);

            itoa(i1,buf,10);
            text.append(buf);
            text.append("/");

            itoa(i2,buf,10);
            text.append(buf);
            text.append("/");

            itoa(i3,buf,10);
            text.append(buf);
            if(i<buffer.size()-1)
                text.append("\x0D\x0A");
            WriteFile(hFile,text.c_str(),text.size(),&dwTemp,NULL);
        }
    }
    CloseHandle(hFile);
}

void LoadObj(LPSTR fName)
{
    allObj.clear();
    points.clear();
    buffer.clear();

    DWORD dwTemp,size;
    HFILE hFile;
    OFSTRUCT tOfStr;
    string text,num;
    CObjScene temp;
    POINT4D point;
    int face[3];

    hFile = OpenFile(fName,&tOfStr,OF_READ);
    if(hFile == HFILE_ERROR)
    {
        CloseHandle(HANDLE(hFile));
        return;
    }
    size = GetFileSize(HANDLE(hFile),NULL);
    char *buf = new char[size+1];
    buf[size] = '\0';
    ReadFile(HANDLE(hFile),buf,sizeof(char[size]),&dwTemp,NULL);
    text.append(buf);

    for(int i=0;i<text.length();i++)
    {
        if(text.at(i)=='v')
        {
            text.erase(0,2);
            for(int j=0;j<4;j++)
            {
                num.clear();
                size = 0;
                for(int k=0;k<5;k++)
                {
                    if(text.at(k)!=' ')
                    {
                        size++;
                        num.push_back(text.at(k));
                    }
                    else
                        break;
                }
                if(j<3)
                    text.erase(0,size+1);
                else
                    text.erase(0,3);

                if(j==0)
                    point.x = atoi((char*)num.c_str());
                if(j==1)
                    point.y = atoi((char*)num.c_str());
                if(j==2)
                    point.z = atoi((char*)num.c_str());
                if(j==3)
                    point.h = atoi((char*)num.c_str());
            }
            temp.AddVertex(point);
            i = -1;
        }
    }
    text.erase(0,2);
    for(int i=0;i<text.length();i++)
    {
        if(text.at(i)=='f')
        {
            text.erase(0,2);
            for(int j=0;j<3;j++)
            {
                num.clear();
                size = 0;
                for(int k=0;k<3;k++)
                {
                    if(k<text.size()&&text.at(k)!='/')
                    {
                        size++;
                        num.push_back(text.at(k));
                    }
                    else
                        break;
                }
                if(j<2)
                    text.erase(0,size+1);
                else
                {
                    text.erase(0,3);
                }
                face[j] = atoi((char*)num.c_str());
            }
            temp.AddFaceV(face[0],face[1],face[2]);
            i = -1;
        }
    }
    allObj.push_back(temp);
    curObj = 0;
    delete buf;
    CloseHandle(HANDLE(hFile));
}
