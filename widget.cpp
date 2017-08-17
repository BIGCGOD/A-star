#include "widget.h"

#include <QPainter>
#include <QDebug>
#include <QTime>
#include <QEventLoop>
#include <QCoreApplication>

//bool MyPointCompare(point *p1, point *p2){return p1->f < p2->f;}//用于QVector排序的比较函数
enum PointColor{WALL,GROUND,START,END,PATH,OPEN,CLOSE};

Widget::Widget(QWidget *parent)
    : QWidget(parent),flag(0)
{
    RestoreMap();
}

Widget::~Widget()
{

}

QVector<point*> Widget::SurrroundPoints(point *p)
{
    QVector<point*> points;
    for(int x = p->x - 1; x <= p->x + 1; ++x)
        for(int y = p->y - 1; y <= p->y + 1; ++y)//遍历以p为中心的9个点
            if((x != p->x || y != p->y) && map[x][y] != 0)//除去p点，没有障碍物
                if(x >= 0 && x < MAPSIZE && y >= 0 && y < MAPSIZE)//没有越界
                    if((x == p->x || y == p->y) || (map[x][p->y] != 0 && map[p->x][y] != 0))//走斜线过程中不得越过障碍
                        if(IndexOfPoint(CloseList,point(x,y)) == -1)//不在关闭列表内
                            points.append(new point(p,x,y));
    return points;
}

int Widget::CalcG(point *start, point *p)
{
    int G = (qAbs(p->x - start->x) + qAbs(p->y - start->y)) == 2 ? MOVECOST2 : MOVECOST1; //移动耗费，直行为10，斜向为14
    int parentG = p->parent != NULL ? p->parent->g : 0;                  //加上父节点的移动耗费
    return G + parentG;
}

int Widget::CalcH(point *end, point *p)
{
    float xDistance = qAbs(p->x - end->x);
    float yDistance = qAbs(p->y - end->y);
//    if(xDistance > yDistance)
//        return MOVECOST2 * yDistance + MOVECOST1 * (xDistance - yDistance);
//    else
//        return MOVECOST2 * xDistance + MOVECOST1 * (yDistance - xDistance);
    return MOVECOST1 * (xDistance + yDistance) + (MOVECOST2 - 2*MOVECOST1) * qMin(xDistance, yDistance);
}

int Widget::IndexOfPoint(QVector<point*> vector, point p)
{
    for(int i = 0;i < vector.size();++i)
    {
        if(*vector[i] == p)
            return i;
    }
    return -1;
}

int Widget::FoundOptimalPoint(QVector<point*> &list)
{
    int min = 0;
    for(int i = 1; i < list.size(); ++i)
        if(list[i]->f <= list[min]->f)
            min = i;
    return min;
}

point* Widget::FindPath()
{
    point* s = new point(start.x, start.y);
    OpenList.append(s);
    while(!OpenList.isEmpty())
    {
//        qSort(OpenList.begin(),OpenList.end(),MyPointCompare);//排序，选择最优点
        int min =  FoundOptimalPoint(OpenList);
        point *tempStart = OpenList[min];//OpenList[0];
        OpenList.removeAt(min);
        CloseList.append(tempStart);//讲最优点放入close队列
#ifdef ISDISPLAY
        map[tempStart->x][tempStart->y] = CLOSE;
#endif

        QVector<point*> surroundPoints = SurrroundPoints(tempStart);//查找可前往的点
        for(int i = surroundPoints.size() - 1; i >= 0; --i)
        {
            int num = IndexOfPoint(OpenList,*surroundPoints[i]);
            if(num != -1)//如果该点已在open队列中则检验新路径的G值，如果新路径更优则更新父节点和相关数值
            {
                int G = CalcG(tempStart, surroundPoints[i]);
                surroundPoints[i]->num--;
                delete surroundPoints[i];//删除重复数据
                if (G < OpenList[num]->g)
                {
                    point::setParent(OpenList[num],tempStart);//重新指定父节点(也就是改为较短路径)
                    OpenList[num]->g = G;
                    OpenList[num]->CalcF();
                }
            }
            else//如果该点未被处理过，则计算相关数据并将其放入open队列中
            {
                point::setParent(surroundPoints[i],tempStart);
                surroundPoints[i]->g = CalcG(tempStart, surroundPoints[i]);
                surroundPoints[i]->h = CalcH(&end, surroundPoints[i]);
                surroundPoints[i]->CalcF();

#ifdef ISDISPLAY
                for(int j = 0; j < OpenList.size(); j++)
                    map[OpenList[j]->x][OpenList[j]->y] = OPEN;
                for(int j = 0; j < CloseList.size(); j++)
                    map[CloseList[j]->x][CloseList[j]->y] = OPEN;

                map[surroundPoints[i]->x][surroundPoints[i]->y] = OPEN;

                point *path = surroundPoints[i];
                while(path->parent != NULL)//通过父节点指针来还原路径
                {
                    map[path->x][path->y] = PATH;
                    path = path->parent;
                }
#endif

                if(*surroundPoints[i] == end)//如果该点是目的点则直接返回
                    return surroundPoints[i];

                OpenList.append(surroundPoints[i]);
            }
        }

#ifdef ISDISPLAY
        update();
        QTime dieTime = QTime::currentTime().addMSecs(DISPLAYINTERVAL);
        while( QTime::currentTime() < dieTime )
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
#endif
    }
    return NULL;
}

