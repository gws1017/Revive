#pragma once

namespace client_fw
{
	class Vec4 : public XMFLOAT4
	{
	public:
		constexpr explicit Vec4() : XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) {}
		constexpr explicit Vec4(float x, float y, float z, float w = 1.0f) : XMFLOAT4(x, y, z, w) {}
		
		Vec4& operator+=(const Vec4& rhs);
		Vec4& operator-=(const Vec4& rhs);
		Vec4& operator*=(const Vec4& rhs);
		Vec4& operator*=(float scalar);
		Vec4& operator/=(const Vec4& rhs);
		Vec4& operator/=(float scalar);

		std::string ToString() const;
	};

	Vec4 operator+(const Vec4& v1, const Vec4& v2);
	Vec4 operator-(const Vec4& v1, const Vec4& v2);
	Vec4 operator*(const Vec4& v1, const Vec4& v2);
	Vec4 operator*(const Vec4& v, float scalar);
	Vec4 operator*(float scalar, const Vec4& v);
	Vec4 operator/(const Vec4& v1, const Vec4& v2);
	Vec4 operator/(const Vec4& v, float scalar);
	Vec4 operator/(float scalar, const Vec4& v);

	bool operator==(const Vec4& v1, const Vec4& v2);
	bool operator!=(const Vec4& v1, const Vec4& v2);
	bool operator<(const Vec4& v1, const Vec4& v2);
	bool operator<=(const Vec4& v1, const Vec4& v2);
	bool operator>(const Vec4& v1, const Vec4& v2);
	bool operator>=(const Vec4& v1, const Vec4& v2);

	std::ostream& operator<<(std::ostream& os, const Vec4& v);
}


