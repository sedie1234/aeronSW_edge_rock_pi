#ifndef _UTILS_H
#define _UTILS_H
#include <iostream>
#include <vector>
#include <iterator>

//Vector 터미널 출력을 위한 오버라이드  
template<class T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T>& values)
{
	copy( begin(values), end(values), std::ostream_iterator<T>(stream, ", ") );
	return stream;
}

//pthread 자동 삭제용
struct PThreadDeleter {
    void operator()(pthread_t* thread) const {
        if (thread) {
            pthread_join(*thread, nullptr);
            delete thread;
			std::cout << "Thread joined and deleted." << std::endl;
        }
    }
};

#endif