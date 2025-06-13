#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <vector> 
#include <clocale>
#include <math.h>

using namespace std;
using namespace cv;

int n_small[6] = { 0 }, n_med[6] = { 0 }, n_big[6] = { 0 };
int total[4] = { 0 };
int x_1, x_2, y_1, y_2;
string imagen;


struct list_pines
{
	int id; //pos in  rectangles
	int size; // 0 no clasiffied, 1 small, 2 medium, 3 large
	int color;// 0 red, 1 maroon, 2 green, 3 black, 4 blue, 5 grey
	float lenght;
	list_pines* siguiente;
};

list_pines* principio = NULL;

void borrar(list_pines*& p)
{
	list_pines* pin;
	while (p != NULL)
	{
		pin = p;
		p = p->siguiente;
		pin->siguiente = NULL;
		delete pin;
	}
}
void insertar(list_pines*& p, int id, float lenght, int size, int color)
{
	list_pines* nuevo;
	nuevo = new list_pines;
	nuevo->id = id;
	nuevo->lenght = lenght;
	nuevo->size = size;
	nuevo->color = color;
	nuevo->siguiente = p;
	p = nuevo;
}

double mean_lenght(list_pines*& p, int size)
{
	list_pines* actual;
	double media=0;
	actual = p;
	while (actual != NULL)
	{
		if (actual->size == size)
		{
			media = media + actual->lenght;
		}
		actual = actual->siguiente;
	}
	media = media / total[size];
	return media;
}

float deviation(list_pines*& p, int size,int media)
{
	list_pines* actual;
	float deviation = 0;
	actual = p;
	while (actual != NULL)
	{
		if (actual->size == size)
		{
			deviation = deviation + pow((actual->lenght-media),2);
		}
		actual = actual->siguiente;
	}
	deviation = sqrt(deviation / total[size]);
	return deviation;
}

void buscar(list_pines*& p, int size, int media, int deviation)
{
	list_pines* actual;
	actual = p;
	while (actual != NULL)
	{
		cout << "Examino el pin " << actual->id << " con size " << actual->size << endl;
		if ((actual->lenght > (media + 3 * deviation)) && (size != 3) && (size==actual->size)) {
			actual->size = size + 1;
			cout << "He cambiado el pin " << actual->id << "con largo " << actual->lenght << " a su rango superior" << endl;
		}
		if ((actual->lenght < (media - 3 * deviation)) && (size != 1) && (size == actual->size)) {
			actual->size = size - 1;
			cout << "He cambiado el pin " << actual->id << "con largo " << actual->lenght << " a su rango inferior" << endl;
		}
		actual = actual->siguiente;
	}
}

void size_classify(list_pines*& p)
{
	list_pines* actual;
	actual = p;
	while (actual != NULL)
	{
		if (actual->size == 1) {
			n_small[actual->color] = n_small[actual->color] + 1;
			total[1] = total[1] + 1;
		}
		else if (actual->size == 2) {
			n_med[actual->color] = n_med[actual->color] + 1;
			total[2] = total[2] + 1;
		}
		else if (actual->size == 3) {
			n_big[actual->color] = n_big[actual->color] + 1;
			total[3] = total[3] + 1;
		}else if (actual->size == 0) {
			n_small[actual->color] = n_small[actual->color] + 1;
			total[0] = total[0] + 1;
		}
		actual = actual->siguiente;
	}
}

