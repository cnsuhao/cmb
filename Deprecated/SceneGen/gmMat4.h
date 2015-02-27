// gmMatrix4.h - 4x4 element matrix class
//
// libgm++: Graphics Math Library
// Ferdi Scheepers and Stephen F May
// 15 June 1994
// 1-17-2005: Interface with C++ Standard Library: removed bool.h type;
//    renamed headers (J. Edward Swan II)
// 1-29-2005: Added input and output operators (J. Edward Swan II)

#ifndef GMMATRIX4_H
#define GMMATRIX4_H

#include <cassert>
#include "gmUtils.h"

class gmVector3;
class gmVector4;

class gmMatrix4 {

protected:
  double m_[4][4];

public:
  gmMatrix4();
  gmMatrix4(const gmMatrix4&);
  gmMatrix4(double, double, double, double,
            double, double, double, double,
            double, double, double, double,
            double, double, double, double);

  // array access

  double* operator [](int);
  const double* operator [](int) const;

  // assignment

  gmMatrix4& assign(double, double, double, double,
                    double, double, double, double,
                    double, double, double, double,
                    double, double, double, double);
  gmMatrix4& operator =(const gmMatrix4&);

  // operators

  gmMatrix4& operator +=(const gmMatrix4&);
  gmMatrix4& operator -=(const gmMatrix4&);
  gmMatrix4& operator *=(const gmMatrix4&);
  gmMatrix4& operator *=(double);
  gmMatrix4& operator /=(double);

  gmMatrix4 operator +(const gmMatrix4&) const;
  gmMatrix4 operator -(const gmMatrix4&) const;
  gmMatrix4 operator -() const;
  gmMatrix4 operator *(const gmMatrix4&) const;
  gmMatrix4 operator *(double) const;
  gmMatrix4 operator /(double) const;

  friend gmMatrix4 operator *(double, const gmMatrix4&);

  bool operator ==(const gmMatrix4&) const; // fuzzy equality
  bool operator !=(const gmMatrix4&) const;

  gmVector4 operator *(const gmVector4&) const;

  friend gmVector4 operator *(const gmVector4&, const gmMatrix4&);

  // operations

  gmMatrix4 inverse() const;
  gmMatrix4 transpose() const;
  gmMatrix4 adjoint() const;

  double determinant() const;
  bool isSingular() const;

  gmVector3 transform(const gmVector3&) const;

  void copyTo(float [4][4]) const;
  void copyTo(double [4][4]) const;

  void MakeOpenGL( double [16]) const;
  void FromOpenGL( double [16]);
  void MakeQuat( double& x, double& y, double& z, double& w );
  void FromQuat( double x, double y, double z, double w );

  // transformation matrices

  static gmMatrix4 identity();
  //  ensures: gmMatrix4::identity() == the identity matrix

  static gmMatrix4 rotate( double angle, const gmVector3& axis );
  //  ensures: gmMatrix4::rotate( angle, axis ) = the matrix that rotates by angle
  //           degrees around axis.

  static gmMatrix4 scale( double x, double y, double z );
  // ensures: gmMatrix4::scale( x, y, z ) == the matrix that scales by
  //          x along the x-axis, y along the y-axis, and z along the z-axis

  static gmMatrix4 translate( double x, double y, double z );
  // ensures: gmMatrix4::translate( x, y, z ) == the matrix that translates by
  //          x along the x-axis, y along the y-axis, and z along the z-axis

  // cubic basis matrices

  static gmMatrix4 bezierBasis();
  static gmMatrix4 bsplineBasis();
  static gmMatrix4 catmullromBasis();
  static gmMatrix4 hermiteBasis();

  static gmMatrix4 tensedBSplineBasis(double);
  static gmMatrix4 cardinalBasis(double);
  static gmMatrix4 tauBasis(double, double);
  static gmMatrix4 betaSplineBasis(double, double);

  // input & output

  friend std::istream & operator >> ( std::istream &, gmMatrix4 & );
  friend std::ostream & operator << ( std::ostream &, const gmMatrix4 & );

  private:
    float ReciprocalSqrt( float x);

};

// ARRAY ACCESS

