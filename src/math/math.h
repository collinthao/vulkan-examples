#pragma once

std::array<float, 2> h(float x, float y)
{
	float whole1, whole2;	

	std::array<float,2> t  = {std::pow(10.f,4.f)*(x+(y/7.7f)), std::pow(10.f,4.f)*((x/5.7f)+(y/1.1f))};
	
	t = std::array<float,2>{abs(std::modf(std::pow(10.f, 4.f) * sin(t[0]), &whole1)), 
				abs(std::modf(std::pow(10.f, 4.f) * sin(t[1]), &whole2))};	

	return t;
};

std::array<float,2> H(float x, float y)
{
	std::array<float,2> h1 = h(std::floor(x),std::floor(y));

	std::array<float,2> t = std::array<float,2>{(2.f * h1[0]) - 1.f, (2.f * h1[1]) - 1.f};

//	std::cout  << "H: "<< "x: " << x << ", y: " << y << '|' << "t0: " << t[0] << ", t1: " << t[1] << '\n';
	return t;
}

float f(float x)
{
	float whole;
	
	return 1.f - abs(std::modf(x,&whole));
};

float f2(float x)
{
	float whole;
	//std::cout << std::floor(x) << " == " << x << '\n';
	if (std::floor(x) == x) x += 0.1f;	
	//std::cout << "std::modf((3 * std::pow(f(" << x << "), 2))-(2*std::pow(f(" << x << "), 3)), &whole) = " << std::modf((3 * std::pow(f(x), 2))-(2*std::pow(f(x), 3)), &whole) << '\n';

	return std::modf((3 * std::pow(f(x), 2))-(2*std::pow(f(x), 3)), &whole);
}

float d(std::array<float,2> a, std::array<float,2> b)
{
	//std::cout << "(" << a[0] << " * " << b[0] << ")" << " + " << "(" << a[1] << " * " <<  b[1]<< ")" << " = " << (a[0] * b[0]) + (a[1] * b[1]) << '\n';
	return (a[0] * b[0]) + (a[1] * b[1]);
}

float p(float x, float y)
{
	x/=10.f;
	y/=10.f;
	std::array<float,2> fa1{f(x), f(y)};
	float d1 = d(H(x,y), fa1);	
	float fb1 = f2(-x)*f2(-y);
	float r1 = d1 * fb1;
	
	std::array<float,2> fa2{-f(-x), f(y)};
	float d2 = d(H(x + 1, y), fa2);
	float fb2 = f2(x)*f2(-y);
	float r2 = d2 * fb2;

	std::array<float,2> fa3{f(x), -f(-y)};
	float d3 = d(H(x, y + 1), fa3);
	float fb3 = f2(-x)*f2(y);
	float r3 = d3 * fb3;

	std::array<float,2> fa4{-f(-x), -f(-y)};
	float d4 = d(H(x + 1, y + 1), fa4);
	float fb4 = f2(x)*f2(y);
	float r4 = d4 * fb4;
	if ((r1 + r2 + r3 + r4) > 1)
	{
		//std::cout << "d1 start\n";
		//std::cout << "fa1[0]: " << fa1[0] << ", fa1[1]: " << fa1[1] << '\n';
		//std::cout << "d1: " << d1 << '\n';
		//std::cout << "fb1: " << fb1 << '\n';
		//std::cout << "r1: " << r1 << '\n';
		//std::cout << r1 << ' ' << r2 << ' ' << r3 << ' ' << r4 << '\n';	
		//std::cout << "x: " << x << ", y: " << y << '|'  << r1 << '+' << r2 << '+' << r3 << '+' << r4 << '=' << (r1 + r2 + r3 + r4) << '\n';
	}
	return std::max(std::round((r1 + r2 + r3 + r4) * 10.f), 1.0f);
}

float P(float x, float y)
{
	float result = 0;
	for (int i = 1; i < 5; i++)
	{
		float p_i = p(std::pow(2, i) * x, std::pow(2, i) * y);	
		float d_i = 1/std::pow(2, i);
		result += (d_i * p_i);	
	}
	return 0.5 * result;
};

float T(float x, float y)
{
	float r = 0;
	float c1 = p(x, y);	

	std::array<float, 4> ps{
		p(x + 1, y),
		p(x - 1, y),
		p(x, y - 1),
		p(x, y + 1)
	};
	
	std::array<std::string, 4> psd{
		"p(x + 1, y)",
		"p(x - 1, y)",
		"p(x, y - 1)",
		"p(x, y + 1)"
	};
	
	float max = -10000.f;
	
	for (int i = 0; i < ps.size(); i++)
	{
		if (ps[i] > max) 
		{
			max = ps[i];
		}
	}
	float lerp = c1 + (max - c1) * 1.0f;
	
	return lerp;
}
