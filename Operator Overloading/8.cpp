// Unary Prefix Increment Operator  as Member

#include<iostream>


class Point
{
	friend std::ostream& operator<<(std::ostream& os, const Point& p);
	
public:
	Point() = default;
	Point(double x, double y) : 
		m_x(x), m_y(y){
	}

	void operator++() {
		++m_x;
		++m_y;
	}

	~Point() = default;

private: 
	double length() const;   // Function to calculate distance from the point(0,0)

private : 
	double m_x{}; 
	double m_y{}; 
};

inline std::ostream& operator<<(std::ostream& os, const Point& p){
	os << "Point [ x : " << p.m_x << ", y : " << p.m_y << "]";	
	return os;
}

int main(){

    Point p1(10,10);
    std::cout << "p1 : " << p1 << std::endl;

   ++p1;
   //p1.operator++();

    std::cout << "p1 : " << p1 << std::endl; // 11
    return 0;
}


/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
// Unary Prefix Increment Operator as Non Member 

#include<iostream>

class Point
{
	friend std::ostream& operator<<(std::ostream& os, const Point& p);
	friend void operator++(Point& operand);
	
public:
	Point() = default;
	Point(double x, double y) : 
		m_x(x), m_y(y){
	}

	~Point() = default;

private : 
	double m_x{}; 
	double m_y{}; 
};


inline void operator++(Point& operand){
	++(operand.m_x);
	++(operand.m_y);
}


inline std::ostream& operator<<(std::ostream& os, const Point& p){
	os << "Point [ x : " << p.m_x << ", y : " << p.m_y << "]";	
	return os;
}



int main(){

    Point p1(10,10);
    std::cout << "p1 : " << p1 << std::endl;

    //++p1;
  
    operator++(p1);
    

    std::cout << "p1 : " << p1 << std::endl; // 11
}