int color_classify(Mat& mask, Mat& mats_colours, int size, int i)
{
	Mat test, cropped, test2;
	int color;
	int factor = 5;
	bitwise_and(mats_colours, mats_colours, test, mask);
	Rect crop(Point(x_1, y_1), Point(x_2, y_2));
	cropped = test(crop);
	cropped.copyTo(test2);
	resize(test2, test2, Size(test2.cols / factor, test2.rows / factor));
	color = mean(test2)[0];
	test.release();
	cropped.release();
	test2.release();
	if (color > 30) return i;
	else return 6;
}
int menu()
{
	int seleccion = 0;
	cout << "Programa de detecci�n y clasificaci�n de piezas de ensamblaje de robots educativos" << endl << endl;
	cout << "Seleccione que quiere hacer:" << endl;
	cout << "1- detectar desde una foto" << endl;
	cout << "0- salir" << endl;
	cout << "Selecci�n: ";
	cin >> seleccion;
	if (seleccion > 1 || seleccion < 0)
	{
		cout << "elecci�n no valida" << endl;
		return menu();
	}
	if (seleccion == 1)
	{
		cout << "Por favor introduzca el nombre de la imagen: ";
		cin >> imagen;
	}
	return seleccion;

}

void abrir_imagen(Mat& original)
{
	original = imread(imagen, IMREAD_COLOR);
	if (!original.data) {
		cout << "error loading image" << endl;
		cout << "Introduzca de nuevo el nombre del archivo: ";
		cin >> imagen;
		abrir_imagen(original);
	}
}

