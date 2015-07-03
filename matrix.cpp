#include "matrix.h"

Matrix::Matrix()
{
    int i,j;
    for(i=0;i<4;i++)
        for(j=0;j<4;j++)
            matrix[i][j] = 0;
}

Matrix::Matrix(double *value)
{
    int i,j;
    for(i=0;i<4;i++)
        for(j=0;j<4;j++)
            matrix[i][j] = value[4*i+j];
}

Matrix::Matrix(const Matrix &m)
{
    int i,j;
    for(i=0;i<4;i++)
        for(j=0;j<4;j++)
            matrix[i][j] = m.matrix[i][j];
}

Matrix &Matrix::operator =(const Matrix &m)
{
    int i,j;
    for(i=0;i<4;i++)
        for(j=0;j<4;j++)
            matrix[i][j] = m.matrix[i][j];
    return *this;
}

Matrix &Matrix::operator *(const Matrix &m)
{
    int i,j;
    Matrix temp,*p;
    for(i=0;i<4;i++)
        for(j=0;j<4;j++)
            temp.SetValue(i,j,matrix[i][0]*m.matrix[0][j] + matrix[i][1]*m.matrix[1][j] + matrix[i][2]*m.matrix[2][j] + matrix[i][3]*m.matrix[3][j]);
    *p = temp;
    return *p; //оригинальные приколы, без этого на конце массива бред!
}

POINT4D &Matrix::operator *(const POINT4D &p)
{
    POINT4D temp;
    temp.x = p.x*matrix[0][0] + p.y*matrix[0][1] + p.z*matrix[0][2] + p.h*matrix[0][3];
    temp.y = p.x*matrix[1][0] + p.y*matrix[1][1] + p.z*matrix[1][2] + p.h*matrix[1][3];
    temp.z = p.x*matrix[2][0] + p.y*matrix[2][1] + p.z*matrix[2][2] + p.h*matrix[2][3];
    temp.h = p.x*matrix[3][0] + p.y*matrix[3][1] + p.z*matrix[3][2] + p.h*matrix[3][3];
    return temp;
}

Matrix Matrix::Transposed()
{
    int i,j;
    Matrix temp;
    for(i=0;i<4;i++)
        for(j=0;j<4;j++)
            temp.SetValue(j,i,matrix[i][j]);
    return temp;
}

void Matrix::SetValue(int i, int j, double value)
{
    matrix[i][j] = value;
}

void Matrix::isIdentity()
{
    int i,j;
    for(i=0;i<4;i++)
        for(j=0;j<4;j++)
            matrix[i][j] = 0;
    for(i=0;i<4;i++)
        matrix[i][i] = 1;
}

double Matrix::GetIx()
{
    return matrix[0][0];
}

double Matrix::GetIy()
{
    return matrix[0][1];
}

double Matrix::GetIz()
{
    return matrix[0][2];
}

double Matrix::GetIh()
{
    return matrix[0][3];
}

double Matrix::GetJx()
{
    return matrix[1][0];
}

double Matrix::GetJy()
{
    return matrix[1][1];
}

double Matrix::GetJz()
{
    return matrix[1][2];
}

double Matrix::GetJh()
{
    return matrix[1][3];
}

double Matrix::GetKx()
{
    return matrix[2][0];
}

double Matrix::GetKy()
{
    return matrix[2][1];
}

double Matrix::GetKz()
{
    return matrix[2][2];
}

double Matrix::GetKh()
{
    return matrix[2][3];
}

double Matrix::GetCx()
{
    return matrix[3][0];
}

double Matrix::GetCy()
{
    return matrix[3][1];
}

double Matrix::GetCz()
{
    return matrix[3][2];
}

double Matrix::GetCh()
{
    return matrix[3][3];
}
