#pragma once
#include <iostream>
#include <glm/glm.hpp>

#define PI 3.141592653589793238460

namespace flo {
	/**
	<summary> A generic complex number, along with the associated math.</summary>
	*/
	template<typename Type>
	struct Complex {
		Type real = 0, imag = 0;

		Complex() = default;

		Complex(const Type real, const Type imag);

		Complex<Type> operator+(const Complex<Type> right);

		Complex<Type> operator-(const Complex<Type> right);

		Complex<Type> operator*(const Complex<Type> right);

		Complex<Type> operator/(const Complex<Type> right);

		Complex<Type> operator+(const Type right);

		Complex<Type> operator-(const Type right);

		Complex<Type> operator*(const Type right);

		Complex<Type> operator/(const Type right);

		Type abs();

		double theta();
	};
}

/*
<summary> Definitions
*/
namespace flo {
	//Complex definitions

	template<typename Type>
	Complex<Type>::Complex(const Type real, const Type imag) : real(real), imag(imag) {}

	template<typename Type>
	Complex<Type> Complex<Type>::operator+(const Complex<Type> right) {
		return Complex(real + right.real, imag + right.imag);
	}

	template<typename Type>
	Complex<Type> Complex<Type>::operator-(const Complex<Type> right) {
		return Complex(real - right.real, imag - right.imag);
	}

	template<typename Type>
	Complex<Type> Complex<Type>::operator*(const Complex<Type> right) {
		return Complex(real*right.real - imag*right.imag, real*right.imag + imag*right.real);
	}

	template<typename Type>
	Complex<Type> Complex<Type>::operator/(const Complex<Type> right) {
		return Complex(real / right.real + imag / right.imag, -real / right.imag + imag / right.real);
	}

	template<typename Type>
	Complex<Type> Complex<Type>::operator*(const Type right) {
		return Complex(real * right, imag * right);
	}

	template<typename Type>
	Complex<Type> Complex<Type>::operator/(const Type right) {
		return Complex(real / right, imag / right);
	}

	template<typename Type>
	Complex<Type> Complex<Type>::operator+(const Type right) {
		return Complex(real + right, imag);
	}

	template<typename Type>
	Complex<Type> Complex<Type>::operator-(const Type right) {
		return Complex(real - right, imag);
	}

	template<typename Type>
	Complex<Type> operator*(const Type left, const Complex<Type> right) {
		return Complex(right.real * left, right.imag * left);
	}

	template<typename Type>
	Complex<Type> operator/(const Type left, const Complex<Type> right) {
		return Complex(left, 0.) / right;
	}

	template<typename Type>
	Complex<Type> operator+(const Type left, const Complex<Type> right) {
		return Complex(right.real + left, right.imag);
	}

	template<typename Type>
	Complex<Type> operator-(const Type left, const Complex<Type> right) {
		return Complex(right.real - left, right.imag);
	}

	template<typename Type>
	Type Complex<Type>::abs() {
		return glm::sqrt(real*real + imag*imag);
	}

	template<typename Type>
	double Complex<Type>::theta() {
		return glm::atan(imag, real);
	}

	template<typename Type>
	Complex<Type> exp(const Complex<Type> right) {
		Type phase = right.imag;
		Type abs = glm::exp(right.real);
		return Complex(abs * glm::cos(phase), abs * glm::sin(phase));
	}

	template<typename Type>
	Complex<Type> log(Complex<Type> right) {
		return Complex(glm::log(right.abs()), right.theta());
	}

	template<typename Type>
	Complex<Type> conjugate(const Complex<Type> right) {
		return Complex(right.real, -right.imag);
	}

	template<typename Type>
	Complex<Type> pow(const Complex<Type> left, const Complex<Type> right) {
		return exp(log(left) * right);
	}

	float cross2D(glm::vec2 a, glm::vec2 b);

	glm::vec2 skew(glm::vec2 v);
}
