// glut.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdlib.h>
#include <vector>

using namespace std;

const int ESCAPE = 27;
const int ENTER = 13;
const int SPACE = 32;
const int DELETE = 127;


// идентификаторы меню
int fillMenu, figureMenu, mainMenu, colorMenu, aliasingMenu;
int menuFlag = 0;



// Ширина и высота окна
int width = 512;
int height = 512;

// Класс, описывающий точку
struct Point
{
        GLint x, y;
        Point(GLint _x, GLint _y) 
		{
                x = _x;
                y = _y;
        }
};

// Класс описывающий цвета 
struct Color 
{
        GLubyte r, g, b;
};

// Класс описывающий полигон 
class Polygon
{
public:
        vector<Point> points;
        Color color;
		bool convex;
};

// Все полигоны, которые должны отображаться
vector<Polygon> polygons;
// Указывает на полигон, который в данный момент рисуется
int nowPolygon;

#pragma region sort
// Ищет поворот относительно точки А точек В и С: если для А, В правее С, то результат положителен, иначе отрицателен
GLint rotate (Point A, Point B, Point C)
{
	return (B.x - A.x) * (C.y - B.y) - (B.y - A.y) * (C.x - B.x);
}

// Сортировка точек относительно их поворота относительно самой левой точки
void sort (Polygon& poly)
{
	int left = 0;
	// Поиск самой левой точки
	for (int i = 0; i < poly.points.size(); i++)
	{
		if (poly.points[i].x < poly.points[left].x)
			left = i;
	}
	swap(poly.points[0],poly.points[left]);
	// Сортировка вставками по "левизне" относительно самой левой точки
	int j = 0;
	for (int i = 2; i < poly.points.size(); i++)
	{
		j = i;
		while (j > 1 && rotate(poly.points[0], poly.points[j - 1], poly.points[j]) < 0)
		{
			swap(poly.points[j - 1], poly.points[j]);
			j--;
		}
	}
}

// Построение минимальной выпуклой оболочки
void Graham(Polygon& poly)
{
	sort(poly);
	// Цикл удаления точек, которые не дают выпуклость
	for (int i = 2; i < poly.points.size(); i++)
	{
		while (i < poly.points.size() && rotate(poly.points[i-2], poly.points[i - 1], poly.points[i]) < 0)
			poly.points.erase(poly.points.begin() + i - 1);
	}
}
#pragma endregion построение выпуклой оболочки

// метод трассирующего луча
bool isInner(Polygon& poly, double x, double y)
{
        bool c = false;
        int n = poly.points.size();
        for (int i = 0, j = n - 1; i < n; j = i++)
        {
                if ((
                        (poly.points[i].y < poly.points[j].y) && (poly.points[i].y <= y) && (y <= poly.points[j].y) &&
                        ((poly.points[j].y - poly.points[i].y) * (x - poly.points[i].x) > (poly.points[j].x - poly.points[i].x) * (y - poly.points[i].y))
                        ) || (
                        (poly.points[i].y > poly.points[j].y) && (poly.points[j].y <= y) && (y <= poly.points[i].y) &&
                        ((poly.points[j].y - poly.points[i].y) * (x - poly.points[i].x) < (poly.points[j].x - poly.points[i].x) * (y - poly.points[i].y))
                        ))
                {
                        c = !c;
                }
        }
        return c;
}

#pragma region poly
// Создаем новый Полигон и добавляем в вектор
void createPolygon() 
{
        Polygon poly;
        poly.color.r = 255;
        poly.color.g = 0;
        poly.color.b = 0;
		poly.convex = true;
        polygons.push_back(poly);
        nowPolygon = polygons.size() - 1;
}

void removePolygon()
{
	if (nowPolygon != 0)
	{
        polygons.erase(polygons.begin() + nowPolygon);
		nowPolygon--;
	}
	else 
	{
		if(polygons.size() > 1)
			polygons.erase(polygons.begin() + nowPolygon);
		else 
			polygons[nowPolygon].points.clear();
	}
}
#pragma endregion создание и удаление полигона