void Widget::RestoreMap()
{
    for(int i = 0; i < MAPSIZE; ++i)
        for(int j = 0; j < MAPSIZE; ++j)
        {
            if((i == MAPSIZE/8 && j < MAPSIZE/4*3) || (i == MAPSIZE/8*7 && j > MAPSIZE/4) ||
               (j == MAPSIZE/8 && i > MAPSIZE/4) || (j == MAPSIZE/8*7 && i < MAPSIZE/4*3))
                map[i][j] = WALL;
            else
                map[i][j] = GROUND;
        }
}

void Widget::paintEvent(QPaintEvent *)
{
    float w = (float)width()/MAPSIZE;
    float h = (float)height()/MAPSIZE;

    QPainter painter(this);

    for(int i = 0; i < MAPSIZE; ++i)
        for(int j = 0; j < MAPSIZE; ++j)
        {
            switch(map[i][j])
            {
            case WALL:
                painter.setBrush(QBrush(QColor(255,255,255)));
                break;
            case GROUND:
                painter.setBrush(QBrush(QColor(0,0,0)));
                break;
            case START:
                painter.setBrush(QBrush(QColor(0,255,0)));
                break;
            case END:
                painter.setBrush(QBrush(QColor(255,255,0)));
                break;
            case PATH:
                painter.setBrush(QBrush(QColor(0,0,255)));
                break;
#ifdef ISDISPLAY
            case OPEN:
                painter.setBrush(QBrush(QColor(0,255,255)));
                break;
            case CLOSE:
                painter.setBrush(QBrush(QColor(255,0,0)));
                break;
#endif
            }
            painter.drawRect(w*i,h*j,w,h);
        }
}

void Widget::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->x() >= 0 && e->x() <= width() &&
            e->y() >= 0 && e->y() <= height())
    {
        if(flag == 0)
        {
            RestoreMap();
            OpenList.clear();
            CloseList.clear();

            start.x = (float)e->x()/width()*MAPSIZE;
            start.y = (float)e->y()/height()*MAPSIZE;

            if(map[start.x][start.y] == WALL)
                flag = 1;
            else
                map[start.x][start.y] = START;
        }
        else if(flag == 1)
        {
            end.x = (float)e->x()/width()*MAPSIZE;
            end.y = (float)e->y()/height()*MAPSIZE;

            if(map[end.x][end.y] == WALL)
                flag = 0;
            else
            {
                point *path = FindPath();
                if(path == NULL)
                    exit(0);
                while(path->parent != NULL)//通过父节点指针来还原路径
                {
                    map[path->x][path->y] = PATH;
                    path = path->parent;
                }
                point::deletePoint(path);

                map[end.x][end.y] = END;
                map[start.x][start.y] = START;
            }
        }
        flag = 1 - flag;
        update();
    }
}
