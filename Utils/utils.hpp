#ifndef UTILS_H
# define UTILS_H

# include <string>
# include <vector>
# include <iostream>


bool is_whitespace(char c);
void trim_whitespace(std::string &s);
int	ft_isdigit(int c);
int	ft_isdigit_str(const char *str);
int	ft_isxdigit(int c);
int	ft_isxdigit_str(const char *str);


template<typename T>
void displayVec(std::vector<T> v, char separator = ' ')
{
	typename std::vector<T>::iterator it = v.begin();
	while (it != v.end())
	{
		std::cout << *it << separator;
		++it;
	}
	std::cout << std::endl;
}

template<typename T>
bool is_higher (const T& value1, const T& value2)
{
	return (value2 > value1);
}

template<typename T>
bool is_lower (const T& value1, const T& value2)
{
	return (value2 < value1);
}

template<typename T>
bool same_as (const T& value1, const T& value2)
{
	return (value2 == value1);
}

#endif