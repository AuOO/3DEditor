#ifndef MATRIX_H
#define MATRIX_H

struct POINT4D
{
    double x;
    double y;
    double z;
    double h;
};

class Matrix
{
private:
    double matrix[4][4];
public:
    Matrix();
    Matrix(double *);
    Matrix(const Matrix&);
    Matrix &operator =(const Matrix&);
    Matrix &operator *(const Matrix&);
    POINT4D &operator *(const POINT4D&);
    Matrix Transposed();
    void isIdentity();
    void SetValue(int,int,double);

    //координаты вектора i
    double GetIx();
    double GetIy();
    double GetIz();
    double GetIh();

    //координаты вектора j
    double GetJx();
    double GetJy();
    double GetJz();
    double GetJh();

    //координаты вектора k
    double GetKx();
    double GetKy();
    double GetKz();
    double GetKh();

    //координаты вектора c
    double GetCx();
    double GetCy();
    double GetCz();
    double GetCh();
};

#endif // MATRIX_H
