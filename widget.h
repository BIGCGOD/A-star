#ifndef WIDGET_H
#define WIDGET_H

#include <QMouseEvent>
#include <QWidget>
#include <QVector>

#define MAPSIZE 50

#define MOVECOST1 10
#define MOVECOST2 14

#define ISDISPLAY
#define DISPLAYINTERVAL 20

struct point
{
    point():parent(NULL),x(0),y(0),f(0),g(0),h(0),num(0){}
    point(int x_, int y_):parent(NULL),x(x_),y(y_),f(0),g(0),h(0),num(0){}
    point(point *p, int x_, int y_):parent(p),x(x_),y(y_),f(0),g(0),h(0),num(0){p->num++;}
    ~point(){if(parent != NULL) parent->num--;}

    point *parent;
    int x;
    int y;
    int f;
    int g;//移动耗费
    int h;//预计耗费

    int num;//引用计数，用于释放内存

    bool operator ==(const point p){ return this->x == p.x && this->y == p.y; }
    void CalcF(){f = g + h;}
    static void setParent(point *p ,point *parent)
    {
        if(p->parent != NULL)
            p->parent->num--;
        p->parent = parent;
        parent->num++;
    }
    static void deletePoint(point *p)
    {
        if(p->num > 0)
            return;
        if(p->parent != NULL)
        {
            p->parent->num--;
            deletePoint(p->parent);
        }
        delete p;
    }
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();
private:
    int map[MAPSIZE][MAPSIZE];
    QVector<point*> OpenList;
    QVector<point*> CloseList;

    int flag;
    point start;
    point end;

    QVector<point*> SurrroundPoints(point *p);//寻找可用点
    int CalcG(point *start, point *p);
    int CalcH(point *end, point *p);
    int IndexOfPoint(QVector<point*> vector, point p);
    int FoundOptimalPoint(QVector<point*> &list);

    point* FindPath();//寻路函数
    void RestoreMap();//复原地图

    void paintEvent(QPaintEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
};

#endif // WIDGET_H