#pragma region screen
/* эта функция управляет всем выводом на экран */
void Display(void)
{
        glClearColor(255, 255, 255, 1); 
        glClear(GL_COLOR_BUFFER_BIT);
        // Обход по полигонам
        for (int j = 0; j < polygons.size(); j++) {
                // Задаем цвет полигона
                glColor3ub(polygons[j].color.r, polygons[j].color.g, polygons[j].color.b);
                glBegin(GL_POLYGON);
                for (int i = 0; i < polygons[j].points.size(); i++) {
                        // Устанавливаем вершины
                        glVertex2i(polygons[j].points[i].x, polygons[j].points[i].y);
                        
                }
                glEnd();
        }

        // Задаем цвет точек
        glColor3ub(0, 0, 0);
        // Размер точки
        glPointSize(3);
        glBegin(GL_POINTS);
        for (int i = 0; i < polygons[nowPolygon].points.size(); i++) {
                // Устанавливаем вершины
                glVertex2i(polygons[nowPolygon].points[i].x, polygons[nowPolygon].points[i].y);
        }
        glEnd();

        glFinish();
}

// Функция вызывается при изменении размеров окна
void Reshape(GLint w, GLint h)
{
        width = w; 
        height = h;
        /* устанавливаем размеры области отображения */
        glViewport(0, 0, w, h);
        /* ортографическая проекция */
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, w, 0, h, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
}
#pragma endregion работа с экраном

#pragma region control
// Функция обрабатывает сообщения от клавиатуры (смена цвета, удаление или создание фигуры)
void Keyboard(unsigned char key, int x, int y)
{
        switch (key)
        {
		case '1':
			if (polygons[nowPolygon].color.r > 1)
				polygons[nowPolygon].color.r -= 2;
			break;
		case '2':
			if (polygons[nowPolygon].color.g > 1)
				polygons[nowPolygon].color.g -= 2;
			break;
		case '3':
			if (polygons[nowPolygon].color.b > 1)
				polygons[nowPolygon].color.b -= 2;
			break;
		case '4':
			if (polygons[nowPolygon].color.r < 254)
				polygons[nowPolygon].color.r += 2;
			break;
		case '5':
			if (polygons[nowPolygon].color.g < 254)
				polygons[nowPolygon].color.g += 2;
			break;
		case '6':
			if (polygons[nowPolygon].color.b < 254)
				polygons[nowPolygon].color.b += 2;
			break;
        case ESCAPE:
                exit(0);
                break;
        case SPACE:
                createPolygon();
                break;
		case DELETE:
				removePolygon();
				break;
        default:
                break;
        }
        // Перерисовка
        glutPostRedisplay();
}
// Функция обрабатывает специальные сообщения от клавиатуры (движение)
void SpecialKeyboard(int key, int x, int y)
{
        switch (key)
        {
        case GLUT_KEY_UP:
                for (int i = 0; i < polygons[nowPolygon].points.size(); i++)
                {
                        polygons[nowPolygon].points[i].y += 10;
                }
                break;
        case GLUT_KEY_DOWN:
                for (int i = 0; i < polygons[nowPolygon].points.size(); i++)
                {
                        polygons[nowPolygon].points[i].y -= 10;
                }
                break;
        case GLUT_KEY_LEFT:
                for (int i = 0; i < polygons[nowPolygon].points.size(); i++)
                {
                        polygons[nowPolygon].points[i].x -= 10;
                }
                break;
        case GLUT_KEY_RIGHT:
                for (int i = 0; i < polygons[nowPolygon].points.size(); i++)
                {
                        polygons[nowPolygon].points[i].x += 10;
                }
                break;
        default:
                break;
        }
        glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y)
{
        /* клавиша была нажата, но не отпущена */
        if (state != GLUT_DOWN) return;

        // новая точка по левому клику
        if (button == GLUT_LEFT_BUTTON)
        {
                polygons[nowPolygon].points.push_back(Point(x, height - y));
				if (polygons[nowPolygon].convex)
					Graham(polygons[nowPolygon]);
        }
        
        // Правый клик, изменение редактируемой фигуры по нажатию по ней
        if (button == GLUT_RIGHT_BUTTON)
		{
			for (int i = polygons.size()-1; i >= 0; i--)
			{
				if (isInner(polygons[i], x, height - y))
				{
					nowPolygon = i;
					break;
				}
			}
		}

        // Клик по колесику
        if (button == GLUT_MIDDLE_BUTTON)
        {
        }
        glutPostRedisplay();
}
#pragma endregion элементы контроля

#pragma region menu
void processMenuStatus(int status, int x, int y) 
{
	if (status == GLUT_MENU_IN_USE)
		menuFlag = 1;
	else
		menuFlag = 0;
}
 
