#ifndef _GRID_H_
#define _GRID_H_
#include <vector>
#include <GL/glut.h>
#include "objects.h"
using namespace std;

class Grid
{
public:
	//properties
	static const int GRID_SIZE = 8;
	static const int N_CELLS = 2 + GRID_SIZE;
	vector<int> _cells[N_CELLS][N_CELLS][N_CELLS];
	float _cellWidthX, _cellWidthY, _cellWidthZ;
	float wallLeft, wallBottom, wallFront;
	//members
	Grid(float wallRadius);
	void ClearCells();
	void PrintGridInfo();
	void DrawGrid();
	void AddIndexToCell(int x, int y, int z, int index);
	void ConstructGrid(vector<sphere>& spheres);
	vector<int>& GetSpheresInCell(int x, int y, int z);
	~Grid();

private:

};

Grid::Grid(float wallRadius)
{
	_cellWidthZ = _cellWidthY = _cellWidthX = (2.0f * wallRadius) / (N_CELLS - 2);
	wallLeft = -1 - _cellWidthX;
	wallBottom = -1 - _cellWidthY;
	wallFront = -1 - _cellWidthZ;
	ClearCells();
	 
}
vector<int>& Grid::GetSpheresInCell(int x, int y, int z)
{
	return _cells[x][y][z];
}
void Grid::AddIndexToCell(int x, int y, int z, int index)
{
	_cells[x][y][z].push_back(index);
}
void Grid::ClearCells()
{
	for (int x = 0; x < N_CELLS; x++)
	{
		for (int y = 0; y < N_CELLS; y++)
		{
			for (int z = 0; z < N_CELLS; z++)
			{
				_cells[x][y][z].clear();
			}
		}
	}
	
}

void Grid::ConstructGrid(vector<sphere>& spheres)
{
	ClearCells();
	
	float cellLeftBound;
	float cellRightBound;
	float cellLowBound;
	float cellHighBound;
	float cellFrontBound;
	float cellBackBound;

	int startX, endX;
	int startY, endY;
	int startZ, endZ;
	startX = startY = startZ = 0;
	endX = endY = endZ = 0;

	for (int i = 0; i < spheres.size(); i++)
	{
		//get x cells
		for (int x = 0; x < N_CELLS; x++)
		{
			
			cellLeftBound = x * _cellWidthX + wallLeft; //0 * 1 -1 = -1
			cellRightBound = x * _cellWidthX + wallLeft + _cellWidthX; //0 * 1 - 1 + 1 = 0
			if (spheres[i].p[0] - spheres[i].r >= cellLeftBound && spheres[i].p[0] - spheres[i].r < cellRightBound)
			{
				startX = x;
				
			}
			if (spheres[i].p[0] + spheres[i].r > cellLeftBound && spheres[i].p[0] + spheres[i].r <= cellRightBound)
			{
				endX = x;
				//cout << endX << endl;
			}
			
		}
		//get y cells
		for (int y = 0; y < N_CELLS; y++)
		{

			cellLowBound = y * _cellWidthY + wallBottom; //0 * 1 -1 = -1
			cellHighBound = y * _cellWidthY + wallBottom + _cellWidthY; //0 * 1 - 1 + 1 = 0
			if (spheres[i].p[1] - spheres[i].r >= cellLowBound && spheres[i].p[1] - spheres[i].r < cellHighBound)
			{
				startY = y;
				//cout <<"y "<< startY << endl;
			}
			if (spheres[i].p[1] + spheres[i].r > cellLowBound && spheres[i].p[1] + spheres[i].r <= cellHighBound)
			{
				endY = y;
			}

		}
		//get z cells
		for (int z = 0; z < N_CELLS; z++)
		{

			cellFrontBound = z * _cellWidthZ + wallFront; //0 * 1 -1 = -1
			cellBackBound = z * _cellWidthZ + wallFront + _cellWidthZ; //0 * 1 - 1 + 1 = 0
			if (spheres[i].p[2] - spheres[i].r >= cellFrontBound && spheres[i].p[2] - spheres[i].r < cellBackBound)
			{
				startZ = z;
			}
			if (spheres[i].p[2] + spheres[i].r > cellFrontBound && spheres[i].p[2] + spheres[i].r <= cellBackBound)
			{
				endZ = z;
			}

		}
		
		if (startX >= 0 && startY >= 0 && startZ >= 0 && endX >= 0 && endY >= 0 && endZ >= 0){
			for (int j = startX; j <= endX; j++)
			{
				for (int k = startY; k <= endY; k++)
				{
					//cout << "y " << startY << endl;
					for (int l = startZ; l <= endZ; l++)
					{
						_cells[j][k][l].push_back(i);
					}
				}
			}
		}
	}

	
}
void Grid::PrintGridInfo()
{
	for (int z = 0; z < N_CELLS; z++)
	{
		for (int y = 0; y < N_CELLS; y++)
		{
			for (int x = 0; x < N_CELLS; x++)
			{
				cout << "(" << x << "," << y << "," << z <<","<<_cells[x][y][z].size()<< "): ";
				
				for (int i = 0; i < _cells[x][y][z].size(); i++)
				{
					cout << _cells[x][y][z].at(i) << " ";
				}
				cout << endl;
			}
			
		}
	}

}
void Grid::DrawGrid()
{
	float cellLeftBound;
	float cellBottomBound;
	float cellFrontBound;
	for (int z = 0; z < N_CELLS; z++)
	{
		for (int y = 0; y < N_CELLS; y++)
		{
			for (int x = 0; x < N_CELLS; x++)
			{
				
				cellLeftBound = x * _cellWidthX + wallLeft; //0 * 1 - 2 = -2
				cellBottomBound = y * _cellWidthY + wallLeft;
				cellFrontBound = z * _cellWidthZ + wallLeft;
				glPushMatrix();
				glDisable(GL_CULL_FACE);
				glDisable(GL_COLOR_MATERIAL);
				glDisable(GL_LIGHTING);
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glBegin(GL_QUADS);
				//glColor4f(1.0f, 0, 0, 0.2);
				
				//left
				glVertex3d(cellLeftBound, cellBottomBound, cellFrontBound); // ahh, really should use the array form of glVertex here
				glVertex3d(cellLeftBound, cellBottomBound + _cellWidthY, cellFrontBound);
				glVertex3d(cellLeftBound, cellBottomBound + _cellWidthY, cellFrontBound + _cellWidthZ);
				glVertex3d(cellLeftBound, cellBottomBound, cellFrontBound + _cellWidthZ);

				
				//back
				glVertex3d(cellLeftBound, cellBottomBound, cellFrontBound + _cellWidthZ);
				glVertex3d(cellLeftBound, cellBottomBound + _cellWidthY, cellFrontBound + _cellWidthZ);
				glVertex3d(cellLeftBound + _cellWidthX, cellBottomBound + _cellWidthY, cellFrontBound + _cellWidthZ);
				glVertex3d(cellLeftBound + _cellWidthX, cellBottomBound, cellFrontBound + _cellWidthZ);

				//bottom
				glVertex3d(cellLeftBound, cellBottomBound, cellFrontBound);
				glVertex3d(cellLeftBound, cellBottomBound, cellFrontBound + _cellWidthZ);
				glVertex3d(cellLeftBound + _cellWidthX, cellBottomBound, cellFrontBound + _cellWidthZ);
				glVertex3d(cellLeftBound + _cellWidthX, cellBottomBound, cellFrontBound);
				
				
				glEnd();
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glPopMatrix();
			}

		}
	}
	
}
Grid::~Grid()
{
}

#endif _GRID_H_