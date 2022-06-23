#ifndef QUAT4F_H
#define QUAT4F_H

class Vector3f;
class Vector4f;

#include "Matrix3f.h"

class Quat4f {
public:
    static const Quat4f ZERO;
    static const Quat4f IDENTITY;

    Quat4f();

    // q = w + x * i + y * j + z * k
    Quat4f(double w, double x, double y, double z);
        
    Quat4f(const Quat4f& rq); // Copy constructor
    Quat4f& operator=(const Quat4f& rq); // Assignment operator
    // No destructor necessary

    // Returns a quaternion with 0 real part
    Quat4f(const Vector3f& v);

    // Copies the components of a Vector4f directly into this quaternion
    Quat4f(const Vector4f& v);

    // Returns the ith element
    const double& operator[](int i) const;
    double& operator[](int i);

    double w() const;
    double x() const;
    double y() const;
    double z() const;
    Vector3f xyz() const;
    Vector4f wxyz() const;

    double abs() const;
    double absSquared() const;
    void normalize();
    Quat4f normalized() const;

    void conjugate();
    Quat4f conjugated() const;

    void invert();
    Quat4f inverse() const;

    // Log and exponential maps
    Quat4f log() const;
    Quat4f exp() const;
    
    // Returns unit vector for rotation and radians about the unit vector
    Vector3f getAxisAngle(double* radiansOut);

    // Sets this quaternion to be a rotation of fRadians about v = <fx, fy, fz>, v need not necessarily be unit length
    void setAxisAngle(double radians, const Vector3f& axis);

    // ---- Utility ----
    void print();
 
    // Quaternion dot product (a la vector)
    static double dot(const Quat4f& q0, const Quat4f& q1);    
    
    // Linear (stupid) interpolation
    static Quat4f lerp(const Quat4f& q0, const Quat4f& q1, double alpha);

    // Spherical linear interpolation
    static Quat4f slerp(const Quat4f& a, const Quat4f& b, double t, bool allowFlip = true);
    
    // Spherical quadratic interoplation between a and b at point t
    // Given quaternion tangents tanA and tanB (can be computed using squadTangent)    
    static Quat4f squad(const Quat4f& a, const Quat4f& tanA, const Quat4f& tanB, const Quat4f& b, double t);

    static Quat4f cubicInterpolate(const Quat4f& q0, const Quat4f& q1, const Quat4f& q2, const Quat4f& q3, double t);

    // Log-difference between a and b, used for squadTangent
    // Returns log(a^-1 b)    
    static Quat4f logDifference(const Quat4f& a, const Quat4f& b);

    // Computes a tangent at center, defined by the before and after quaternions
    // Useful for squad()
    static Quat4f squadTangent(const Quat4f& before, const Quat4f& center, const Quat4f& after);        

    static Quat4f fromRotationMatrix(const Matrix3f& m);

    static Quat4f fromRotatedBasis(const Vector3f& x, const Vector3f& y, const Vector3f& z);

    // Returns a unit quaternion that's a uniformly distributed rotation
    // Given u[i] is a uniformly distributed random number in [0, 1]
    // Taken from Graphics Gems II
    static Quat4f randomRotation(double u0, double u1, double u2);

private:
    double m_elements[4];
};

Quat4f operator+(const Quat4f& q0, const Quat4f& q1);
Quat4f operator-(const Quat4f& q0, const Quat4f& q1);
Quat4f operator*(const Quat4f& q0, const Quat4f& q1);
Quat4f operator*(double f, const Quat4f& q);
Quat4f operator*(const Quat4f& q, double f);

#endif // QUAT4F_H