int main(int argc, char** argv)
{
	while (1 == 1)
	{
		double a, b, t;
		int factor = 5;
		int seleccion=0;
		Mat original, filtered, value, saturation, open_close;
		Mat granate, rojo, verde, gris, azul, negro, sombra;
		setlocale(LC_CTYPE, "Spanish");

		seleccion = menu();
		if (seleccion == 0) return 0;
		else if (seleccion == 1) abrir_imagen(original);

		a = getTickCount();
		//value and saturation piece filtering 
		cvtColor(original, filtered, COLOR_RGB2HSV);

		inRange(filtered, Scalar(0, 150, 0), Scalar(179, 255, 255), value);
		inRange(filtered, Scalar(0, 0, 50), Scalar(179, 200, 105), saturation);
		inRange(filtered, Scalar(116, 50, 50), Scalar(126, 149, 149), granate);
		inRange(filtered, Scalar(115, 150, 150), Scalar(140, 255, 255), rojo);
		inRange(filtered, Scalar(45, 150, 0), Scalar(60, 255, 255), verde);
		inRange(filtered, Scalar(0, 0, 50), Scalar(120, 100, 100), gris);
		inRange(filtered, Scalar(0, 150, 0), Scalar(20, 255, 255), azul);
		inRange(filtered, Scalar(0, 0, 0), Scalar(100, 100, 50), negro);
		inRange(filtered, Scalar(0, 0, 80), Scalar(38, 150, 200), sombra);

		open_close = value + saturation + granate + negro - sombra;

		//Opening and closing
		int x = 5;
		int y = 7;

		morphologyEx(open_close, open_close, MORPH_OPEN, getStructuringElement(MORPH_RECT, Size(x, x)), Point(-1, -1), 2);
		morphologyEx(open_close, open_close, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(y, y)));

		vector<vector<Point>> cont;
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		findContours(open_close, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
		hierarchy.clear();

		vector<vector<Point>>contours_poly(contours.size());
		vector<RotatedRect>rectangles(contours.size());
		vector<float> descriptors(contours.size());
		Point2f punto[4];
		int resizeFactor = 3.5;
		double color;
		total[0] = 0;
		total[1] = 0;
		total[2] = 0;
		total[3] = 0;
		int size;
		string colours[7] = { "Rojo", "Granate", "Verde", "Negro", "Azul", "Gris", "No clasificado" };
		Scalar colores[7] = { Scalar(0,0,255),Scalar(21,0,136),Scalar(0,255,0),Scalar(0,0,0),Scalar(255,0,0),Scalar(100,100,100),Scalar(0,128,255) };

		float length;
		double mean_bigs = 0, mean_meds = 0, mean_smalls = 0;
		float deviation_bigs = 0, deviation_meds = 0, deviation_smalls = 0;
		namedWindow("Original", WINDOW_NORMAL);
		namedWindow("Filtered", WINDOW_NORMAL);
		namedWindow("Open_close", WINDOW_NORMAL);

		for (int i = 0; i < contours.size(); i++)
		{
			if (contourArea(contours[i]) > 500)
			{
				int color;
				Mat mask = Mat::zeros(original.rows, original.cols, CV_8SC1);
				vector<Point> points;
				approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
				rectangles[i] = minAreaRect((Mat)contours_poly[i]);
				rectangles[i].points(punto);
				x_1 = 0;
				x_2 = 0;
				y_1 = 0;
				y_2 = 0;

				//Compare which is the length and which is the width of the rectangle to always maintain the same proportion.
				if ((norm(punto[1] - punto[0])) > norm(punto[2] - punto[1])) {
					descriptors[i] = ((norm(punto[1] - punto[0])) / norm(punto[2] - punto[1]));
					length = (norm(punto[1] - punto[0]));
				}
				else {
					descriptors[i] = ((norm(punto[2] - punto[1])) / norm(punto[1] - punto[0]));
					length = (norm(punto[2] - punto[1]));
				}

				// size classification from descriptors
				if ((descriptors[i] >= 3.9) && (descriptors[i] < 5.4)) {
					size = 3; //big size
					total[3] = total[3] + 1;
				}
				else if ((descriptors[i] >= 2.6) && (descriptors[i] < 3.9)) {
					size = 2; // medium size
					total[2] = total[2] + 1;
				}
				else if ((descriptors[i] > 1.9) && (descriptors[i] < 2.6)) {
					size = 1; //size
					total[1] = total[1] + 1;
				}
				else {
					size = 0; //No classified
					total[0] = total[0] + 1;
				}

				//rectangle enumeration
				putText(original, to_string(i), punto[1], 0, 2, Scalar(0, 0, 255), 2, LINE_AA);

				//Draws the rectangle in the original image
				for (int j = 0; j < 4; j++) {
					points.push_back(Point(punto[j]));
					if (j == 0)
					{
						x_1 = punto[j].x;
						x_2 = x_1;
						y_1 = punto[j].y;
						y_2 = y_1;
					}
					else
					{
						if (punto[j].x < x_1) x_1 = punto[j].x;
						if (punto[j].x > x_2) x_2 = punto[j].x;
						if (punto[j].y < y_1) y_1 = punto[j].y;
						if (punto[j].y > y_2) y_2 = punto[j].y;
					}
				}

				//color classification
				fillConvexPoly(mask, points, Scalar(255, 255, 255));
				color = color_classify(mask, rojo, size, 0);
				if (color == 6)
				{
					color = color_classify(mask, granate, size, 1);
					if (color == 6)
					{
						color = color_classify(mask, verde, size, 2);
						if (color == 6)
						{
							color = color_classify(mask, negro, size, 3);
							if (color == 6)
							{
								color = color_classify(mask, azul, size, 4);
								if (color == 6)
								{
									color = color_classify(mask, gris, size, 5);
								}
							}
						}
					}
				}
				imshow("Original", original);
				imshow("Filtered", mask);
				imshow("Open_close", granate);
				resizeWindow("Original", original.cols / resizeFactor, original.rows / resizeFactor);
				resizeWindow("Filtered", mask.cols / resizeFactor, mask.rows / resizeFactor);
				resizeWindow("Filtered", granate.cols / resizeFactor, granate.rows / resizeFactor);
				cout << "color: " << color << endl;
				polylines(original, points, true, colores[color], 5, LINE_AA);
				insertar(principio, i, length, size, color);
				mask.release();
				points.clear();
			}
		}

		//normal distribution calculations
		//averages
		mean_bigs = mean_lenght(principio, 3);
		cout << "La media de los grandes es: " << mean_bigs << endl;
		mean_meds = mean_lenght(principio, 2);
		cout << "La media de los medianos es: " << mean_meds << endl;
		mean_smalls = mean_lenght(principio, 1);
		cout << "La media de los peque�os es: " << mean_smalls << endl;

		//standard deviation
		deviation_bigs = deviation(principio, 3, mean_bigs);
		cout << "La desviaci�n de los grandes es: " << deviation_bigs << endl;
		deviation_meds = deviation(principio, 2, mean_meds);
		cout << "La desviaci�n de los medianos es: " << deviation_meds << endl;
		deviation_smalls = deviation(principio, 1, mean_smalls);
		cout << "La desviaci�n de los peque�os es: " << deviation_smalls << endl;

		//Range adjustment, if necessary, defined by ±(mean ± 2*standard deviation) for 95% accuracy.
		cout << "grandes" << endl;
		buscar(principio, 3, mean_bigs, deviation_bigs);
		cout << "medianos" << endl;
		buscar(principio, 2, mean_meds, deviation_meds);
		cout << "peque�os" << endl;
		buscar(principio, 1, mean_smalls, deviation_smalls);

		total[0] = 0;
		total[1] = 0;
		total[2] = 0;
		total[3] = 0;

		size_classify(principio);

		b = getTickCount();
		t = (b - a) / getTickFrequency();
		cout << "he tardado " << setprecision(100) << t << " segundos" << endl;

		if (total[3] != 0) {
			cout << "Se han encontrado " << total[3] << " rect�ngulos grandes y son: " << endl;
			for (int i = 0; i < 7; i++) {
				if (n_big[i] != 0) cout << n_big[i] << " " << colours[i] << endl;
			}
		}

		if (total[2] != 0) {
			cout << "Se han encontrado " << total[2] << " rect�ngulos medianos y son: " << endl;
			for (int i = 0; i < 7; i++) {
				if (n_med[i] != 0) cout << n_med[i] << " " << colours[i] << endl;
			}
		}
		if (total[1] != 0) {
			cout << "Se han encontrado " << total[1] << " rect�ngulos peque�os y son: " << endl;
			for (int i = 0; i < 7; i++) {
				if (n_small[i] != 0) cout << n_small[i] << " " << colours[i] << endl;
			}
		}
		if (total[0] != 0) {
			cout << "Se han encontrado " << total[0] << " rect�ngulos no clasificados y son: " << endl;
			for (int i = 0; i < 7; i++) {
				if (n_small[i] != 0) cout << n_small[i] << " " << colours[i] << endl;
			}
		}

		namedWindow("Original", WINDOW_NORMAL);
		namedWindow("Filtered", WINDOW_NORMAL);
		namedWindow("Open_close", WINDOW_NORMAL);

		imshow("Original", original);
		imshow("Filtered", filtered);
		imshow("Open_close", open_close);

		resizeWindow("Original", original.cols / resizeFactor, original.rows / resizeFactor);
		resizeWindow("Filtered", filtered.cols / resizeFactor, filtered.rows / resizeFactor);
		resizeWindow("Open_close", open_close.cols / resizeFactor, open_close.rows / resizeFactor);
		
		int c = waitKey(0);
		if ((char)c == 27)
		{
			original.release();
			filtered.release();
			value.release();
			saturation.release();
			open_close.release();
			granate.release();
			rojo.release();
			gris.release();
			verde.release();
			negro.release();
			azul.release();
			sombra.release();
			borrar(principio);
			break;
		}
		original.release();
		filtered.release();
		value.release();
		saturation.release();
		open_close.release();
		granate.release();
		rojo.release();
		gris.release();
		verde.release();
		negro.release();
		azul.release();
		sombra.release();
		borrar(principio);
		fill(begin(n_big), end(n_big), 0);
		fill(begin(n_med), end(n_med), 0);
		fill(begin(n_small), end(n_small), 0);
		fill(begin(total), end(total), 0);
		cont.clear();
		contours.clear();
		contours_poly.clear();
		rectangles.clear();
		descriptors.clear();
	}
	destroyAllWindows();
	return 0;
}