void processMainMenu(int option) 
{

}
 
void processFillMenu(int option) 
{
	switch (option) 
	{
		case 1: glPolygonMode(GL_FRONT, GL_FILL); break;
		case 2: glPolygonMode(GL_FRONT, GL_LINE); break;
	}
	// Перерисовка
    glutPostRedisplay();
}

void processAliasingMenu(int option) 
{
	switch (option) 
	{
		case 1:
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_POINT_SMOOTH);
				glHint(GL_POINT_SMOOTH_HINT, GL_LINEAR);
				glEnable(GL_LINE_SMOOTH);
				glHint(GL_LINE_SMOOTH_HINT, GL_LINEAR);
				break;
			}
		case 2:  
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_POINT_SMOOTH);
				glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
				glEnable(GL_LINE_SMOOTH);               
				glHint(GL_LINE_SMOOTH_HINT, GL_NICEST); break;
			}
	}
	// Перерисовка
    glutPostRedisplay();
}
 
void processFigureMenu(int option) 
{
	switch (option) 
	{
	case 1:
		if (nowPolygon > 0)
		{
			nowPolygon--;
		}
		break;
	case 2:
		if (nowPolygon < polygons.size())
		{
			nowPolygon++;
		}
		break;
	case 3:
		createPolygon();
		break;
	case 4:
		removePolygon();
		break;
	}
	// Перерисовка
    glutPostRedisplay();
}
 
void processColorMenu(int option) 
{
	switch (option) 
	{
	case 1:
		if (polygons[nowPolygon].color.r < 246)
			polygons[nowPolygon].color.r += 10;
		break;
	case 2:
		if (polygons[nowPolygon].color.g < 246)
			polygons[nowPolygon].color.g += 10;
		break;
	case 3:
		if (polygons[nowPolygon].color.b < 246)
			polygons[nowPolygon].color.b += 10;
		break;
	case 4:
		if (polygons[nowPolygon].color.r > 9)
			polygons[nowPolygon].color.r -= 10;
		break;
	case 5:
		if (polygons[nowPolygon].color.g > 9)
			polygons[nowPolygon].color.g -= 10;
		break;
	case 6:
		if (polygons[nowPolygon].color.b > 9)
			polygons[nowPolygon].color.b -= 10;
		break;
	}
	// Перерисовка
    glutPostRedisplay();
}

void createPopupMenus() {
	figureMenu = glutCreateMenu(processFigureMenu);
	glutAddMenuEntry("Previous",1);
	glutAddMenuEntry("Next",2);
	glutAddMenuEntry("Make",3);
	glutAddMenuEntry("Delete",4);
	fillMenu = glutCreateMenu(processFillMenu);
	glutAddMenuEntry("Fill",1);
	glutAddMenuEntry("Line",2);
	aliasingMenu = glutCreateMenu(processAliasingMenu);
	glutAddMenuEntry("Linear",1);
	glutAddMenuEntry("Nicest",2);
	colorMenu = glutCreateMenu(processColorMenu);
	glutAddMenuEntry("Add red",1);
	glutAddMenuEntry("Add green",2);
	glutAddMenuEntry("Add blue",3);
	glutAddMenuEntry("Reduce red",4);
	glutAddMenuEntry("Reduce green",5);
	glutAddMenuEntry("Reduce blue",6);
	mainMenu = glutCreateMenu(processMainMenu);
	glutAddSubMenu("Figure", figureMenu);
	glutAddSubMenu("Color", colorMenu);
	glutAddSubMenu("Fill", fillMenu);
	glutAddSubMenu("Aliasing", aliasingMenu);
	// прикрепить меню к колесу
	glutAttachMenu(GLUT_MIDDLE_BUTTON);
	//статус активности меню
	glutMenuStatusFunc(processMenuStatus);
}
#pragma endregion работа с меню

int _tmain(int argc, _TCHAR* argv[])
{
        createPolygon();
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGB);
        glutInitWindowSize(width, height);
        glutCreateWindow("Компьютерная графика. Лабораторная работа #1.");
        glutDisplayFunc(Display);
        glutReshapeFunc(Reshape);
        glutKeyboardFunc(Keyboard);
        glutSpecialFunc(SpecialKeyboard);
        glutMouseFunc(Mouse);
		createPopupMenus();
        glutMainLoop();

}