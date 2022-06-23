#ifndef VECTOR_2F_H
#define VECTOR_2F_H

#include <cmath>

class Vector3f;

class Vector2f {
public:
    static const Vector2f ZERO;
    static const Vector2f UP;
    static const Vector2f RIGHT;

    Vector2f(double f = 0.);
    Vector2f(double x, double y);

    // Copy constructors
    Vector2f(const Vector2f& rv);

    // Assignment operators
    Vector2f& operator=(const Vector2f& rv);

    // No destructor necessary

    // Returns the ith element
    const double& operator[](int i) const;
    double& operator[](int i);

    double& x();
    double& y();

    double x() const;
    double y() const;

    Vector2f xy() const;
    Vector2f yx() const;
    Vector2f xx() const;
    Vector2f yy() const;

    // Returns (-y, x)
    Vector2f normal() const;

    double abs() const;
    double absSquared() const;
    void normalize();
    Vector2f normalized() const;

    void negate();

    // ---- Utility ----
    operator const double*() const; // Automatic type conversion for OpenGL 
    operator double*(); // Automatic type conversion for OpenGL 
    void print() const;

    Vector2f& operator+=(const Vector2f& v);
    Vector2f& operator-=(const Vector2f& v);
    Vector2f& operator*=(double f);

    static double dot(const Vector2f& v0, const Vector2f& v1);

    static Vector3f cross(const Vector2f& v0, const Vector2f& v1);

    // Returns v0 * (1 - alpha) * v1 * alpha
    static Vector2f lerp(const Vector2f& v0, const Vector2f& v1, double alpha);

private:
    double m_elements[2];
};

// Component-wise operators
Vector2f operator+(const Vector2f& v0, const Vector2f& v1);
Vector2f operator-(const Vector2f& v0, const Vector2f& v1);
Vector2f operator*(const Vector2f& v0, const Vector2f& v1);
Vector2f operator/(const Vector2f& v0, const Vector2f& v1);

// Unary negation
Vector2f operator-(const Vector2f& v);

// Multiply and divide by scalar
Vector2f operator*(double f, const Vector2f& v);
Vector2f operator*(const Vector2f& v, double f);
Vector2f operator/(const Vector2f& v, double f);

bool operator==(const Vector2f& v0, const Vector2f& v1);
bool operator!=(const Vector2f& v0, const Vector2f& v1);

#endif // VECTOR_2F_H