inline double* gmMatrix4::operator [](int i)
{
  assert(i == 0 || i == 1 || i == 2 || i == 3);
  return &m_[i][0];
}

inline const double* gmMatrix4::operator [](int i) const
{
  assert(i == 0 || i == 1 || i == 2 || i == 3);
  return &m_[i][0];
}

inline void gmMatrix4::copyTo(float f[4][4]) const
{
  f[0][0] = m_[0][0]; f[0][1] = m_[0][1];
  f[0][2] = m_[0][2]; f[0][3] = m_[0][3];
  f[1][0] = m_[1][0]; f[1][1] = m_[1][1];
  f[1][2] = m_[1][2]; f[1][3] = m_[1][3];
  f[2][0] = m_[2][0]; f[2][1] = m_[2][1];
  f[2][2] = m_[2][2]; f[2][3] = m_[2][3];
  f[3][0] = m_[3][0]; f[3][1] = m_[3][1];
  f[3][2] = m_[3][2]; f[3][3] = m_[3][3];
}

inline void gmMatrix4::copyTo(double f[4][4]) const
{
  f[0][0] = m_[0][0]; f[0][1] = m_[0][1];
  f[0][2] = m_[0][2]; f[0][3] = m_[0][3];
  f[1][0] = m_[1][0]; f[1][1] = m_[1][1];
  f[1][2] = m_[1][2]; f[1][3] = m_[1][3];
  f[2][0] = m_[2][0]; f[2][1] = m_[2][1];
  f[2][2] = m_[2][2]; f[2][3] = m_[2][3];
  f[3][0] = m_[3][0]; f[3][1] = m_[3][1];
  f[3][2] = m_[3][2]; f[3][3] = m_[3][3];
}

#define MT4_COLUMNOR 1  (allows for both column and row orientation with a switch)

inline void gmMatrix4::MakeOpenGL( double f[16]) const
{
#ifdef MT4_COLUMNOR
  f[0] = m_[0][0];  f[1] = m_[0][1];  f[2] = m_[0][2];  f[3] = m_[0][3];
  f[4] = m_[1][0];  f[5] = m_[1][1];  f[6] = m_[1][2];  f[7] = m_[1][3];
  f[8] = m_[2][0];  f[9] = m_[2][1];  f[10] = m_[2][2];  f[11] = m_[2][3];
  f[12] = m_[3][0];  f[13] = m_[3][1];  f[14] = m_[3][2];  f[15] = m_[3][3];
#else
  f[0] = m_[0][0];  f[1] = m_[1][0];  f[2] = m_[2][0];  f[3] = m_[3][0];
  f[4] = m_[0][1];  f[5] = m_[1][1];  f[6] = m_[2][1];  f[7] = m_[3][1];
  f[8] = m_[0][2];  f[9] = m_[1][2];  f[10] = m_[2][2];  f[11] = m_[3][2];
  f[12] = m_[0][3];  f[13] = m_[1][3];  f[14] = m_[2][3];  f[15] = m_[3][3];
#endif
}

inline void gmMatrix4::FromOpenGL( double f[16])
{
#ifdef MT4_COLUMNOR
  m_[0][0] = f[0];  m_[0][1] = f[1];  m_[0][2] = f[2];  m_[0][3] = f[3];
  m_[1][0] = f[4];  m_[1][1] = f[5];  m_[1][2] = f[6];  m_[1][3] = f[7];
  m_[2][0] = f[8];  m_[2][1] = f[9];  m_[2][2] = f[10];  m_[2][3] = f[11];
  m_[3][0] = f[12];  m_[3][1] = f[13];  m_[3][2] = f[14];  m_[3][3] = f[15];
#else
  m_[0][0] = f[0];  m_[1][0] = f[1];  m_[2][0] = f[2];  m_[3][0] = f[3];
  m_[0][1] = f[4];  m_[1][1] = f[5];  m_[2][1] = f[6];  m_[3][1] = f[7];
  m_[0][2] = f[8];  m_[1][2] = f[9];  m_[2][2] = f[10];  m_[3][2] = f[11];
  m_[0][3] = f[12];  m_[1][3] = f[13];  m_[2][3] = f[14];  m_[3][3] = f[15];
#endif
}
#endif // GMMATRIX4_H
